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

#define SNAKE 1

#define LED_PORT PORTA
#define LED_DDR  DDRA
#define LED_BITS	(1<<PA7)

#define ON PORTA |= (1<<PA6)
#define OFF PORTA &= ~(1<<PA6)

#define _M(a)	do { a; } while (0)
// ON;OFF;OFF + ON;ON;OFF does not work
// ON;ON;OFF;OFF + ON;ON;ON;OFF does not work
// ON;ON;OFF;OFF;OFF + ON;ON;ON;OFF;OFF does not work
#define BIT0	_M(ON; OFF; OFF; OFF)
#define BIT1	_M(ON;  ON; OFF; OFF)

#define SENDBYTE_255	_M(BIT1; BIT1; BIT1; BIT1; BIT1; BIT1; BIT1; BIT1)
#define SENDBYTE_191	_M(BIT1; BIT0; BIT1; BIT1; BIT1; BIT1; BIT1; BIT1)
#define SENDBYTE_127	_M(BIT0; BIT1; BIT1; BIT1; BIT1; BIT1; BIT1; BIT1)
#define SENDBYTE_95	_M(BIT0; BIT1; BIT0; BIT1; BIT1; BIT1; BIT1; BIT1)
#define SENDBYTE_63	_M(BIT0; BIT0; BIT1; BIT1; BIT1; BIT1; BIT1; BIT1)
#define SENDBYTE_47	_M(BIT0; BIT0; BIT1; BIT0; BIT1; BIT1; BIT1; BIT1)
#define SENDBYTE_31	_M(BIT0; BIT0; BIT0; BIT1; BIT1; BIT1; BIT1; BIT1)
#define SENDBYTE_15	_M(BIT0; BIT0; BIT0; BIT0; BIT1; BIT1; BIT1; BIT1)
#define SENDBYTE_0	_M(BIT0; BIT0; BIT0; BIT0; BIT0; BIT0; BIT0; BIT0)

#define SEND_R		_M(SENDBYTE_0;   SENDBYTE_255; SENDBYTE_0)
#define SEND_G		_M(SENDBYTE_255; SENDBYTE_0;   SENDBYTE_0)
#define SEND_B		_M(SENDBYTE_0;   SENDBYTE_0;   SENDBYTE_255)
#define SEND_C		_M(SENDBYTE_255; SENDBYTE_0;   SENDBYTE_255)
#define SEND_M		_M(SENDBYTE_0;   SENDBYTE_255; SENDBYTE_255)
#define SEND_Y		_M(SENDBYTE_191; SENDBYTE_255; SENDBYTE_0)
#define SEND_ORANGE	_M(SENDBYTE_95;  SENDBYTE_255; SENDBYTE_0)


#define SEND_W		_M(SENDBYTE_255; SENDBYTE_255; SENDBYTE_127)
#define SEND_W127	_M(SENDBYTE_127; SENDBYTE_127; SENDBYTE_63)
#define SEND_W63	_M(SENDBYTE_63;  SENDBYTE_63;  SENDBYTE_31)
#define SEND_W31	_M(SENDBYTE_31;  SENDBYTE_31;  SENDBYTE_15)
#define SEND_K		_M(SENDBYTE_0;   SENDBYTE_0;   SENDBYTE_0)

int main()
{
  LED_DDR |= LED_BITS;			// all pins outout
  DDRA |= (1<<PA6);
  // works with 3.7V: 0,   0 --> 0, 105
  // works with 5.0V: 0,   0 --> 0, 108
  // fails with 3.7V: 0, 106 --> 0, 127 
  // fails with 5.0V: 0, 109 --> 0, 127 
  OSCCAL = (0<<7) | (50 & 0x7f);		// doc2588p32
  // 0, 0:  16sec for 20x400msec
  // 1, 0:  11sec for 20x400msec
  // 0, 90: 11sec for 20x400msec
  // 0, 127: 9sec for 20x400msec
#ifdef SNAKE
  for (;;)
    {
      int8_t i;
      for (i = 0; i < 24; i++)
	{
	  int8_t n; for (n = 0; n < i; n++) SEND_K;
	  SEND_R; SEND_ORANGE; SEND_Y; SEND_Y; SEND_Y; SEND_W;
	  for (; n < 24; n++) SEND_K;
	  _delay_ms(10);
	}
      for (i = 0; i < 30; i++) SEND_W; _delay_ms(20);
      for (i = 24; i >=0; i--)
	{
	  int8_t n; for (n = 0; n < i; n++) SEND_K;
	  SEND_W; SEND_Y; SEND_Y; SEND_Y; SEND_ORANGE; SEND_R;
	  for (; n < 24; n++) SEND_K;
	  _delay_ms(10);
	}
      for (i = 0; i < 30; i++) SEND_R; _delay_ms(40);
    }
#endif

#if 0
  for (;;)
    {
      SEND_K; SEND_K; SEND_K;
      _delay_ms(200.0); LED_PORT &= ~(LED_BITS);        // pull low ...
      SEND_W; SEND_Y;
      _delay_ms(200.0); LED_PORT |=   LED_BITS;         // pull high ...

      SEND_K; SEND_K;
      _delay_ms(200.0); LED_PORT &= ~(LED_BITS);        // pull low ...
      SEND_W31; SEND_K; SEND_ORANGE;
      _delay_ms(200.0); LED_PORT |=   LED_BITS;         // pull high ...

      SEND_K; SEND_K; 
      _delay_ms(200.0); LED_PORT &= ~(LED_BITS);        // pull low ...
      SEND_G; SEND_M;
      _delay_ms(200.0); LED_PORT |=   LED_BITS;         // pull high ...

      SEND_K; SEND_K;
      _delay_ms(200.0); LED_PORT &= ~(LED_BITS);        // pull low ...
      SEND_R;
      _delay_ms(200.0); LED_PORT |=   LED_BITS;         // pull high ...
    }
#endif
}
