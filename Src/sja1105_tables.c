/*
 * sja1105_tables.c
 *
 *  Created on: Aug 2, 2025
 *      Author: bens1
 */

#include "memory.h"
#include "stdlib.h"
#include "assert.h"

#include "sja1105.h"
#include "internal/sja1105_tables.h"
#include "internal/sja1105_regs.h"
#include "internal/sja1105_conf.h"
#include "internal/sja1105_io.h"


const sja1105_table_type_t SJA1105_TABLE_TYPE_LUT[SJA1105_BLOCK_ID_SGMII_CONF + 1] = {
    [SJA1105_BLOCK_ID_SCHEDULE]                     = SJA1105_TABLE_VARIABLE_LENGTH,
    [SJA1105_BLOCK_ID_SCHEDULE_ENTRY_POINTS]        = SJA1105_TABLE_VARIABLE_LENGTH,
    [SJA1105_BLOCK_ID_VL_LOOKUP]                    = SJA1105_TABLE_VARIABLE_LENGTH,
    [SJA1105_BLOCK_ID_VL_POLICING]                  = SJA1105_TABLE_VARIABLE_LENGTH,
    [SJA1105_BLOCK_ID_VL_FORWARDING]                = SJA1105_TABLE_VARIABLE_LENGTH,
    [SJA1105_BLOCK_ID_L2_ADDR_LOOKUP]               = SJA1105_TABLE_VARIABLE_LENGTH,
    [SJA1105_BLOCK_ID_L2_POLICING]                  = SJA1105_TABLE_VARIABLE_LENGTH,
    [SJA1105_BLOCK_ID_VLAN_LOOKUP]                  = SJA1105_TABLE_VARIABLE_LENGTH,
    [SJA1105_BLOCK_ID_L2_FORWARDING]                = SJA1105_TABLE_VARIABLE_LENGTH,
    [SJA1105_BLOCK_ID_MAC_CONF]                     = SJA1105_TABLE_FIXED_LENGTH,
    [SJA1105_BLOCK_ID_SCHEDULE_PARAMS]              = SJA1105_TABLE_FIXED_LENGTH,
    [SJA1105_BLOCK_ID_SCHEDULE_ENTRY_POINTS_PARAMS] = SJA1105_TABLE_FIXED_LENGTH,
    [SJA1105_BLOCK_ID_VL_FORWARDING_PARAMS]         = SJA1105_TABLE_FIXED_LENGTH,
    [SJA1105_BLOCK_ID_L2_LOOKUP_PARAMS]             = SJA1105_TABLE_FIXED_LENGTH,
    [SJA1105_BLOCK_ID_L2_FORWARDING_PARAMS]         = SJA1105_TABLE_FIXED_LENGTH,
    [SJA1105_BLOCK_ID_CLK_SYNC_PARAMS]              = SJA1105_TABLE_FIXED_LENGTH,
    [SJA1105_BLOCK_ID_AVB_PARAMS]                   = SJA1105_TABLE_FIXED_LENGTH,
    [SJA1105_BLOCK_ID_GENERAL_PARAMS]               = SJA1105_TABLE_FIXED_LENGTH,
    [SJA1105_BLOCK_ID_RETAGGING]                    = SJA1105_TABLE_VARIABLE_LENGTH,
    [SJA1105_BLOCK_ID_CBS]                          = SJA1105_TABLE_VARIABLE_LENGTH,
    [SJA1105_BLOCK_ID_XMII_MODE]                    = SJA1105_TABLE_FIXED_LENGTH,
    [SJA1105_BLOCK_ID_CGU]                          = SJA1105_TABLE_FIXED_LENGTH,
    [SJA1105_BLOCK_ID_RGU]                          = SJA1105_TABLE_FIXED_LENGTH,
    [SJA1105_BLOCK_ID_ACU]                          = SJA1105_TABLE_FIXED_LENGTH,
    [SJA1105_BLOCK_ID_SGMII_CONF]                   = SJA1105_TABLE_FIXED_LENGTH,
};

