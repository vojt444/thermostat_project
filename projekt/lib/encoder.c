/*
 * encoder.c
 *
 * Created: 18.03.2024 23:36:09
 *  Author: xpiroc03
 */ 
#include "encoder.h"

void encoder_init(void)
{
	EICRA |= (1<<ISC01) | (1<<ISC00);
	EIMSK |= (1<<INT0);
	
	DDRB &= ~((1<<DT_PIN) | (1<<BUTTON_PIN));
	DDRD &= ~(1<<CLK_PIN); 
	PORTB |= ((1<<DT_PIN) | (1<<BUTTON_PIN));
	PORTD |= (1<<CLK_PIN);
}

void encoder_update(void)
{
	current_clk = (PIND & (1 << CLK_PIN));
	if((current_clk != last_clk) && (current_clk >> CLK_PIN) == 1)
	{
		if(((PINB & (1 << DT_PIN)) >> DT_PIN) != (current_clk >> CLK_PIN))
		{
			pulse_count--;
		}
		else
		{
			pulse_count++;
		}
	}
	last_clk = current_clk;
}
