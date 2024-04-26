/*
 * encoder.h
 *
 * Created: 18.03.2024 23:36:24
 *  Author: xpiroc03
 */ 

#pragma once

#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer.h"
#include "display.h"

#define CLK_PIN		PIND2
#define DT_PIN		PINB3
#define BUTTON_PIN	PINB2
//#define CURRENT_CLK	(PIND & (1<<CLK_PIN))
#define CURRENT_DT	(PINB & (1<<DT_PIN))

extern volatile int pulse_count;
extern volatile uint16_t button_count;
extern volatile uint8_t current_clk;
extern volatile uint8_t last_clk;

void encoder_init(void);
void encoder_update(void);

