/*
 * EEPROM.h
 *
 * Created: 27.03.2024 20:46:51
 *  Author: xpiroc03
 */ 


#pragma once

#include <stdbool.h>
#include "twi.h"

#define EEPROM_ADDRESS	0x50
#define TIME_ADDRESS	0x0000
#define DATE_ADDRESS	0x0004
#define HYST_ADDRESS	0x0008
#define CALIB_ADDRESS	0x000B

void EEPROM_write_data(uint16_t first_word_address, uint8_t *data, uint8_t size);
void EEPROM_read_data(uint16_t first_word_address, uint8_t *data, uint8_t size);
void EEPROM_write_float(uint16_t first_word_address, float *data);
void EEPROM_read_float(uint16_t first_word_address, float *data);

uint8_t EEPROM_transmit(uint8_t address, uint16_t word_address, uint8_t *data, uint8_t length);
uint8_t EEPROM_receive(uint8_t address, uint16_t word_address, uint8_t *data, uint8_t length);

uint8_t EEPROM_transmit_float(uint8_t address, uint16_t word_address, float *data);
uint8_t EEPROM_receive_float(uint8_t address, uint16_t word_address, float *data);

