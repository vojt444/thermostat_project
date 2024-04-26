/*
 * keypad.c
 *
 * Created: 20.03.2024 20:18:40
 *  Author: xpiroc03
 */ 

#include "keypad.h"

char keypad[ROWS][COLS] = {
	{'1', '2', '3', 'A'},
	{'4', '5', '6', 'B'},
	{'7', '8', '9', 'C'},
	{'*', '0', '#', 'D'}
};

void keypad_init(void)
{
	DDRD |= (1<<DDD2) | (1<<DDD3); //row 1,2
	DDRB |= (1<<DDB3) | (1<<DDB4); //row 3,4
	DDRC &= ~((1<<DDC1) | (1<<DDC2) | (1<<DDC3) | (1<<DDC4)); //col 1,2,3,4
	
	PORTD |= (1<<PORTD2) | (1<<PORTD3);
	PORTB |= (1<<PORTB3) | (1<<PORTB4);
	PORTC |= (1<<PORTC1) | (1<<PORTC2) | (1<<PORTC3) | (1<<PORTC4);
}

char keypad_get_key(void)
{
	char key = '\0';
	
	for(uint8_t i = 0; i < ROWS; i++)
	{
		if(i < 2)
			PORTD &= ~(1<<(PORTD2 + i));
		else
			PORTB &= ~(1<<(PORTB1 + i));
		
		for(uint8_t j = 0; j < COLS; j++)
		{
			if(!(PINC & (1 << (PINC1 + j))))
			{
				while(!(PINC & (1 << (PINC1 + j))));
				delay(30000);
				if(i < 2)
					PORTD |= (1<<(PORTD2 + i));
				else
					PORTB |= (1<<(PORTB1 + i));
				key = keypad[i][j];
				break;
			}			
		}
		if(i < 2)
			PORTD |= (1<<(PORTD2 + i));
		else
			PORTB |= (1<<(PORTB1 + i));
		if(key != '\0')
			break;
	}
	return key;
}

