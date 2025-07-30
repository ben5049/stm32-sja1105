/*
 * sja1105_io.h
 *
 *  Created on: Jul 30, 2025
 *      Author: bens1
 */

#ifndef SJA1105_INC_SJA1105_IO_H_
#define SJA1105_INC_SJA1105_IO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sja1105.h"

SJA1105_StatusTypeDef SJA1105_ReadRegister(SJA1105_HandleTypeDef *dev, uint32_t addr, uint32_t *data, uint32_t size);
SJA1105_StatusTypeDef SJA1105_WriteRegister(SJA1105_HandleTypeDef *dev, uint32_t addr, const uint32_t *data, uint32_t size);

void SJA1105_Reset(SJA1105_HandleTypeDef *dev);

#ifdef __cplusplus
}
#endif

#endif /* SJA1105_INC_SJA1105_IO_H_ */
