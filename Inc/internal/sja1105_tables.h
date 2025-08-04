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


void SJA1105_FreeAllTableMemory(sja1105_handle_t *dev);

sja1105_status_t SJA1105_MACConfTableCheck(sja1105_handle_t *dev, const uint32_t *table, uint32_t size);
sja1105_status_t SJA1105_MACConfTableGetSpeed(const uint32_t *table, uint32_t size, uint8_t port_num, sja1105_speed_t *speed);
sja1105_status_t SJA1105_MACConfTableSetSpeed(uint32_t *table, uint32_t size, uint8_t port_num, sja1105_speed_t speed);
sja1105_status_t SJA1105_MACConfTableWrite(sja1105_handle_t *dev, uint8_t port_num);
sja1105_status_t SJA1105_MACConfTableGetIngress(const uint32_t *table, uint32_t size, uint8_t port_num, bool *ingress);
sja1105_status_t SJA1105_MACConfTableSetIngress(uint32_t *table, uint32_t size, uint8_t port_num, bool ingress);
sja1105_status_t SJA1105_MACConfTableGetEgress(const uint32_t *table, uint32_t size, uint8_t port_num, bool *egress);
sja1105_status_t SJA1105_MACConfTableSetEgress(uint32_t *table, uint32_t size, uint8_t port_num, bool egress);
sja1105_status_t SJA1105_MACConfTableGetDynLearn(const uint32_t *table, uint32_t size, uint8_t port_num, bool *dyn_learn);
sja1105_status_t SJA1105_MACConfTableSetDynLearn(uint32_t *table, uint32_t size, uint8_t port_num, bool dyn_learn);

sja1105_status_t SJA1105_GeneralParamsTableCheck(sja1105_handle_t *dev, const uint32_t *table, uint32_t size);
sja1105_status_t SJA1105_GeneralParamsTableGetMACFilters(const uint32_t *table, uint32_t size, sja1105_mac_filters_t *mac_filters);

sja1105_status_t SJA1105_xMIIModeTableCheck(sja1105_handle_t *dev, const uint32_t *table, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif /* SJA1105_INC_SJA1105_TABLES_H_ */
