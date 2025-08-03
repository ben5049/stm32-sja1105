/*
 * sja1105_tables.c
 *
 *  Created on: Aug 2, 2025
 *      Author: bens1
 */

#include "memory.h"
#include "stdlib.h"

#include "sja1105_tables.h"
#include "sja1105_regs.h"


void SJA1105_FreeAllTableMemory(SJA1105_HandleTypeDef *dev){

    SJA1105_TablesTypeDef tables = *(dev->tables);

    if (tables.mac_config_size > 0){
        free(tables.mac_config);
        tables.mac_config_size = 0;
    }
    if (tables.general_params_size > 0){
        free(tables.general_params);
        tables.general_params_size = 0;
    }
}


SJA1105_StatusTypeDef SJA1105_MACConfTableCheck(SJA1105_HandleTypeDef *dev, const uint32_t *table, uint32_t size){

    SJA1105_StatusTypeDef status = SJA1105_OK;
    SJA1105_SpeedTypeDef  speed;

    /* Check the size is correct */
    if (size != (SJA1105_NUM_PORTS * SJA1105_STATIC_CONF_MAC_CONF_ENTRY_SIZE)) status = SJA1105_STATIC_CONF_ERROR;
    if (status != SJA1105_OK) return status;

    /* Check each port's speed */
    for (uint8_t port_num = 0; port_num < SJA1105_NUM_PORTS; port_num++){
        SJA1105_MACConfTableGetSpeed(table, size, port_num, &speed);
        if (speed != dev->ports[port_num].speed) status = SJA1105_STATIC_CONF_ERROR;
        if (status != SJA1105_OK) return status;
    }

    return status;
}

SJA1105_StatusTypeDef SJA1105_MACConfTableGetSpeed(const uint32_t *table, uint32_t size, uint8_t port_num, SJA1105_SpeedTypeDef *speed){

    SJA1105_StatusTypeDef status = SJA1105_OK;
    uint8_t index = SJA1105_STATIC_CONF_MAC_CONF_WORD(port_num, SJA1105_STATIC_CONF_MAC_CONF_SPEED_OFFSET);

    if (index >= size) status = SJA1105_PARAMETER_ERROR;
    if (status != SJA1105_OK) return status;

    *speed = (table[index] & SJA1105_STATIC_CONF_MAC_CONF_SPEED_MASK) >> SJA1105_STATIC_CONF_MAC_CONF_SPEED_SHIFT;

    return status;
}

SJA1105_StatusTypeDef SJA1105_MACConfTableSetSpeed(uint32_t *table, uint32_t size, uint8_t port_num, SJA1105_SpeedTypeDef speed){

    SJA1105_StatusTypeDef status = SJA1105_OK;
    uint16_t index = SJA1105_STATIC_CONF_MAC_CONF_WORD(port_num, SJA1105_STATIC_CONF_MAC_CONF_SPEED_OFFSET);

    if (index >= size) status = SJA1105_PARAMETER_ERROR;
    if (status != SJA1105_OK) return status;

    /* Clear and set the speed */
    table[index] &= ~SJA1105_STATIC_CONF_MAC_CONF_SPEED_MASK;
    table[index] |= (speed << SJA1105_STATIC_CONF_MAC_CONF_SPEED_SHIFT) & SJA1105_STATIC_CONF_MAC_CONF_SPEED_MASK;

    return status;
}

SJA1105_StatusTypeDef SJA1105_MACConfTableWrite(SJA1105_HandleTypeDef *dev, uint8_t port_num){

    SJA1105_StatusTypeDef status = SJA1105_NOT_IMPLEMENTED_ERROR;

    return status;
}

SJA1105_StatusTypeDef SJA1105_GeneralParamsTableCheck(SJA1105_HandleTypeDef *dev, const uint32_t *table, uint32_t size){

    SJA1105_StatusTypeDef status = SJA1105_OK;
    uint8_t index;
    uint8_t host_port;

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
 * 
 * XXXX  XXXX  XXXX  XXXX XXXX XXXX XXXX XXXX
 *    |       / |      /|      ||     |
 *     XXXXXX   |     | |     / |     /
 *              XXXXXX  |    |  |    |
 *                      XXXXXX  |    |
 *                              XXXXXX
 */
SJA1105_StatusTypeDef SJA1105_GeneralParamsTableGetMACFilters(const uint32_t *table, uint32_t size, SJA1105_MACFiltersTypeDef *mac_filters){

    SJA1105_StatusTypeDef status = SJA1105_OK;

    /* Calculate the pointer to the first MAC address as a uint8_t pointer */
    uint8_t *start_ptr = ((uint8_t *) (&table[SJA1105_MAC_FLT_START_OFFSET_W])) + SJA1105_MAC_FLT_START_OFFSET_B;

    /* Bounds checking */
    if ((start_ptr + (MAC_ADDR_SIZE * 4)) > (uint8_t *) (table + size)) status = SJA1105_PARAMETER_ERROR;
    if (status != SJA1105_OK) return status;

    /* Copy MAC addresses */
    memcpy(mac_filters->mac_flt0,    start_ptr + (MAC_ADDR_SIZE * 0), MAC_ADDR_SIZE);
    memcpy(mac_filters->mac_flt1,    start_ptr + (MAC_ADDR_SIZE * 1), MAC_ADDR_SIZE);
    memcpy(mac_filters->mac_fltres0, start_ptr + (MAC_ADDR_SIZE * 2), MAC_ADDR_SIZE);
    memcpy(mac_filters->mac_fltres1, start_ptr + (MAC_ADDR_SIZE * 3), MAC_ADDR_SIZE);

    return status;
}

SJA1105_StatusTypeDef SJA1105_xMIIModeTableCheck(SJA1105_HandleTypeDef *dev, const uint32_t *table, uint32_t size){

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
