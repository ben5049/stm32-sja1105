/*
 * sja1105_static_conf.c
 *
 *  Created on: Aug 6, 2025
 *      Author: bens1
 */

#include "memory.h"

#include "sja1105.h"
#include "internal/sja1105_conf.h"
#include "internal/sja1105_tables.h"
#include "internal/sja1105_regs.h"


sja1105_status_t SJA1105_FreeAllTableMemory(sja1105_handle_t *dev) {

    sja1105_status_t status = SJA1105_OK;
    sja1105_table_t  table;

    /* Go through each table */
    for (uint_fast8_t i = 0; i < SJA1105_NUM_TABLES; i++) {
        table = dev->tables.by_index[i];

        /* Ignore unused tables */
        if (!table.in_use) {
            continue;
        }

        /* Free differently based on the type */
        switch (SJA1105_GET_TABLE_LENGTH_TYPE(*table.id)) {

            /* Fixed length tables don't need anything freed */
            case SJA1105_TABLE_FIXED_LENGTH:
                table.in_use         = false;
                table.data_crc_valid = false;
                break;

            /* Variable length tables need everything freed */
            case SJA1105_TABLE_VARIABLE_LENGTH:
                status = dev->callbacks->callback_free(dev, (uint32_t *) table.id);
                if (status != SJA1105_OK) return status;
                status = dev->callbacks->callback_free(dev, table.size);
                if (status != SJA1105_OK) return status;
                status = dev->callbacks->callback_free(dev, table.header_crc);
                if (status != SJA1105_OK) return status;
                status = dev->callbacks->callback_free(dev, table.data);
                if (status != SJA1105_OK) return status;
                status = dev->callbacks->callback_free(dev, table.data_crc);
                if (status != SJA1105_OK) return status;
                table.in_use         = false;
                table.data_crc_valid = false;
                break;

            /* Invalid table ID */
            default:
                status = SJA1105_PARAMETER_ERROR;
                return status;
                break;
        }
    }

    /* Reset the fixed length table array */
    dev->tables.first_free = dev->tables.fixed_length_buffer + 1; /* Leave one space for the device ID */

    dev->tables.loaded           = false;
    dev->tables.written          = false;
    dev->tables.global_crc_valid = false;

    return status;
}


sja1105_status_t SJA1105_AllocateFixedLengthTable(sja1105_handle_t *dev, const uint32_t *block, uint8_t block_size) {

    sja1105_status_t status = SJA1105_OK;
    uint8_t          id;
    uint32_t         size;
    uint32_t         header_crc;
    uint32_t         data_crc;
    sja1105_table_t *table;

    /* Check table isn't already in use */
    id    = (block[SJA1105_STATIC_CONF_BLOCK_ID_OFFSET] & SJA1105_STATIC_CONF_BLOCK_ID_MASK) >> SJA1105_STATIC_CONF_BLOCK_ID_SHIFT;
    table = &dev->tables.by_index[SJA1105_GET_TABLE_INDEX(id)];
    if (table->in_use) status = SJA1105_ALREADY_CONFIGURED_ERROR;
    if (status != SJA1105_OK) return status;

    /* Check size argument */
    size = (block[SJA1105_STATIC_CONF_BLOCK_SIZE_OFFSET] & SJA1105_STATIC_CONF_BLOCK_SIZE_MASK) >> SJA1105_STATIC_CONF_BLOCK_SIZE_SHIFT;
    if (block_size != (size + SJA1105_STATIC_CONF_BLOCK_HEADER + SJA1105_STATIC_CONF_BLOCK_HEADER_CRC + SJA1105_STATIC_CONF_BLOCK_DATA_CRC)) status = SJA1105_STATIC_CONF_ERROR;
    if (status != SJA1105_OK) return status;

    /* Check header CRC */
    status = dev->callbacks->callback_crc_reset(dev);
    if (status != SJA1105_OK) return status;
    status = dev->callbacks->callback_crc_accumulate(dev, block, SJA1105_STATIC_CONF_BLOCK_HEADER, &header_crc);
    if (status != SJA1105_OK) return status;
    if (header_crc != block[SJA1105_STATIC_CONF_HEADER_CRC_OFFSET]) {
        status = SJA1105_CRC_ERROR;
        dev->events.crc_errors++;
        return status;
    }

    /* Check data CRC */
    status = dev->callbacks->callback_crc_reset(dev);
    if (status != SJA1105_OK) return status;
    status = dev->callbacks->callback_crc_accumulate(dev, block + SJA1105_STATIC_CONF_DATA_OFFSET, size, &data_crc);
    if (status != SJA1105_OK) return status;
    if (data_crc != block[block_size - 1]) {
        status = SJA1105_CRC_ERROR;
        dev->events.crc_errors++;
        return status;
    }

    /* Setup the pointers */
    table->id         = ((uint8_t *) dev->tables.first_free) + 3;
    table->size       = dev->tables.first_free + SJA1105_STATIC_CONF_BLOCK_SIZE_OFFSET;
    table->header_crc = dev->tables.first_free + SJA1105_STATIC_CONF_HEADER_CRC_OFFSET;
    table->data       = dev->tables.first_free + SJA1105_STATIC_CONF_DATA_OFFSET;
    table->header_crc = dev->tables.first_free + block_size - 1;

    /* Copy in the block and advance the free pointer */
    memcpy(dev->tables.first_free, block, block_size * sizeof(uint32_t));
    dev->tables.first_free += block_size;

    /* Check the values were copied correctly */
    if (*table->id != id) status = SJA1105_MEMORY_ERROR;
    if (*table->size != size) status = SJA1105_MEMORY_ERROR;
    if (status != SJA1105_OK) return status;

    /* Set the flags in the entry */
    table->in_use         = true;
    table->data_crc_valid = true;

    return status;
}


