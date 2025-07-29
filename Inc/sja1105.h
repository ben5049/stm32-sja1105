/*
 * sja1105.h
 *
 *  Created on: Jul 27, 2025
 *      Author: bens1
 *
 */

#ifndef SJA1105_INC_SJA1105_H_
#define SJA1105_INC_SJA1105_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes*/
#include "stm32h5xx_hal.h" /* Needed for SPI */
#include "stm32h573xx.h" /* Needed for GPIO_TypeDef */
#include "stdint.h"
#include "stdbool.h"


/* Defines */
#define SJA1105_SPI_WRITE_FRAME      (1 << 31)     /* 1 in the most significant bit signifies a write */
#define SJA1105_SPI_READ_FRAME       (0)           /* 0 in the most significant bit signifies a write */
#define SJA1105_SPI_MAX_PAYLOAD_SIZE (64)          /* 2080 bits / 32 = 65 frames (minus 1 for the control frame) */
#define SJA1105_SPI_ADDR_MASK        (0x001fffff)  /* 21-bit address */
#define SJA1105_SPI_ADDR_POSITION    (4)           /* occupies bits[24:4] */ 
#define SJA1105_SPI_SIZE_MASK        (0x0000003f)  /* 6-bit size */
#define SJA1105_SPI_SIZE_POSITION    (25)          /* occupies bits[30:25] */

#define SJA1105_STATIC_CONF_ADDR 0x20000

/* Typedefs */

typedef enum {
  SJA1105_OK              = HAL_OK,
  SJA1105_ERROR           = HAL_ERROR,
  SJA1105_BUSY            = HAL_BUSY,
  SJA1105_TIMEOUT         = HAL_TIMEOUT,
  SJA1105_PARAMETER_ERROR
} SJA1105_StatusTypeDef;

typedef enum {
  VARIANT_SJA1105  = 0x00,
  VARIANT_SJA1105T = 0x01,
  VARIANT_SJA1105P = 0x02,
  VARIANT_SJA1105Q = 0x03,
  VARIANT_SJA1105R = 0x04,
  VARIANT_SJA1105S = 0x05
} SJA1105_VariantTypeDef;

typedef void                  (*SJA1105_CallbackDelayMSTypeDef)     (uint32_t ms);
typedef SJA1105_StatusTypeDef (*SJA1105_CallbackTakeMutexTypeDef)   (uint32_t timeout);
typedef SJA1105_StatusTypeDef (*SJA1105_CallbackGiveMutexTypeDef)   (void);

typedef struct {
    SJA1105_CallbackDelayMSTypeDef   callback_delay_ms;
    SJA1105_CallbackTakeMutexTypeDef callback_take_mutex;
    SJA1105_CallbackGiveMutexTypeDef callback_give_mutex;
} SJA1105_CallbacksTypeDef;

typedef struct {
    SJA1105_VariantTypeDef          variant;
    const SJA1105_CallbacksTypeDef *callbacks;
    SPI_HandleTypeDef              *spi_handle;  /* SPI Handle */
    GPIO_TypeDef                   *cs_port;
    uint16_t                        cs_pin;
    GPIO_TypeDef                   *rst_port;
    uint16_t                        rst_pin;
    uint32_t                        timeout;  /* Timeout in ms for doing anything with a timeout (read, write, take mutex etc) */
    bool                            initialised;
} SJA1105_HandleTypeDef;


/* Functions */

/* Initialisation */
SJA1105_StatusTypeDef SJA1105_Init(
    SJA1105_HandleTypeDef *dev,
    const SJA1105_VariantTypeDef variant,
    const SJA1105_CallbacksTypeDef* callbacks,
    SPI_HandleTypeDef *spi_handle,
    GPIO_TypeDef *cs_port,
    uint16_t cs_pin,
    GPIO_TypeDef *rst_port,
    uint16_t rst_pin,
    uint32_t timeout,
    const uint32_t *static_conf,
    const uint32_t static_conf_size
);

/* User Functions */

/* Low-Level Functions */
/* Note that the nanosecond delays specified in the datasheet have not been explicitely added. This is because the target MCU ran slowly enough that these were guaranteed to be met. */
SJA1105_StatusTypeDef SJA1105_ReadRegister(SJA1105_HandleTypeDef *dev, uint32_t addr, uint32_t *data, uint32_t size);
SJA1105_StatusTypeDef SJA1105_WriteRegister(SJA1105_HandleTypeDef *dev, uint32_t addr, const uint32_t *data, uint32_t size);
void SJA1105_Reset(SJA1105_HandleTypeDef *dev);

#ifdef __cplusplus
}
#endif

#endif /* SJA1105_INC_SJA1105_H_ */
