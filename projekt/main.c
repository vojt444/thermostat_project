/*
 * projekt.c
 *
 * Created: 31.03.2024 15:27:57
 * Author : xpiroc03
 */ 

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "lib/timer.h"
#include "lib/adc.h"
#include "lib/display.h"
#include "lib/encoder.h"
#include "lib/uart.h"
#include "lib/RTC.h"
#include "lib/EEPROM.h"
#include "lib/utils.h"
#include "lib/relay.h"

#define MAX_PAGE			6
#define MIN_HYSTERESIS		0.1
#define MAX_HYSTERESIS		2.0
#define MAX_CALIBRATION		5.0
#define COOLING_OFFSET		0.1
#define MAIN_PAGE			1
#define HYSTERESIS_PAGE		2
#define CALIBRATION_PAGE	3
#define DATE_PAGE			4
#define TIME_PAGE			5
#define MODE_PAGE			6

FILE lcd_out = FDEV_SETUP_STREAM(LCD_write_char, NULL, _FDEV_SETUP_WRITE);
FILE uart_out = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

static volatile bool read_temperature = true;
static volatile bool uart_received = false;
static volatile uint8_t uart_receive = 0;
bool is_heating_relay_on = false;
bool is_cooling_relay_on = false;

ISR(INT1_vect)
{
	read_temperature = true;
}

ISR(USART_RX_vect)
{
	uart_received = true;
}

