/*
 * keypad.h
 *
 * Created: 20.03.2024 20:18:29
 *  Author: xpiroc03
 */ 
#pragma once
#include <avr/io.h>
#include "timer.h"

#define ROWS 4
#define COLS 4

void keypad_init(void);
char keypad_get_key(void);