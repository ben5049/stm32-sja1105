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


#define CONSTRAIN(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))


/* Control frame */
#define SJA1105_SPI_WRITE_FRAME         (1 << 31)    /* 1 in the most significant bit signifies a write */
#define SJA1105_SPI_READ_FRAME          (0)          /* 0 in the most significant bit signifies a write */
#define SJA1105_SPI_MAX_TX_PAYLOAD_SIZE (UINT16_MAX) /* Max TX payload size is "virtually unlimited" */
#define SJA1105_SPI_MAX_RX_PAYLOAD_SIZE (64)         /* Max RX payload size is 64 bytes (6-bit size field) */
#define SJA1105_SPI_ADDR_MASK           (0x001fffff) /* 21-bit address */
#define SJA1105_SPI_ADDR_POSITION       (4)          /* occupies bits[24:4] */
#define SJA1105_SPI_SIZE_MASK           (0x0000003f) /* 6-bit size */
#define SJA1105_SPI_SIZE_POSITION       (25)         /* occupies bits[30:25] */

/* Timings */
#define SJA1105_T_RST            (5000)   /* Reset pin pulse width 5000ns (5us) */
#define SJA1105_T_RST_STARTUP_HW (329000) /* 329000ns (329us) */
#define SJA1105_T_RST_STARTUP_SW (2000)   /* 2000ns (2us) */
#define SJA1105_T_SPI_WR         (130)    /* ns */
#define SJA1105_T_SPI_CTRL_DATA  (64)     /* Time between writing the command frame and reading data in ns */
#define SJA1105_T_SPI_LEAD       (40)     /* ns */
#define SJA1105_T_SPI_LAG        (40)     /* ns */


sja1105_status_t SJA1105_ReadRegister(sja1105_handle_t *dev, uint32_t addr, uint32_t *data, uint32_t size);
sja1105_status_t SJA1105_ReadRegisterWithCheck(sja1105_handle_t *dev, uint32_t addr, uint32_t *data, uint32_t size);
sja1105_status_t SJA1105_WriteRegister(sja1105_handle_t *dev, uint32_t addr, const uint32_t *data, uint32_t size);

sja1105_status_t SJA1105_ReadFlag(sja1105_handle_t *dev, uint32_t addr, uint32_t mask, bool *result);
sja1105_status_t SJA1105_PollFlag(sja1105_handle_t *dev, uint32_t addr, uint32_t mask, bool polarity);

sja1105_status_t SJA1105_WriteTable(sja1105_handle_t *dev, uint32_t addr, sja1105_table_t *table, bool safe);

sja1105_status_t SJA1105_L2LUTInvalidateRange(sja1105_handle_t *dev, uint16_t low_i, uint16_t high_i);

void             SJA1105_FullReset(sja1105_handle_t *dev);
sja1105_status_t SJA1105_CfgReset(sja1105_handle_t *dev);


#ifdef __cplusplus
}
#endif

#endif /* SJA1105_INC_SJA1105_IO_H_ */
