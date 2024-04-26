/*
 * RTC.c
 *
 * Created: 27.03.2024 1:03:23
 *  Author: xpiroc03
 */ 
#include "RTC.h"


void RTC_init(void)
{
	uint8_t data = {0x10};
	RTC_transmit(RTC_ADDRESS, CONTROL, &data, 1);
	EICRA |= (1 << ISC11);
	EIMSK |= (1 << INT1);
	PORTD |= (1 << PORTD3);
}

void RTC_set_time(uint8_t *data)
{
	RTC_transmit(RTC_ADDRESS, SECONDS, data, 3);
}

void RTC_get_time(uint8_t *data)
{
	RTC_receive(RTC_ADDRESS, SECONDS, data, 3);
}

void RTC_set_date(uint8_t *data)
{
	RTC_transmit(RTC_ADDRESS, DAY, data, 3);
}

void RTC_get_date(uint8_t *data)
{
	RTC_receive(RTC_ADDRESS, DAY, data, 3);
}

//void RTC_set_day(uint8_t data)
//{
	//RTC_transmit(RTC_ADDRESS, DAY, data, 1);
//}
//void RTC_set_month(uint8_t data)
//{
	//RTC_transmit(RTC_ADDRESS, MONTH, data, 1);
//}
//void RTC_set_year(uint8_t data)
//{
	//RTC_transmit(RTC_ADDRESS, YEAR, data, 1);
//}

void RTC_set(uint8_t *data)
{
	RTC_transmit(RTC_ADDRESS, SECONDS, data, BUFFER_SIZE);
}

void RTC_get(uint8_t *data)
{
	RTC_receive(RTC_ADDRESS, SECONDS, data, BUFFER_SIZE);
}

uint8_t RTC_transmit(uint8_t address, uint8_t reg_addr, uint8_t *data, uint8_t length)
{
	TWI_start(address, 0);
	if(twi_error_code != 0)
		return twi_error_code;
	
	TWI_send_byte(reg_addr);
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

uint8_t RTC_receive(uint8_t address, uint8_t reg_addr, uint8_t *data, uint8_t length)
{
	TWI_start(address, 0);
	if(twi_error_code != 0)
		return twi_error_code;
	
	TWI_send_byte(reg_addr);
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