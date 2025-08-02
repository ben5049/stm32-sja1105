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

#include "nx_stm32_phy_driver.h"
#include "nx_stm32_eth_config.h"

#include "sja1105.h"

extern SJA1105_HandleTypeDef hsja1105;


int32_t nx_eth_phy_init(void){

    int32_t ret = ETH_PHY_STATUS_OK;

    return ret;
}

int32_t nx_eth_phy_get_link_state(void){

    int32_t linkstate = ETH_PHY_STATUS_LINK_ERROR;

    return linkstate;
}

nx_eth_phy_handle_t nx_eth_phy_get_handle(void){

    nx_eth_phy_handle_t handle = &hsja1105;

    return handle;
}

#endif
