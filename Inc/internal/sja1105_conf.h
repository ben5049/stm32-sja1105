/*
 * sja1105_conf.h
 *
 *  Created on: Jul 30, 2025
 *      Author: bens1
 */

#ifndef SJA1105_INC_SJA1105_CONF_H_
#define SJA1105_INC_SJA1105_CONF_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "sja1105.h"


sja1105_status_t SJA1105_CheckPartID(sja1105_handle_t *dev);

sja1105_status_t SJA1105_ConfigureACU(sja1105_handle_t *dev);
sja1105_status_t SJA1105_ConfigureACUPort(sja1105_handle_t *dev, uint8_t port_num);

sja1105_status_t SJA1105_ConfigureCGU(sja1105_handle_t *dev);
sja1105_status_t SJA1105_ConfigureCGUPort(sja1105_handle_t *dev, uint8_t port_num);

sja1105_status_t SJA1105_WriteStaticConfig(sja1105_handle_t *dev, const uint32_t *static_conf, uint32_t static_conf_size);


#ifdef __cplusplus
}
#endif

#endif /* SJA1105_INC_SJA1105_CONF_H_ */
