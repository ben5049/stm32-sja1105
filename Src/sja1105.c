/*
 * sja1105.c
 *
 *  Created on: Jul 27, 2025
 *      Author: bens1
 */

#include "sja1105.h"

#define CONSTRAIN(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))


/* ---------------------------------------------------------------------------- */
/* Initialisation */
/* ---------------------------------------------------------------------------- */

SJA1105_StatusTypeDef SJA1105_ConfigurePort(SJA1105_PortTypeDef *ports, uint8_t port_num, SJA1105_InterfaceTypeDef interface, SJA1105_SpeedTypeDef speed){

	SJA1105_StatusTypeDef status = SJA1105_OK;

	/* Check the parameters */
	if (port_num >= SJA1105_NUM_PORTS)       status = SJA1105_PARAMETER_ERROR;
	if (interface > SJA1105_INTERFACE_SGMII) status = SJA1105_PARAMETER_ERROR;
	if (speed > SJA1105_SPEED_1G)            status = SJA1105_PARAMETER_ERROR;
	if (ports[port_num].configured == true)  status = SJA1105_ALREADY_CONFIGURED_ERROR;  /* Note this may cause an unintended error if the struct uses non-zeroed memory. */
	if (status != SJA1105_OK) return status;

	/* Assign the parameters */
	ports[port_num].port_num   = port_num;
	ports[port_num].interface  = interface;
	ports[port_num].speed      = speed;
	ports[port_num].configured = true;

	return status;
}

SJA1105_StatusTypeDef SJA1105_Init(
		SJA1105_HandleTypeDef          *dev,
		const SJA1105_VariantTypeDef    variant,
		const SJA1105_CallbacksTypeDef *callbacks,
		SPI_HandleTypeDef              *spi_handle,
		GPIO_TypeDef                   *cs_port,
		uint16_t                        cs_pin,
		GPIO_TypeDef                   *rst_port,
		uint16_t                        rst_pin,
		uint32_t                        timeout,
		const uint32_t                 *static_conf,
		uint32_t                        static_conf_size,
		SJA1105_PortTypeDef            *ports){

	SJA1105_StatusTypeDef status = SJA1105_OK;

	/* Check the struct hasn't already been initialised. Note this may cause an unintended error if the struct uses non-zeroed memory. */
	if (dev->initialised == true) status = SJA1105_ALREADY_CONFIGURED_ERROR;
	if (status != SJA1105_OK) return status;

	dev->variant     = variant;
	dev->callbacks   = callbacks;
	dev->spi_handle  = spi_handle;
	dev->cs_port     = cs_port;
	dev->cs_pin      = cs_pin;
	dev->rst_port    = rst_port;
	dev->rst_pin     = rst_pin;
	dev->timeout     = timeout;
	dev->ports       = ports;
	dev->initialised = false;

	/* Only the SJA1105Q has been implemented. TODO: Add more */
	if (dev->variant != VARIANT_SJA1105Q) status = SJA1105_PARAMETER_ERROR;

	/* Check SPI parameters */
	if (dev->spi_handle->Init.DataSize    != SPI_DATASIZE_32BIT) status = SJA1105_PARAMETER_ERROR;
	if (dev->spi_handle->Init.CLKPolarity != SPI_POLARITY_LOW  ) status = SJA1105_PARAMETER_ERROR;
	if (dev->spi_handle->Init.CLKPhase    != SPI_PHASE_1EDGE   ) status = SJA1105_PARAMETER_ERROR;
	if (dev->spi_handle->Init.NSS         != SPI_NSS_SOFT      ) status = SJA1105_PARAMETER_ERROR;
	if (dev->spi_handle->Init.FirstBit    != SPI_FIRSTBIT_MSB  ) status = SJA1105_PARAMETER_ERROR;

	/* If there are invalid parameters then return */
	if (status != SJA1105_OK) return status;

	/* Set pins to a known state */
	HAL_GPIO_WritePin(dev->rst_port, dev->rst_pin, SET);
	HAL_GPIO_WritePin(dev->cs_port,  dev->cs_pin,  SET);


	/* Configure the SJA1105 following the steps from UM11040 figure 2 */

	/* Step 1: RESET */
	SJA1105_Reset(dev);

	/* Step 2: ACU REGISTER SETUP */

	/* Step 3: CGU REGISTER SETUP */

	/* Step 4: SGMII PHY/PCS (optional) */

	/* Step 5: STATIC CONFIGURATION */
	status = SJA1105_WriteStaticConfig(dev, static_conf, static_conf_size);
	if (status != SJA1105_OK) return status;

	dev->initialised = true;

	return status;
}




/* Low-Level Functions */
SJA1105_StatusTypeDef SJA1105_ReadRegister(SJA1105_HandleTypeDef *dev, uint32_t addr, uint32_t *data, uint32_t size){

	SJA1105_StatusTypeDef status = SJA1105_OK;

	/* TODO: If a read follows a write to the same register then insert a 130ns delay here */

	/* Check the parameters */
	if (size == 0)                              status = SJA1105_PARAMETER_ERROR;  /* Empty check */
	if (addr & ~SJA1105_SPI_ADDR_MASK)          status = SJA1105_PARAMETER_ERROR;  /* Start address check */
	if ((addr + size) & ~SJA1105_SPI_ADDR_MASK) status = SJA1105_PARAMETER_ERROR;  /* End address check */
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
	if (size == 0)                              status = SJA1105_PARAMETER_ERROR;  /* Empty check */
	if (addr & ~SJA1105_SPI_ADDR_MASK)          status = SJA1105_PARAMETER_ERROR;  /* Start address check */
	if ((addr + size) & ~SJA1105_SPI_ADDR_MASK) status = SJA1105_PARAMETER_ERROR;  /* End address check */
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

void SJA1105_Reset(SJA1105_HandleTypeDef *dev){
	HAL_GPIO_WritePin(dev->rst_port, dev->rst_pin, RESET);
	dev->callbacks->callback_delay_ns(SJA1105_T_RST);  /* 5us delay */
	HAL_GPIO_WritePin(dev->rst_port, dev->rst_pin, SET);
	dev->callbacks->callback_delay_ms(1);  /* 329us minimum until SPI commands can be written. Use a 1ms non-blocking delay so the RTOS can do other work */
}

SJA1105_StatusTypeDef SJA1105_WriteStaticConfig(SJA1105_HandleTypeDef *dev, const uint32_t *static_conf, uint32_t static_conf_size){

	SJA1105_StatusTypeDef status = SJA1105_OK;

	/* Check the static config matches the device type */
	switch (dev->variant) {
	case VARIANT_SJA1105P:
	case VARIANT_SJA1105R:
		if (static_conf[0] != SJA1105PR_SWITCH_CORE_ID) status = SJA1105_STATIC_CONF_ERROR;
		break;
	case VARIANT_SJA1105Q:
	case VARIANT_SJA1105S:
		if (static_conf[0] != SJA1105QS_SWITCH_CORE_ID) status = SJA1105_STATIC_CONF_ERROR;
		break;
	default:
		break;
	}
	if (status != SJA1105_OK) return status;

	/* Write the config */
	status = (dev, SJA1105_STATIC_CONF_ADDR, static_conf, static_conf_size);
	if (status != SJA1105_OK) return status;

	return status;
}
