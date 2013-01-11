#define F_CPU 8000000UL

#include <util/delay.h>
#include <avr/io.h>

#define NUMLEDS 10

#define ON PORTA |= (1<<PA6)
#define OFF PORTA &= ~(1<<PA6)
// #define ON PORTA = 0xff
// #define OFF PORTA = 0x00

// uncomment exactly one of these:
//#define CANDLE
//#define SINUS
#define BOUNCE

void clear(void)
{
    uint8_t i, k;
    for (i=0; i<3*NUMLEDS; i++) {
        for (k=0; k<8; k++) {
            ON;
            OFF;
        }
    }
    _delay_us(100);
}

void init(void)
{
#if F_CPU == 8000000UL
    CLKPR = 1 << CLKPCE;
    CLKPR = 0;
#endif
    DDRA = ((1<<PA7)|(1<<PA6));
    clear();
}


#ifdef CANDLE
uint32_t seed = 1;
uint16_t rand(void)
{
    seed = seed * 1103515245 + 12345;
    return (seed / 65536) % 32768;
}

int16_t lowpass(int16_t *state, int16_t sample)
{
    *state += (sample - *state) / 64;
    return *state;
}

#define LOW 14000
#define HI (LOW+4095)

int main(void)
{
    uint8_t i, j, k, l = 0, x;
    int16_t state[NUMLEDS*3];
    int16_t filtered;
    uint8_t values[NUMLEDS*3];

    init();

    for (i=0; i<NUMLEDS*3; i++) {
        state[i] = (HI+LOW)/2;
    }
    
    for (;;) {
        for (i=0; i<NUMLEDS*3; i++) {
            filtered = lowpass(&state[i], rand());
            if (filtered < LOW) filtered = LOW;
            if (filtered > HI) filtered = HI;
            filtered = (filtered-LOW)/(4096/256);
            values[i] = filtered;
        }
        j = 0;
        for (i=0; i<NUMLEDS*3; i++) {
            x = values[i];
            switch (j++) {
            case 0:
                break;
            case 1:
                break;
            case 2:
                x >>= 2;
                j = 0;
                break;
            }
            for (k=0; k<8; k++) {
                if (x & 0x80) {
                    ON;
                }
                ON; OFF;
                x <<= 1;
            }
        }
        _delay_ms(10);
    }
}
#endif
#ifdef SINUS
int main(void)
{
    uint8_t i, j, k, l = 0, x, z = 0;
    uint8_t sinus[] = {
        0x01, 0x03, 0x0b, 0x18, 0x2a, 0x40, 0x58, 0x72, 0x8d, 0xa7,
        0xc0, 0xd5, 0xe7, 0xf5, 0xfd, 0xff, 0xfd, 0xf5, 0xe8, 0xd6,
        0xc0, 0xa8, 0x8e, 0x73, 0x59, 0x40, 0x2b, 0x19, 0x0b, 0x03,
    };
    init();
    for (;;) {
        for (i=0; i<30; i++) {
            if (++l == 29) {
                l = 0;
            }
            for (j=0; j<3; j++) {
                switch (j) {
                case 0: // green
                    z = l+10;
                    break;;
                case 1: // red
                    z = l;
                    break;;
                case 2: // blue
                    z = l+20;
                    break;
                }
                if (z > 29) z -= 30;
                x = sinus[z];
                switch (j) {
                case 0: // green
                    //x >>= 2;
                    break;;
                case 1: // red
                    //x = x;
                    break;;
                case 2: // blue
                    //x >>= 5;
                    break;
                }
                for (k=0; k<8; k++) {
                    if (x & 0x80) {
                        ON;
                    }
                    ON;
                    OFF;
                    x += x;
                }
            }
        }
        _delay_ms(50);
    }
}
#endif
#ifdef BOUNCE
int main(void)
{
    uint8_t i, j, k, l = 0, x, z = 0, up = 0;
    init();
    
    for (;;) {
        for (i=0; i<NUMLEDS; i++) {
            if (i == l) {
                z = 0xff;
            } else {
                z = 0;
            }
            for (j=0; j<3; j++) {
                x = z;
                for (k=0; k<8; k++) {
                    if (x & 0x80) {
                        ON;
                    }
                    ON; OFF;
                    x <<= 1;
                }
            }
        }
        _delay_ms(10);
        if (up) {
            if (++l == NUMLEDS-1) {
                up = 0;
            }
        } else {
            if (--l == 0) {
                up = 1;
            }
        }
    }
}
#endif