sja1105_status_t SJA1105_AllocateVariableLengthTable(sja1105_handle_t *dev, const uint32_t *block, uint8_t block_size) {

    sja1105_status_t status = SJA1105_OK;
    uint8_t          id;
    uint32_t         size;
    uint32_t         header_crc;
    uint32_t         data_crc;
    sja1105_table_t *table;

    /* Check table isn't already in use */
    id    = (block[SJA1105_STATIC_CONF_BLOCK_ID_OFFSET] & SJA1105_STATIC_CONF_BLOCK_ID_MASK) >> SJA1105_STATIC_CONF_BLOCK_ID_SHIFT;
    table = &dev->tables.by_index[SJA1105_GET_TABLE_INDEX(id)];
    if (table->in_use) status = SJA1105_ALREADY_CONFIGURED_ERROR;
    if (status != SJA1105_OK) return status;

    /* Check size argument */
    size = (block[SJA1105_STATIC_CONF_BLOCK_SIZE_OFFSET] & SJA1105_STATIC_CONF_BLOCK_SIZE_MASK) >> SJA1105_STATIC_CONF_BLOCK_SIZE_SHIFT;
    if (block_size != (size + SJA1105_STATIC_CONF_BLOCK_HEADER + SJA1105_STATIC_CONF_BLOCK_HEADER_CRC + SJA1105_STATIC_CONF_BLOCK_DATA_CRC)) status = SJA1105_STATIC_CONF_ERROR;
    if (status != SJA1105_OK) return status;

    /* Check header CRC */
    status = dev->callbacks->callback_crc_reset(dev);
    if (status != SJA1105_OK) return status;
    status = dev->callbacks->callback_crc_accumulate(dev, block, SJA1105_STATIC_CONF_BLOCK_HEADER, &header_crc);
    if (status != SJA1105_OK) return status;
    if (header_crc != block[SJA1105_STATIC_CONF_HEADER_CRC_OFFSET]) {
        status = SJA1105_CRC_ERROR;
        dev->events.crc_errors++;
        return status;
    }

    /* Check data CRC */
    status = dev->callbacks->callback_crc_reset(dev);
    if (status != SJA1105_OK) return status;
    status = dev->callbacks->callback_crc_accumulate(dev, block + SJA1105_STATIC_CONF_DATA_OFFSET, size, &data_crc);
    if (status != SJA1105_OK) return status;
    if (data_crc != block[block_size - 1]) {
        status = SJA1105_CRC_ERROR;
        dev->events.crc_errors++;
        return status;
    }

    /* Allocate the memory */
    status = dev->callbacks->callback_allocate(dev, (uint32_t *) table->id, SJA1105_STATIC_CONF_BLOCK_ID);
    if (status != SJA1105_OK) return status;
    status = dev->callbacks->callback_allocate(dev, table->size, SJA1105_STATIC_CONF_BLOCK_SIZE);
    if (status != SJA1105_OK) return status;
    status = dev->callbacks->callback_allocate(dev, table->header_crc, SJA1105_STATIC_CONF_BLOCK_HEADER_CRC);
    if (status != SJA1105_OK) return status;
    status = dev->callbacks->callback_allocate(dev, table->data, size);
    if (status != SJA1105_OK) return status;
    status = dev->callbacks->callback_allocate(dev, table->data_crc, SJA1105_STATIC_CONF_BLOCK_DATA_CRC);
    if (status != SJA1105_OK) return status;

    /* Copy in the values */
    *table->id         = id;
    *table->size       = size;
    *table->header_crc = block[SJA1105_STATIC_CONF_HEADER_CRC_OFFSET];
    memcpy(table->data, block + SJA1105_STATIC_CONF_DATA_OFFSET, size);
    *table->data_crc = block[block_size - 1];

    /* Set the flags in the entry */
    table->in_use         = true;
    table->data_crc_valid = true;

    return status;
}


