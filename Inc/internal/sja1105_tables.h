/*
 * sja1105_tables.h
 *
 *  Created on: Aug 2, 2025
 *      Author: bens1
 */

#ifndef SJA1105_INC_SJA1105_TABLES_H_
#define SJA1105_INC_SJA1105_TABLES_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "sja1105.h"


void SJA1105_FreeAllTableMemory(SJA1105_HandleTypeDef *dev);

SJA1105_StatusTypeDef SJA1105_MACConfTableCheck(SJA1105_HandleTypeDef *dev, const uint32_t *table, uint32_t size);
SJA1105_StatusTypeDef SJA1105_MACConfTableGetSpeed(const uint32_t *table, uint32_t size, uint8_t port_num, SJA1105_SpeedTypeDef *speed);
SJA1105_StatusTypeDef SJA1105_MACConfTableSetSpeed(uint32_t *table, uint32_t size, uint8_t port_num, SJA1105_SpeedTypeDef speed);
SJA1105_StatusTypeDef SJA1105_MACConfTableWrite(SJA1105_HandleTypeDef *dev, uint8_t port_num);
SJA1105_StatusTypeDef SJA1105_MACConfTableGetIngress(const uint32_t *table, uint32_t size, uint8_t port_num, bool *ingress);
SJA1105_StatusTypeDef SJA1105_MACConfTableSetIngress(uint32_t *table, uint32_t size, uint8_t port_num, bool ingress);
SJA1105_StatusTypeDef SJA1105_MACConfTableGetEgress(const uint32_t *table, uint32_t size, uint8_t port_num, bool *egress);
SJA1105_StatusTypeDef SJA1105_MACConfTableSetEgress(uint32_t *table, uint32_t size, uint8_t port_num, bool egress);
SJA1105_StatusTypeDef SJA1105_MACConfTableGetDynLearn(const uint32_t *table, uint32_t size, uint8_t port_num, bool *dyn_learn);
SJA1105_StatusTypeDef SJA1105_MACConfTableSetDynLearn(uint32_t *table, uint32_t size, uint8_t port_num, bool dyn_learn);

SJA1105_StatusTypeDef SJA1105_GeneralParamsTableCheck(SJA1105_HandleTypeDef *dev, const uint32_t *table, uint32_t size);
SJA1105_StatusTypeDef SJA1105_GeneralParamsTableGetMACFilters(const uint32_t *table, uint32_t size, SJA1105_MACFiltersTypeDef *mac_filters);

SJA1105_StatusTypeDef SJA1105_xMIIModeTableCheck(SJA1105_HandleTypeDef *dev, const uint32_t *table, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif /* SJA1105_INC_SJA1105_TABLES_H_ */
