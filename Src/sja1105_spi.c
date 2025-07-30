/*
 * sja1105_spi.c
 *
 *  Created on: Jul 30, 2025
 *      Author: bens1
 */

#include "sja1105.h"
#include "sja1105_spi.h"
#include "sja1105_regs.h"


#define CONSTRAIN(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))


SJA1105_StatusTypeDef SJA1105_WriteStaticConfig(SJA1105_HandleTypeDef *dev, const uint32_t *static_conf, uint32_t static_conf_size){

    SJA1105_StatusTypeDef status = SJA1105_OK;

    /* Check the static config matches the device type */
    switch (dev->variant) {
        case VARIANT_SJA1105:
        case VARIANT_SJA1105T:
            if (static_conf[0] != SJA1105_T_SWITCH_CORE_ID) status = SJA1105_STATIC_CONF_ERROR;
            break;
        case VARIANT_SJA1105P:
        case VARIANT_SJA1105R:
            if (static_conf[0] != SJA1105PR_SWITCH_CORE_ID) status = SJA1105_STATIC_CONF_ERROR;
            break;
        case VARIANT_SJA1105Q:
        case VARIANT_SJA1105S:
            if (static_conf[0] != SJA1105QS_SWITCH_CORE_ID) status = SJA1105_STATIC_CONF_ERROR;
            break;
        default:
            status = SJA1105_PARAMETER_ERROR;  /* Unknown variant */
            break;
    }
    if (status != SJA1105_OK) return status;

    /* Write the config */
    status = SJA1105_WriteRegister(dev, SJA1105_STATIC_CONF_ADDR, static_conf, static_conf_size);
    if (status != SJA1105_OK) return status;

    return status;
}


/* Low-Level Functions */
SJA1105_StatusTypeDef SJA1105_ReadRegister(SJA1105_HandleTypeDef *dev, uint32_t addr, uint32_t *data, uint32_t size){

    SJA1105_StatusTypeDef status = SJA1105_OK;

    /* Check the parameters */
    if (size          ==  0                    ) status = SJA1105_PARAMETER_ERROR;  /* Empty check */
    if (addr          &  ~SJA1105_SPI_ADDR_MASK) status = SJA1105_PARAMETER_ERROR;  /* Start address check */
    if ((addr + size) &  ~SJA1105_SPI_ADDR_MASK) status = SJA1105_PARAMETER_ERROR;  /* End address check */
    if (status != SJA1105_OK) return status;

    /* Take the mutex */
    status = dev->callbacks->callback_take_mutex(dev->timeout);
    if (status != SJA1105_OK) return status;

    /* Initialise counter for the number of double words remaining to receive */
    uint32_t dwords_remaining = size;

    /* If the number of double words to read is greater than SJA1105_SPI_MAX_PAYLOAD_SIZE, then the read needs to be broken into smaller transactions */
    do {

        /* Create the command frame */
        uint32_t command_frame = SJA1105_SPI_READ_FRAME;
        command_frame |= ((addr + size - dwords_remaining) & SJA1105_SPI_ADDR_MASK) << SJA1105_SPI_ADDR_POSITION;
        command_frame |= (CONSTRAIN(dwords_remaining, 0, SJA1105_SPI_MAX_PAYLOAD_SIZE) & SJA1105_SPI_SIZE_MASK) << SJA1105_SPI_SIZE_POSITION;  /* Note that if the read size = SPI_MAX_PAYLOAD_SIZE it will wrap to 0 as intended */

        /* Start the transaction after a delay (ensures successive transactions meet timing requirements) */
        dev->callbacks->callback_delay_ns(SJA1105_T_SPI_WR);
        HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, RESET);
        dev->callbacks->callback_delay_ns(SJA1105_T_SPI_LEAD);

        /* Send command frame */
        if (HAL_SPI_Transmit(dev->spi_handle, (uint8_t *) &command_frame, 1, dev->timeout) != HAL_OK){
            status = SJA1105_ERROR;
            return status;
        }

        /* Insert delay to allow the device to fetch the data */
        dev->callbacks->callback_delay_ns(SJA1105_T_SPI_CTRL_DATA);

        /* Receive payload */
        if (HAL_SPI_Receive(dev->spi_handle, (uint8_t *) &data[size - dwords_remaining], CONSTRAIN(dwords_remaining, 0, SJA1105_SPI_MAX_PAYLOAD_SIZE), dev->timeout) != HAL_OK){
            status = SJA1105_ERROR;
            return status;
        }

        /* End the transaction */
        dev->callbacks->callback_delay_ns(SJA1105_T_SPI_LAG);
        HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, SET);

        /* Calculate the double words to receive remaining */
        dwords_remaining -= CONSTRAIN(dwords_remaining, 0, SJA1105_SPI_MAX_PAYLOAD_SIZE);

    } while (dwords_remaining > 0);

    /* Give the mutex */
    status = dev->callbacks->callback_give_mutex();
    if (status != SJA1105_OK) return status;

    return status;
}

