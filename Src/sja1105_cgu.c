/*
 * sja1105_cgu.c
 *
 *  Created on: Jul 31, 2025
 *      Author: bens1
 */

#include "sja1105.h"
#include "sja1105_conf.h"
#include "sja1105_io.h"
#include "sja1105_regs.h"


SJA1105_StatusTypeDef SJA1105_ConfigureCGU(SJA1105_HandleTypeDef *dev){

    SJA1105_StatusTypeDef status = SJA1105_OK;
    uint32_t reg_data;

    /* Setup PLL0 (f = 125MHz) */
    reg_data = SJA1105_CGU_P23EN;  /* Enable 120 and 240 degree outputs for better EMC performance */
    status = SJA1105_WriteRegister(dev, SJA1105_CGU_REG_PLL_0_C, &reg_data, 1);
    if (status != SJA1105_OK) return status;

    /* Setup PLL1 (f = 50MHz, integer mode) */
    reg_data = 0;
    reg_data &= ~SJA1105_CGU_PD;         /* Disable power down */
    reg_data &= ~SJA1105_CGU_BYPASS;     /* Disable bypass */
    reg_data |=  SJA1105_CGU_P23EN;      /* Enable 120 and 240 degree outputs for better EMC performance */
    reg_data |=  SJA1105_CGU_FBSEL;      /* Enable PLL feedback */
    reg_data |=  SJA1105_CGU_AUTOBLOCK;  /* Enable autoblock to prevent glitches */
    reg_data |= (0x1 << SJA1105_CGU_PSEL_SHIFT) & SJA1105_CGU_PSEL_MASK;  /* PSEL = 1 */
    reg_data |= (0x1 << SJA1105_CGU_MSEL_SHIFT) & SJA1105_CGU_MSEL_MASK;  /* MSEL = 1 */
    reg_data |= (0x0 << SJA1105_CGU_NSEL_SHIFT) & SJA1105_CGU_NSEL_MASK;  /* NSEL = 0 */
    status = SJA1105_WriteRegister(dev, SJA1105_CGU_REG_PLL_0_C, &reg_data, 1);
    if (status != SJA1105_OK) return status;

    /* Configure each port */
    for (uint8_t port_num = 0; port_num < SJA1105_NUM_PORTS; port_num++){
        status = SJA1105_ConfigureCGUPort(dev, port_num);
        if (status != SJA1105_OK) return status;
    }

    return status;
}

