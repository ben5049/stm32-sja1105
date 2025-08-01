/*
 * sja1105_conf.c
 *
 *  Created on: Jul 30, 2025
 *      Author: bens1
 */

#include "sja1105.h"
#include "sja1105_conf.h"
#include "sja1105_io.h"
#include "sja1105_regs.h"


SJA1105_StatusTypeDef SJA1105_PortConfigure(SJA1105_PortTypeDef *ports, uint8_t port_num, SJA1105_InterfaceTypeDef interface, SJA1105_ModeTypeDef mode, bool output_rmii_refclk, SJA1105_SpeedTypeDef speed, SJA1105_IOVoltageTypeDef voltage){

    SJA1105_StatusTypeDef status = SJA1105_OK;

    /* Check the parameters */
    if ( port_num  >= SJA1105_NUM_PORTS                                       ) status = SJA1105_PARAMETER_ERROR;
    if ( interface >= SJA1105_INTERFACE_INVALID                               ) status = SJA1105_PARAMETER_ERROR;
    if ( mode      >= SJA1105_MODE_INVALID                                    ) status = SJA1105_PARAMETER_ERROR;
    if ( speed     >= SJA1105_SPEED_INVALID                                   ) status = SJA1105_PARAMETER_ERROR;
    if ( voltage   >= SJA1105_IO_INVALID_V                                    ) status = SJA1105_PARAMETER_ERROR;
    if ((interface == SJA1105_INTERFACE_MII)  && (speed == SJA1105_SPEED_1G)  ) status = SJA1105_PARAMETER_ERROR;           /* MII Interface doesn't support 1G speeds */
    if ((interface == SJA1105_INTERFACE_RMII) && (speed == SJA1105_SPEED_1G)  ) status = SJA1105_PARAMETER_ERROR;           /* RMII Interface doesn't support 1G speeds */
    if ((voltage   == SJA1105_IO_1V8) && (interface == SJA1105_INTERFACE_RMII)) status = SJA1105_PARAMETER_ERROR;           /* 1V8 RMII not supported */
    if (ports[port_num].configured == true                                    ) status = SJA1105_ALREADY_CONFIGURED_ERROR;  /* Note this may cause an unintended error if the struct uses non-zeroed memory. */
    if (status != SJA1105_OK) return status;

    /* Assign the parameters */
    ports[port_num].port_num           = port_num;
    ports[port_num].interface          = interface;
    ports[port_num].mode               = mode;
    ports[port_num].output_rmii_refclk = output_rmii_refclk;
    ports[port_num].speed              = speed;
    ports[port_num].dyanamic_speed     = speed;
    ports[port_num].voltage            = voltage;
    ports[port_num].configured         = true;

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

    /* Check the device hasn't already been initialised. Note this may cause an unintended error if the struct uses non-zeroed memory. */
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
    dev->config->host_port  = config->host_port;
    dev->callbacks          = callbacks;
    dev->ports              = ports;
    dev->static_conf_loaded = false;
    dev->static_conf_crc32  = static_conf[static_conf_size-1];
    dev->initialised        = false;

    /* Only the SJA1105Q has been implemented. TODO: Add more */
    if (dev->config->variant != VARIANT_SJA1105Q) status = SJA1105_NOT_IMPLEMENTED_ERROR;

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
    status = SJA1105_CheckPartID(dev);
    if (status != SJA1105_OK) return status;


    /* Configure the SJA1105 */

    /* Step 1: RESET */
    SJA1105_Reset(dev);

    /* Step 2: STATIC CONFIGURATION (Try up to 3 times) */
    bool crc_success = false;
    for (uint8_t attempt = 0; !crc_success && (attempt < 3); attempt++){

        /* Write the configuration */
        status = SJA1105_WriteStaticConfig(dev, static_conf, static_conf_size);

        /* Check for CRC errors */
        if (status != SJA1105_OK) {
            if (status != SJA1105_CRC_ERROR) return status;
        }
        else crc_success = true;
    }
    if (status != SJA1105_OK) return status;

    /* Step 3: ACU REGISTER SETUP */
    status = SJA1105_ConfigureACU(dev);
    if (status != SJA1105_OK) return status;
    
    /* Step 4: CGU REGISTER SETUP */
    status = SJA1105_ConfigureCGU(dev);
    if (status != SJA1105_OK) return status;

    /* Step 5: SGMII PHY/PCS (optional) */
    

    /* Check the status registers */
    status = SJA1105_CheckStatusRegisters(dev);
    if (status != SJA1105_OK) return status;

    dev->initialised = true;

    return status;
}