const uint8_t SJA1105_TABLE_INDEX_LUT[SJA1105_BLOCK_ID_SGMII_CONF + 1] = {
    [SJA1105_BLOCK_ID_SCHEDULE]                     = 0,
    [SJA1105_BLOCK_ID_SCHEDULE_ENTRY_POINTS]        = 1,
    [SJA1105_BLOCK_ID_VL_LOOKUP]                    = 2,
    [SJA1105_BLOCK_ID_VL_POLICING]                  = 3,
    [SJA1105_BLOCK_ID_VL_FORWARDING]                = 4,
    [SJA1105_BLOCK_ID_L2_ADDR_LOOKUP]               = 5,
    [SJA1105_BLOCK_ID_L2_POLICING]                  = 6,
    [SJA1105_BLOCK_ID_VLAN_LOOKUP]                  = 7,
    [SJA1105_BLOCK_ID_L2_FORWARDING]                = 8,
    [SJA1105_BLOCK_ID_MAC_CONF]                     = 9,
    [SJA1105_BLOCK_ID_SCHEDULE_PARAMS]              = 10,
    [SJA1105_BLOCK_ID_SCHEDULE_ENTRY_POINTS_PARAMS] = 11,
    [SJA1105_BLOCK_ID_VL_FORWARDING_PARAMS]         = 12,
    [SJA1105_BLOCK_ID_L2_LOOKUP_PARAMS]             = 13,
    [SJA1105_BLOCK_ID_L2_FORWARDING_PARAMS]         = 14,
    [SJA1105_BLOCK_ID_CLK_SYNC_PARAMS]              = 15,
    [SJA1105_BLOCK_ID_AVB_PARAMS]                   = 16,
    [SJA1105_BLOCK_ID_GENERAL_PARAMS]               = 17,
    [SJA1105_BLOCK_ID_RETAGGING]                    = 18,
    [SJA1105_BLOCK_ID_CBS]                          = 19,
    [SJA1105_BLOCK_ID_XMII_MODE]                    = 20,
    [SJA1105_BLOCK_ID_CGU]                          = 21,
    [SJA1105_BLOCK_ID_RGU]                          = 22,
    [SJA1105_BLOCK_ID_ACU]                          = 23,
    [SJA1105_BLOCK_ID_SGMII_CONF]                   = 24,
};


sja1105_status_t SJA1105_CheckTable(sja1105_handle_t *dev, uint8_t id, const uint32_t *table, uint32_t size) {
    sja1105_status_t status = SJA1105_NOT_IMPLEMENTED_ERROR;

    switch (id) {

        case SJA1105_BLOCK_ID_MAC_CONF: {
            status = SJA1105_MACConfTableCheck(dev, table, size);
            break;
        }

        case SJA1105_BLOCK_ID_GENERAL_PARAMS: {
            status = SJA1105_GeneralParamsTableCheck(dev, table, size);
            break;
        }

        case SJA1105_BLOCK_ID_XMII_MODE: {
            status = SJA1105_xMIIModeTableCheck(dev, table, size);
            break;
        }

        default:
            status = SJA1105_ID_ERROR;
            break;
    }

    return status;
}


sja1105_status_t SJA1105_MACConfTableCheck(sja1105_handle_t *dev, const uint32_t *table, uint32_t size) {
    /* TODO: ADD NEW TABLE LOGIC*/
    sja1105_status_t status = SJA1105_OK;
    sja1105_speed_t  speed;

    /* Check the size is correct */
    if (size != (SJA1105_NUM_PORTS * SJA1105_STATIC_CONF_MAC_CONF_ENTRY_SIZE)) status = SJA1105_STATIC_CONF_ERROR;
    if (status != SJA1105_OK) return status;

    /* Check each port's speed */
    for (uint_fast8_t port_num = 0; port_num < SJA1105_NUM_PORTS; port_num++) {
        SJA1105_MACConfTableGetSpeed(table, size, port_num, &speed);
        if (speed != dev->config->ports[port_num].speed) status = SJA1105_STATIC_CONF_ERROR;
        if (status != SJA1105_OK) return status;
    }

    return status;
}


