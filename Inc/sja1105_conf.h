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


SJA1105_StatusTypeDef SJA1105_CheckPartID(SJA1105_HandleTypeDef *dev);
SJA1105_StatusTypeDef SJA1105_ConfigureACU(SJA1105_HandleTypeDef *dev);
SJA1105_StatusTypeDef SJA1105_ConfigureACUPort(SJA1105_HandleTypeDef *dev, uint8_t port_num);
SJA1105_StatusTypeDef SJA1105_ConfigureCGU(SJA1105_HandleTypeDef *dev);
SJA1105_StatusTypeDef SJA1105_ConfigureCGUPort(SJA1105_HandleTypeDef *dev, uint8_t port_num);
SJA1105_StatusTypeDef SJA1105_WriteStaticConfig(SJA1105_HandleTypeDef *dev, const uint32_t *static_conf, uint32_t static_conf_size);
SJA1105_StatusTypeDef SJA1105_CheckMACConfTable(SJA1105_HandleTypeDef *dev, const uint32_t *table, uint32_t size);


#ifdef __cplusplus
}
#endif

#endif /* SJA1105_INC_SJA1105_CONF_H_ */
