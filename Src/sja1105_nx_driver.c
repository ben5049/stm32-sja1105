/*
 * sja1105_nx_driver.c
 *
 *  Created on: Aug 1, 2025
 *      Author: bens1
 *
 *  This file is the equivalent to nx_stm32_phy_driver.c
 *
 */


#ifdef SJA1105_ENABLE_NX_DRIVER  /* Should be defined as a compiler flag to enable */

#include "tx_api.h"
#include "nx_stm32_phy_driver.h"
#include "nx_stm32_eth_config.h"

#include "sja1105.h"

extern SJA1105_HandleTypeDef hsja1105;


int32_t nx_eth_phy_init(void){

    int32_t ret = ETH_PHY_STATUS_OK;

    /* Poll the initialised flag and time out after 500ms */
    bool initialised = false;
    for (uint_fast8_t attempt = 0; !initialised && (attempt < 50); attempt++){
        initialised = hsja1105.initialised;
        if (!initialised) tx_thread_sleep((10 * TX_TIMER_TICKS_PER_SECOND) / 1000);
    }

    /* Return an error if not initialised */
    if (!initialised) ret = ETH_PHY_STATUS_ERROR;

    return ret;
}

int32_t nx_eth_phy_get_link_state(void){

    int32_t linkstate = ETH_PHY_STATUS_LINK_ERROR;

    SJA1105_StatusTypeDef status = SJA1105_OK;
    SJA1105_PortState_TypeDef port_state = SJA1105_PORT_STATE_DISCARDING;
    
    status = SJA1105_PortGetState(&hsja1105, hsja1105.config->host_port, &port_state);
    if (status != SJA1105_OK) linkstate = ETH_PHY_STATUS_LINK_ERROR;

    // TODO: Finish this function, should return speed and duplex

    return linkstate;
}

nx_eth_phy_handle_t nx_eth_phy_get_handle(void){

    nx_eth_phy_handle_t handle = &hsja1105;

    return handle;
}

#endif
