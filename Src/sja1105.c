/*
 * sja1105.c
 *
 *  Created on: Jul 27, 2025
 *      Author: bens1
 */

#include "sja1105.h"


/* Initialisation */
SJA1105_StatusTypeDef SJA1105_Init(SJA1105_HandleTypeDef *dev, SJA1105_VariantTypeDef variant, const SJA1105_CallbacksTypeDef *callbacks, SPI_HandleTypeDef *spi_handle, GPIO_TypeDef *cs_port, uint16_t cs_pin, GPIO_TypeDef *rst_port, uint16_t rst_pin, uint32_t timeout){

    SJA1105_StatusTypeDef status = SJA1105_OK;

    dev->variant    = variant;
    dev->callbacks  = callbacks;
    dev->spi_handle = spi_handle;
    dev->cs_port    = cs_port;
    dev->cs_pin     = cs_pin;
    dev->rst_port   = rst_port;
    dev->rst_pin    = rst_pin;
    dev->timeout    = timeout;

    /* Only the SJA1105Q has been implemented. TODO: Add more */
    if (dev->variant != VARIANT_SJA1105Q) status = SJA1105_ERROR;

    /* Check SPI parameters */
    if (dev->spi_handle->Init.DataSize != SPI_DATASIZE_32BIT) status = SJA1105_ERROR;
    if (dev->spi_handle->Init.CLKPolarity != SPI_POLARITY_LOW) status = SJA1105_ERROR;
    if (dev->spi_handle->Init.CLKPhase != SPI_PHASE_1EDGE) status = SJA1105_ERROR;
    if (dev->spi_handle->Init.NSS != SPI_NSS_SOFT) status = SJA1105_ERROR;
    if (status != SJA1105_OK) return status;

    /* Set pins to a known state */
    HAL_GPIO_WritePin(dev->rst_port, dev->rst_pin, SET);
    HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, SET);


    /* Configure the SJA1105 following the steps from UM11040 figure 2 */

    /* Step 1: RESET */
    SJA1105_Reset(dev);

    /* Step 2: ACU REGISTER SETUP */

    /* Step 3: CGU REGISTER SETUP */

    /* Step 4: SGMII PHY/PCS (optional) */

    /* Step 5: STATIC CONFIGURATION */

    return status;
}




/* Low-Level Functions */
SJA1105_StatusTypeDef SJA1105_ReadRegister(SJA1105_HandleTypeDef *dev, uint32_t addr, uint32_t *data, uint32_t size){

    SJA1105_StatusTypeDef status = SJA1105_OK;

    return status;
}

SJA1105_StatusTypeDef SJA1105_WriteRegister(SJA1105_HandleTypeDef *dev, uint32_t addr, uint32_t *data, uint32_t size){

    SJA1105_StatusTypeDef status = SJA1105_OK;

    /* Take the mutex */
    if (dev->callbacks->callback_take_mutex(dev->timeout) != SJA1105_OK){
        status = SJA1105_MUTEX_ERROR;
        return status;
    }

    /* Create the command frame */    
    static uint32_t command_frame;
    command_frame = WRITE_ACCESS_FRAME;
    command_frame |= (addr & WRITE_ACCESS_ADDR_MASK) << WRITE_ACCESS_ADDR_POSITION;

    /* Start the transaction */
    HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, RESET);
    
    /* Send command frame */
    if (HAL_SPI_Transmit(dev->spi_handle, (uint8_t *) &command_frame, 1, dev->timeout) != HAL_OK){
        status = SJA1105_ERROR;
        return status;
    }
    
    /* Send payload */
    if (HAL_SPI_Transmit(dev->spi_handle, (uint8_t *) data, size, dev->timeout) != HAL_OK){
        status = SJA1105_ERROR;
        return status;
    }
    
    /* End the transaction */
    HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, SET);

    /* Give the mutex */
    if (dev->callbacks->callback_give_mutex() != SJA1105_OK){
        status = SJA1105_MUTEX_ERROR;
        return status;
    }

    return status;
}

void SJA1105_Reset(SJA1105_HandleTypeDef *dev){
    HAL_GPIO_WritePin(dev->rst_port, dev->rst_pin, RESET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(dev->rst_port, dev->rst_pin, SET);
}