sja1105_status_t SJA1105_LoadStaticConfig(sja1105_handle_t *dev, const uint32_t *static_conf, uint32_t static_conf_size) {

    sja1105_status_t status = SJA1105_OK;

    /* Argument checking */
    if (static_conf_size < SJA1105_STATIC_CONF_MIN_SIZE) status = SJA1105_PARAMETER_ERROR;
    if (status != SJA1105_OK) return status;

    /* Free all memory used by tables (also resets flags) */
    status = SJA1105_FreeAllTableMemory(dev);
    if (status != SJA1105_OK) return status;

    /* Check the device ID in the new static config is valid */
    status = SJA1105_CheckDeviceID(dev, static_conf[0]);
    if (status != SJA1105_OK) return status;
    *dev->tables.device_id = static_conf[0];

    /* Setup block variables */
    uint32_t block_index       = SJA1105_STATIC_CONF_BLOCK_FIRST_OFFSET; /* Index of static_conf_size used for the start of the current block. Starts at 1 because the SWITCH_CORE_ID comes first. */
    uint32_t block_index_next  = 0;
    uint8_t  block_id          = 0;
    uint16_t block_size        = 0; /* Block size listed in the static config */
    uint16_t block_size_actual = 0; /* Actual block size including headers and CRCs */
    bool     last_block        = false;

    do {

        /* Get the block ID and size */
        block_id   = (static_conf[block_index + SJA1105_STATIC_CONF_BLOCK_ID_OFFSET] & SJA1105_STATIC_CONF_BLOCK_ID_MASK) >> SJA1105_STATIC_CONF_BLOCK_ID_SHIFT;
        block_size = (static_conf[block_index + SJA1105_STATIC_CONF_BLOCK_SIZE_OFFSET] & SJA1105_STATIC_CONF_BLOCK_SIZE_MASK) >> SJA1105_STATIC_CONF_BLOCK_SIZE_SHIFT;

        /* Get the actual block size (with headers and CRCs) */
        if (block_size != 0) {
            block_size_actual = block_size + SJA1105_STATIC_CONF_BLOCK_HEADER + SJA1105_STATIC_CONF_BLOCK_HEADER_CRC + SJA1105_STATIC_CONF_BLOCK_DATA_CRC;
            block_index_next  = block_index + block_size_actual;
            if ((block_index_next) >= static_conf_size) status = SJA1105_STATIC_CONF_ERROR;
            if (status != SJA1105_OK) return status;
        }

        /* Last block has size = 0 and id = 0 */
        else if (block_id == 0) {
            last_block        = true;
            block_size_actual = SJA1105_STATIC_CONF_BLOCK_LAST_SIZE;
            if ((block_index + block_size_actual) != static_conf_size) status = SJA1105_STATIC_CONF_ERROR;
            if (status != SJA1105_OK) return status;
            break;
        }

        /* Non-final block has size = 0 */
        else {
            status = SJA1105_STATIC_CONF_ERROR;
            if (status != SJA1105_OK) return status;
        }

        /* Check blocks before copying them */
        SJA1105_CheckTable(dev, block_id, static_conf + block_index + SJA1105_STATIC_CONF_BLOCK_HEADER + SJA1105_STATIC_CONF_BLOCK_HEADER_CRC, block_size);
        if (status != SJA1105_OK) return status;

        /* Allocate and store the table */
        switch (SJA1105_GET_TABLE_LENGTH_TYPE(block_id)) {

            case SJA1105_TABLE_FIXED_LENGTH:
                SJA1105_AllocateFixedLengthTable(dev, static_conf + block_index, block_size_actual);
                break;

            case SJA1105_TABLE_VARIABLE_LENGTH:
                SJA1105_AllocateVariableLengthTable(dev, static_conf + block_index, block_size_actual);
                break;

            default:
                break;
        }

        /* Set the block index for the next block */
        block_index = block_index_next;

    } while (!last_block);

    /* Check all required tables are present */
    status = SJA1105_CheckRequiredTables(dev);
    if (status != SJA1105_OK) return status;

    /* TODO: Add the CGU config */

    /* TODO: Add the ACU config */

    /* TODO: Disable ingress, egress and learning in MAC config table */

    dev->tables.loaded = true;

    return status;
}

