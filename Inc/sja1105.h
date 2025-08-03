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
#include "hal.h"
#include "stdint.h"
#include "stdbool.h"
#include "stdatomic.h"


/* Defines */

#define SJA1105_NUM_PORTS (5)
#define MAC_ADDR_SIZE     (6)


/* Typedefs */
typedef struct SJA1105_HandleTypeDef SJA1105_HandleTypeDef;

typedef enum {
    SJA1105_OK      = HAL_OK,
    SJA1105_ERROR   = HAL_ERROR,
    SJA1105_BUSY    = HAL_BUSY,
    SJA1105_TIMEOUT = HAL_TIMEOUT,
    SJA1105_PARAMETER_ERROR,
    SJA1105_ALREADY_CONFIGURED_ERROR,  /* Error when attempting to init an already initialised device or configure a port that has already been configured. */
    SJA1105_NOT_CONFIGURED_ERROR,
    SJA1105_SPI_ERROR,
    SJA1105_ID_ERROR,
    SJA1105_STATIC_CONF_ERROR,
    SJA1105_L2_BUSY_ERROR,
    SJA1105_CRC_ERROR,
    SJA1105_RAM_PARITY_ERROR,
    SJA1105_NOT_IMPLEMENTED_ERROR,
    SJA1105_MUTEX_ERROR,               /* Serious mutex error, will normally just return SJA1105_BUSY if it tries to take a mutex held by another thread */
    SJA1105_DYNAMIC_MEMORY_ERROR,
} SJA1105_StatusTypeDef;

typedef enum {
    VARIANT_SJA1105E               = 0x0,
    VARIANT_SJA1105T               = 0x1,
    VARIANT_SJA1105P               = 0x2,
    VARIANT_SJA1105Q               = 0x3,
    VARIANT_SJA1105R               = 0x4,
    VARIANT_SJA1105S               = 0x5,
    VARIANT_SJA1105_INVALID        = 0x6   /* Dummmy value for argument checking */
} SJA1105_VariantTypeDef;

typedef enum {
    SJA1105_INTERFACE_MII          = 0x0,
    SJA1105_INTERFACE_RMII         = 0x1,
    SJA1105_INTERFACE_RGMII        = 0x2,
    SJA1105_INTERFACE_SGMII        = 0x3,
    SJA1105_INTERFACE_INVALID      = 0x4   /* Dummmy value for argument checking */
} SJA1105_InterfaceTypeDef;

typedef enum{
    SJA1105_MODE_MAC               = 0x0,  /* The port acts like a MAC */
    SJA1105_MODE_PHY               = 0x1,  /* The port acts like a PHY and must connect to a true MAC */
    SJA1105_MODE_INVALID           = 0x2   /* Dummmy value for argument checking */
} SJA1105_ModeTypeDef;

typedef enum {
    SJA1105_SPEED_DYNAMIC          = 0x0,  /* Used for ports that need to negotiate a speed */
    SJA1105_SPEED_1G               = 0x1,
    SJA1105_SPEED_100M             = 0x2,
    SJA1105_SPEED_10M              = 0x3,
    SJA1105_SPEED_INVALID          = 0x4   /* Dummmy value for argument checking */
} SJA1105_SpeedTypeDef;

typedef enum {
    SJA1105_IO_VOLTAGE_UNSPECIFIED = 0x0,
    SJA1105_IO_1V8                 = 0x1,
    SJA1105_IO_2V5                 = 0x2,
    SJA1105_IO_3V3                 = 0x3,
    SJA1105_IO_INVALID_V           = 0x4   /* Dummmy value for argument checking */
} SJA1105_IOVoltageTypeDef;

typedef enum {
    SJA1105_PORT_STATE_FORWARDING = 0x0,
    SJA1105_PORT_STATE_DISCARDING = 0x1,
    SJA1105_PORT_STATE_LEARNING   = 0x2
} SJA1105_PortState_TypeDef;

typedef struct {
    uint8_t                   port_num;
    SJA1105_SpeedTypeDef      speed;
    SJA1105_InterfaceTypeDef  interface;
    SJA1105_ModeTypeDef       mode;
    bool                      output_rmii_refclk;  /* Only used when interface = SJA1105_INTERFACE_RMII and mode = SJA1105_MODE_PHY */
    SJA1105_IOVoltageTypeDef  voltage;
    bool                      configured;
    SJA1105_PortState_TypeDef state;
} SJA1105_PortTypeDef;