sja1105_status_t SJA1105_MACConfTableGetIngress(const uint32_t *table, uint32_t size, uint8_t port_num, bool *ingress) {
    /* TODO: ADD NEW TABLE LOGIC*/
    sja1105_status_t status = SJA1105_OK;
    uint8_t          index  = SJA1105_STATIC_CONF_MAC_CONF_WORD(port_num, SJA1105_STATIC_CONF_MAC_CONF_INGRESS_OFFSET);

    if (index >= size) status = SJA1105_PARAMETER_ERROR;
    if (status != SJA1105_OK) return status;

    *ingress = (table[index] & SJA1105_STATIC_CONF_MAC_CONF_INGRESS_MASK) != 0;

    return status;
}


sja1105_status_t SJA1105_MACConfTableSetIngress(uint32_t *table, uint32_t size, uint8_t port_num, bool ingress) {
    /* TODO: ADD NEW TABLE LOGIC*/
    sja1105_status_t status = SJA1105_OK;
    uint8_t          index  = SJA1105_STATIC_CONF_MAC_CONF_WORD(port_num, SJA1105_STATIC_CONF_MAC_CONF_INGRESS_OFFSET);

    if (index >= size) status = SJA1105_PARAMETER_ERROR;
    if (status != SJA1105_OK) return status;

    if (ingress) {
        table[index] |= SJA1105_STATIC_CONF_MAC_CONF_INGRESS_MASK;
    } else {
        table[index] &= ~SJA1105_STATIC_CONF_MAC_CONF_INGRESS_MASK;
    }

    return status;
}


sja1105_status_t SJA1105_MACConfTableGetEgress(const uint32_t *table, uint32_t size, uint8_t port_num, bool *egress) {
    /* TODO: ADD NEW TABLE LOGIC*/
    sja1105_status_t status = SJA1105_OK;
    uint8_t          index  = SJA1105_STATIC_CONF_MAC_CONF_WORD(port_num, SJA1105_STATIC_CONF_MAC_CONF_EGRESS_OFFSET);

    if (index >= size) status = SJA1105_PARAMETER_ERROR;
    if (status != SJA1105_OK) return status;

    *egress = (table[index] & SJA1105_STATIC_CONF_MAC_CONF_EGRESS_MASK) != 0;

    return status;
}


sja1105_status_t SJA1105_MACConfTableSetEgress(uint32_t *table, uint32_t size, uint8_t port_num, bool egress) {
    /* TODO: ADD NEW TABLE LOGIC*/
    sja1105_status_t status = SJA1105_OK;
    uint8_t          index  = SJA1105_STATIC_CONF_MAC_CONF_WORD(port_num, SJA1105_STATIC_CONF_MAC_CONF_EGRESS_OFFSET);

    if (index >= size) status = SJA1105_PARAMETER_ERROR;
    if (status != SJA1105_OK) return status;

    if (egress) {
        table[index] |= SJA1105_STATIC_CONF_MAC_CONF_EGRESS_MASK;
    } else {
        table[index] &= ~SJA1105_STATIC_CONF_MAC_CONF_EGRESS_MASK;
    }

    return status;
}


sja1105_status_t SJA1105_MACConfTableGetDynLearn(const uint32_t *table, uint32_t size, uint8_t port_num, bool *dyn_learn) {
    /* TODO: ADD NEW TABLE LOGIC*/
    sja1105_status_t status = SJA1105_OK;
    uint8_t          index  = SJA1105_STATIC_CONF_MAC_CONF_WORD(port_num, SJA1105_STATIC_CONF_MAC_CONF_DYN_LEARN_OFFSET);

    if (index >= size) status = SJA1105_PARAMETER_ERROR;
    if (status != SJA1105_OK) return status;

    *dyn_learn = (table[index] & SJA1105_STATIC_CONF_MAC_CONF_DYN_LEARN_MASK) != 0;

    return status;
}


