/*
 * twi.c
 *
 * Created: 26.03.2024 18:28:11
 *  Author: xpiroc03
 */ 

#include "twi.h"

volatile uint8_t twi_error_code = 0;

void TWI_init(void)
{
	TWBR = (uint8_t)TWBR_VALUE;
	TWCR = (1 << TWEN);
	
	PORTC |= (1 << PORTC4 | 1 << PORTC5);
}

void TWI_start(uint8_t address, uint8_t receive)
{
	uint16_t timeout = TWI_TIMEOUT;
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN) | (1 << TWEA);
	while(!(TWCR & (1 << TWINT)) && (timeout != 0))
	{
		timeout--;
		if(timeout == 0)
		{
			twi_error_code = TWI_START_ERR;
			return;	
		}
	}
	
	if(receive == 0)
		TWDR = (address << 1);
	else
		TWDR = (address << 1) | 1;
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
	
	timeout = TWI_TIMEOUT;
	while(!(TWCR & (1 << TWINT)) && (timeout != 0))
	{
		timeout--;
		if(timeout == 0)
		{
			twi_error_code = TWI_ADDRESS_ERR;
			return;
		}
	}
	
	twi_error_code = TWI_NO_ERR;
}

void TWI_stop(void)
{
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

void TWI_send_byte(uint8_t data)
{
	TWDR = data;
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
	uint16_t timeout = TWI_TIMEOUT;
	while(!(TWCR & (1 << TWINT)) && (timeout != 0))
	{
		timeout--;
		if(timeout == 0)
		{
			twi_error_code = TWI_BYTE_ERR;
			return;
		}
	}
	twi_error_code = TWI_NO_ERR;
}

void TWI_read_byte(bool ack)
{
	TWCR = (1 << TWINT) | (1 << TWEN) | (ack ? (1 << TWEA) : 0);
	uint16_t timeout = TWI_TIMEOUT;
	while(!(TWCR & (1 << TWINT)) && (timeout != 0))
	{
		timeout--;
		if(timeout == 0)
		{
			twi_error_code = TWI_BYTE_ERR;
			return;
		}
	}
	twi_error_code = TWI_NO_ERR;
}
