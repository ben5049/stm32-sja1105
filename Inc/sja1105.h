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
#define MAC_ADDR_SIZE                 (6)
#define SJA1105_NUM_PORTS             (5)
#define SJA1105_NUM_MGMT_SLOTS        (4)
#define SJA1105_MAX_ATTEMPTS          (10) /* Maximum number of attempts to try anything. E.g. polling a flag with timeout = 100ms will result in 10 reads 10ms apart. Must be > 0 */
#define SJA1105_L2ADDR_LU_ENTRY_SIZE  (5)
#define SJA1105_L2ADDR_LU_NUM_ENTRIES (1024)

/* Typedefs */

typedef struct sja1105_handle_t sja1105_handle_t;

typedef enum {
    SJA1105_OK      = HAL_OK,
    SJA1105_ERROR   = HAL_ERROR,
    SJA1105_BUSY    = HAL_BUSY,
    SJA1105_TIMEOUT = HAL_TIMEOUT,
    SJA1105_PARAMETER_ERROR,
    SJA1105_ALREADY_CONFIGURED_ERROR, /* Error when attempting to init an already initialised device or configure a port that has already been configured. */
    SJA1105_NOT_CONFIGURED_ERROR,
    SJA1105_SPI_ERROR,
    SJA1105_ID_ERROR,
    SJA1105_STATIC_CONF_ERROR,
    SJA1105_CRC_ERROR,
    SJA1105_RAM_PARITY_ERROR,
    SJA1105_NOT_IMPLEMENTED_ERROR,
    SJA1105_MUTEX_ERROR,            /* Serious mutex error, will normally just return SJA1105_BUSY if it tries to take a mutex held by another thread */
    SJA1105_DYNAMIC_MEMORY_ERROR,   /* Attempted to re-allocate without free, or free after free */
    SJA1105_DYNAMIC_RECONFIG_ERROR, /* VALID bit not set after performing a dynamic reconfiguration */
    SJA1105_REVERT_ERROR,           /* Catastrophic error has occured such as an error while fixing another error */
    SJA1105_NO_FREE_MGMT_ROUTES_ERROR,
} sja1105_status_t;

typedef enum {
    VARIANT_SJA1105E        = 0x0,
    VARIANT_SJA1105T        = 0x1,
    VARIANT_SJA1105P        = 0x2,
    VARIANT_SJA1105Q        = 0x3,
    VARIANT_SJA1105R        = 0x4,
    VARIANT_SJA1105S        = 0x5,
    VARIANT_SJA1105_INVALID = 0x6 /* Dummmy value for argument checking */
} sja1105_variant_t;

typedef enum {
    SJA1105_INTERFACE_MII     = 0x0,
    SJA1105_INTERFACE_RMII    = 0x1,
    SJA1105_INTERFACE_RGMII   = 0x2,
    SJA1105_INTERFACE_SGMII   = 0x3,
    SJA1105_INTERFACE_INVALID = 0x4 /* Dummmy value for argument checking */
} sja1105_interface_t;

typedef enum {
    SJA1105_MODE_MAC     = 0x0, /* The port acts like a MAC */
    SJA1105_MODE_PHY     = 0x1, /* The port acts like a PHY and must connect to a true MAC */
    SJA1105_MODE_INVALID = 0x2  /* Dummmy value for argument checking */
} sja1105_mode_t;

typedef enum {
    SJA1105_SPEED_DYNAMIC = 0x0, /* Used for ports that need to negotiate a speed */
    SJA1105_SPEED_1G      = 0x1,
    SJA1105_SPEED_100M    = 0x2,
    SJA1105_SPEED_10M     = 0x3,
    SJA1105_SPEED_INVALID = 0x4 /* Dummmy value for argument checking */
} sja1105_speed_t;

typedef enum {
    SJA1105_IO_VOLTAGE_UNSPECIFIED = 0x0,
    SJA1105_IO_1V8                 = 0x1,
    SJA1105_IO_2V5                 = 0x2,
    SJA1105_IO_3V3                 = 0x3,
    SJA1105_IO_INVALID_V           = 0x4 /* Dummmy value for argument checking */
} sja1105_io_voltage_t;

typedef struct {
    uint8_t              port_num;
    sja1105_speed_t      speed;
    sja1105_interface_t  interface;
    sja1105_mode_t       mode;
    bool                 output_rmii_refclk; /* Only used when interface = SJA1105_INTERFACE_RMII and mode = SJA1105_MODE_PHY */
    sja1105_io_voltage_t voltage;
    bool                 configured;
} sja1105_port_t;

typedef struct {
    sja1105_variant_t  variant;
    SPI_HandleTypeDef *spi_handle;
    GPIO_TypeDef      *cs_port;
    uint16_t           cs_pin;
    GPIO_TypeDef      *rst_port;
    uint16_t           rst_pin;
    uint32_t           timeout;      /* Timeout in ms for doing anything with a timeout (read, write, take mutex etc) */
    uint32_t           mgmt_timeout; /* Time in ms after creating a manamegement route that it can be overwriten if it hasn't been used */
    uint8_t            host_port;
    bool               skew_clocks;  /* Make xMII clocks use different phases (where possible) to improve EMC performance */
    uint8_t            switch_id;    /* Used to identify the switch that trapped a frame */
} sja1105_config_t;

typedef struct {
    uint32_t *mac_config;
    uint32_t  mac_config_size; /* Number of uint32_t in mac_config */
    uint32_t *general_params;
    uint32_t  general_params_size;
} sja1105_tables_t;

