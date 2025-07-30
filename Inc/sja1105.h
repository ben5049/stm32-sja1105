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

/* Control frame */
#define SJA1105_SPI_WRITE_FRAME      (1 << 31)     /* 1 in the most significant bit signifies a write */
#define SJA1105_SPI_READ_FRAME       (0)           /* 0 in the most significant bit signifies a write */
#define SJA1105_SPI_MAX_PAYLOAD_SIZE (64)          /* 2080 bits / 32 = 65 frames (minus 1 for the control frame) */
#define SJA1105_SPI_ADDR_MASK        (0x001fffff)  /* 21-bit address */
#define SJA1105_SPI_ADDR_POSITION    (4)           /* occupies bits[24:4] */ 
#define SJA1105_SPI_SIZE_MASK        (0x0000003f)  /* 6-bit size */
#define SJA1105_SPI_SIZE_POSITION    (25)          /* occupies bits[30:25] */

/* Timings */
#define SJA1105_T_RST                (5000)        /* 5000ns (5us) */
#define SJA1105_T_SPI_WR             (130)         /* ns */
#define SJA1105_T_SPI_CTRL_DATA      (64)          /* Time between writing the command frame and reading data in ns */
#define SJA1105_T_SPI_LEAD           (40)          /* ns */
#define SJA1105_T_SPI_LAG            (40)          /* ns */





/* Typedefs */

typedef enum {
	SJA1105_OK                       = HAL_OK,
	SJA1105_ERROR                    = HAL_ERROR,
	SJA1105_BUSY                     = HAL_BUSY,
	SJA1105_TIMEOUT                  = HAL_TIMEOUT,
	SJA1105_PARAMETER_ERROR,
	SJA1105_ALREADY_CONFIGURED_ERROR,
	SJA1105_STATIC_CONF_ERROR,
	SJA1105_ID_ERROR,

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
	SJA1105_SPEED_UNKNOWN,  /* Used for ports that need to negotiate a speed */
	SJA1105_SPEED_10M,
	SJA1105_SPEED_100M,
	SJA1105_SPEED_1G,
} SJA1105_SpeedTypeDef;

typedef enum {
    SJA1105_IO_VOLTAGE_UNSPECIFIED,
    SJA1105_IO_1V8,
    SJA1105_IO_2V5,
    SJA1105_IO_3V3
} SJA1105_IOVoltageTypeDef;

typedef struct {
	uint8_t                  port_num;
	SJA1105_SpeedTypeDef     speed;
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
	SJA1105_VariantTypeDef          variant;
	const SJA1105_CallbacksTypeDef *callbacks;
	SPI_HandleTypeDef              *spi_handle;  /* SPI Handle */
	GPIO_TypeDef                   *cs_port;
	uint16_t                        cs_pin;
	GPIO_TypeDef                   *rst_port;
	uint16_t                        rst_pin;
	uint32_t                        timeout;  /* Timeout in ms for doing anything with a timeout (read, write, take mutex etc) */
	bool                            initialised;
	SJA1105_PortTypeDef            *ports;
} SJA1105_HandleTypeDef;


/* Functions */

/* Initialisation */
SJA1105_StatusTypeDef SJA1105_ConfigurePort(SJA1105_PortTypeDef *ports, uint8_t port_num, SJA1105_InterfaceTypeDef interface, SJA1105_SpeedTypeDef speed, SJA1105_IOVoltageTypeDef voltage);

SJA1105_StatusTypeDef SJA1105_Init(
		SJA1105_HandleTypeDef *dev,
		const SJA1105_VariantTypeDef variant,
		const SJA1105_CallbacksTypeDef *callbacks,
		SPI_HandleTypeDef *spi_handle,
		GPIO_TypeDef *cs_port,
		uint16_t cs_pin,
		GPIO_TypeDef *rst_port,
		uint16_t rst_pin,
		uint32_t timeout,
		const uint32_t *static_conf,
		uint32_t static_conf_size,
		SJA1105_PortTypeDef *ports
);

/* User Functions */
SJA1105_StatusTypeDef SJA1105_CheckPartNR(SJA1105_HandleTypeDef *dev);
SJA1105_StatusTypeDef SJA1105_ConfigureACU(SJA1105_HandleTypeDef *dev);
SJA1105_StatusTypeDef SJA1105_ConfigureACUPort(SJA1105_HandleTypeDef *dev, uint8_t port_num);
SJA1105_StatusTypeDef SJA1105_ConfigureCGU(SJA1105_HandleTypeDef *dev);
SJA1105_StatusTypeDef SJA1105_WriteStaticConfig(SJA1105_HandleTypeDef *dev, const uint32_t *static_conf, uint32_t static_conf_size);


void SJA1105_Reset(SJA1105_HandleTypeDef *dev);

#ifdef __cplusplus
}
#endif

#endif /* SJA1105_INC_SJA1105_H_ */