int main(void)
{
	timerInit();
	ADC_init();
	LCD_init();
	USART_Init();
	TWI_init();
	RTC_init();
	RELAY_INIT();
	sei();
	
	uint8_t menu_page = 1;
	uint8_t mode = 0;
	float req_temp = 20;
	float hysteresis = 0.5;
	float calibration = 0;
	
	//seconds, minutes, hours, day of the week, day, month, year
	uint8_t buffer_get[BUFFER_SIZE] = {0x50, 0x15, 0x18, 0x01, 0x07, 0x04, 0x24};
	uint8_t buffer_set_date[3] = {0x01, 0x01, 0x01};
	uint8_t buffer_set_time[3] = {0x00, 0x00, 0x00};
	uint8_t set_time_index = 0;
	uint8_t set_date_index = 0;
	char set_time_string[3][10] = {"Sekundy", "Minuty", "Hodiny"};
	char set_date_string[3][10] = {"Den", "Mesic", "Rok"};
	char mode_string[4][16] = {"Top:ON Chla:ON ", "Top:OFF Chla:ON", "Top:ON Chla:OFF", "Top:OFF Chla:OF"};
	char heating_cooling_indicator = ' ';
	const uint8_t buffer_empty[16] = "                ";
	uint8_t uart_buffer[UART_BUFFER_SIZE] = {0};
	uint8_t uart_start_byte = 0;
	uint8_t uart_command_byte = 0;
	float tmp_float = 0;
			
	EEPROM_read_data(0x0000, buffer_get, sizeof(buffer_get));
	EEPROM_read_data(TIME_ADDRESS, buffer_set_time, sizeof(buffer_set_time));
	EEPROM_read_data(DATE_ADDRESS, buffer_set_date, sizeof(buffer_set_date));
	EEPROM_read_float(HYST_ADDRESS, &hysteresis);
	EEPROM_read_float(CALIB_ADDRESS, &calibration);
	
	float act_temp = get_temperature(ADC_read_voltage(1)) + calibration;
	RTC_set(buffer_get);
	LCD_clear();

    while(1)
    {
		if(uart_received == true)
		{
			uart_start_byte = USART_Receive();
			switch(uart_start_byte)
			{
				case 'Z':
					uart_command_byte = USART_Receive();
					switch(uart_command_byte)
					{
						case 'T':
							for(uint8_t i = 0; i < 5; i++)
								uart_buffer[i] = USART_Receive();
							tmp_float = (float)atof((char*)uart_buffer);
							req_temp = tmp_float;
							fprintf(&uart_out, "Pozadovana teplota nastavena na: %.1f C\r\n", req_temp);
							break;
						case 'H':
							for(uint8_t i = 0; i < 3; i++)
								uart_buffer[i] = USART_Receive();
							tmp_float = (float)atof((char*)uart_buffer);
							hysteresis = (tmp_float <= MAX_HYSTERESIS && tmp_float >= MIN_HYSTERESIS) ? (tmp_float) : hysteresis;
							EEPROM_write_float(HYST_ADDRESS, &hysteresis);
							fprintf(&uart_out, "Hystereze nastavena na: %.1f C\r\n", hysteresis);
							break;
						case 'K':
							for(uint8_t i = 0; i < 4; i++)
								uart_buffer[i] = USART_Receive();
							tmp_float = (float)atof((char*)uart_buffer);
							calibration = (tmp_float <= MAX_CALIBRATION && tmp_float >= -MAX_CALIBRATION) ? (tmp_float) : calibration;
							EEPROM_write_float(CALIB_ADDRESS, &calibration);
							fprintf(&uart_out, "Kalibrace nastavena na: %.1f C\r\n", calibration);
							break;
						case 'D':
							for(uint8_t i = 0; i < 8; i++)
								uart_buffer[i] = USART_Receive();
							buffer_set_date[0] = ((uart_buffer[0] - '0') << 4) | (uart_buffer[1] - '0');
							buffer_set_date[1] = ((uart_buffer[3] - '0') << 4) | (uart_buffer[4] - '0');
							buffer_set_date[2] = ((uart_buffer[6] - '0') << 4) | (uart_buffer[7] - '0');
							RTC_set_date(buffer_set_date);
							EEPROM_write_data(DATE_ADDRESS, buffer_set_date, sizeof(buffer_set_date));
							fprintf(&uart_out, "Datum nastaven na: %x.%x.%x\r\n", buffer_set_date[0], buffer_set_date[1], buffer_set_date[2]);
							break;
						case 'C':
							for(uint8_t i = 0; i < 8; i++)
								uart_buffer[i] = USART_Receive();
							buffer_set_time[2] = ((uart_buffer[0] - '0') << 4) | (uart_buffer[1] - '0');
							buffer_set_time[1] = ((uart_buffer[3] - '0') << 4) | (uart_buffer[4] - '0');
							buffer_set_time[0] = ((uart_buffer[6] - '0') << 4) | (uart_buffer[7] - '0');
							RTC_set_time(buffer_set_time);
							EEPROM_write_data(TIME_ADDRESS, buffer_set_time, sizeof(buffer_set_time));
							fprintf(&uart_out, "Datum nastaven na: %x:%x:%x\r\n", buffer_set_time[2], buffer_set_time[1], buffer_set_time[0]);
							break;
						case 'M':
							uart_buffer[0] = USART_Receive() - '0';
							mode = (uart_buffer[0] <= 3 && uart_buffer[0] >= 0) ? (uart_buffer[0]) : mode;
							fprintf(&uart_out, "Mod nastaven na: %d\r\n", mode);
							break;
						default:
							fprintf(&uart_out, "%s\r\n", "Prikazovy byte je neplatny!");
							break;
					}
					break;
				case 'C':
					uart_command_byte = USART_Receive();
					switch(uart_command_byte)
					{
						case 'T':
							uart_command_byte = USART_Receive();
							if(uart_command_byte == 'A')
								fprintf(&uart_out, "Aktualni teplota je: %.1f C\r\n", act_temp);
							else if(uart_command_byte == 'P')
								fprintf(&uart_out, "Pozadovana teplota je: %.1f C\r\n", req_temp);
							else
								fprintf(&uart_out, "%s", "Prikazovy byte 2 je neplatny!\r\n");
							break;
						case 'H':
							fprintf(&uart_out, "Aktualni hystereze je: %.1f C\r\n", hysteresis);
							break;
						case 'K':
							fprintf(&uart_out, "Aktualni kalibrace je: %.1f C\r\n", calibration);
							break;
						case 'D':
							fprintf(&uart_out, "Aktualni datum je: %x.%x.%x\r\n", buffer_get[4], buffer_get[5], buffer_get[6]);
							break;
						case 'C':
							fprintf(&uart_out, "Aktualni cas je: %x:%x:%x\r\n", buffer_get[2], buffer_get[1], buffer_get[0]);
							break;
						case 'M':
							fprintf(&uart_out, "Aktualni mod je: %d\r\n", mode);
							break;
						default:
							fprintf(&uart_out, "%s\r\n", "Prikazovy byte je neplatny!");
							break;
					}
					break;
				default:
					fprintf(&uart_out, "%s\r\n", "Start byte je neplatny!");
					break;
			}
			uart_received = false;
		}
		
		if(read_temperature == true)
		{
			act_temp = get_temperature(ADC_read_voltage(1)) + calibration; //read temperature
			read_temperature = false;
		}	
		
		switch(mode) //switch for setting relay
		{
			case 0:
				if(act_temp < (req_temp - hysteresis - COOLING_OFFSET))
					set_relay_heating();
				else if(act_temp > (req_temp + hysteresis))
					reset_relay_heating();
				if(act_temp > (req_temp + hysteresis + COOLING_OFFSET))
					set_relay_cooling();
				else if(act_temp < (req_temp - hysteresis))
					reset_relay_cooling();
				break;
			case 1:
				reset_relay_heating();
				if(act_temp > (req_temp + hysteresis))
					set_relay_cooling();
				else if(act_temp < (req_temp - hysteresis))
					reset_relay_cooling();
				break;
			case 2:
				reset_relay_cooling();
				if(act_temp < (req_temp - hysteresis))
					set_relay_heating();
				else if(act_temp > (req_temp + hysteresis))
					reset_relay_heating();
				break;
			case 3:
				reset_relay_heating();
				reset_relay_cooling();
				break;
			default:
				break;
		}
		
		if(is_heating_relay_on == true && is_cooling_relay_on == false)
			heating_cooling_indicator = 'T';
		else if(is_heating_relay_on == false && is_cooling_relay_on == true)
			heating_cooling_indicator = 'C';
		else if(is_heating_relay_on == true && is_cooling_relay_on == true)
			heating_cooling_indicator = 'E';
		else
			heating_cooling_indicator = ' ';
				
		switch(buttons_read()) //switch for buttons handling
		{
			case BUTTON_DOWN: 
				menu_page = (menu_page < MAX_PAGE) ? (menu_page + 1) : menu_page;
				switch(menu_page)
				{
					case DATE_PAGE:
						RTC_get_date(buffer_set_date);
						break;
					case TIME_PAGE:
						RTC_get_time(buffer_set_time);
						break;
					default:
						break;
				}
				break;
			case BUTTON_UP:
				menu_page = (menu_page > 1) ? (menu_page - 1) : menu_page;
				switch(menu_page)
				{
					case DATE_PAGE:
						RTC_get_date(buffer_set_date);
						break;
					case TIME_PAGE:
						RTC_get_time(buffer_set_time);
						break;
					default:
						break;
				}
				break;
			case BUTTON_RIGHT:
				switch(menu_page)
				{
					case MAIN_PAGE:
						req_temp = req_temp + 0.1;
						break;
					case HYSTERESIS_PAGE:
						hysteresis = (hysteresis < MAX_HYSTERESIS) ? (hysteresis + 0.1) : hysteresis;
						EEPROM_write_float(HYST_ADDRESS, &hysteresis);
						break;
					case CALIBRATION_PAGE:
						calibration = (calibration < MAX_CALIBRATION) ? (calibration + 0.1) : calibration;
						EEPROM_write_float(CALIB_ADDRESS, &calibration);
						break;
					case DATE_PAGE:
						switch(set_date_index)
						{
							case 0:
								buffer_set_date[0] = (buffer_set_date[0] < 0x31) ? BCD_increment(buffer_set_date[0]) : buffer_set_date[0];
								break;
							case 1:
								buffer_set_date[1] = (buffer_set_date[1] < 0x12) ? BCD_increment(buffer_set_date[1]) : buffer_set_date[1];
								break;
							case 2:
								buffer_set_date[2] = (buffer_set_date[2] < 0x99) ? BCD_increment(buffer_set_date[2]) : buffer_set_date[2];
								break;
							default:
								break;
						}
						RTC_set_date(buffer_set_date);
						EEPROM_write_data(DATE_ADDRESS, buffer_set_date, 3);
						break;
					case TIME_PAGE: 
						switch(set_time_index)
						{
							case 0:
								buffer_set_time[0] = (buffer_set_time[0] < 0x59) ? BCD_increment(buffer_set_time[0]) : buffer_set_time[0];
								break;
							case 1:
								buffer_set_time[1] = (buffer_set_time[1] < 0x59) ? BCD_increment(buffer_set_time[1]) : buffer_set_time[1];
								break;
							case 2:
								buffer_set_time[2] = (buffer_set_time[2] < 0x23) ? BCD_increment(buffer_set_time[2]) : buffer_set_time[2];
								break;
							default:
								break;
						}
						RTC_set_time(buffer_set_time);
						EEPROM_write_data(TIME_ADDRESS, buffer_set_time, 3);
						break;
					case MODE_PAGE: 
						mode = (mode < 3) ? (mode + 1) : mode;
						break;
					default:
						break;
				}
				break;
			case BUTTON_LEFT:
				switch(menu_page)
				{
					case MAIN_PAGE:
						req_temp = req_temp - 0.1;
						break;
					case HYSTERESIS_PAGE:
						hysteresis = (hysteresis > MIN_HYSTERESIS) ? (hysteresis - 0.1) : hysteresis;
						if(hysteresis < 0.1)
							hysteresis = 0.1;
						EEPROM_write_float(HYST_ADDRESS, &hysteresis);
						break;
					case CALIBRATION_PAGE:
						calibration = (calibration > -MAX_CALIBRATION) ? (calibration - 0.1) : calibration;
						EEPROM_write_float(CALIB_ADDRESS, &calibration);
						break;
					case DATE_PAGE:
						switch(set_date_index)
						{
							case 0:
								buffer_set_date[0] = (buffer_set_date[0] > 1) ? BCD_decrement(buffer_set_date[0]) : buffer_set_date[0];
								break;
							case 1:
								buffer_set_date[1] = (buffer_set_date[1] > 1) ? BCD_decrement(buffer_set_date[1]) : buffer_set_date[1];
								break;
							case 2:
								buffer_set_date[2] = (buffer_set_date[2] > 1) ? BCD_decrement(buffer_set_date[2]) : buffer_set_date[2];
								break;
							default:
								break;
						}
						RTC_set_date(buffer_set_date);
						EEPROM_write_data(DATE_ADDRESS, buffer_set_date, 3);
						break;
					case TIME_PAGE:
						switch(set_time_index)
						{
							case 0:
								buffer_set_time[0] = (buffer_set_time[0] > 0) ? BCD_decrement(buffer_set_time[0]) : buffer_set_time[0];
								break;
							case 1:
								buffer_set_time[1] = (buffer_set_time[1] > 0) ? BCD_decrement(buffer_set_time[1]) : buffer_set_time[1];
								break;
							case 2:
								buffer_set_time[2] = (buffer_set_time[2] > 0) ? BCD_decrement(buffer_set_time[2]) : buffer_set_time[2];
								break;
							default:
								break;
						}
						RTC_set_time(buffer_set_time);
						EEPROM_write_data(TIME_ADDRESS, buffer_set_time, 3);
						break;
					case MODE_PAGE:
						mode = (mode > 0) ? (mode - 1) : mode;
						break;
					default:
						break;
				}
				break;
			case BUTTON_SELECT:
				switch(menu_page)
				{
					case DATE_PAGE:
						set_date_index = (set_date_index < 2) ? (set_date_index + 1) : 0;
						break;
					case TIME_PAGE:
						set_time_index = (set_time_index < 2) ? (set_time_index + 1) : 0;
						break;
					default:
						break;
				}
				break;
			default:
				break;
		}
		
		switch(menu_page) //switch for display handling
		{
			case MAIN_PAGE:
				RTC_get(buffer_get);
				LCD_set_cursor(0,0);
				fprintf(&lcd_out, "%02x.%02x.%02x %02x:%02x %c", buffer_get[4], buffer_get[5], buffer_get[6], buffer_get[2], buffer_get[1], heating_cooling_indicator);
				LCD_set_cursor(1,0);
				fprintf(&lcd_out, "%.1fßC   %.1fßC  ", act_temp, req_temp); //ß symbol is ° on LCD
				break;
			case HYSTERESIS_PAGE:
				LCD_set_cursor(0,0);
				fprintf(&lcd_out, "Hystereze:%.1fßC ", hysteresis);
				LCD_set_cursor(1,0);
				fprintf(&lcd_out, "%s", buffer_empty);
				break;
			case CALIBRATION_PAGE:
				LCD_set_cursor(0,0);
				fprintf(&lcd_out, "Kalibrace:%.1fßC ", calibration);
				LCD_set_cursor(1,0);
				fprintf(&lcd_out, "%s", buffer_empty);
				break;
			case DATE_PAGE:
				LCD_set_cursor(0,0);
				fprintf(&lcd_out, "Datum - %s     ", set_date_string[set_date_index]);
				LCD_set_cursor(1,0);
				fprintf(&lcd_out, "%02x.%02x.%02x        ", buffer_set_date[0], buffer_set_date[1], buffer_set_date[2]);
				break;
			case TIME_PAGE:
				LCD_set_cursor(0,0);
				fprintf(&lcd_out, "Cas - %s    ", set_time_string[set_time_index]);
				LCD_set_cursor(1,0);
				fprintf(&lcd_out, "%02x:%02x:%02x        ", buffer_set_time[2], buffer_set_time[1], buffer_set_time[0]);
				break;
			case MODE_PAGE:
				LCD_set_cursor(0,0);
				fprintf(&lcd_out, "%s   ", "Mod           ");
				LCD_set_cursor(1,0);
				fprintf(&lcd_out, "%s  ", mode_string[mode]);
				break;
			default:
				break;
		}
    }
}
 