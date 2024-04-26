/*
 * EEPROM.c
 *
 * Created: 27.03.2024 20:46:41
 *  Author: xpiroc03
 */ 

#include "EEPROM.h"


void EEPROM_write_data(uint16_t first_word_address, uint8_t *data, uint8_t size)
{
	EEPROM_transmit(EEPROM_ADDRESS, first_word_address, data, size);
}

void EEPROM_read_data(uint16_t first_word_address, uint8_t *data, uint8_t size)
{
	EEPROM_receive(EEPROM_ADDRESS, first_word_address, data, size);
}

void EEPROM_write_float(uint16_t first_word_address, float *data)
{
	EEPROM_transmit_float(EEPROM_ADDRESS, first_word_address, data);
}

void EEPROM_read_float(uint16_t first_word_address, float *data)
{
	EEPROM_receive_float(EEPROM_ADDRESS, first_word_address, data);
}

uint8_t EEPROM_transmit(uint8_t address, uint16_t word_address, uint8_t *data, uint8_t length)
{
	TWI_start(address, 0);
	if(twi_error_code != 0)
		return twi_error_code;
	
	TWI_send_byte((uint8_t)(word_address >> 8));
	if(twi_error_code != 0)
		return twi_error_code;
	
	TWI_send_byte((uint8_t)(word_address & 0xFF));
	if(twi_error_code != 0)
		return twi_error_code;
	
	for(uint8_t i = 0; i < length; i++)
	{
		TWI_send_byte(data[i]);
		if(twi_error_code != 0)
			return twi_error_code;
	}
	TWI_stop();
	return TWI_NO_ERR;
}

uint8_t EEPROM_receive(uint8_t address, uint16_t word_address, uint8_t *data, uint8_t length)
{
	TWI_start(address, 0);
	if(twi_error_code != 0)
		return twi_error_code;
	
	TWI_send_byte((uint8_t)(word_address >> 8));
	if(twi_error_code != 0)
		return twi_error_code;
	
	TWI_send_byte((uint8_t)(word_address & 0xFF));
	if(twi_error_code != 0)
		return twi_error_code;
	
	TWI_start(address, 1);
	if(twi_error_code != 0)
		return twi_error_code;
	
	for(uint8_t i = 0; i < (length - 1); i++)
	{
		TWI_read_byte(1);
		data[i] = TWDR;
		if(twi_error_code != 0)
			return twi_error_code;
	}
	TWI_read_byte(0);
	data[length - 1] = TWDR;
	TWI_stop();
	return TWI_NO_ERR;
}

uint8_t EEPROM_transmit_float(uint8_t address, uint16_t word_address, float *data)
{
	int value = (int)(*data * 10);
	uint8_t is_negative = 0;
	
	if(value < 0)
	{
		is_negative = 1;
		value = value * (-1);
	}
	
	TWI_start(address, 0);
	if(twi_error_code != 0)
		return twi_error_code;
	
	TWI_send_byte((uint8_t)(word_address >> 8));
	if(twi_error_code != 0)
		return twi_error_code;
	
	TWI_send_byte((uint8_t)(word_address & 0xFF));
	if(twi_error_code != 0)
		return twi_error_code;
	
	TWI_send_byte(value);
	if(twi_error_code != 0)
		return twi_error_code;
		
	if(is_negative == 1)
	{
		TWI_send_byte(1);
		if(twi_error_code != 0)
			return twi_error_code;
	}
	else
	{
		TWI_send_byte(0);
		if(twi_error_code != 0)
			return twi_error_code;
	}
	
	TWI_stop();
	return TWI_NO_ERR;
}

uint8_t EEPROM_receive_float(uint8_t address, uint16_t word_address, float *data)
{
	float value = 0;
	uint8_t is_negative = 0;
	
	
	TWI_start(address, 0);
	if(twi_error_code != 0)
		return twi_error_code;
	
	TWI_send_byte((uint8_t)(word_address >> 8));
	if(twi_error_code != 0)
		return twi_error_code;
	
	TWI_send_byte((uint8_t)(word_address & 0xFF));
	if(twi_error_code != 0)
		return twi_error_code;
	
	TWI_start(address, 1);
	if(twi_error_code != 0)
		return twi_error_code;
	
	TWI_read_byte(1);
	value = TWDR;
	if(twi_error_code != 0)
		return twi_error_code;
	
	TWI_read_byte(0);
	is_negative = TWDR;
	
	TWI_stop();
	
	if(is_negative == 1)
		*data = (value/10*(-1));
	else
		*data = (value/10);
		
	return TWI_NO_ERR;
}