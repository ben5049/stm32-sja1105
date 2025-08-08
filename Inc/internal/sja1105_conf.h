/*
 * sja1105_conf.h
 *
 *  Created on: Jul 30, 2025
 *      Author: bens1
 */

#ifndef SJA1105_INC_SJA1105_CONF_H_
#define SJA1105_INC_SJA1105_CONF_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "sja1105.h"


#define SJA1105_INIT_CHECK                                            \
    do {                                                              \
        if (!dev->initialised) status = SJA1105_NOT_CONFIGURED_ERROR; \
        if (status != SJA1105_OK) return status;                      \
    } while (0)

#define SJA1105_LOCK                                                             \
    do {                                                                         \
        status = dev->callbacks->callback_take_mutex(dev, dev->config->timeout); \
        if (status != SJA1105_OK) return status;                                 \
    } while (0)

#define SJA1105_UNLOCK       dev->callbacks->callback_give_mutex(dev)

#define SJA1105_DELAY_NS(ns) dev->callbacks->callback_delay_ms(dev, (ns))
#define SJA1105_DELAY_MS(ms) dev->callbacks->callback_delay_ns(dev, (ms))

void SJA1105_ResetTables(sja1105_handle_t *dev, uint32_t fixed_length_table_buffer[SJA1105_FIXED_BUFFER_SIZE]);
void SJA1105_ResetManagementRoutes(sja1105_handle_t *dev);
void SJA1105_ResetEventCounters(sja1105_handle_t *dev);

sja1105_status_t SJA1105_CheckPartID(sja1105_handle_t *dev);
sja1105_status_t SJA1105_CheckDeviceID(sja1105_handle_t *dev, uint32_t device_id);

sja1105_status_t SJA1105_ConfigureACU(sja1105_handle_t *dev, bool write);
sja1105_status_t SJA1105_ConfigureACUPort(sja1105_handle_t *dev, uint8_t port_num, bool write);

sja1105_status_t SJA1105_ConfigureCGU(sja1105_handle_t *dev, bool write);
sja1105_status_t SJA1105_ConfigureCGUPort(sja1105_handle_t *dev, uint8_t port_num, bool write);

sja1105_status_t SJA1105_LoadStaticConfig(sja1105_handle_t *dev, const uint32_t *static_conf, uint32_t static_conf_size);
sja1105_status_t SJA1105_WriteStaticConfig(sja1105_handle_t *dev);
sja1105_status_t SJA1105_SyncStaticConfig(sja1105_handle_t *dev);
sja1105_status_t SJA1105_CheckRequiredTables(sja1105_handle_t *dev);

sja1105_status_t SJA1105_FreeAllTableMemory(sja1105_handle_t *dev);
sja1105_status_t SJA1105_AllocateFixedLengthTable(sja1105_handle_t *dev, const uint32_t *block, uint8_t block_size);
sja1105_status_t SJA1105_AllocateVariableLengthTable(sja1105_handle_t *dev, const uint32_t *block, uint8_t block_size);


#ifdef __cplusplus
}
#endif

#endif /* SJA1105_INC_SJA1105_CONF_H_ */