typedef struct {
    SJA1105_VariantTypeDef  variant;
    SPI_HandleTypeDef      *spi_handle;
    GPIO_TypeDef           *cs_port;
    uint16_t                cs_pin;
    GPIO_TypeDef           *rst_port;
    uint16_t                rst_pin;
    uint32_t                timeout;      /* Timeout in ms for doing anything with a timeout (read, write, take mutex etc) */
    uint8_t                 host_port;
    bool                    skew_clocks;  /* Make xMII clocks use different phases (where possible) to improve EMC performance */
    uint8_t                 switch_id;    /* Used to identify the switch that trapped a frame */
} SJA1105_ConfigTypeDef;

typedef struct {
    uint32_t *mac_config;
    uint32_t  mac_config_size;  /* Number of uint32_t in mac_config */
    uint32_t *general_params;
    uint32_t  general_params_size;
} SJA1105_TablesTypeDef;

typedef struct {
    uint8_t mac_fltres0[MAC_ADDR_SIZE];
    uint8_t mac_flt0[MAC_ADDR_SIZE];
    uint8_t mac_fltres1[MAC_ADDR_SIZE];
    uint8_t mac_flt1[MAC_ADDR_SIZE];
} SJA1105_MACFiltersTypeDef;

typedef void                  (*SJA1105_CallbackDelayMSTypeDef)   (SJA1105_HandleTypeDef *dev, uint32_t ms);
typedef void                  (*SJA1105_CallbackDelayNSTypeDef)   (SJA1105_HandleTypeDef *dev, uint32_t ns);
typedef SJA1105_StatusTypeDef (*SJA1105_CallbackTakeMutexTypeDef) (SJA1105_HandleTypeDef *dev, uint32_t timeout);
typedef SJA1105_StatusTypeDef (*SJA1105_CallbackGiveMutexTypeDef) (SJA1105_HandleTypeDef *dev);

typedef struct {
    SJA1105_CallbackDelayMSTypeDef   callback_delay_ms;  /* Non-blocking delay in ms */
    SJA1105_CallbackDelayNSTypeDef   callback_delay_ns;  /* Blocking delay in ns */
    SJA1105_CallbackTakeMutexTypeDef callback_take_mutex;
    SJA1105_CallbackGiveMutexTypeDef callback_give_mutex;
} SJA1105_CallbacksTypeDef;

struct SJA1105_HandleTypeDef {
    const SJA1105_ConfigTypeDef    *config;
    SJA1105_PortTypeDef            *ports;
    const SJA1105_CallbacksTypeDef *callbacks;
    SJA1105_TablesTypeDef          *tables;
    SJA1105_MACFiltersTypeDef       filters;
    bool                            static_conf_loaded;
    uint32_t                        static_conf_crc32;
    atomic_bool                     initialised;

};


/* Functions */

/* Initialisation */
SJA1105_StatusTypeDef SJA1105_PortConfigure(SJA1105_PortTypeDef *ports, uint8_t port_num, SJA1105_InterfaceTypeDef interface, SJA1105_ModeTypeDef mode, bool output_rmii_refclk, SJA1105_SpeedTypeDef speed, SJA1105_IOVoltageTypeDef voltage);
SJA1105_StatusTypeDef SJA1105_Init(SJA1105_HandleTypeDef *dev, const SJA1105_ConfigTypeDef *config, SJA1105_PortTypeDef *ports, const SJA1105_CallbacksTypeDef *callbacks, const uint32_t *static_conf, uint32_t static_conf_size);
SJA1105_StatusTypeDef SJA1105_ReInit(SJA1105_HandleTypeDef *dev, const uint32_t *static_conf, uint32_t static_conf_size);

/* User Functions */
SJA1105_StatusTypeDef SJA1105_PortGetSpeed(SJA1105_HandleTypeDef *dev, uint8_t port_num, SJA1105_SpeedTypeDef *speed);
SJA1105_StatusTypeDef SJA1105_PortSetSpeed(SJA1105_HandleTypeDef *dev, uint8_t port_num, SJA1105_SpeedTypeDef speed);
SJA1105_StatusTypeDef SJA1105_PortGetState(SJA1105_HandleTypeDef *dev, uint8_t port_num, SJA1105_PortState_TypeDef *port_state);
SJA1105_StatusTypeDef SJA1105_PortSleep(SJA1105_HandleTypeDef *dev, uint8_t port_num);
SJA1105_StatusTypeDef SJA1105_PortWake(SJA1105_HandleTypeDef *dev, uint8_t port_num);

SJA1105_StatusTypeDef SJA1105_ReadTemperatureX10(SJA1105_HandleTypeDef *dev, int16_t *temp);
SJA1105_StatusTypeDef SJA1105_CheckStatusRegisters(SJA1105_HandleTypeDef *dev);
SJA1105_StatusTypeDef SJA1105_MACAddrTrapTest(SJA1105_HandleTypeDef *dev, const uint8_t *addr, bool *trapped);


#ifdef __cplusplus
}
#endif

#endif /* SJA1105_INC_SJA1105_H_ */
