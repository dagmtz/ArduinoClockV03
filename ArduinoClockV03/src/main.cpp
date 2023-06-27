/*
 * GccApplication1.c
 *
 * Created: 22/06/2023 04:20:07 p. m.
 * Author : dagmtz
 */

#include <Arduino.h>
#include <Wire.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "clock.h"

volatile bool g_heartbeat_s = 0;

void setup()
{

}

void loop()
{
	clock_control_t clockControl = {STOP, {0, 0, 0}, {2000, 1, 1}, SATURDAY, DAYS_PER_MONTH_MAX, "00:00:00", "2000-01-01"};
	settings_control_t settingsControl = {0, 0, 0, NOT_PRESSED, NOT_PRESSED, NO_UNIT, SECONDS, 0};
	//uint8_t timeBCD[6] = {0, 0, 0, 0, 0, 0};
	// uint8_t dateBCD[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	clockControl.clockState = RUNNING;

	// Interruptions configuration
	//cli();

	// Normal mode selected via these registers, they are always initializaed as zero by the MCU
	TCCR1A = 0;
	TCCR1B = 0;

	// Set start value for the timer 1 counter register
	TCNT1 = 3036;

	// Set prescaler to 256
	TCCR1B |= (1 << CS12);

	// Enable Overflow Interrupt
	TIMSK1 |= (1 << TOIE1);

	sei();

	Wire.begin();
	Serial.begin(9600);

	// Set pins A0 through A3 as inputs, let the rest of the register unchanged
	DDRC &= ~((1 << PORTC0) | (1 << PORTC1) | (1 << PORTC2));
	// Set pins A0 through A3 HIGH (to set the pull-up resistors), let the rest of the register unchanged
	PORTC |= ((1 << PORTC0) | (1 << PORTC1) | (1 << PORTC2));

	// Set as outputs
	DDRB |= ((1 << PORTB0) | (1 << PORTB1) | (1 << PORTB2) | (1 << PORTB3) | (1 << PORTB4) | (1 << PORTB5));
	DDRD |= ((1 << PORTD2) | (1 << PORTD3) | (1 << PORTD4) | (1 << PORTD5) | (1 << PORTD6) | (1 << PORTD7));

	while (1)
	{
		if (g_heartbeat_s)
		{
			g_heartbeat_s = 0;
			PORTB ^= (1 << PORTB5);
			update_clock(&clockControl, SECONDS, PLUS, true, true);
			//time_to_BCD(clockControl.time, timeBCD);
			//update_led_displays(timeBCD);
			//clock_to_serial(&clockControl);
		}
		read_buttons(&settingsControl);
		update_settings(&clockControl, &settingsControl);
	}

	//return 0;
}

ISR(TIMER1_OVF_vect)
{
	TCNT1 = 3036;
	g_heartbeat_s = 1;
}
