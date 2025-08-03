/*
 * sja1105.c
 *
 *  Created on: Jul 27, 2025
 *      Author: bens1
 */

#include "sja1105.h"
#include "internal/sja1105_conf.h"
#include "internal/sja1105_io.h"
#include "internal/sja1105_regs.h"
#include "internal/sja1105_tables.h"


SJA1105_StatusTypeDef SJA1105_PortGetSpeed(SJA1105_HandleTypeDef *dev, uint8_t port_num, SJA1105_SpeedTypeDef *speed){

    SJA1105_StatusTypeDef status = SJA1105_OK;

    /* Take the mutex */
    SJA1105_LOCK;

    /* For dynamic ports look at the MAC Configuration table */
    if (dev->ports[port_num].speed == SJA1105_SPEED_DYNAMIC){
        SJA1105_MACConfTableGetSpeed(dev->tables->mac_config, dev->tables->mac_config_size, port_num, speed);
    }

    /* For static ports look at the port config struct */
    else {
        *speed = dev->ports[port_num].speed;
    }

    /* Give the mutex and return */
    SJA1105_UNLOCK;
    return status;
}

SJA1105_StatusTypeDef SJA1105_PortSetSpeed(SJA1105_HandleTypeDef *dev, uint8_t port_num, SJA1105_SpeedTypeDef new_speed){

    SJA1105_StatusTypeDef status = SJA1105_OK;
    
    /* Take the mutex */
    SJA1105_LOCK;

    SJA1105_PortTypeDef port = dev->ports[port_num];
    SJA1105_SpeedTypeDef current_speed;

    /* Get the current speed */
    SJA1105_PortGetSpeed(dev, port_num, &current_speed);

    /* Check the speed argument */
    if (new_speed           == current_speed          ) {status = SJA1105_OK; goto end;};    /* New speed should be different */
    if (port.speed          != SJA1105_SPEED_DYNAMIC  )  status = SJA1105_PARAMETER_ERROR;       /* Only ports configured as dynamic can have their speed changed */
    if (new_speed           == SJA1105_SPEED_DYNAMIC  )  status = SJA1105_PARAMETER_ERROR;       /* Speed shouldn't be set to dynamic after the initial configuration */
    if (new_speed           >= SJA1105_SPEED_INVALID  )  status = SJA1105_PARAMETER_ERROR;       /* Invalid speed */
    if (port.configured     == false                  )  status = SJA1105_NOT_CONFIGURED_ERROR;  /* Port should have already been configured once with interface and voltage */
    if (status != SJA1105_OK) goto end;

    /* TODO: Set SGMII speed */
    if (port.interface == SJA1105_INTERFACE_SGMII) {
        status = SJA1105_NOT_IMPLEMENTED_ERROR;
        goto end;
    }

    /* Set MII, RMII or RGMII port speed */
    else {
        /* Configure the ACU with new options */
        status = SJA1105_ConfigureACUPort(dev, port_num);
        if (status != SJA1105_OK) goto end;

        /* Configure the CGU with new options */
        status = SJA1105_ConfigureCGUPort(dev, port_num);
        if (status != SJA1105_OK) goto end;

        /* Update the MAC Configuration table */
        status = SJA1105_MACConfTableSetSpeed(dev->tables->mac_config, dev->tables->mac_config_size, port_num, new_speed);
        if (status != SJA1105_OK) goto end;

        /* Write the MAC Configuration table */
        status = SJA1105_MACConfTableWrite(dev, port_num);
        if (status != SJA1105_OK) goto end;
    }

    /* TODO: is there more that needs doing?? */

    /* Give the mutex and return */
    end:
    SJA1105_UNLOCK;
    return status;
}

SJA1105_StatusTypeDef SJA1105_ReadTemperatureX10(SJA1105_HandleTypeDef *dev, int16_t *temp_x10){

    SJA1105_StatusTypeDef status = SJA1105_OK;

    /* Setup variables */
    uint8_t  temp_low_i     = 0;
    uint8_t  temp_high_i    = SJA1105_TS_LUT_SIZE;
    uint8_t  guess          = 0;
    uint8_t  previous_guess = 0;
    uint32_t reg_data       = 0;
    
    /* Take the mutex */
    SJA1105_LOCK;
    
    /* Check the temperature sensor is enabled */
    status = SJA1105_ReadRegisterWithCheck(dev, SJA1105_ACU_REG_TS_CONFIG, &reg_data, 1);
    if (status != SJA1105_OK) goto end;
    
    /* Enable it if it isn't */
    if (reg_data & SJA1105_TS_PD){
        reg_data &= ~SJA1105_TS_PD;
        status = SJA1105_WriteRegister(dev, SJA1105_ACU_REG_TS_CONFIG, &reg_data, 1);
        if (status != SJA1105_OK) goto end;
        SJA1105_DELAY_MS(1);  /* A slight delay to let the sensor stabilise */
    }
    
    /* Perform a binary search for the temperature.  */
    for (uint_fast8_t i = 0; i < 7; i++){
        
        /* Calculate the next guess by splitting the range in half.
        * If the guess is the same as the previous_guess then the guesses have converged on the answer. */
       guess = (temp_low_i + temp_high_i) / 2;
       if (guess == previous_guess) break;
       
       /* Write to the TS_CONFIG register */
        reg_data = guess & SJA1105_TS_THRESHOLD_MASK;
        status = SJA1105_WriteRegister(dev, SJA1105_ACU_REG_TS_CONFIG, &reg_data, 1);
        if (status != SJA1105_OK) goto end;

        /* Read from the TS_STATUS register */
        status = SJA1105_ReadRegisterWithCheck(dev, SJA1105_ACU_REG_TS_STATUS, &reg_data, 1);
        if (status != SJA1105_OK) goto end;
        
        /* Adjust the range based on the result */
        if (reg_data & SJA1105_TS_EXCEEDED){
            temp_low_i = guess;
        } else {
            temp_high_i = guess;
        }
        
        previous_guess = guess;
    }
    
    /* Check the answer is valid */
    if ((guess >= SJA1105_TS_LUT_SIZE) || (guess == 0)) status = SJA1105_ERROR;
    if (status != SJA1105_OK) goto end;
    
    /* Get the temp (multiplied by 10). E.g. temp_x10 = 364 means 36.4 degrees */
    /* Note that this is the lower end of the range, and the answer could be up to SJA1105_TS_LUT[guess + 1]*/
    *temp_x10 = SJA1105_TS_LUT[guess];
    
    /* Give the mutex and return */
    end:
    SJA1105_UNLOCK;
    return status;
}