sja1105_status_t SJA1105_MACConfTableSetDynLearn(uint32_t *table, uint32_t size, uint8_t port_num, bool dyn_learn) {
    /* TODO: ADD NEW TABLE LOGIC*/
    sja1105_status_t status = SJA1105_OK;
    uint8_t          index  = SJA1105_STATIC_CONF_MAC_CONF_WORD(port_num, SJA1105_STATIC_CONF_MAC_CONF_DYN_LEARN_OFFSET);

    if (index >= size) status = SJA1105_PARAMETER_ERROR;
    if (status != SJA1105_OK) return status;

    if (dyn_learn) {
        table[index] |= SJA1105_STATIC_CONF_MAC_CONF_DYN_LEARN_MASK;
    } else {
        table[index] &= ~SJA1105_STATIC_CONF_MAC_CONF_DYN_LEARN_MASK;
    }

    return status;
}


sja1105_status_t SJA1105_MACConfTableGetSpeed(const uint32_t *table, uint32_t size, uint8_t port_num, sja1105_speed_t *speed) {
    /* TODO: ADD NEW TABLE LOGIC*/
    sja1105_status_t status = SJA1105_OK;
    uint8_t          index  = SJA1105_STATIC_CONF_MAC_CONF_WORD(port_num, SJA1105_STATIC_CONF_MAC_CONF_SPEED_OFFSET);

    if (index >= size) status = SJA1105_PARAMETER_ERROR;
    if (status != SJA1105_OK) return status;

    *speed = (table[index] & SJA1105_STATIC_CONF_MAC_CONF_SPEED_MASK) >> SJA1105_STATIC_CONF_MAC_CONF_SPEED_SHIFT;

    return status;
}


sja1105_status_t SJA1105_MACConfTableSetSpeed(uint32_t *table, uint32_t size, uint8_t port_num, sja1105_speed_t speed) {
    /* TODO: ADD NEW TABLE LOGIC*/
    sja1105_status_t status = SJA1105_OK;
    uint16_t         index  = SJA1105_STATIC_CONF_MAC_CONF_WORD(port_num, SJA1105_STATIC_CONF_MAC_CONF_SPEED_OFFSET);

    if (index >= size) status = SJA1105_PARAMETER_ERROR;
    if (status != SJA1105_OK) return status;

    /* Clear and set the speed */
    table[index] &= ~SJA1105_STATIC_CONF_MAC_CONF_SPEED_MASK;
    table[index] |= ((uint32_t) speed << SJA1105_STATIC_CONF_MAC_CONF_SPEED_SHIFT) & SJA1105_STATIC_CONF_MAC_CONF_SPEED_MASK;

    return status;
}


