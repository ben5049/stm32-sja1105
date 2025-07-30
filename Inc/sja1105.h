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


/* ---------------------------------------------------------------------------- */
/* Auxiliary Configurations Unit */
/* ---------------------------------------------------------------------------- */

#define SJA1105_ACU_REG_CFG_PAD_MIIX_BASE          (0x100800)
#define SJA1105_ACU_REG_CFG_PAD_MIIX_TX(port_num)  (SJA1105_ACU_REG_CFG_PAD_MIIX_BASE + 2 * port_num)
#define SJA1105_ACU_REG_CFG_PAD_MIIX_RX(port_num)  (SJA1105_ACU_REG_CFG_PAD_MIIX_BASE + 2 * port_num + 1)
#define SJA1105_ACU_REG_CFG_PAD_MIIX_ID(port_num)  (SJA1105_ACU_REG_CFG_PAD_MIIX_BASE + 16 + port_num)

#define SJA1105_ACU_REG_CFG_PAD_MISC               (0x100840)
#define SJA1105_ACU_REG_CFG_PAD_SPI                (0x100880)
#define SJA1105_ACU_REG_CFG_PAD_JTAG               (0x100881)

#define SJA1105_ACU_REG_PORT_STATUS_MIIX_BASE      (0x100900)
#define SJA1105_ACU_REG_PORT_STATUS_MIIX(port_num) (SJA1105_ACU_REG_PORT_STATUS_MIIX_BASE + port_num)

#define SJA1105_ACU_REG_TS_CONFIG                  (0x100a00)
#define SJA1105_ACU_REG_TS_STATUS                  (0x100a01)
#define SJA1105_ACU_REG_PROD_CFG                   (0x100bc0)
#define SJA1105_ACU_REG_PROD_ID                    (0x100bc3)
#define SJA1105_ACU_REG_ACCESS_DISABLE             (0x100bfd)

#define SJA1105_ACU_PAD_CFG_TX                     (0)
#define SJA1105_ACU_PAD_CFG_RX                     (1)
#define SJA1105_ACU_PAD_CFG_SIZE                   (2)

#define SJA1105_CLK_OS_LOW                         (0 << 3)
#define SJA1105_CLK_OS_MEDIUM                      (1 << 3)
#define SJA1105_CLK_OS_FAST                        (2 << 3)
#define SJA1105_CLK_OS_HIGH                        (3 << 3)
#define SJA1105_CTRL_OS_LOW                        (0 << 11)
#define SJA1105_CTRL_OS_MEDIUM                     (1 << 11)
#define SJA1105_CTRL_OS_FAST                       (2 << 11)
#define SJA1105_CTRL_OS_HIGH                       (3 << 11)
#define SJA1105_D10_OS_LOW                         (0 << 19)
#define SJA1105_D10_OS_MEDIUM                      (1 << 19)
#define SJA1105_D10_OS_FAST                        (2 << 19)
#define SJA1105_D10_OS_HIGH                        (3 << 19)
#define SJA1105_D32_OS_LOW                         (0 << 27)
#define SJA1105_D32_OS_MEDIUM                      (1 << 27)
#define SJA1105_D32_OS_FAST                        (2 << 27)
#define SJA1105_D32_OS_HIGH                        (3 << 27)

#define SJA1105_OS_LOW                             (SJA1105_CLK_OS_LOW    | SJA1105_CTRL_OS_LOW    | SJA1105_D10_OS_LOW    | SJA1105_D32_OS_LOW   )
#define SJA1105_OS_MEDIUM                          (SJA1105_CLK_OS_MEDIUM | SJA1105_CTRL_OS_MEDIUM | SJA1105_D10_OS_MEDIUM | SJA1105_D32_OS_MEDIUM)
#define SJA1105_OS_FAST                            (SJA1105_CLK_OS_FAST   | SJA1105_CTRL_OS_FAST   | SJA1105_D10_OS_FAST   | SJA1105_D32_OS_FAST  )
#define SJA1105_OS_HIGH                            (SJA1105_CLK_OS_HIGH   | SJA1105_CTRL_OS_HIGH   | SJA1105_D10_OS_HIGH   | SJA1105_D32_OS_HIGH  )