SJA1105_StatusTypeDef SJA1105_CheckStatusRegisters(SJA1105_HandleTypeDef *dev){

    SJA1105_StatusTypeDef status = SJA1105_OK;    
    
    /* Take the mutex */
    SJA1105_LOCK;

    /* Read the status registers */
    uint32_t reg_data[SJA1105_REGULAR_CHECK_SIZE];
    status = SJA1105_ReadRegister(dev, SJA1105_REGULAR_CHECK_ADDR, reg_data, SJA1105_REGULAR_CHECK_SIZE);
    if (status != SJA1105_OK) goto end;
    
    /* TODO: Check other registers */
    
    /* Check for RAM parity errors */
    if (reg_data[SJA1105_REG_GENERAL_STATUS_10 - SJA1105_REGULAR_CHECK_ADDR] || reg_data[SJA1105_REG_GENERAL_STATUS_11 - SJA1105_REGULAR_CHECK_ADDR]){
        status = SJA1105_RAM_PARITY_ERROR;
        goto end;
    }

    /* Give the mutex and return */
    end:
    SJA1105_UNLOCK;
    return status;
}


SJA1105_StatusTypeDef SJA1105_PortGetState(SJA1105_HandleTypeDef *dev, uint8_t port_num, SJA1105_PortState_TypeDef *port_state){

    SJA1105_StatusTypeDef status = SJA1105_NOT_IMPLEMENTED_ERROR;

    /* Take the mutex */
    SJA1105_LOCK;
    
    /* Give the mutex and return */
    SJA1105_UNLOCK;
    return status;
}

/* TODO: Disable transmitted clocks */
SJA1105_StatusTypeDef SJA1105_PortSleep(SJA1105_HandleTypeDef *dev, uint8_t port_num){

    SJA1105_StatusTypeDef status = SJA1105_NOT_IMPLEMENTED_ERROR;
    
    /* Take the mutex */
    SJA1105_LOCK;
    
    /* Give the mutex and return */
    SJA1105_UNLOCK;
    return status;
}

/* TODO: Enable transmitted clocks */
SJA1105_StatusTypeDef SJA1105_PortWake(SJA1105_HandleTypeDef *dev, uint8_t port_num){

    SJA1105_StatusTypeDef status = SJA1105_NOT_IMPLEMENTED_ERROR;

    /* Take the mutex */
    SJA1105_LOCK;
    
    /* Give the mutex and return */
    SJA1105_UNLOCK;
    return status;
}


SJA1105_StatusTypeDef SJA1105_CreateManagementRoute(SJA1105_HandleTypeDef *dev, uint8_t port_num, SJA1105_PortState_TypeDef *port_state){

    SJA1105_StatusTypeDef status = SJA1105_NOT_IMPLEMENTED_ERROR;

    /* Take the mutex */
    SJA1105_LOCK;

    /* Give the mutex and return */
//    end:
    SJA1105_UNLOCK;
    return status;
}

SJA1105_StatusTypeDef SJA1105_MACAddrTrapTest(SJA1105_HandleTypeDef *dev, const uint8_t *addr, bool *trapped){

    SJA1105_StatusTypeDef status = SJA1105_OK;

    /* Take the mutex */
    SJA1105_LOCK;

    /* Test against the first filter */
    *trapped = true;
    for (uint_fast8_t i = 0; i < MAC_ADDR_SIZE; i++){
        if ((addr[i] & dev->filters.mac_flt0[i]) != dev->filters.mac_fltres0[i]){
            *trapped = false;
            break;
        }
    }
    if (*trapped) goto end;

    /* Test against the second filter */
    *trapped = true;
    for (uint_fast8_t i = 0; i < MAC_ADDR_SIZE; i++){
        if ((addr[i] & dev->filters.mac_flt1[i]) != dev->filters.mac_fltres1[i]){
            *trapped = false;
            break;
        }
    }

    /* Give the mutex and return */
    end:
    SJA1105_UNLOCK;
    return status;
}
