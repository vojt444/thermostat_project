/*
 * twi.h
 *
 * Created: 26.03.2024 18:28:24
 *  Author: xpiroc03
 */ 


#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <avr/io.h>
#include "timer.h"

#ifndef F_CPU
 #define F_CPU 16000000
#endif
#define F_TWI			100000
#define TWI_PRESCALER	1
#define TWBR_VALUE		((F_CPU / F_TWI) - 16) / (TWI_PRESCALER * 2)
#define TWI_TIMEOUT		3000

#define TWI_NO_ERR			0
#define TWI_START_ERR		1
#define TWI_ADDRESS_ERR		2
#define TWI_BYTE_ERR		3

extern volatile uint8_t twi_error_code;


void TWI_init(void);
void TWI_start(uint8_t address, uint8_t receive);
void TWI_stop(void);
void TWI_send_byte(uint8_t data);
void TWI_read_byte(bool ack);
uint8_t TWI_transmit_RTC(uint8_t address, uint8_t reg_addr, uint8_t *data, uint8_t length);
uint8_t TWI_receive_RTC(uint8_t address, uint8_t reg_addr, uint8_t *data, uint8_t length);

uint8_t TWI_transmit_EEPROM(uint8_t address, uint16_t word_address, uint8_t *data, uint8_t length);
uint8_t TWI_receive_EEPROM(uint8_t address, uint16_t word_address, uint8_t *data, uint8_t length);