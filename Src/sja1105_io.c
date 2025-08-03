/*
 * sja1105_spi.c
 *
 *  Created on: Jul 30, 2025
 *      Author: bens1
 */

#include "sja1105.h"
#include "internal/sja1105_io.h"
#include "internal/sja1105_regs.h"


SJA1105_StatusTypeDef __SJA1105_ReadRegister(SJA1105_HandleTypeDef *dev, uint32_t addr, uint32_t *data, uint32_t size, bool integrity_check){

    SJA1105_StatusTypeDef status        = SJA1105_OK;
    static const uint32_t dummy_payload = 0xcccc5555;  /* When size = 1 and integrity_check = true, send this payload as we are reading data to confirm everything is working. */

    /* Check the parameters */
    if ( size         ==  0                    ) status = SJA1105_PARAMETER_ERROR;  /* Empty check */
    if ( addr         &  ~SJA1105_SPI_ADDR_MASK) status = SJA1105_PARAMETER_ERROR;  /* Start address check */
    if ((addr + size) &  ~SJA1105_SPI_ADDR_MASK) status = SJA1105_PARAMETER_ERROR;  /* End address check */
    if (status != SJA1105_OK) return status;

    /* Take the mutex */
    SJA1105_LOCK;
    
    /* Initialise counter for the number of double words remaining to receive */
    uint32_t dwords_remaining = size;
    uint32_t command_frame;
    uint16_t block_size;
    
    /* If the number of double words to read is greater than SJA1105_SPI_MAX_PAYLOAD_SIZE, then the read needs to be broken into smaller transactions */
    do {

        /* Create the command frame */
        command_frame = SJA1105_SPI_READ_FRAME;
        command_frame |= ((addr + size - dwords_remaining) & SJA1105_SPI_ADDR_MASK) << SJA1105_SPI_ADDR_POSITION;
        command_frame |= (CONSTRAIN(dwords_remaining, 0, SJA1105_SPI_MAX_RX_PAYLOAD_SIZE) & SJA1105_SPI_SIZE_MASK) << SJA1105_SPI_SIZE_POSITION;  /* Note that if the read size = SPI_MAX_PAYLOAD_SIZE it will wrap to 0 as intended */

        /* Start the transaction after a delay (ensures successive transactions meet timing requirements) */
        SJA1105_DELAY_NS(SJA1105_T_SPI_WR);
        HAL_GPIO_WritePin(dev->config->cs_port, dev->config->cs_pin, RESET);
        SJA1105_DELAY_NS(SJA1105_T_SPI_LEAD);

        /* Send command frame */
        if (HAL_SPI_Transmit(dev->config->spi_handle, (uint8_t *) &command_frame, 1, dev->config->timeout) != HAL_OK){
            status = SJA1105_SPI_ERROR;
            goto end;
        }
        dev->events.words_written++;

        /* Insert delay to allow the device to fetch the data */
        SJA1105_DELAY_NS(SJA1105_T_SPI_CTRL_DATA);

        /* Receive data, if size = 1 then send dummy payload to test for faults */
        block_size = CONSTRAIN(dwords_remaining, 0, SJA1105_SPI_MAX_RX_PAYLOAD_SIZE);
        if ((size == 1) && integrity_check){
            if (HAL_SPI_TransmitReceive(dev->config->spi_handle, (uint8_t *) &dummy_payload, (uint8_t *) data, 1, dev->config->timeout) != HAL_OK){
                status = SJA1105_SPI_ERROR;
                goto end;
            }
            if (data[0] == dummy_payload){
                status = SJA1105_SPI_ERROR;
                goto end;
        }
        } else {
            if (HAL_SPI_Receive(dev->config->spi_handle, (uint8_t *) &data[size - dwords_remaining], block_size, dev->config->timeout) != HAL_OK){
                status = SJA1105_SPI_ERROR;
                goto end;
            }
        }
        dev->events.words_read += block_size;

        /* End the transaction */
        SJA1105_DELAY_NS(SJA1105_T_SPI_LAG);
        HAL_GPIO_WritePin(dev->config->cs_port, dev->config->cs_pin, SET);

        /* Calculate the double words to receive remaining */
        dwords_remaining -= CONSTRAIN(dwords_remaining, 0, SJA1105_SPI_MAX_RX_PAYLOAD_SIZE);

    } while (dwords_remaining > 0);

    /* Give the mutex and return */
    end:
    SJA1105_UNLOCK;
    return status;
}


SJA1105_StatusTypeDef SJA1105_ReadRegister(SJA1105_HandleTypeDef *dev, uint32_t addr, uint32_t *data, uint32_t size){
    return __SJA1105_ReadRegister(dev, addr, data, size, false);
}


SJA1105_StatusTypeDef SJA1105_ReadRegisterWithCheck(SJA1105_HandleTypeDef *dev, uint32_t addr, uint32_t *data, uint32_t size){
    return __SJA1105_ReadRegister(dev, addr, data, size, true);
}