SJA1105_StatusTypeDef SJA1105_WriteRegister(SJA1105_HandleTypeDef *dev, uint32_t addr, const uint32_t *data, uint32_t size){

    SJA1105_StatusTypeDef status = SJA1105_OK;

    /* Check the parameters */
    if (size          ==  0                    ) status = SJA1105_PARAMETER_ERROR;  /* Empty check */
    if (addr          &  ~SJA1105_SPI_ADDR_MASK) status = SJA1105_PARAMETER_ERROR;  /* Start address check */
    if ((addr + size) &  ~SJA1105_SPI_ADDR_MASK) status = SJA1105_PARAMETER_ERROR;  /* End address check */
    if (status != SJA1105_OK) return status;

    /* Take the mutex */
    status = dev->callbacks->callback_take_mutex(dev->timeout);
    if (status != SJA1105_OK) return status;

    /* Initialise counter for the number of double words remaining to transmit */
    uint32_t dwords_remaining = size;

    /* If the payload size is greater than SJA1105_SPI_MAX_PAYLOAD_SIZE, then the write needs to be broken into smaller transactions */
    do {

        /* Create the command frame */
        uint32_t command_frame = SJA1105_SPI_WRITE_FRAME;
        command_frame |= ((addr + size - dwords_remaining) & SJA1105_SPI_ADDR_MASK) << SJA1105_SPI_ADDR_POSITION;

        /* Start the transaction after a delay (ensures successive transactions meet timing requirements) */
        dev->callbacks->callback_delay_ns(SJA1105_T_SPI_WR);
        HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, RESET);
        dev->callbacks->callback_delay_ns(SJA1105_T_SPI_LEAD);

        /* Send command frame */
        if (HAL_SPI_Transmit(dev->spi_handle, (uint8_t *) &command_frame, 1, dev->timeout) != HAL_OK){
            status = SJA1105_ERROR;
            return status;
        }

        /* Send payload */
        if (HAL_SPI_Transmit(dev->spi_handle, (uint8_t *) &data[size - dwords_remaining], CONSTRAIN(dwords_remaining, 0, SJA1105_SPI_MAX_PAYLOAD_SIZE), dev->timeout) != HAL_OK){
            status = SJA1105_ERROR;
            return status;
        }

        /* End the transaction */
        dev->callbacks->callback_delay_ns(SJA1105_T_SPI_LAG);
        HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, SET);

        /* Calculate the double words to transmit remaining */
        dwords_remaining -= CONSTRAIN(dwords_remaining, 0, SJA1105_SPI_MAX_PAYLOAD_SIZE);

    } while (dwords_remaining > 0);

    /* Give the mutex */
    status = dev->callbacks->callback_give_mutex();
    if (status != SJA1105_OK) return status;

    return status;
}
