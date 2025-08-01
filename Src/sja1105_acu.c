/*
 * sja1105_acu.c
 *
 *  Created on: Jul 31, 2025
 *      Author: bens1
 */

#include "sja1105.h"
#include "sja1105_conf.h"
#include "sja1105_io.h"
#include "sja1105_regs.h"


SJA1105_StatusTypeDef SJA1105_ConfigureACU(SJA1105_HandleTypeDef *dev){

    SJA1105_StatusTypeDef status = SJA1105_OK;

    /* Configure the ACU with each port's IO pad configuration */
    for (uint8_t port_num = 0; port_num < SJA1105_NUM_PORTS; port_num++){
        status = SJA1105_ConfigureACUPort(dev, port_num);
        if (status != SJA1105_OK) return status;
    }

    /* TODO: Configure MISC, SPI, and JTAG IO pads. Also configure temperature sensor. */

    return status;
}

SJA1105_StatusTypeDef SJA1105_ConfigureACUPort(SJA1105_HandleTypeDef *dev, uint8_t port_num){

    SJA1105_StatusTypeDef status = SJA1105_OK;
    SJA1105_PortTypeDef port = dev->ports[port_num];

    /* Skip port 4 in variants that don't have one */
    if (((dev->config->variant == VARIANT_SJA1105R) || (dev->config->variant == VARIANT_SJA1105S)) && (port_num == 4)) return status;

    /* Don't continue if no configuration is supplied. This isn't an error since a default register values will be used instead. */
    if (port.configured == false){
        status = SJA1105_OK;
        return status;
    }

    /* Check port numbers match */
    if (port.port_num != port_num) status = SJA1105_PARAMETER_ERROR;
    if (status != SJA1105_OK) return status;

    /* Create and clear buffer */
    uint32_t reg_data[SJA1105_ACU_PAD_CFG_SIZE];
    reg_data[SJA1105_ACU_PAD_CFG_TX] = 0;
    reg_data[SJA1105_ACU_PAD_CFG_RX] = 0;

    /* Set slew rates */
    switch (port.interface)
    {
        case SJA1105_INTERFACE_MII:

            /* Low speed */
            reg_data[SJA1105_ACU_PAD_CFG_TX] |= SJA1105_OS_LOW;
            break;

        case SJA1105_INTERFACE_RMII:

            /* 1V8 RMII not supported */
            if (port.voltage == SJA1105_IO_1V8) status = SJA1105_PARAMETER_ERROR;
            if (status != SJA1105_OK) return status;

            /* Low speed */
            reg_data[SJA1105_ACU_PAD_CFG_TX] |= SJA1105_OS_LOW;
            break;

        case SJA1105_INTERFACE_RGMII:

            switch (port.voltage) {

                case SJA1105_IO_2V5:
                case SJA1105_IO_3V3:

                    /* Medium speed */
                    reg_data[SJA1105_ACU_PAD_CFG_TX] |= SJA1105_OS_MEDIUM;
                    break;

                case SJA1105_IO_1V8:
                default:

                    /* Fast speed */
                    reg_data[SJA1105_ACU_PAD_CFG_TX] |= SJA1105_OS_HIGH;
                    break;
                }
                break;

                default:
                break;
            }

    /* Disable internal TX pull downs */
    reg_data[SJA1105_ACU_PAD_CFG_TX] |= SJA1105_IPUD_PI;

    /* Set TX CLK input hysteresis to non-Schmitt  */
    reg_data[SJA1105_ACU_PAD_CFG_TX] |= SJA1105_CLK_IH_NON_SCHMITT;

    /* Disable internal RX pull downs and set input hysteresis to non-Schmitt */
    reg_data[SJA1105_ACU_PAD_CFG_TX] |= SJA1105_IPUD_PI;
    reg_data[SJA1105_ACU_PAD_CFG_TX] |= SJA1105_IH_NON_SCHMITT;

    /* Write the pad config */
    SJA1105_WriteRegister(dev, SJA1105_ACU_REG_CFG_PAD_MIIX_TX(port_num), reg_data, SJA1105_ACU_PAD_CFG_SIZE);

    /* TODO: Update the internal delay (ID) register (SJA1105_ACU_REG_CFG_PAD_MIIX_ID) if internal RGMII
     *       CLK delays are needed. Many PHYs also implement this and it is only needed once per TX or RX
     *       channel. Since the SJA1105's ID implementation uses phase (not time) delays and requires
     *       managing frequency transitions, the PHY implementation is preferred.
     */

    return status;
}