SJA1105_StatusTypeDef SJA1105_ReInit(SJA1105_HandleTypeDef *dev, const uint32_t *static_conf, uint32_t static_conf_size){

    SJA1105_StatusTypeDef status = SJA1105_OK;
    
    dev->initialised = false;
    status = SJA1105_Init(dev, dev->config, dev->ports, dev->callbacks, static_conf, static_conf_size);

    return status;
}

SJA1105_StatusTypeDef SJA1105_CheckPartID(SJA1105_HandleTypeDef *dev){

    SJA1105_StatusTypeDef status = SJA1105_OK;
    uint32_t reg_data;

    /* Read the DEVICE_ID register */
    status = SJA1105_ReadRegisterWithCheck(dev, SJA1105_REG_DEVICE_ID, &reg_data, 1);
    if (status != SJA1105_OK) return status;
    
    /* Check the device ID config matches the variant */
    switch (reg_data) {

        case SJA1105ET_DEVICE_ID:
            if ((dev->config->variant != VARIANT_SJA1105E) && (dev->config->variant != VARIANT_SJA1105T)) status = SJA1105_ID_ERROR;
            break;

        case SJA1105QS_DEVICE_ID:
            if ((dev->config->variant != VARIANT_SJA1105Q) && (dev->config->variant != VARIANT_SJA1105S)) status = SJA1105_ID_ERROR;
            break;

        case SJA1105PR_DEVICE_ID:
            if ((dev->config->variant != VARIANT_SJA1105P) && (dev->config->variant != VARIANT_SJA1105R)) status = SJA1105_ID_ERROR;
            break;

        default:
            status = SJA1105_ID_ERROR;  /* Unknown device */
            break;
        }
        if (status != SJA1105_OK) return status;
        
        /* Read the PROD_ID register */
        status = SJA1105_ReadRegisterWithCheck(dev, SJA1105_ACU_REG_PROD_ID, &reg_data, 1);
        if (status != SJA1105_OK) return status;
        
        /* Extract and check the PART_NR */
        switch ((reg_data & SJA1105_PART_NR_MASK) >> SJA1105_PART_NR_OFFSET){
            case PART_NR_SJA1105ET:
                if ((dev->config->variant != VARIANT_SJA1105E) && (dev->config->variant != VARIANT_SJA1105T)) status = SJA1105_ID_ERROR;
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
        if (status != SJA1105_OK) return status;
        
    return status;
}


SJA1105_StatusTypeDef SJA1105_WriteStaticConfig(SJA1105_HandleTypeDef *dev, const uint32_t *static_conf, uint32_t static_conf_size){

    SJA1105_StatusTypeDef status   = SJA1105_OK;
    uint32_t              reg_data = 0;

    /* Can't upload a static config if one has already been loaded */
    if (dev->static_conf_loaded){
        status = SJA1105_ALREADY_CONFIGURED_ERROR;
        return status;
    }

    /* Update the device struct. This is done here so that if a SJA1105_STATIC_CONF_ERROR is returned the program knows which config caused the error */
    dev->static_conf_crc32 = static_conf[static_conf_size - 1];

    /* Check the static config matches the device type */
    switch (dev->config->variant) {
        case VARIANT_SJA1105E:
        case VARIANT_SJA1105T:
            if (static_conf[0] != SJA1105ET_DEVICE_ID) status = SJA1105_STATIC_CONF_ERROR;
            break;
        case VARIANT_SJA1105P:
        case VARIANT_SJA1105R:
            if (static_conf[0] != SJA1105PR_DEVICE_ID) status = SJA1105_STATIC_CONF_ERROR;
            break;
        case VARIANT_SJA1105Q:
        case VARIANT_SJA1105S:
            if (static_conf[0] != SJA1105QS_DEVICE_ID) status = SJA1105_STATIC_CONF_ERROR;
            break;
        default:
            status = SJA1105_PARAMETER_ERROR;  /* Unknown variant */
            break;
    }
    if (status != SJA1105_OK) return status;

    /* Write the device ID */
    status = SJA1105_WriteRegister(dev, SJA1105_STATIC_CONF_ADDR, static_conf, SJA1105_STATIC_CONF_BLOCK_FIRST_OFFSET);
    if (status != SJA1105_OK) return status;
    
    /* Check the device ID was accepted */
    status = SJA1105_ReadRegisterWithCheck(dev, SJA1105_REG_STATIC_CONF_FLAGS, &reg_data, 1);
    if (status != SJA1105_OK) return status;
    if ((reg_data & SJA1105_IDS_MASK) != 0) {
        status = SJA1105_ID_ERROR;
        return status;
    }

    /* Setup block writing variables */
    uint32_t block_index       = SJA1105_STATIC_CONF_BLOCK_FIRST_OFFSET;  /* Index of static_conf_size used for the start of the current block. Starts at 1 because the SWITCH_CORE_ID comes first. */
    uint32_t block_index_next  = 0;
    uint8_t  block_id          = 0;
    uint16_t block_size        = 0;  /* Block size listed in the static config */
    uint16_t block_size_actual = 0;  /* Actual block size including headers and CRCs */
    bool     last_block        = false;

    do {

        /* Get the block ID and size */
        block_id   = static_conf[((block_index + SJA1105_STATIC_CONF_BLOCK_ID_OFFSET  ) & SJA1105_STATIC_CONF_BLOCK_ID_MASK  ) >> SJA1105_STATIC_CONF_BLOCK_ID_SHIFT  ];
        block_size = static_conf[((block_index + SJA1105_STATIC_CONF_BLOCK_SIZE_OFFSET) & SJA1105_STATIC_CONF_BLOCK_SIZE_MASK) >> SJA1105_STATIC_CONF_BLOCK_SIZE_SHIFT];
        
        /* Get the actual block size (with headers and CRCs) */
        if (block_size != 0){
            block_size_actual = block_size + SJA1105_STATIC_CONF_BLOCK_HEADER + SJA1105_STATIC_CONF_BLOCK_HEADER_CRC + SJA1105_STATIC_CONF_BLOCK_DATA_CRC;
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

            /* Check the MAC configuration table */
            case SJA1105_STATIC_CONF_BLOCK_ID_MAC_CONF:
                status = SJA1105_CheckMACConfTable(dev, &static_conf[block_index + SJA1105_STATIC_CONF_BLOCK_HEADER + SJA1105_STATIC_CONF_BLOCK_HEADER_CRC], block_size);
                break;
            
            /* Check the xMII mode parameters table */
            case SJA1105_STATIC_CONF_BLOCK_ID_XMII_MODE:
                status = SJA1105_CheckxMIIModeTable(dev, &static_conf[block_index + SJA1105_STATIC_CONF_BLOCK_HEADER + SJA1105_STATIC_CONF_BLOCK_HEADER_CRC], block_size);
                break;

            /* Check the L2BUSYS flag is set before writing the L2 address table */
            case SJA1105_STATIC_CONF_BLOCK_ID_L2ADDR_LU:

                /* Check the general status 1 register for L2BUSYS (0 = initialised, 1 = not initialised). Try up to 10 times. */
                bool ready = false;
                for (uint8_t attempt = 0; !ready && (attempt < 10); attempt++){

                    /* Read the flag */
                    status = SJA1105_ReadFlag(dev, SJA1105_REG_GENERAL_STATUS_1, SJA1105_L2BUSYS_MASK, &ready);
                    if (status != SJA1105_OK) return status;
                    
                    /* Delay and try again if not set */
                    if (!ready) dev->callbacks->callback_delay_ms(dev->config->timeout / 10);
                }
                if (!ready) status = SJA1105_L2_BUSY_ERROR;
                break;

            default:
                break;
        }
        if (status != SJA1105_OK) return status;

        /* Write the block */
        status = SJA1105_WriteRegister(dev, SJA1105_STATIC_CONF_ADDR + block_index, &static_conf[block_index], block_size_actual);
        if (status != SJA1105_OK) return status;
        
        /* Check the block had no CRC errors */
        if (!last_block){
            status = SJA1105_ReadRegisterWithCheck(dev, SJA1105_REG_STATIC_CONF_FLAGS, &reg_data, 1);
            if (status != SJA1105_OK) return status;

            /* If there is a CRC error then report it */
            if ((reg_data & SJA1105_CRCCHKL_MASK) != 0){
                status = SJA1105_CRC_ERROR;
                return status;
            }
        }

        /* Set the block index for the next block */
        block_index = block_index_next;

    } while (!last_block);

    /* Read the intial config flags register */
    status = SJA1105_ReadRegisterWithCheck(dev, SJA1105_REG_STATIC_CONF_FLAGS, &reg_data, 1);
    if (status != SJA1105_OK) return status;

    /* Check for global CRC errors */
    if ((reg_data & SJA1105_CRCCHKG_MASK) != 0) {
        status = SJA1105_CRC_ERROR;
        return status;
    }

    /* Check that the config was accepted */
    if ((reg_data & SJA1105_CONFIGS_MASK) == 0) {
        status = SJA1105_STATIC_CONF_ERROR;
        return status;
    }

    /* Update the device struct */
    dev->static_conf_loaded = true;

    return status;
}

SJA1105_StatusTypeDef SJA1105_CheckMACConfTable(SJA1105_HandleTypeDef *dev, const uint32_t *table, uint32_t size){
    
    SJA1105_StatusTypeDef status = SJA1105_OK;
    SJA1105_SpeedTypeDef  speed;

    /* Check the size is correct */
    if (size != (SJA1105_NUM_PORTS * SJA1105_STATIC_CONF_MAC_CONF_ENTRY_SIZE)) status = SJA1105_STATIC_CONF_ERROR;
    if (status != SJA1105_OK) return status;

    /* Check each port's speed */
    for (uint8_t port_num = 0; port_num < SJA1105_NUM_PORTS; port_num++){
        speed = (table[SJA1105_STATIC_CONF_MAC_CONF_WORD(port_num, SJA1105_STATIC_CONF_MAC_CONF_SPEED_OFFSET)] & SJA1105_STATIC_CONF_MAC_CONF_SPEED_MASK) >> SJA1105_STATIC_CONF_MAC_CONF_SPEED_SHIFT;
        if (speed != dev->ports[port_num].speed) status = SJA1105_STATIC_CONF_ERROR;
        if (status != SJA1105_OK) return status;
    }

    return status;
}

SJA1105_StatusTypeDef SJA1105_CheckxMIIModeTable(SJA1105_HandleTypeDef *dev, const uint32_t *table, uint32_t size){
    
    SJA1105_StatusTypeDef    status = SJA1105_OK;
    SJA1105_ModeTypeDef      mode;
    SJA1105_InterfaceTypeDef interface;

    /* Check the size is correct */
    if (size != 1) status = SJA1105_STATIC_CONF_ERROR;
    if (status != SJA1105_OK) return status;

    /* Check each port's mode and interface */
    for (uint8_t port_num = 0; port_num < SJA1105_NUM_PORTS; port_num++){

        /* Check the mode */
        mode = (table[0] & SJA1105_STATIC_CONF_XMII_MODE_PHY_MAC_MASK(port_num)) >> SJA1105_STATIC_CONF_XMII_MODE_PHY_MAC_SHIFT(port_num);
        if (mode != dev->ports[port_num].mode){

            /* Special case: The switch is acting as a PHY, but the MAC it is connected to cannot supply REFCLK.
            *               In this case the switch port is actually configured as a MAC, but operates as a PHY.
            */
            if ((dev->ports[port_num].mode      == SJA1105_MODE_PHY      ) &&
                (dev->ports[port_num].interface == SJA1105_INTERFACE_RMII) &&
                 dev->ports[port_num].output_rmii_refclk) status = SJA1105_OK;

            /* Otherwise its an error */
            else status = SJA1105_STATIC_CONF_ERROR;
        }
        if (status != SJA1105_OK) return status;

        /* Check the interface */
        interface = (table[0] & SJA1105_STATIC_CONF_XMII_MODE_INTERFACE_MASK(port_num)) >> SJA1105_STATIC_CONF_XMII_MODE_INTERFACE_SHIFT(port_num);
        if (interface != dev->ports[port_num].interface) status = SJA1105_STATIC_CONF_ERROR;
        if (status != SJA1105_OK) return status;

        /* Check SGMII is configured correctly */
        if ((dev->ports[port_num].interface == SJA1105_INTERFACE_SGMII) && (mode != SJA1105_MODE_MAC)){
            status = SJA1105_STATIC_CONF_ERROR;
            return status;
        }
    }

    return status;
}
