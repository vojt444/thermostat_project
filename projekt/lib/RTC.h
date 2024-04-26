/*
 * RTC.h
 *
 * Created: 27.03.2024 1:03:31
 *  Author: xpiroc03
 */ 


#pragma once

#include "twi.h"
#include <avr/interrupt.h>

#define RTC_ADDRESS		0x68
#define BUFFER_SIZE		7
#define YEAR_OFFSET		2000

#define SECONDS			0x00
#define MINUTES			0x01
#define HOURS			0x02
#define DATE			0x03
#define DAY				0x04
#define MONTH			0x05
#define YEAR			0x06
#define CONTROL			0x07

void RTC_init(void);
void RTC_set_time(uint8_t *data);
void RTC_get_time(uint8_t *data);
void RTC_set_date(uint8_t *data);
void RTC_get_date(uint8_t *data);
//void RTC_set_day(uint8_t data);
//void RTC_set_month(uint8_t data);
//void RTC_set_year(uint8_t data);
void RTC_set(uint8_t *data);
void RTC_get(uint8_t *data);

uint8_t RTC_transmit(uint8_t address, uint8_t reg_addr, uint8_t *data, uint8_t length);
uint8_t RTC_receive(uint8_t address, uint8_t reg_addr, uint8_t *data, uint8_t length);

