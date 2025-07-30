/*
 * sja1105.c
 *
 *  Created on: Jul 27, 2025
 *      Author: bens1
 */

#include "sja1105.h"
#include "sja1105_conf.h"


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
