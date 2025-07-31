/*
 * sja1105.c
 *
 *  Created on: Jul 27, 2025
 *      Author: bens1
 */

#include "sja1105.h"
#include "sja1105_conf.h"
#include "sja1105_io.h"
#include "sja1105_regs.h"


SJA1105_StatusTypeDef SJA1105_UpdatePortSpeed(SJA1105_HandleTypeDef *dev, uint8_t port_num, SJA1105_SpeedTypeDef speed){

    SJA1105_StatusTypeDef status = SJA1105_OK;
    SJA1105_PortTypeDef   port   = dev->ports[port_num];

    /* Check the speed argument */
    if (port.dyanamic_speed == speed                ) status = SJA1105_ALREADY_CONFIGURED_ERROR;  /* New speed should be different */
    if (port.speed          != SJA1105_SPEED_DYNAMIC) status = SJA1105_PARAMETER_ERROR;           /* Only ports configured as dynamic can have their speed changed */
    if (speed               == SJA1105_SPEED_DYNAMIC) status = SJA1105_PARAMETER_ERROR;           /* Speed shouldn't be set to dynamic after the initial configuration */
    if (speed               >= SJA1105_SPEED_MAX    ) status = SJA1105_PARAMETER_ERROR;           /* Invalid speed */
    if (port.configured     == false                ) status = SJA1105_NOT_CONFIGURED_ERROR;      /* Port should have already been configured once with interface and voltage */
    if (status != SJA1105_OK) return status;

    /* Configure the ACU with new options */
    status = SJA1105_ConfigureACUPort(dev, port_num);
    if (status != SJA1105_OK) return status;
    
    /* Configure the CGU with new options */
    status = SJA1105_ConfigureCGUPort(dev, port_num);
    if (status != SJA1105_OK) return status;

    /* TODO: Update the MAC configuration table */
    
    /* TODO: is there more that needs doing?? */

    /* Update the port struct */
    dev->ports[port_num].dyanamic_speed = speed;

    return status;
}

SJA1105_StatusTypeDef SJA1105_ReadTemperatureX10(SJA1105_HandleTypeDef *dev, int16_t *temp_x10){

    SJA1105_StatusTypeDef status = SJA1105_OK;

    uint8_t  temp_low_i     = 0;
    uint8_t  temp_high_i    = SJA1105_TS_LUT_SIZE;
    uint8_t  guess          = 0;
    uint8_t  previous_guess = 0;
    uint32_t reg_data       = 0;

    /* Check the temperature sensor is enabled */
    status = SJA1105_ReadRegister(dev, SJA1105_ACU_REG_TS_CONFIG, &reg_data, 1);
    if (status != SJA1105_OK) return status;

    /* Enable it if it isn't */
    if (reg_data & SJA1105_TS_PD){
        reg_data &= ~SJA1105_TS_PD;
        status = SJA1105_WriteRegister(dev, SJA1105_ACU_REG_TS_CONFIG, &reg_data, 1);
        if (status != SJA1105_OK) return status;
        dev->callbacks->callback_delay_ms(1);  /* A slight delay to let the sensor stabilise */
    }

    /* Perform a binary search for the temperature.  */
    for (uint8_t i = 0; i < 7; i++){

        /* Calculate the next guess by splitting the range in half.
         * If the guess is the same as the previous_guess then the guesses have converged on the answer. */
        guess = (temp_low_i + temp_high_i) / 2;
        if (guess == previous_guess) break;

        /* Write to the TS_CONFIG register */
        reg_data = guess & SJA1105_TS_THRESHOLD_MASK;
        status = SJA1105_WriteRegister(dev, SJA1105_ACU_REG_TS_CONFIG, &reg_data, 1);
        if (status != SJA1105_OK) return status;

        /* Read from the TS_STATUS register */
        status = SJA1105_WriteRegister(dev, SJA1105_ACU_REG_TS_STATUS, &reg_data, 1);
        if (status != SJA1105_OK) return status;

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
    if (status != SJA1105_OK) return status;

    /* Get the temp (multiplied by 10). E.g. temp_x10 = 364 means 36.4 degrees */
    /* Note that this is the lower end of the range, and the answer could be up to SJA1105_TS_LUT[guess + 1]*/
    *temp_x10 = SJA1105_TS_LUT[guess];

    return status;
}


SJA1105_StatusTypeDef SJA1105_CheckStatus(SJA1105_HandleTypeDef *dev){
    
    SJA1105_StatusTypeDef status = SJA1105_OK;    
    uint32_t reg_data[SJA1105_REGULAR_CHECK_SIZE];

    /* Read the status registers */
    status = SJA1105_ReadRegister(dev, SJA1105_REGULAR_CHECK_ADDR, reg_data, SJA1105_REGULAR_CHECK_SIZE);
    if (status != SJA1105_OK) return status;

    /* TODO: Check other registers */

    /* Check for RAM parity errors */
    if (reg_data[SJA1105_REG_GENERAL_STATUS_10 - SJA1105_REGULAR_CHECK_ADDR] || reg_data[SJA1105_REG_GENERAL_STATUS_11 - SJA1105_REGULAR_CHECK_ADDR]){
        status = SJA1105_RAM_PARITY_ERROR;
        return status;
    }

    return status;
}
