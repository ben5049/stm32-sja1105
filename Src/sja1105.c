/*
 * sja1105.c
 *
 *  Created on: Jul 27, 2025
 *      Author: bens1
 */

#include "sja1105.h"
#include "sja1105_io.h"
#include "sja1105_regs.h"


/* ---------------------------------------------------------------------------- */
/* Initialisation */
/* ---------------------------------------------------------------------------- */

SJA1105_StatusTypeDef SJA1105_ConfigurePort(SJA1105_PortTypeDef *ports, uint8_t port_num, SJA1105_InterfaceTypeDef interface, SJA1105_SpeedTypeDef speed, SJA1105_IOVoltageTypeDef voltage){

    SJA1105_StatusTypeDef status = SJA1105_OK;

    /* Check the parameters */
    if ( port_num  >= SJA1105_NUM_PORTS                                       ) status = SJA1105_PARAMETER_ERROR;
    if ( interface >  SJA1105_INTERFACE_SGMII                                 ) status = SJA1105_PARAMETER_ERROR;
    if ( speed     >  SJA1105_SPEED_1G                                        ) status = SJA1105_PARAMETER_ERROR;
    if ( voltage   >  SJA1105_IO_3V3                                          ) status = SJA1105_PARAMETER_ERROR;
    if ((voltage   == SJA1105_IO_1V8) && (interface == SJA1105_INTERFACE_RMII)) status = SJA1105_PARAMETER_ERROR;           /* 1V8 RMII not supported */
    if (ports[port_num].configured == true                                    ) status = SJA1105_ALREADY_CONFIGURED_ERROR;  /* Note this may cause an unintended error if the struct uses non-zeroed memory. */
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
        const SJA1105_ConfigTypeDef    *config,
        SJA1105_PortTypeDef            *ports,
        const SJA1105_CallbacksTypeDef *callbacks,
        const uint32_t                 *static_conf,
        uint32_t                        static_conf_size){

    SJA1105_StatusTypeDef status = SJA1105_OK;

    /* Check the struct hasn't already been initialised. Note this may cause an unintended error if the struct uses non-zeroed memory. */
    if (dev->initialised == true) status = SJA1105_ALREADY_CONFIGURED_ERROR;
    if (status != SJA1105_OK) return status;

    /* Assign the input arguments */
    dev->config->variant    = config->variant;
    dev->config->spi_handle = config->spi_handle;
    dev->config->cs_port    = config->cs_port;
    dev->config->cs_pin     = config->cs_pin;
    dev->config->rst_port   = config->rst_port;
    dev->config->rst_pin    = config->rst_pin;
    dev->config->timeout    = config->timeout;
    dev->callbacks          = callbacks;
    dev->ports              = ports;
    dev->initialised        = false;

    /* Only the SJA1105Q has been implemented. TODO: Add more */
    if (dev->config->variant != VARIANT_SJA1105Q) status = SJA1105_PARAMETER_ERROR;

    /* Check SPI parameters */
    if (dev->config->spi_handle->Init.DataSize    != SPI_DATASIZE_32BIT) status = SJA1105_PARAMETER_ERROR;
    if (dev->config->spi_handle->Init.CLKPolarity != SPI_POLARITY_LOW  ) status = SJA1105_PARAMETER_ERROR;
    if (dev->config->spi_handle->Init.CLKPhase    != SPI_PHASE_1EDGE   ) status = SJA1105_PARAMETER_ERROR;
    if (dev->config->spi_handle->Init.NSS         != SPI_NSS_SOFT      ) status = SJA1105_PARAMETER_ERROR;
    if (dev->config->spi_handle->Init.FirstBit    != SPI_FIRSTBIT_MSB  ) status = SJA1105_PARAMETER_ERROR;

    /* If there are invalid parameters then return */
    if (status != SJA1105_OK) return status;
    
    /* Set pins to a known state */
    HAL_GPIO_WritePin(dev->config->rst_port, dev->config->rst_pin, SET);
    HAL_GPIO_WritePin(dev->config->cs_port,  dev->config->cs_pin,  SET);
    
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
            if ((dev->config->variant != VARIANT_SJA1105) && (dev->config->variant != VARIANT_SJA1105T)) status = SJA1105_ID_ERROR;
            break;
        
        case PART_NR_SJA1105P:
            if (dev->config->variant != VARIANT_SJA1105P) status = SJA1105_ID_ERROR;
            break;
        
        case PART_NR_SJA1105Q:
            if (dev->config->variant != VARIANT_SJA1105Q) status = SJA1105_ID_ERROR;
            break;
        
        case PART_NR_SJA1105R:
            if (dev->config->variant != VARIANT_SJA1105R) status = SJA1105_ID_ERROR;
            break;
        
        case PART_NR_SJA1105S:
            if (dev->config->variant != VARIANT_SJA1105S) status = SJA1105_ID_ERROR;
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

SJA1105_StatusTypeDef SJA1105_ConfigureCGUPort(SJA1105_HandleTypeDef *dev, uint8_t port_num){

    SJA1105_StatusTypeDef status = SJA1105_OK;

    /* TODO: Implement */
    status = SJA1105_ERROR;

    return status;
}

SJA1105_StatusTypeDef SJA1105_WriteStaticConfig(SJA1105_HandleTypeDef *dev, const uint32_t *static_conf, uint32_t static_conf_size){

    SJA1105_StatusTypeDef status = SJA1105_OK;

    /* Check the static config matches the device type */
    switch (dev->config->variant) {
        case VARIANT_SJA1105:
        case VARIANT_SJA1105T:
            if (static_conf[0] != SJA1105_T_SWITCH_CORE_ID) status = SJA1105_STATIC_CONF_ERROR;
            break;
        case VARIANT_SJA1105P:
        case VARIANT_SJA1105R:
            if (static_conf[0] != SJA1105PR_SWITCH_CORE_ID) status = SJA1105_STATIC_CONF_ERROR;
            break;
        case VARIANT_SJA1105Q:
        case VARIANT_SJA1105S:
            if (static_conf[0] != SJA1105QS_SWITCH_CORE_ID) status = SJA1105_STATIC_CONF_ERROR;
            break;
        default:
            status = SJA1105_PARAMETER_ERROR;  /* Unknown variant */
            break;
    }
    if (status != SJA1105_OK) return status;

    /* Write the device ID */
    status = SJA1105_WriteRegister(dev, SJA1105_STATIC_CONF_ADDR, static_conf, SJA1105_STATIC_CONF_BLOCK_FIRST_OFFSET);
    if (status != SJA1105_OK) return status;

    /* Setup block writing variables */
    uint32_t block_index       = SJA1105_STATIC_CONF_BLOCK_FIRST_OFFSET;  /* Index of static_conf_size used for the start of the current block. Starts at 1 because the SWITCH_CORE_ID comes first. */
    uint32_t block_index_next  = 0;
    uint8_t  block_id          = 0;
    uint16_t block_size        = 0;  /* Block size listed in the static config */
    uint16_t block_size_actual = 0;  /* Actual block size including headers and CRCs */
    bool     last_block        = false;
    bool     skip_block        = false;
    uint32_t skipped_size      = 0;  /* Every time a block is skipped this counter should be incremented by block_size_actual */

    /* TODO: Get and check the CRC-32s */

    do {

        /* Get the block ID and size */
        block_id   = static_conf[((block_index + SJA1105_STATIC_CONF_BLOCK_ID_OFFSET  ) & SJA1105_STATIC_CONF_BLOCK_ID_MASK  ) >> SJA1105_STATIC_CONF_BLOCK_ID_SHIFT  ];
        block_size = static_conf[((block_index + SJA1105_STATIC_CONF_BLOCK_SIZE_OFFSET) & SJA1105_STATIC_CONF_BLOCK_SIZE_MASK) >> SJA1105_STATIC_CONF_BLOCK_SIZE_SHIFT];
        
        /* Get the actual block size (with headers and CRCs) */
        if (block_size != 0){
            block_size_actual = block_size + SJA1105_STATIC_CONF_BLOCK_NUM_HEADERS + SJA1105_STATIC_CONF_BLOCK_NUM_CRCS;
            block_index_next  = block_index + block_size_actual;
            if ((block_index_next) > static_conf_size) status = SJA1105_STATIC_CONF_ERROR;
        }

        /* Last block has size = 0 */
        else {
            last_block = true;
            block_size_actual = SJA1105_STATIC_CONF_BLOCK_LAST_SIZE;
            if ((block_index + block_size_actual) != static_conf_size) status = SJA1105_STATIC_CONF_ERROR;
        }

        /* Pre-write options */
        switch (block_id) {

            /* The CGU and ACU are configured by SJA1105_Init() and should not be overwritten */
            case SJA1105_STATIC_CONF_BLOCK_ID_CGU:
            case SJA1105_STATIC_CONF_BLOCK_ID_ACU:
                skip_block = true;  
                break;

            /* Check the L2BUSYS flag is set */
            case SJA1105_STATIC_CONF_BLOCK_ID_L2ADDR_LU:

                uint32_t reg_data = 0;
                bool     ready    = false;

                /* Check the general status 1 register for L2BUSYS (0 = initialised, 1 = not initialised). Try up to 10 times. */
                for (uint8_t retries = 0; !ready && (retries < 10); retries++){

                    /* Read the register */
                    status = SJA1105_ReadRegister(dev, SJA1105_REG_GENERAL_STATUS_1, &reg_data, 1);
                    if (status != SJA1105_OK) return status;

                    /* Extract L2BUSYS and delay if not set */
                    ready = (bool) (((reg_data & SJA1105_L2BUSYS_MASK) >> SJA1105_L2BUSYS_SHIFT) == 1);
                    if (!ready) dev->callbacks->callback_delay_ms(dev->config->timeout / 10);
                }

                break;

            default:
                break;
        }

        /* Skip the block */
        if (skip_block){
            skipped_size += block_size_actual;
        }

        /* Write the block */
        else {
            status = SJA1105_WriteRegister(dev, SJA1105_STATIC_CONF_ADDR + block_index - skipped_size, &static_conf[block_index], block_size_actual);
            if (status != SJA1105_OK) return status;
        }

        /* Set the block index for the next block */
        block_index = block_index_next;

    } while (!last_block);

    return status;
}


SJA1105_StatusTypeDef SJA1105_UpdatePort(SJA1105_HandleTypeDef *dev, uint8_t port_num, SJA1105_InterfaceTypeDef interface, SJA1105_SpeedTypeDef speed, SJA1105_IOVoltageTypeDef voltage){

    SJA1105_StatusTypeDef status = SJA1105_OK;

    /* Set the port to unconfigured */
    dev->ports[port_num].configured = false;
    
    /* Load the new configuration options */
    status = SJA1105_ConfigurePort(dev->ports, port_num, interface, speed, voltage);
    if (status != SJA1105_OK) return status;

    /* Configure the ACU with new port parameters */
    status = SJA1105_ConfigureACUPort(dev, port_num);
    if (status != SJA1105_OK) return status;
    
    /* Configure the CGU with new port parameters */
    status = SJA1105_ConfigureACUPort(dev, port_num);
    if (status != SJA1105_OK) return status;

    return status;
}