#define SJA1105_CLK_IPUD_PU                        (0 << 0)
#define SJA1105_CLK_IPUD_R                         (1 << 0)
#define SJA1105_CLK_IPUD_PI                        (2 << 0)
#define SJA1105_CLK_IPUD_PD                        (3 << 0)
#define SJA1105_CTRL_IPUD_PU                       (0 << 8)
#define SJA1105_CTRL_IPUD_R                        (1 << 8)
#define SJA1105_CTRL_IPUD_PI                       (2 << 8)
#define SJA1105_CTRL_IPUD_PD                       (3 << 8)
#define SJA1105_D10_IPUD_PU                        (0 << 16)
#define SJA1105_D10_IPUD_R                         (1 << 16)
#define SJA1105_D10_IPUD_PI                        (2 << 16)
#define SJA1105_D10_IPUD_PD                        (3 << 16)
#define SJA1105_D32_IPUD_PU                        (0 << 24)
#define SJA1105_D32_IPUD_R                         (1 << 24)
#define SJA1105_D32_IPUD_PI                        (2 << 24)
#define SJA1105_D32_IPUD_PD                        (3 << 24)

#define SJA1105_IPUD_PU                            (SJA1105_CLK_IPUD_PU | SJA1105_CTRL_IPUD_PU | SJA1105_D10_IPUD_PU | SJA1105_D32_IPUD_PU)
#define SJA1105_IPUD_R                             (SJA1105_CLK_IPUD_R  | SJA1105_CTRL_IPUD_R  | SJA1105_D10_IPUD_R  | SJA1105_D32_IPUD_R )
#define SJA1105_IPUD_PI                            (SJA1105_CLK_IPUD_PI | SJA1105_CTRL_IPUD_PI | SJA1105_D10_IPUD_PI | SJA1105_D32_IPUD_PI)
#define SJA1105_IPUD_PD                            (SJA1105_CLK_IPUD_PD | SJA1105_CTRL_IPUD_PD | SJA1105_D10_IPUD_PD | SJA1105_D32_IPUD_PD)

#define SJA1105_CLK_IH_SCHMITT                     (0 << 2)
#define SJA1105_CLK_IH_NON_SCHMITT                 (1 << 2)
#define SJA1105_CTRL_IH_SCHMITT                    (0 << 10)
#define SJA1105_CTRL_IH_NON_SCHMITT                (1 << 10)
#define SJA1105_D10_IH_SCHMITT                     (0 << 18)
#define SJA1105_D10_IH_NON_SCHMITT                 (1 << 18)
#define SJA1105_D32_IH_SCHMITT                     (0 << 26)
#define SJA1105_D32_IH_NON_SCHMITT                 (1 << 26)

#define SJA1105_IH_SCHMITT                         (SJA1105_CLK_IH_SCHMITT     | SJA1105_CTRL_IH_SCHMITT     | SJA1105_D10_IH_SCHMITT     | SJA1105_D32_IH_SCHMITT)
#define SJA1105_IH_NON_SCHMITT                     (SJA1105_CLK_IH_NON_SCHMITT | SJA1105_CTRL_IH_NON_SCHMITT | SJA1105_D10_IH_NON_SCHMITT | SJA1105_D32_IH_NON_SCHMITT)

#define SJA1105_PART_NR_OFFSET                     (4)
#define SJA1105_PART_NR_MASK                       (0xffff << SJA1105_PART_NR_OFFSET)
#define PART_NR_SJA1105_T                          (0x9a82)
#define PART_NR_SJA1105P                           (0x9a84)
#define PART_NR_SJA1105Q                           (0x9a85)
#define PART_NR_SJA1105R                           (0x9a86)
#define PART_NR_SJA1105S                           (0x9a87)

/* ---------------------------------------------------------------------------- */
/* Clock Generation Unit */
/* ---------------------------------------------------------------------------- */





#define SJA1105_STATIC_CONF_ADDR (0x20000)
#define SJA1105_T_SWITCH_CORE_ID (0x9f00030e)
#define SJA1105PR_SWITCH_CORE_ID (0xaf00030e)
#define SJA1105QS_SWITCH_CORE_ID (0xae00030e)



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

/* Low-Level Functions */
SJA1105_StatusTypeDef SJA1105_ReadRegister(SJA1105_HandleTypeDef *dev, uint32_t addr, uint32_t *data, uint32_t size);
SJA1105_StatusTypeDef SJA1105_WriteRegister(SJA1105_HandleTypeDef *dev, uint32_t addr, const uint32_t *data, uint32_t size);
void SJA1105_Reset(SJA1105_HandleTypeDef *dev);

#ifdef __cplusplus
}
#endif

#endif /* SJA1105_INC_SJA1105_H_ */
