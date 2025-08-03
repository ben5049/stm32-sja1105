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


#define CONSTRAIN(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))


#define SJA1105_INIT_CHECK   do {if (!dev->initialised) status = SJA1105_NOT_CONFIGURED_ERROR; if (status != SJA1105_OK) return status;} while (0)
#define SJA1105_LOCK         do {status = dev->callbacks->callback_take_mutex(dev, dev->config->timeout); if (status != SJA1105_OK) return status;} while (0)
#define SJA1105_UNLOCK       dev->callbacks->callback_give_mutex(dev)
#define SJA1105_DELAY_NS(ns) dev->callbacks->callback_delay_ms(dev, (ns))
#define SJA1105_DELAY_MS(ms) dev->callbacks->callback_delay_ns(dev, (ms))


/* Control frame */
#define SJA1105_SPI_WRITE_FRAME         (1 << 31)     /* 1 in the most significant bit signifies a write */
#define SJA1105_SPI_READ_FRAME          (0)           /* 0 in the most significant bit signifies a write */
#define SJA1105_SPI_MAX_TX_PAYLOAD_SIZE (UINT16_MAX)  /* Max TX payload size is "virtually unlimited" */
#define SJA1105_SPI_MAX_RX_PAYLOAD_SIZE (64)          /* Max RX payload size is 64 bytes (6-bit size field) */
#define SJA1105_SPI_ADDR_MASK           (0x001fffff)  /* 21-bit address */
#define SJA1105_SPI_ADDR_POSITION       (4)           /* occupies bits[24:4] */ 
#define SJA1105_SPI_SIZE_MASK           (0x0000003f)  /* 6-bit size */
#define SJA1105_SPI_SIZE_POSITION       (25)          /* occupies bits[30:25] */

/* Timings */
#define SJA1105_T_RST                   (5000)        /* 5000ns (5us) */
#define SJA1105_T_SPI_WR                (130)         /* ns */
#define SJA1105_T_SPI_CTRL_DATA         (64)          /* Time between writing the command frame and reading data in ns */
#define SJA1105_T_SPI_LEAD              (40)          /* ns */
#define SJA1105_T_SPI_LAG               (40)          /* ns */


SJA1105_StatusTypeDef SJA1105_ReadRegister(SJA1105_HandleTypeDef *dev, uint32_t addr, uint32_t *data, uint32_t size);
SJA1105_StatusTypeDef SJA1105_ReadRegisterWithCheck(SJA1105_HandleTypeDef *dev, uint32_t addr, uint32_t *data, uint32_t size);
SJA1105_StatusTypeDef SJA1105_WriteRegister(SJA1105_HandleTypeDef *dev, uint32_t addr, const uint32_t *data, uint32_t size);

SJA1105_StatusTypeDef SJA1105_ReadFlag(SJA1105_HandleTypeDef *dev, uint32_t addr, uint32_t mask, bool *result);
SJA1105_StatusTypeDef SJA1105_PollFlag(SJA1105_HandleTypeDef *dev, uint32_t addr, uint32_t mask, bool polarity);

void SJA1105_Reset(SJA1105_HandleTypeDef *dev);

#ifdef __cplusplus
}
#endif

#endif /* SJA1105_INC_SJA1105_IO_H_ */
