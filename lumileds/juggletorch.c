/*
 * juggletorch.c -- simple multi-LED blinker for juggling clubs
 *
 * copyright (C) 2008, jw@suse.de, distribute under GPL, use with mercy.
 *
 *
 * 2008-08-31, jw, v0.01 -- initial draft for an ATtiny861, one LED blinks.
 * 2008-09-01, jw, v0.02 -- 10 LEDs blink, fixed pattern, obyeing MAX_CONCURRENT
 */

#if 0
/*          ATtiny861
                          ________    _______     
  	                 |        \__/       |         
       	      STK200 -- 1|MOSI            PA0|20 -----47ohm--- RD1
              STK200 -- 2|MISO            PA1|19 -----40ohm--- YL1
              STK200 -- 3|SCK             PA2|18 -----20ohm--- BL1
  YL3-----40ohm-------- 4|PB3             PA3|17 -----20ohm--- WH1
             +3.7V ---- 5|VCC            AGND|16 ----|
                   |--- 6|GND            AVCC|15 ---- +3.7V
  YL4 ----40ohm-------- 7|PB4/ADC7        PA4|14 -----47ohm--- RD2    
            ?adc? ----- 8|PB5/ADC8        PA5|13 -----40ohm--- YL2 
            IR_in ----- 9|PB6/INT0/ADC9   PA6|12 -----20ohm--- BL2 
              STK200 - 10|RESET           PA7|11 -----22ohm--- GN1
                         |___________________|
                   
 tiny861  has 1MHz, Xkb flash, XXX bytes eeprom, XXX bytes RAM.
---------------------------------------------------------------

Warning, this is a special pin-out, I use a newer pin-out on all 
my other devices.

 stk200 is not a real STK200 connector, but a mini-6-pin female 
 connector for better safety and weight:
            --
   MISO----|1=
   SCK ----|2=
   Reset --|3=
   GND ----|4=
   MOSI ---|5=
   VCC ----|6=
            --

---------------------------------------------------------------
Unipolar differential ADC mode, doc102865p152:
ADC = (Vpos-Vneg)*1024*Gain/Vref

REFS2..0 = 110	internal 2.56 V, disconnected from AREF pin.

Add 47k between PA0 and PA7, add 47k between PA7 and GND.
With PA0 High and PA7 as input without PUD, we can measure VCC/2 at PA7.
This is max 4.2V/2=2.1V below internal reference 2.56V

TODO: The RED LED at PA0 influences such measurements how?

-------------------------------------------------------------
Power consumption

All LEDs are connected to Ground. Thus the Pins allways source current, and
All current must go out through the ground pins.

with the basic blink pattern of version 0.4, 
- with MAX_CONCURRENT == 3, we measure a max current of 103mAh
- with MAX_CONCURRENT == 6, we measure a max current of 165mAh
- with MAX_CONCURRENT == 7, we measure a max current of 172mAh

Max official current through GND or VCC is 200mA for attiny461 series,
so we should be safe up to
MAX_CONCURRENT==8
-------------------------------------------------------------
IR Reciever
TSOP 3226, 2.7V 36khz

-------------------------------------------------------------

Fuse high byte:
bit name	default
7 RSTDISBL	1 (false)
6 DWEN		1 (false)
5 SPIEN		0 (enabled)
4 WDTON		1 (false)
3 EESAVE	1 (not preserved)
2 BODLEVEL2	1 
1 BODLEVEL1	1 
0 BODLEVEL0	1 

BODLEVEL2..0
111	disabled
110	1.8V
101	2.7V
100	4.3V

fuse high 0xd5: BOD 2.7V, WDToff, eesave yes.
*/
#endif


#include <avr/io.h>
#include "cpu_mhz.h"
#include "config.h"

#if (CPU_MHZ == 1)
# define F_CPU 1000000UL	// 1Mhz
#else
# error "must run at 1Mhz"		// oops, 8mhz should work too, but does not.
#endif


#include <util/delay.h>
static uint8_t arr[N_CHAN];
static int8_t last_forced_off = 0;
static uint8_t last_turned_on = 0;

#if (MAX_CONCURRENT <= 0)
# error "MAX_CONCURRENT must be at least 1"
#endif

static void led_off(uint8_t idx)
{
  // tristate switching is ugly in AVRs.
  // drive_high -> floating needs two instructions, and leaves
  // the pullup on between these instructions.
  // Only other alternative is to drive_low during one instruction.
  // (we don't want to drive low, this risks to draw current 
  // in case of an hardware fault)

  arr[idx] = 0;
  if (idx == 8)     { DDRB &= ~(1<<PB3); PORTB = DDRB; }
  else if (idx > 8) { DDRB &= ~(1<<PB4); PORTB = DDRB; }
  else              { DDRA &= ~(1<<idx); PORTA = DDRA; }
}


