#define F_CPU 1000000UL
#include <stdio.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include "Lcdlbr/lcd.h"


void adc_init()
{
	ADMUX = (1<<REFS0);
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(0<<ADPS0);
}
uint16_t adc_read(uint8_t ch)
{
	ch &= 0b00000111;
	ADMUX = (ADMUX & 0xF8)|ch;
	ADCSRA |= (1<<ADSC);
	while(ADCSRA & (1<<ADSC));
	return (ADC);
}
int main()
{
	DDRB=0x00;
	PORTB=0b10010010;
	DDRD=0xff;
	uint16_t adc0, adc1;
	uint8_t wh,wl,rh,rl ;
	int temp,far,s,a,b;
	char buffer[10];
	adc_init();
	lcd_init(LCD_DISP_ON_CURSOR);
	lcd_clrscr();
	lcd_gotoxy(0,0);
	lcd_puts("Starting System");
	lcd_gotoxy(0,1);
	lcd_puts(".");
	_delay_ms(100);
	lcd_puts("..");
	_delay_ms(100);
	lcd_puts("...");
	_delay_ms(100);
	lcd_puts("....");
	_delay_ms(100);
	lcd_puts(".....");
	_delay_ms(100);
	while (1)
	{
		lcd_clrscr();
		lcd_gotoxy(0,0);
		lcd_puts("System Started");
		lcd_gotoxy(0,1);
		lcd_puts("Initial?Reload?");
		_delay_ms(300);
		if((PINB & (1<<7))==0)//rel
		{
			rh = eeprom_read_byte((uint8_t*)0);
			a=rh;
			rl = eeprom_read_byte((uint8_t*)4);
			b=rl;
			_delay_ms(300);
			goto measure;
		}
		if((PINB & (1<<4))==0)//ini
		{
			eeprom_update_byte ((uint8_t*) 0, 30);
			rh = eeprom_read_byte((uint8_t*)0);
			a=rh;
			eeprom_update_byte ((uint8_t*) 4, 15);
			rl = eeprom_read_byte((uint8_t*)4);
			b=rl;
			while(1)
			{
				high:
				lcd_clrscr();
				lcd_gotoxy(0,0);
				lcd_puts("High temp=");
				itoa(a,buffer,10);
				lcd_puts(buffer);
				lcd_gotoxy(12,0);
				lcd_puts("'C");
				lcd_gotoxy(0,1);
				lcd_puts("Set? Inc? Dec?");
				_delay_ms(300);
				while((PINB & (1<<4))==0)
				{
					_delay_ms(300);
					a++;
				}
				while((PINB & (1<<7))==0)
				{
					_delay_ms(300);
					a--;
				}
				while((PINB & (1<<1))==0)//enter
				{
					while(1)
					{
						lcd_clrscr();
						lcd_home();
						lcd_gotoxy(0,0);
						lcd_puts("Low temp=");
						itoa(b,buffer,10);
						lcd_puts(buffer);
						lcd_gotoxy(11,0);
						lcd_puts("'C");
						lcd_gotoxy(0,1);
						lcd_puts("Set? Inc? Dec?");
						_delay_ms(300);
						while((PINB & (1<<4))==0)
						{
							_delay_ms(300);
							b++;
						}
						while((PINB & (1<<7))==0)
						{
							_delay_ms(300);
							b--;
						}
						while((PINB & (1<<1))==0)//enter
						{
							while(1)//3is
							{
								measure:
								adc0 = adc_read(0);
								adc1 = adc_read(6);
								s = adc0 - 2 * adc1;
								temp = s - 259;
								far=(1.8*temp)+32;
								lcd_gotoxy(0,0);
								itoa(temp,buffer,10);
								lcd_puts("Temp=");
								lcd_puts(buffer);
								lcd_gotoxy(7,0);
								lcd_puts("'C");
								lcd_gotoxy(11,0);
								itoa(far,buffer,10);
								lcd_puts(buffer);
								lcd_gotoxy(14,0);
								lcd_puts("'F");
								_delay_ms(300);
								if(temp>=a)
								{
									lcd_clrscr();
									lcd_home();
									lcd_gotoxy(0,1);
									lcd_puts("High Temp:FAN ON ");
									PORTD=(1<<PIND0);
								}
								else if(temp<=b)
								{
									lcd_clrscr();
									lcd_home();
									lcd_gotoxy(0,1);
									lcd_puts("Low Temp:HEAT ON");
									PORTD=(1<<PIND7);
								}
								else
								{
									lcd_clrscr();
									lcd_home();
									lcd_gotoxy(0,1);
									lcd_puts("Optimal Temp");
									PORTD=(0<<PIND0);
									PORTD=(0<<PIND7);
								}
								if((PINB & (1<<1))==0)
								{
									_delay_ms(300);
									goto high;
								}
								wh=a;
								eeprom_update_byte ((uint8_t*) 0, wh);
								wl=b;
								eeprom_update_byte ((uint8_t*) 4, wl);
							}//Temp measure
						}//lt set
					}//Low temp
				}//ht set
			}//High temp
		}
	}
}