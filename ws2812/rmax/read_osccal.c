#ifdef PRINT_OSCCAL
int main(void)
{
    uint8_t i, j, k, l = 0, x, z = 0, up = 1, a;
    init();

    for (;;) {
        a = OSCCAL;
        for (i=0; i<8; i++) {
            z = (a & 0x80) ? 0x10 : 0;
            a <<= 1;
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
        _delay_ms(20);
    }
}
#endif