/* Write the static config to the chip */
sja1105_status_t SJA1105_WriteStaticConfig(sja1105_handle_t *dev) {

    sja1105_status_t status = SJA1105_NOT_IMPLEMENTED_ERROR;

    // /* Write the device ID */
    // status = SJA1105_WriteRegister(dev, SJA1105_STATIC_CONF_ADDR, static_conf, SJA1105_STATIC_CONF_BLOCK_FIRST_OFFSET);
    // if (status != SJA1105_OK) return status;

    // /* Check the device ID was accepted */
    // status = SJA1105_ReadRegisterWithCheck(dev, SJA1105_REG_STATIC_CONF_FLAGS, &reg_data, 1);
    // if (status != SJA1105_OK) return status;
    // if ((reg_data & SJA1105_IDS_MASK) != 0) {
    //     status = SJA1105_ID_ERROR;
    //     return status;
    // }


    // /* Write the block */
    // status = SJA1105_WriteRegister(dev, SJA1105_STATIC_CONF_ADDR + block_index, &static_conf[block_index], block_size_actual);
    // if (status != SJA1105_OK) return status;

    // /* Check the block had no CRC errors */
    // if (!last_block) {
    //     status = SJA1105_ReadRegisterWithCheck(dev, SJA1105_REG_STATIC_CONF_FLAGS, &reg_data, 1);
    //     if (status != SJA1105_OK) return status;

    //     /* If there is a CRC error then report it */
    //     if ((reg_data & SJA1105_CRCCHKL_MASK) != 0) {
    //         status = SJA1105_CRC_ERROR;
    //         dev->events.crc_errors++;
    //         return status;
    //     }
    // }


    // /* Read the intial config flags register */
    // status = SJA1105_ReadRegisterWithCheck(dev, SJA1105_REG_STATIC_CONF_FLAGS, &reg_data, 1);
    // if (status != SJA1105_OK) return status;

    // /* Check for global CRC errors */
    // if ((reg_data & SJA1105_CRCCHKG_MASK) != 0) {
    //     status = SJA1105_CRC_ERROR;
    //     dev->events.crc_errors++;
    //     return status;
    // }

    // /* Check that the config was accepted */
    // if ((reg_data & SJA1105_CONFIGS_MASK) == 0) {
    //     status = SJA1105_STATIC_CONF_ERROR;
    //     return status;
    // }

    // /* Update the device struct */
    // dev->static_conf_loaded = true;

    return status;
}


sja1105_status_t SJA1105_CheckRequiredTables(sja1105_handle_t *dev) {

    sja1105_status_t status = SJA1105_OK;

    /* Check if any required tables are missing */
    if (!dev->tables.l2_policing.in_use || dev->tables.l2_policing.size == 0) status = SJA1105_MISSING_TABLE_ERROR;
    if (!dev->tables.l2_forwarding.in_use) status = SJA1105_MISSING_TABLE_ERROR;
    if (!dev->tables.l2_forwarding_parameters.in_use) status = SJA1105_MISSING_TABLE_ERROR;
    if (!dev->tables.mac_configuration.in_use) status = SJA1105_MISSING_TABLE_ERROR;
    if (!dev->tables.general_parameters.in_use) status = SJA1105_MISSING_TABLE_ERROR;
    if (!dev->tables.xmii_mode_parameters.in_use) status = SJA1105_MISSING_TABLE_ERROR;

    /* Check table dependencies */
    if (!dev->tables.schedule_entry_point_parameters.in_use && dev->tables.schedule.in_use) status = SJA1105_MISSING_TABLE_ERROR;
    if (!dev->tables.schedule_parameters.in_use && dev->tables.schedule.in_use) status = SJA1105_MISSING_TABLE_ERROR;
    if (!dev->tables.schedule_entry_point_parameters.in_use && dev->tables.schedule.in_use) status = SJA1105_MISSING_TABLE_ERROR;
    if (!dev->tables.vl_forwarding_parameters.in_use && dev->tables.vl_forwarding.in_use) status = SJA1105_MISSING_TABLE_ERROR;

    /* TODO: Check if VL Policing and forwarding tables are present if VL lookup is and has any critical entries */

    return status;
}