typedef struct {
    uint8_t mac_fltres0[MAC_ADDR_SIZE];
    uint8_t mac_flt0[MAC_ADDR_SIZE];
    uint8_t mac_fltres1[MAC_ADDR_SIZE];
    uint8_t mac_flt1[MAC_ADDR_SIZE];
} sja1105_mac_filters_t;

/* Stores information about driver events */
typedef struct {
    uint32_t words_read;
    uint32_t words_written;
    uint32_t crc_errors;
    uint32_t mgmt_frames_sent;
    uint32_t mgmt_entries_dropped;
} sja1105_event_counters_t;

/* Stores information about management routes */
typedef struct {
    bool     slot_taken[SJA1105_NUM_MGMT_SLOTS]; /* true = slot has been taken */
    uint32_t timestamps[SJA1105_NUM_MGMT_SLOTS]; /* Time when route was created, used to evict old rules (TODO) */
    void    *contexts[SJA1105_NUM_MGMT_SLOTS];   /* Context set by SJA1105_ManagementRouteCreate() caller so they can tell if their entry has been evicted. */
} sja1105_mgmt_routes_t;


typedef uint32_t (*sja1105_callback_get_time_ms_t)(sja1105_handle_t *dev);
typedef void (*sja1105_callback_delay_ms_t)(sja1105_handle_t *dev, uint32_t ms);
typedef void (*sja1105_callback_delay_ns_t)(sja1105_handle_t *dev, uint32_t ns);
typedef sja1105_status_t (*sja1105_callback_take_mutex_t)(sja1105_handle_t *dev, uint32_t timeout);
typedef sja1105_status_t (*sja1105_callback_give_mutex_t)(sja1105_handle_t *dev);

typedef struct {
    sja1105_callback_get_time_ms_t callback_get_time_ms; /* Get time in ms */
    sja1105_callback_delay_ms_t    callback_delay_ms;    /* Non-blocking delay in ms */
    sja1105_callback_delay_ns_t    callback_delay_ns;    /* Blocking delay in ns */
    sja1105_callback_take_mutex_t  callback_take_mutex;
    sja1105_callback_give_mutex_t  callback_give_mutex;
} sja1105_callbacks_t;

struct sja1105_handle_t {
    const sja1105_config_t    *config;
    sja1105_port_t            *ports;
    const sja1105_callbacks_t *callbacks;
    sja1105_tables_t           tables;
    sja1105_mac_filters_t      filters;
    sja1105_event_counters_t   events;
    sja1105_mgmt_routes_t      management_routes;
    bool                       static_conf_loaded;
    uint32_t                   static_conf_crc32;
    atomic_bool                initialised;
};


/* Functions */

/* Initialisation */
sja1105_status_t SJA1105_PortConfigure(sja1105_port_t *ports, uint8_t port_num, sja1105_interface_t interface, sja1105_mode_t mode, bool output_rmii_refclk, sja1105_speed_t speed, sja1105_io_voltage_t voltage);
sja1105_status_t SJA1105_Init(sja1105_handle_t *dev, const sja1105_config_t *config, sja1105_port_t *ports, const sja1105_callbacks_t *callbacks, const uint32_t *static_conf, uint32_t static_conf_size);
sja1105_status_t SJA1105_ReInit(sja1105_handle_t *dev, const uint32_t *static_conf, uint32_t static_conf_size);

/* User Functions */
sja1105_status_t SJA1105_PortGetState(sja1105_handle_t *dev, uint8_t port_num, bool *state);
sja1105_status_t SJA1105_PortGetSpeed(sja1105_handle_t *dev, uint8_t port_num, sja1105_speed_t *speed);
sja1105_status_t SJA1105_PortSetSpeed(sja1105_handle_t *dev, uint8_t port_num, sja1105_speed_t speed);
sja1105_status_t SJA1105_PortSetLearning(sja1105_handle_t *dev, uint8_t port_num, bool enable);
sja1105_status_t SJA1105_PortSetForwarding(sja1105_handle_t *dev, uint8_t port_num, bool enable);
sja1105_status_t SJA1105_PortSleep(sja1105_handle_t *dev, uint8_t port_num);
sja1105_status_t SJA1105_PortWake(sja1105_handle_t *dev, uint8_t port_num);

sja1105_status_t SJA1105_ReadTemperatureX10(sja1105_handle_t *dev, int16_t *temp);
sja1105_status_t SJA1105_CheckStatusRegisters(sja1105_handle_t *dev);
sja1105_status_t SJA1105_MACAddrTrapTest(sja1105_handle_t *dev, const uint8_t *addr, bool *trapped);

sja1105_status_t SJA1105_L2EntryReadByIndex(sja1105_handle_t *dev, uint16_t index, bool managment, uint32_t entry[SJA1105_L2ADDR_LU_ENTRY_SIZE]);
sja1105_status_t SJA1105_ManagementRouteCreate(sja1105_handle_t *dev, const uint8_t dst_addr[MAC_ADDR_SIZE], uint8_t dst_ports, bool takets, bool tsreg, void *context);
sja1105_status_t SJA1105_ManagementRouteFree(sja1105_handle_t *dev);
sja1105_status_t SJA1105_FlushTCAM(sja1105_handle_t *dev);

#ifdef __cplusplus
}
#endif

#endif /* SJA1105_INC_SJA1105_H_ */