sja1105_status_t SJA1105_MACConfTableWrite(sja1105_handle_t *dev, uint8_t port_num) {
    /* TODO: ADD NEW TABLE LOGIC*/
    sja1105_status_t status = SJA1105_OK;
    uint32_t         reg_data;
    uint8_t          index = SJA1105_STATIC_CONF_MAC_CONF_WORD(port_num, 0);

    /* Wait for VALID to be 0.
     *
     * Note: The VALID bit should be write only, but the documentation mentions "On read [of ERRORS] it has meaning at times when VALID is found reset".
     *       How can VALID be found reset if it cannot be read? Write only bits return 0 on read so there is no harm in polling until it is 0.
     */
    status = SJA1105_PollFlag(dev, SJA1105_DYN_CONF_MAC_CONF_REG_0, SJA1105_DYN_CONF_MAC_CONF_VALID, false);
    if (status != SJA1105_OK) return status;

    /* Parameter and bounds checking */
    assert(SJA1105_STATIC_CONF_MAC_CONF_ENTRY_SIZE == (SJA1105_DYN_CONF_MAC_CONF_REG_8 - SJA1105_DYN_CONF_MAC_CONF_REG_1 + 1));
    if ((index + SJA1105_STATIC_CONF_MAC_CONF_ENTRY_SIZE) > *dev->tables.mac_configuration.size) status = SJA1105_PARAMETER_ERROR;
    if (status != SJA1105_OK) return status;

    /* Write the table */
    status = SJA1105_WriteRegister(dev, SJA1105_DYN_CONF_MAC_CONF_REG_1, dev->tables.mac_configuration.data + index, SJA1105_STATIC_CONF_MAC_CONF_ENTRY_SIZE);
    if (status != SJA1105_OK) return status;

    /* Apply the table */
    reg_data  = 0;
    reg_data |= SJA1105_DYN_CONF_MAC_CONF_VALID;
    reg_data |= SJA1105_DYN_CONF_MAC_CONF_RDWRSET; /* Operation is a write */
    reg_data |= ((uint32_t) port_num << SJA1105_DYN_CONF_MAC_CONF_PORTID_SHIFT) & SJA1105_DYN_CONF_MAC_CONF_PORTID_MASK;
    status    = SJA1105_WriteRegister(dev, SJA1105_DYN_CONF_MAC_CONF_REG_0, &reg_data, 1);
    if (status != SJA1105_OK) return status;

    /* Wait for VALID to be 0 then check ERRORS, TODO there is a wasted access here */
    status = SJA1105_PollFlag(dev, SJA1105_DYN_CONF_MAC_CONF_REG_0, SJA1105_DYN_CONF_MAC_CONF_VALID, false);
    if (status != SJA1105_OK) return status;
    status = SJA1105_ReadFlag(dev, SJA1105_DYN_CONF_MAC_CONF_REG_0, SJA1105_DYN_CONF_MAC_CONF_ERRORS, (bool *) &reg_data);
    if (status != SJA1105_OK) return status;

    /* If ERRORS is set then the configuration is invalid and was not applied */
    if (reg_data) status = SJA1105_DYNAMIC_RECONFIG_ERROR;
    if (status != SJA1105_OK) return status;

    return status;
}


sja1105_status_t SJA1105_GeneralParamsTableCheck(sja1105_handle_t *dev, const uint32_t *table, uint32_t size) {
    /* TODO: ADD NEW TABLE LOGIC*/
    sja1105_status_t status = SJA1105_OK;
    uint8_t          index;
    uint8_t          host_port;

    /* Check the size is correct */
    if (size != SJA1105_STATIC_CONF_GENERAL_PARAMS_SIZE) status = SJA1105_STATIC_CONF_ERROR;
    if (status != SJA1105_OK) return status;

    /* Check the host port is correct */
    index = SJA1105_STATIC_CONF_GENERAL_PARAMS_HOST_PORT_OFFSET;
    if (index >= size) status = SJA1105_PARAMETER_ERROR;
    if (status != SJA1105_OK) return status;
    host_port = (table[index] & SJA1105_STATIC_CONF_GENERAL_PARAMS_HOST_PORT_MASK) >> SJA1105_STATIC_CONF_GENERAL_PARAMS_HOST_PORT_SHIFT;
    if (host_port != dev->config->host_port) status = SJA1105_PARAMETER_ERROR;
    if (status != SJA1105_OK) return status;

    /* TODO: Check switch ID */

    return status;
}


/*
 * Extract 4x 6-byte MAC filters from the 32-bit gerneral params table (array) that are not aligned.
 *
 * Word #:           4    5    6    7    8    9    10   11
 * Table:        ... XXXX XXXX XXXX XXXX XXXX XXXX XXXX XXXX
 *                      |      /|     /|      ||     |
 * MAC_FLT0:            XXXXXX  |    | |     / |     /
 * MAC_FLT1:                    XXXXXX |    |  |    |
 * MAC_FLTRES0:                        XXXXXX  |    |
 * MAC_FLTRES1:                                XXXXXX
 */