/* Repeatedly read a flag until the flag is set or dev->config->timeout ms have passed 
 * If polarity is high then it will poll until the flag is 1
 * If polarity is low then it will poll until the flag is 0
 */
SJA1105_StatusTypeDef SJA1105_PollFlag(SJA1105_HandleTypeDef *dev, uint32_t addr, uint32_t mask, bool polarity){

    SJA1105_StatusTypeDef status = SJA1105_OK;
    bool flag = false;

    /* Read the flag up to SJA1105_MAX_ATTEMPTS times */
    for (uint_fast8_t i = 0; i < SJA1105_MAX_ATTEMPTS; i++){
        status = SJA1105_ReadFlag(dev, addr, mask, &flag);
        if (status != SJA1105_OK || flag == polarity) break;
        dev->callbacks->callback_delay_ms(dev, dev->config->timeout / SJA1105_MAX_ATTEMPTS);
    }

    /* If the loop reaches the end and the flag hasn't been set */
    if (status == SJA1105_OK && flag != polarity) {
        status = SJA1105_TIMEOUT;
    }

    return status;
}


SJA1105_StatusTypeDef SJA1105_ReadFlag(SJA1105_HandleTypeDef *dev, uint32_t addr, uint32_t mask, bool *result){
    
    SJA1105_StatusTypeDef status = SJA1105_OK;
    uint32_t reg_data;

    status = __SJA1105_ReadRegister(dev, addr, &reg_data, 1, false);

    *result = (reg_data & mask) != 0;

    return status;
}


SJA1105_StatusTypeDef SJA1105_WriteRegister(SJA1105_HandleTypeDef *dev, uint32_t addr, const uint32_t *data, uint32_t size){

    SJA1105_StatusTypeDef status = SJA1105_OK;

    /* Check the parameters */
    if ( size         ==  0                    ) status = SJA1105_PARAMETER_ERROR;  /* Empty check */
    if ( addr         &  ~SJA1105_SPI_ADDR_MASK) status = SJA1105_PARAMETER_ERROR;  /* Start address check */
    if ((addr + size) &  ~SJA1105_SPI_ADDR_MASK) status = SJA1105_PARAMETER_ERROR;  /* End address check */
    if (status != SJA1105_OK) return status;

    /* Take the mutex */
    SJA1105_LOCK;

    /* Initialise counter for the number of double words remaining to transmit */
    uint32_t dwords_remaining = size;
    uint32_t command_frame;
    uint16_t block_size;

    /* If the payload size is greater than SJA1105_SPI_MAX_PAYLOAD_SIZE, then the write needs to be broken into smaller transactions */
    do {

        /* Create the command frame */
        command_frame = SJA1105_SPI_WRITE_FRAME;
        command_frame |= ((addr + size - dwords_remaining) & SJA1105_SPI_ADDR_MASK) << SJA1105_SPI_ADDR_POSITION;

        /* Start the transaction after a delay (ensures successive transactions meet timing requirements) */
        SJA1105_DELAY_NS(SJA1105_T_SPI_WR);
        HAL_GPIO_WritePin(dev->config->cs_port, dev->config->cs_pin, RESET);
        SJA1105_DELAY_NS(SJA1105_T_SPI_LEAD);

        /* Send command frame */
        if (HAL_SPI_Transmit(dev->config->spi_handle, (uint8_t *) &command_frame, 1, dev->config->timeout) != HAL_OK){
            status = SJA1105_SPI_ERROR;
            goto end;
        }
        dev->events.words_written++;

        /* Send payload */
        block_size = CONSTRAIN(dwords_remaining, 0, SJA1105_SPI_MAX_TX_PAYLOAD_SIZE);
        if (HAL_SPI_Transmit(dev->config->spi_handle, (uint8_t *) &data[size - dwords_remaining], block_size, dev->config->timeout) != HAL_OK){
            status = SJA1105_SPI_ERROR;
            goto end;
        }
        dev->events.words_written += block_size;

        /* End the transaction */
        SJA1105_DELAY_NS(SJA1105_T_SPI_LAG);
        HAL_GPIO_WritePin(dev->config->cs_port, dev->config->cs_pin, SET);

        /* Calculate the double words to transmit remaining */
        dwords_remaining -= CONSTRAIN(dwords_remaining, 0, SJA1105_SPI_MAX_TX_PAYLOAD_SIZE);

    } while (dwords_remaining > 0);

    /* Give the mutex and return */
    end:
    SJA1105_UNLOCK;
    return status;
}


void SJA1105_Reset(SJA1105_HandleTypeDef *dev){
    HAL_GPIO_WritePin(dev->config->rst_port, dev->config->rst_pin, RESET);
    SJA1105_DELAY_NS(SJA1105_T_RST);  /* 5us delay */
    HAL_GPIO_WritePin(dev->config->rst_port, dev->config->rst_pin, SET);
    SJA1105_DELAY_MS(1);  /* 329us minimum until SPI commands can be written. Use a 1ms non-blocking delay so the RTOS can do other work */
}
