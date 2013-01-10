/*
 * blink.c -- simple LED blinker
 *
 * Copyright (C) 2008, jw@suse.de, distribute under GPL, use with mercy.
 *
 */
// #include "config.h"
#include "cpu_mhz.h"

#include <util/delay.h>			// needs F_CPU from cpu_mhz.h
#include <avr/io.h>

#ifdef PORTA
# define LED_PORT PORTA
# define LED_DDR  DDRA
#else
# ifdef PORTB
#  define LED_PORT PORTB
#  define LED_DDR  DDRB
# else
#  error "This CPU has no PORTA or PORTB, try somethig different"
# endif
#endif

#define LED_BITS	0xff		// try all ...

int main()
{
  LED_DDR |= LED_BITS;			// all pins outout
  for (;;)
    {
      _delay_ms(200.0); LED_PORT &= ~(LED_BITS);        // pull low ...
      _delay_ms(200.0); LED_PORT |=   LED_BITS;         // pull high ...
    }
}