SJA1105_StatusTypeDef SJA1105_ConfigureCGUPort(SJA1105_HandleTypeDef *dev, uint8_t port_num){

    SJA1105_StatusTypeDef status = SJA1105_OK;
    SJA1105_PortTypeDef port = dev->ports[port_num];
    uint32_t reg_data[SJA1105_CGU_REG_CLK_NUM];

    /* Skip port 4 in variants that don't have one */
    if (((dev->config->variant == VARIANT_SJA1105R) || (dev->config->variant == VARIANT_SJA1105S)) && (port_num == 4)) return status;

    switch (port.interface) {
        case SJA1105_INTERFACE_MII:

            /* MII MAC */
            if (port.mode == SJA1105_MODE_MAC){

                /* Disable IDIVX */
                reg_data[0] = SJA1105_CGU_PD;
                status = SJA1105_WriteRegister(dev, SJA1105_CGU_REG_IDIV_C(port_num), reg_data, 1);
                if (status != SJA1105_OK) return status;

                /* Set CLKSRC of MII_TX_CLK_X to TX_CLK_X */
                reg_data[SJA1105_CGU_MII_TX_CLK  ]  = SJA1105_CGU_CLK_SRC_TX_CLK(port_num) << SJA1105_CGU_CLKSRC_SHIFT;  /* Implicitly sets PD to 0 */
                reg_data[SJA1105_CGU_MII_TX_CLK  ] |= SJA1105_CGU_AUTOBLOCK;  /* Prevent clock glitches */

                /* Set CLKSRC of MII_RX_CLK_X to RX_CLK_X */
                reg_data[SJA1105_CGU_MII_RX_CLK  ]  = SJA1105_CGU_CLK_SRC_RX_CLK(port_num) << SJA1105_CGU_CLKSRC_SHIFT;
                reg_data[SJA1105_CGU_MII_RX_CLK  ] |= SJA1105_CGU_AUTOBLOCK;

                /* Disable all other clock sinks */
                reg_data[SJA1105_CGU_RMII_REF_CLK]  = SJA1105_CGU_PD;
                reg_data[SJA1105_CGU_RGMII_TX_CLK]  = SJA1105_CGU_PD;
                reg_data[SJA1105_CGU_EXT_TX_CLK  ]  = SJA1105_CGU_PD;
                reg_data[SJA1105_CGU_EXT_RX_CLK  ]  = SJA1105_CGU_PD;

                /* Write the configuration */
                status = SJA1105_WriteRegister(dev, SJA1105_CGU_REG_CLK_BASE(port_num), reg_data, SJA1105_CGU_REG_CLK_NUM);
                if (status != SJA1105_OK) return status;
            }

            /* MII PHY */
            else {
                switch (port.speed){

                    /* 10M MII PHY */
                    case SJA1105_SPEED_10M: {

                        /* Enable IDIVX and divide by 10 */
                        reg_data[0]  = (9 << SJA1105_CGU_IDIV_SHIFT) & SJA1105_CGU_IDIV_MASK;
                        reg_data[0] |= SJA1105_CGU_AUTOBLOCK;
                        status = SJA1105_WriteRegister(dev, SJA1105_CGU_REG_IDIV_C(port_num), reg_data, 1);
                        if (status != SJA1105_OK) return status;

                        /* Set CLKSRC of MII_TX_CLK_X to IDIVX */
                        reg_data[SJA1105_CGU_MII_TX_CLK  ]  = SJA1105_CGU_CLK_SRC_IDIV(port_num) << SJA1105_CGU_CLKSRC_SHIFT;
                        reg_data[SJA1105_CGU_MII_TX_CLK  ] |= SJA1105_CGU_AUTOBLOCK;

                        /* Set CLKSRC of MII_RX_CLK_X to RX_CLK_X */
                        reg_data[SJA1105_CGU_MII_RX_CLK  ]  = SJA1105_CGU_CLK_SRC_RX_CLK(port_num) << SJA1105_CGU_CLKSRC_SHIFT;
                        reg_data[SJA1105_CGU_MII_RX_CLK  ] |= SJA1105_CGU_AUTOBLOCK;
                        
                        /* Set CLKSRC of EXT_TX_CLK_X to IDIVX */
                        reg_data[SJA1105_CGU_EXT_TX_CLK  ]  = SJA1105_CGU_CLK_SRC_IDIV(port_num) << SJA1105_CGU_CLKSRC_SHIFT;
                        reg_data[SJA1105_CGU_EXT_TX_CLK  ] |= SJA1105_CGU_AUTOBLOCK;

                        /* Set CLKSRC of EXT_RX_CLK_X to IDIVX */
                        reg_data[SJA1105_CGU_EXT_RX_CLK  ]  = SJA1105_CGU_CLK_SRC_IDIV(port_num) << SJA1105_CGU_CLKSRC_SHIFT;
                        reg_data[SJA1105_CGU_EXT_RX_CLK  ] |= SJA1105_CGU_AUTOBLOCK;

                        /* Disable all other clock sinks */
                        reg_data[SJA1105_CGU_RMII_REF_CLK]  = SJA1105_CGU_PD;
                        reg_data[SJA1105_CGU_RGMII_TX_CLK]  = SJA1105_CGU_PD;

                        /* Write the configuration */
                        status = SJA1105_WriteRegister(dev, SJA1105_CGU_REG_CLK_BASE(port_num), reg_data, SJA1105_CGU_REG_CLK_NUM);
                        if (status != SJA1105_OK) return status;
                        break;
                    }

                    /* 100M MII PHY */
                    case SJA1105_SPEED_100M: {
                        status = SJA1105_ERROR;
                        break;
                    }

                    default:
                        status = SJA1105_PARAMETER_ERROR;
                        break;
                }
            }
            break;

        case SJA1105_INTERFACE_RMII:

            /* RMII MAC */
            if (port.mode == SJA1105_MODE_MAC){

                /* Disable IDIVX */
                reg_data[0] = SJA1105_CGU_PD;
                status = SJA1105_WriteRegister(dev, SJA1105_CGU_REG_IDIV_C(port_num), reg_data, 1);
                if (status != SJA1105_OK) return status;

                /* Set CLKSRC of RMII_REF_CLK_X to TX_CLK_X */
                reg_data[SJA1105_CGU_RMII_REF_CLK]  = SJA1105_CGU_CLK_SRC_TX_CLK(port_num) << SJA1105_CGU_CLKSRC_SHIFT;
                reg_data[SJA1105_CGU_RMII_REF_CLK] |= SJA1105_CGU_AUTOBLOCK;

                /* Set CLKSRC of EXT_TX_CLK_X to PLL1 */
                reg_data[SJA1105_CGU_EXT_TX_CLK  ]  = SJA1105_CGU_CLK_SRC_PLL1(dev->config->skew_clocks ? port_num: 0) << SJA1105_CGU_CLKSRC_SHIFT;
                reg_data[SJA1105_CGU_EXT_TX_CLK  ] |= SJA1105_CGU_AUTOBLOCK;

                /* Disable all other clock sinks */
                reg_data[SJA1105_CGU_MII_TX_CLK  ]  = SJA1105_CGU_PD;
                reg_data[SJA1105_CGU_MII_RX_CLK  ]  = SJA1105_CGU_PD;
                reg_data[SJA1105_CGU_RGMII_TX_CLK]  = SJA1105_CGU_PD;
                reg_data[SJA1105_CGU_EXT_RX_CLK  ]  = SJA1105_CGU_PD;

                /* Write the configuration */
                status = SJA1105_WriteRegister(dev, SJA1105_CGU_REG_CLK_BASE(port_num), reg_data, SJA1105_CGU_REG_CLK_NUM);
                if (status != SJA1105_OK) return status;
                break;
            }

            /* RMII PHY */
            else {

                /* Disable IDIVX */
                reg_data[0] = SJA1105_CGU_PD;
                status = SJA1105_WriteRegister(dev, SJA1105_CGU_REG_IDIV_C(port_num), reg_data, 1);
                if (status != SJA1105_OK) return status;

                /* Set CLKSRC of RMII_REF_CLK_X to TX_CLK_X */
                reg_data[SJA1105_CGU_RMII_REF_CLK]  = SJA1105_CGU_CLK_SRC_TX_CLK(port_num) << SJA1105_CGU_CLKSRC_SHIFT;
                reg_data[SJA1105_CGU_RMII_REF_CLK] |= SJA1105_CGU_AUTOBLOCK;

                /* Set CLKSRC of EXT_TX_CLK_X to PLL1 */
                if (port.output_rmii_refclk){
                    reg_data[SJA1105_CGU_EXT_TX_CLK]  = SJA1105_CGU_CLK_SRC_PLL1(dev->config->skew_clocks ? port_num: 0) << SJA1105_CGU_CLKSRC_SHIFT;
                    reg_data[SJA1105_CGU_EXT_TX_CLK] |= SJA1105_CGU_AUTOBLOCK;
                } else {
                    reg_data[SJA1105_CGU_EXT_TX_CLK]  = SJA1105_CGU_PD;
                }

                /* Disable all other clock sinks */
                reg_data[SJA1105_CGU_MII_TX_CLK  ]  = SJA1105_CGU_PD;
                reg_data[SJA1105_CGU_MII_RX_CLK  ]  = SJA1105_CGU_PD;
                reg_data[SJA1105_CGU_RGMII_TX_CLK]  = SJA1105_CGU_PD;
                reg_data[SJA1105_CGU_EXT_RX_CLK  ]  = SJA1105_CGU_PD;

                /* Write the configuration */
                status = SJA1105_WriteRegister(dev, SJA1105_CGU_REG_CLK_BASE(port_num), reg_data, SJA1105_CGU_REG_CLK_NUM);
                if (status != SJA1105_OK) return status;
                break;

            }
            break;

        case SJA1105_INTERFACE_RGMII:

            /* RGMII */
            switch (port.speed){

                /* 10M RGMII MAC */
                case SJA1105_SPEED_10M: {

                    /* Enable IDIVX and divide by 10 */
                    reg_data[0]  = (9 << SJA1105_CGU_IDIV_SHIFT) & SJA1105_CGU_IDIV_MASK;
                    reg_data[0] |= SJA1105_CGU_AUTOBLOCK;
                    status = SJA1105_WriteRegister(dev, SJA1105_CGU_REG_IDIV_C(port_num), reg_data, 1);
                    if (status != SJA1105_OK) return status;

                    /* Set CLKSRC of RGMII_TXC_X to IDIVX */
                    reg_data[SJA1105_CGU_RGMII_TX_CLK]  = SJA1105_CGU_CLK_SRC_IDIV(port_num) << SJA1105_CGU_CLKSRC_SHIFT;
                    reg_data[SJA1105_CGU_RGMII_TX_CLK] |= SJA1105_CGU_AUTOBLOCK;

                    /* Disable all other clock sinks */
                    reg_data[SJA1105_CGU_MII_TX_CLK  ]  = SJA1105_CGU_PD;
                    reg_data[SJA1105_CGU_MII_RX_CLK  ]  = SJA1105_CGU_PD;
                    reg_data[SJA1105_CGU_RMII_REF_CLK]  = SJA1105_CGU_PD;
                    reg_data[SJA1105_CGU_EXT_TX_CLK  ]  = SJA1105_CGU_PD;
                    reg_data[SJA1105_CGU_EXT_RX_CLK  ]  = SJA1105_CGU_PD;

                    /* Write the configuration */
                    status = SJA1105_WriteRegister(dev, SJA1105_CGU_REG_CLK_BASE(port_num), reg_data, SJA1105_CGU_REG_CLK_NUM);
                    if (status != SJA1105_OK) return status;
                    break;
                }

                /* 100M RGMII MAC */
                case SJA1105_SPEED_100M: {

                    /* Enable IDIVX and divide by 1 */
                    reg_data[0]  = (0 << SJA1105_CGU_IDIV_SHIFT) & SJA1105_CGU_IDIV_MASK;
                    reg_data[0] |= SJA1105_CGU_AUTOBLOCK;
                    status = SJA1105_WriteRegister(dev, SJA1105_CGU_REG_IDIV_C(port_num), reg_data, 1);
                    if (status != SJA1105_OK) return status;

                    /* Set CLKSRC of RGMII_TXC_X to IDIVX */
                    reg_data[SJA1105_CGU_RGMII_TX_CLK]  = SJA1105_CGU_CLK_SRC_IDIV(port_num) << SJA1105_CGU_CLKSRC_SHIFT;
                    reg_data[SJA1105_CGU_RGMII_TX_CLK] |= SJA1105_CGU_AUTOBLOCK;

                    /* Disable all other clock sinks */
                    reg_data[SJA1105_CGU_MII_TX_CLK  ]  = SJA1105_CGU_PD;
                    reg_data[SJA1105_CGU_MII_RX_CLK  ]  = SJA1105_CGU_PD;
                    reg_data[SJA1105_CGU_RMII_REF_CLK]  = SJA1105_CGU_PD;
                    reg_data[SJA1105_CGU_EXT_TX_CLK  ]  = SJA1105_CGU_PD;
                    reg_data[SJA1105_CGU_EXT_RX_CLK  ]  = SJA1105_CGU_PD;

                    /* Write the configuration */
                    status = SJA1105_WriteRegister(dev, SJA1105_CGU_REG_CLK_BASE(port_num), reg_data, SJA1105_CGU_REG_CLK_NUM);
                    if (status != SJA1105_OK) return status;
                    break;
                }

                /* 1G RGMII MAC */
                case SJA1105_SPEED_1G: {

                    /* Disable IDIVX */
                    reg_data[0] = SJA1105_CGU_PD;
                    status = SJA1105_WriteRegister(dev, SJA1105_CGU_REG_IDIV_C(port_num), reg_data, 1);
                    if (status != SJA1105_OK) return status;

                    /* Set CLKSRC of RGMII_TXC_X to PLL0 */
                    reg_data[SJA1105_CGU_RGMII_TX_CLK]  = SJA1105_CGU_CLK_SRC_PLL0(dev->config->skew_clocks ? port_num: 0) << SJA1105_CGU_CLKSRC_SHIFT;
                    reg_data[SJA1105_CGU_RGMII_TX_CLK] |= SJA1105_CGU_AUTOBLOCK;

                    /* Disable all other clock sinks */
                    reg_data[SJA1105_CGU_MII_TX_CLK  ]  = SJA1105_CGU_PD;
                    reg_data[SJA1105_CGU_MII_RX_CLK  ]  = SJA1105_CGU_PD;
                    reg_data[SJA1105_CGU_RMII_REF_CLK]  = SJA1105_CGU_PD;
                    reg_data[SJA1105_CGU_EXT_TX_CLK  ]  = SJA1105_CGU_PD;
                    reg_data[SJA1105_CGU_EXT_RX_CLK  ]  = SJA1105_CGU_PD;

                    /* Write the configuration */
                    status = SJA1105_WriteRegister(dev, SJA1105_CGU_REG_CLK_BASE(port_num), reg_data, SJA1105_CGU_REG_CLK_NUM);
                    if (status != SJA1105_OK) return status;
                    break;
                }

                default:
                    status = SJA1105_PARAMETER_ERROR;
                    break;
            }

        case SJA1105_INTERFACE_SGMII:
            break;

        default:
            status = SJA1105_PARAMETER_ERROR;
            break;
    }

    return status;
}