static void led_on(uint8_t idx, uint8_t value)
{
  // tristate switching is ugly in AVRs.
  // floating -> drive_high needs two instructions, and leaves
  // the pullup on between these instructions.
  // Only other alternative is to drive_low during one instruction.
  // (we don't want to drive low, this risks to draw current while
  // in case of an hardware fault)

  if (!value) value = 1;	// must be >0, so that n_on counting works.
  arr[idx] = value;
  if (idx == 8)     { PORTB |= (1<<PB3); DDRB = PORTB; }
  else if (idx > 8) { PORTB |= (1<<PB4); DDRB = PORTB; }
  else              { PORTA |= (1<<idx); DDRA = PORTA; }
}

// A save method to turn LEDs on.
// We make sure MAX_CONCURRENT is obeyed.
// idx is 0..(N_CHAN-1)
static void set_led(uint8_t idx, uint8_t value)
{
  uint8_t n_on = 0;
  uint8_t i;

  if (!value)
    {
      led_off(idx);
      return;
    }

  // count how many are on, not counting ourselves.
  for (i = 0; i < N_CHAN; i++) { if (i != idx && arr[i]) n_on++; }

  while (n_on > MAX_CONCURRENT)
    {
      // find an LED to turn off, which should be a bit random.
      int8_t dir = (last_turned_on < idx) ? -1 : 1;
      while (!arr[last_forced_off])
        {
	  last_forced_off += dir;
	  if (last_forced_off >= N_CHAN) last_forced_off = 0;
	  else if (last_forced_off < 0) last_forced_off = N_CHAN-1;
	}
      // either an infinite loop or last_forced_off is a burning led now.
      led_off(last_forced_off);	// just changed last_forced_off;
      n_on--;
    }

  // now we are sure we have less than or equal MAX_CONCURRENT LEDs on.
  led_on(idx, value);
}

// LED names:

#define L_RD1 0
#define L_RD2 4
#define L_BL1 2
#define L_BL2 6
#define L_YL1 1
#define L_YL2 5
#define L_YL3 8
#define L_YL4 9
#define L_GN1 7
#define L_WH1 3

static void blink_once(uint8_t led, uint8_t ms10)
{
  uint8_t i;
  set_led(led, 0xff);
  for (i = 0; i < ms10; i++) { _delay_ms(10.0); }
  led_off(led);
  for (i = 0; i < ms10; i++) { _delay_ms(10.0); }
}

// Single-Ended 10bit, highbyte aligned.
// 3.85 V -> adval=192, adc_l = 128 -> 3 Red, 8 Green, 5 blue pulses.
static void blink_voltage(uint8_t adval, uint8_t adc_l)
{
  set_led(L_WH1, 0xff); set_led(L_YL1, 0xff);

  _delay_ms(200.0);
  _delay_ms(200.0);

  while (adval >= 50)
    { 
      adval -= 50;
      blink_once(L_RD2, 25);	// 2Hz
    }
  adval = (adval<<1)+((adc_l & 0x80)?1:0);
  led_off(L_WH1); led_off(L_YL1);
  _delay_ms(200.0);

  set_led(L_WH1, 0xff); set_led(L_YL1, 0xff);
  _delay_ms(200.0);
  while (adval >= 10)
    {
      adval -=10;
      blink_once(L_GN1, 17);	// 3Hz
    }
  led_off(L_WH1); led_off(L_YL1);
  _delay_ms(200.0);

  set_led(L_WH1, 0xff); set_led(L_YL1, 0xff);
  _delay_ms(200.0);
  while (adval-- > 0)
    {
      blink_once(L_BL2, 12);	// 4hz
    }
  led_off(L_WH1); led_off(L_YL1);
  _delay_ms(200.0);
}


int main()
{

  DDRA = 0x00;              PORTA = DDRA;
  DDRB = (0<<PB3)|(0<<PB4); PORTB = DDRB;

  // blink_voltage does not yet do anyhting useful.
  // blink_voltage(192, 0xd0);

  for (;;)
    {
      // basic blink pattern of version 0.4
      _delay_ms(200.0); set_led(L_RD1, 0xff);
                        led_off(L_BL2); 
      _delay_ms(200.0); set_led(L_RD2, 0xff);
                        led_off(L_WH1); 
                        led_off(L_BL1);	// pure red
      _delay_ms(200.0); set_led(L_RD1, 0xff);
      _delay_ms(200.0); 
      _delay_ms(200.0); 
      _delay_ms(200.0); set_led(L_YL1, 0xff);
      _delay_ms(200.0); set_led(L_YL2, 0xff);
      _delay_ms(200.0); set_led(L_YL3, 0xff);
      _delay_ms(200.0); set_led(L_YL4, 0xff);
      _delay_ms(200.0); 
      _delay_ms(200.0); 
      _delay_ms(200.0); set_led(L_GN1, 0xff);
      _delay_ms(200.0); set_led(L_WH1, 0xff);
      _delay_ms(200.0); 
      _delay_ms(200.0); 
      _delay_ms(200.0); set_led(L_BL1, 0xff);
      _delay_ms(200.0); set_led(L_BL2, 0xff);
      _delay_ms(200.0); set_led(L_WH1, 0xff);
      _delay_ms(200.0); 
      _delay_ms(200.0); 

      //
      // use a free running counter 
      // (watchdog timer?) as pseudo random generator
      // and turn off one or two LEDs.
      //
    }
}
