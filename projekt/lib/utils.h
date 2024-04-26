/*
 * utils.h
 *
 * Created: 06.04.2024 20:17:11
 *  Author: xpiroc03
 */ 


#pragma once

#include <avr/io.h>

uint8_t BCD_to_decimal(uint8_t bcd_value);
uint8_t BCD_increment(uint8_t number);
uint8_t BCD_decrement(uint8_t number);
