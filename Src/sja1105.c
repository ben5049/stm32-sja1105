/*
 * sja1105.c
 *
 *  Created on: Jul 27, 2025
 *      Author: bens1
 */

#include "sja1105.h"
#include "sja1105_spi.h"
#include "sja1105_regs.h"

/* ---------------------------------------------------------------------------- */
/* Initialisation */
/* ---------------------------------------------------------------------------- */

SJA1105_StatusTypeDef SJA1105_ConfigurePort(SJA1105_PortTypeDef *ports, uint8_t port_num, SJA1105_InterfaceTypeDef interface, SJA1105_SpeedTypeDef speed, SJA1105_IOVoltageTypeDef voltage){

    SJA1105_StatusTypeDef status = SJA1105_OK;

    /* Check the parameters */
    if (port_num >= SJA1105_NUM_PORTS)       status = SJA1105_PARAMETER_ERROR;
    if (interface > SJA1105_INTERFACE_SGMII) status = SJA1105_PARAMETER_ERROR;
    if (speed > SJA1105_SPEED_1G)            status = SJA1105_PARAMETER_ERROR;
    if (voltage > SJA1105_IO_3V3)            status = SJA1105_PARAMETER_ERROR;
    if (ports[port_num].configured == true)  status = SJA1105_ALREADY_CONFIGURED_ERROR;  /* Note this may cause an unintended error if the struct uses non-zeroed memory. */
    if (status != SJA1105_OK) return status;

    /* Assign the parameters */
    ports[port_num].port_num   = port_num;
    ports[port_num].interface  = interface;
    ports[port_num].speed      = speed;
    ports[port_num].voltage    = voltage;
    ports[port_num].configured = true;

    return status;
}

SJA1105_StatusTypeDef SJA1105_Init(
        SJA1105_HandleTypeDef          *dev,
        const SJA1105_VariantTypeDef    variant,
        const SJA1105_CallbacksTypeDef *callbacks,
        SPI_HandleTypeDef              *spi_handle,
        GPIO_TypeDef                   *cs_port,
        uint16_t                        cs_pin,
        GPIO_TypeDef                   *rst_port,
        uint16_t                        rst_pin,
        uint32_t                        timeout,
        const uint32_t                 *static_conf,
        uint32_t                        static_conf_size,
        SJA1105_PortTypeDef            *ports){

    SJA1105_StatusTypeDef status = SJA1105_OK;

    /* Check the struct hasn't already been initialised. Note this may cause an unintended error if the struct uses non-zeroed memory. */
    if (dev->initialised == true) status = SJA1105_ALREADY_CONFIGURED_ERROR;
    if (status != SJA1105_OK) return status;

    dev->variant     = variant;
    dev->callbacks   = callbacks;
    dev->spi_handle  = spi_handle;
    dev->cs_port     = cs_port;
    dev->cs_pin      = cs_pin;
    dev->rst_port    = rst_port;
    dev->rst_pin     = rst_pin;
    dev->timeout     = timeout;
    dev->ports       = ports;
    dev->initialised = false;

    /* Only the SJA1105Q has been implemented. TODO: Add more */
    if (dev->variant != VARIANT_SJA1105Q) status = SJA1105_PARAMETER_ERROR;

    /* Check SPI parameters */
    if (dev->spi_handle->Init.DataSize    != SPI_DATASIZE_32BIT) status = SJA1105_PARAMETER_ERROR;
    if (dev->spi_handle->Init.CLKPolarity != SPI_POLARITY_LOW  ) status = SJA1105_PARAMETER_ERROR;
    if (dev->spi_handle->Init.CLKPhase    != SPI_PHASE_1EDGE   ) status = SJA1105_PARAMETER_ERROR;
    if (dev->spi_handle->Init.NSS         != SPI_NSS_SOFT      ) status = SJA1105_PARAMETER_ERROR;
    if (dev->spi_handle->Init.FirstBit    != SPI_FIRSTBIT_MSB  ) status = SJA1105_PARAMETER_ERROR;

    /* If there are invalid parameters then return */
    if (status != SJA1105_OK) return status;
    
    /* Set pins to a known state */
    HAL_GPIO_WritePin(dev->rst_port, dev->rst_pin, SET);
    HAL_GPIO_WritePin(dev->cs_port,  dev->cs_pin,  SET);
    
    /* Check the part number matches the specified variant */
    status = SJA1105_CheckPartNR(dev);
    if (status != SJA1105_OK) return status;


    /* Configure the SJA1105 following the steps from UM11040 figure 2 */

    /* Step 1: RESET */
    SJA1105_Reset(dev);

    /* Step 2: ACU REGISTER SETUP */
    status = SJA1105_ConfigureACU(dev);
    if (status != SJA1105_OK) return status;
    
    /* Step 3: CGU REGISTER SETUP */
    status = SJA1105_ConfigureCGU(dev);
    if (status != SJA1105_OK) return status;

    /* Step 4: SGMII PHY/PCS (optional) */

    /* Step 5: STATIC CONFIGURATION */
    status = SJA1105_WriteStaticConfig(dev, static_conf, static_conf_size);
    if (status != SJA1105_OK) return status;

    dev->initialised = true;

    return status;
}


SJA1105_StatusTypeDef SJA1105_CheckPartNR(SJA1105_HandleTypeDef *dev){

    SJA1105_StatusTypeDef status = SJA1105_OK;
    uint32_t reg_data;

    /* Read the PROD_ID register */
    status = SJA1105_ReadRegister(dev, SJA1105_ACU_REG_PROD_ID, &reg_data, 1);
    if (status != SJA1105_OK) return status;

    /* Extract and check the PART_NR */
    switch ((reg_data & SJA1105_PART_NR_MASK) >> SJA1105_PART_NR_OFFSET){
        case PART_NR_SJA1105_T:
            if ((dev->variant != VARIANT_SJA1105) && (dev->variant != VARIANT_SJA1105T)) status = SJA1105_ID_ERROR;
            break;
        
        case PART_NR_SJA1105P:
            if (dev->variant != VARIANT_SJA1105P) status = SJA1105_ID_ERROR;
            break;
        
        case PART_NR_SJA1105Q:
            if (dev->variant != VARIANT_SJA1105Q) status = SJA1105_ID_ERROR;
            break;
        
        case PART_NR_SJA1105R:
            if (dev->variant != VARIANT_SJA1105R) status = SJA1105_ID_ERROR;
            break;
        
        case PART_NR_SJA1105S:
            if (dev->variant != VARIANT_SJA1105S) status = SJA1105_ID_ERROR;
            break;
        
        default:
            status = SJA1105_ID_ERROR;
            break;
    }

    return status;
}

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

SJA1105_StatusTypeDef SJA1105_ConfigureCGU(SJA1105_HandleTypeDef *dev){

    SJA1105_StatusTypeDef status = SJA1105_OK;

    /* TODO: Implement */
    status = SJA1105_ERROR;

    return status;
}


void SJA1105_Reset(SJA1105_HandleTypeDef *dev){
    HAL_GPIO_WritePin(dev->rst_port, dev->rst_pin, RESET);
    dev->callbacks->callback_delay_ns(SJA1105_T_RST);  /* 5us delay */
    HAL_GPIO_WritePin(dev->rst_port, dev->rst_pin, SET);
    dev->callbacks->callback_delay_ms(1);  /* 329us minimum until SPI commands can be written. Use a 1ms non-blocking delay so the RTOS can do other work */
}
