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

#define SJA1105_NUM_PORTS (5)


/* Typedefs */

typedef enum {
    SJA1105_OK                       = HAL_OK,
    SJA1105_ERROR                    = HAL_ERROR,
    SJA1105_BUSY                     = HAL_BUSY,
    SJA1105_TIMEOUT                  = HAL_TIMEOUT,
    SJA1105_PARAMETER_ERROR,
    SJA1105_ALREADY_CONFIGURED_ERROR,  /* Error when attempting to init an already initialised device or configure a port that has already been configured. */
    SJA1105_NOT_CONFIGURED_ERROR,
    SJA1105_SPI_ERROR,
    SJA1105_ID_ERROR,
    SJA1105_STATIC_CONF_ERROR,
    SJA1105_L2_BUSY_ERROR,
    SJA1105_CRC_ERROR,
    SJA1105_RAM_PARITY_ERROR,
} SJA1105_StatusTypeDef;

typedef enum {
    VARIANT_SJA1105  = 0x00,
    VARIANT_SJA1105T = 0x01,
    VARIANT_SJA1105P = 0x02,
    VARIANT_SJA1105Q = 0x03,
    VARIANT_SJA1105R = 0x04,
    VARIANT_SJA1105S = 0x05
} SJA1105_VariantTypeDef;

typedef enum {
    SJA1105_INTERFACE_MII  = HAL_ETH_MII_MODE,
    SJA1105_INTERFACE_RMII = HAL_ETH_RMII_MODE,
    SJA1105_INTERFACE_RGMII,
    SJA1105_INTERFACE_SGMII
} SJA1105_InterfaceTypeDef;

typedef enum {
    SJA1105_SPEED_DYNAMIC = 0x0,  /* Used for ports that need to negotiate a speed */
    SJA1105_SPEED_1G      = 0x1,
    SJA1105_SPEED_100M    = 0x2,
    SJA1105_SPEED_10M     = 0x3,
    SJA1105_SPEED_MAX     = 0x4   /* Dummmy value for argument checking */
} SJA1105_SpeedTypeDef;

typedef enum {
    SJA1105_IO_VOLTAGE_UNSPECIFIED = 0x0,
    SJA1105_IO_1V8                 = 0x1,
    SJA1105_IO_2V5                 = 0x2,
    SJA1105_IO_3V3                 = 0x3
} SJA1105_IOVoltageTypeDef;

typedef struct {
    uint8_t                  port_num;
    SJA1105_SpeedTypeDef     speed;
    SJA1105_SpeedTypeDef     dyanamic_speed;  /* When speed == SJA1105_SPEED_DYNAMIC, this value specifies the currently configured speed */
    SJA1105_InterfaceTypeDef interface;
    bool                     configured;
    SJA1105_IOVoltageTypeDef voltage;
} SJA1105_PortTypeDef;


typedef void                  (*SJA1105_CallbackDelayMSTypeDef)   (uint32_t ms);
typedef void                  (*SJA1105_CallbackDelayNSTypeDef)   (uint32_t ns);
typedef SJA1105_StatusTypeDef (*SJA1105_CallbackTakeMutexTypeDef) (uint32_t timeout);
typedef SJA1105_StatusTypeDef (*SJA1105_CallbackGiveMutexTypeDef) (void);

typedef struct {
    SJA1105_CallbackDelayMSTypeDef   callback_delay_ms;  /* Non-blocking delay in ms */
    SJA1105_CallbackDelayNSTypeDef   callback_delay_ns;  /* Blocking delay in ns */
    SJA1105_CallbackTakeMutexTypeDef callback_take_mutex;
    SJA1105_CallbackGiveMutexTypeDef callback_give_mutex;
} SJA1105_CallbacksTypeDef;

typedef struct {
    SJA1105_VariantTypeDef  variant;
    SPI_HandleTypeDef      *spi_handle;
    GPIO_TypeDef           *cs_port;
    uint16_t                cs_pin;
    GPIO_TypeDef           *rst_port;
    uint16_t                rst_pin;
    uint32_t                timeout;  /* Timeout in ms for doing anything with a timeout (read, write, take mutex etc) */
} SJA1105_ConfigTypeDef;

typedef struct {
    SJA1105_ConfigTypeDef          *config;
    SJA1105_PortTypeDef            *ports;
    const SJA1105_CallbacksTypeDef *callbacks;
    bool                            static_conf_loaded;
    uint32_t                        static_conf_crc32;
    bool                            initialised;
} SJA1105_HandleTypeDef;


/* Functions */

/* Initialisation */
SJA1105_StatusTypeDef SJA1105_ConfigurePort(SJA1105_PortTypeDef *ports, uint8_t port_num, SJA1105_InterfaceTypeDef interface, SJA1105_SpeedTypeDef speed, SJA1105_IOVoltageTypeDef voltage);
SJA1105_StatusTypeDef SJA1105_Init(SJA1105_HandleTypeDef *dev, const SJA1105_ConfigTypeDef *config, SJA1105_PortTypeDef *ports, const SJA1105_CallbacksTypeDef *callbacks, const uint32_t *static_conf, uint32_t static_conf_size);

/* User Functions */
SJA1105_StatusTypeDef SJA1105_UpdatePortSpeed(SJA1105_HandleTypeDef *dev, uint8_t port_num, SJA1105_SpeedTypeDef speed);
SJA1105_StatusTypeDef SJA1105_ReadTemperatureX10(SJA1105_HandleTypeDef *dev, int16_t *temp);

SJA1105_StatusTypeDef SJA1105_CheckStatus(SJA1105_HandleTypeDef *dev);


#ifdef __cplusplus
}
#endif

#endif /* SJA1105_INC_SJA1105_H_ */