sja1105_status_t SJA1105_GetMACFilters(sja1105_handle_t *dev, sja1105_mac_filters_t *mac_filters) {

    sja1105_status_t status = SJA1105_OK;

    /* Check the table is valid */
    if (!dev->tables.general_parameters.in_use) status = SJA1105_NOT_CONFIGURED_ERROR;
    if (status != SJA1105_OK) return status;

    /* Calculate the pointer to the first MAC address as a uint8_t pointer */
    uint32_t *table     = dev->tables.general_parameters.data;
    uint8_t  *start_ptr = ((uint8_t *) (table + SJA1105_MAC_FLT_START_OFFSET_W)) + SJA1105_MAC_FLT_START_OFFSET_B;

    /* Bounds checking */
    if ((start_ptr + (MAC_ADDR_SIZE * 4)) > (uint8_t *) (table + SJA1105_STATIC_CONF_GENERAL_PARAMS_SIZE)) status = SJA1105_PARAMETER_ERROR;
    if (status != SJA1105_OK) return status;

    /* Copy MAC addresses */
    memcpy(mac_filters->mac_flt0, start_ptr + (MAC_ADDR_SIZE * 0), MAC_ADDR_SIZE);
    memcpy(mac_filters->mac_flt1, start_ptr + (MAC_ADDR_SIZE * 1), MAC_ADDR_SIZE);
    memcpy(mac_filters->mac_fltres0, start_ptr + (MAC_ADDR_SIZE * 2), MAC_ADDR_SIZE);
    memcpy(mac_filters->mac_fltres1, start_ptr + (MAC_ADDR_SIZE * 3), MAC_ADDR_SIZE);

    return status;
}

sja1105_status_t SJA1105_xMIIModeTableCheck(sja1105_handle_t *dev, const uint32_t *table, uint32_t size) {
    /* TODO: ADD NEW TABLE LOGIC*/
    sja1105_status_t    status = SJA1105_OK;
    sja1105_mode_t      mode;
    sja1105_interface_t interface;

    /* Check the size is correct */
    if (size != 1) status = SJA1105_STATIC_CONF_ERROR;
    if (status != SJA1105_OK) return status;

    /* Check each port's mode and interface */
    for (uint_fast8_t port_num = 0; port_num < SJA1105_NUM_PORTS; port_num++) {

        /* Check the mode */
        mode = (table[0] & SJA1105_STATIC_CONF_XMII_MODE_PHY_MAC_MASK(port_num)) >> SJA1105_STATIC_CONF_XMII_MODE_PHY_MAC_SHIFT(port_num);
        if (mode != dev->config->ports[port_num].mode) {

            /* Special case: The switch is acting as a PHY, but the MAC it is connected to cannot supply REFCLK.
             *               In this case the switch port is actually configured as a MAC, but operates as a PHY.
             */
            if ((dev->config->ports[port_num].mode == SJA1105_MODE_PHY) &&
                (dev->config->ports[port_num].interface == SJA1105_INTERFACE_RMII) &&
                dev->config->ports[port_num].output_rmii_refclk) status = SJA1105_OK;

            /* Otherwise its an error */
            else
                status = SJA1105_STATIC_CONF_ERROR;
        }
        if (status != SJA1105_OK) return status;

        /* Check the interface */
        interface = (table[0] & SJA1105_STATIC_CONF_XMII_MODE_INTERFACE_MASK(port_num)) >> SJA1105_STATIC_CONF_XMII_MODE_INTERFACE_SHIFT(port_num);
        if (interface != dev->config->ports[port_num].interface) status = SJA1105_STATIC_CONF_ERROR;
        if (status != SJA1105_OK) return status;

        /* Check SGMII is configured correctly */
        if ((dev->config->ports[port_num].interface == SJA1105_INTERFACE_SGMII) && (mode != SJA1105_MODE_MAC)) {
            status = SJA1105_STATIC_CONF_ERROR;
            return status;
        }
    }

    return status;
}
