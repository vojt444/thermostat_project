/*
 * utils.c
 *
 * Created: 06.04.2024 20:17:02
 *  Author: xpiroc03
 */ 

#include "utils.h"

uint8_t BCD_to_decimal(uint8_t bcd_value)
{
	return ((bcd_value >> 4) * 10) + (bcd_value & 0x0F);
}

uint8_t BCD_increment(uint8_t number)
{
	uint8_t tens = number >> 4;
	uint8_t ones = number & 0x0F;
	ones++;
	
	if(ones > 9)
	{
		ones = 0;
		tens++;
	}
	if(tens > 9)
		tens = 0;
	
	return (tens << 4) | (ones & 0x0F);
}

uint8_t BCD_decrement(uint8_t number)
{
	uint8_t tens = number >> 4;
	uint8_t ones = number & 0x0F;
		
	if(ones == 0)
	{
		ones = 9;
		if(tens == 0)
			tens = 9;
		else
			tens--;
	}
	else
		ones--;
	
	return (tens << 4) | (ones & 0x0F);
}

