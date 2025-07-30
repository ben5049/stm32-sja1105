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

    /* Set the port to unconfigured */
    dev->ports[port_num].configured = false;

    /* Load the new configuration options */
    status = SJA1105_ConfigurePort(dev->ports, port_num, dev->ports[port_num].interface, speed, dev->ports[port_num].voltage);
    if (status != SJA1105_OK) return status;

    /* Configure the ACU with new options */
    status = SJA1105_ConfigureACUPort(dev, port_num);
    if (status != SJA1105_OK) return status;
    
    /* Configure the CGU with new options */
    status = SJA1105_ConfigureCGUPort(dev, port_num);
    if (status != SJA1105_OK) return status;

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
