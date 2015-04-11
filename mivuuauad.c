#include <avr/io.h> 
#define F_CPU 8000000
#define BAUD 115200
#include <util/setbaud.h>
#include "lcd-routines.h"

#define PHASE_A     (PINA & 1<<PA0)
#define PHASE_B     (PINA & 1<<PA1)
#define CLICKER     (PIND & 1<<PD2)

#define CMDSIZE 40

#define sbi(a, b) (a) |= (1 << (b))
#define cbi(a, b) (a) &= ~(1 << (b))

static int8_t enc_delta, last, down, clicked; // ** 8 byte
static char cmd[CMDSIZE]; // ** CMDSIZE byte
 
static inline void uart_init() {
    /* BAUD rate */
    UBRRH = UBRRH_VALUE;
    UBRRL = UBRRL_VALUE;
    #if USE_2X
        UCSRA |= (1 << U2X);
    #else
        UCSRA &= ~(1 << U2X);
    #endif

    /* enable UART TX and RX */
    UCSRB |= (1<<TXEN)|(1<<RXEN);
    /* set UART 8N1 */
    UCSRC = (3<<UCSZ0);  
}

static inline int uart_putc(unsigned char c) {
    /* wait until sending is possible */
    while(!(UCSRA & (1<<UDRE))) {}
 
    /* send char */
    UDR = c;

    return 0;
}
 
static inline void uart_puts (char *s) {
    while(*s) {
        uart_putc(*s);
        s++;
    }
}

static inline uint8_t uart_getc() {
    /* wait until char is available */
    while (!(UCSRA & (1<<RXC)));
    
    /* receive char */
    return UDR;
}
 
static inline void uart_gets() { // ** 2 byte
    uint8_t c, i;

    for(i=0; i<CMDSIZE; i++) cmd[i] = '\0';

    c = uart_getc();

    /* read chars until CMDSIZE, or CR or LF */
    while((c != '\n' && c != '\r') && i < 1024) {
        if(i < CMDSIZE - 1) {
            cmd[i++] = c;
            c = uart_getc();
        }
    }

    *cmd = '\0';
}

static inline void encode_init() {
    last = 0;
    if(PHASE_A)
        last = 3;
    if(PHASE_B)
        last ^= 1; // convert gray to binary

    enc_delta = 0;

    down = 0;
    clicked = 0;
}

static inline void encode_read() { // ** 2 byte
    int8_t new, diff;

    new = 0;
    if(PHASE_A)
        new = 3;
    if(PHASE_B)
        new ^= 1; // convert gray to binary

    diff = last - new; // difference last - new
    if(diff & 1) { // bit 0 = value (1)
        last = new; // store new as next last
        enc_delta += (diff & 2) - 1; // bit 1 = direction (+/-)
    }

    if(down==1) {
        if(CLICKER) {
            if(clicked<=10) clicked++;
        } else {
            clicked--;
            if(clicked==0) down = 2;
        }
    } else {
        if(CLICKER) {
            clicked++;
            if(clicked>10) down = 1;
        } else {
            if(clicked>0) clicked--;
        }
    }
}

static inline int8_t encode_read4() { // ** 1 byte
    int8_t val;

    val = enc_delta;
    enc_delta = val & 3;
    return val >> 2;
}

int main (void) { // 3 byte
    int8_t i, mode = 0, val;

    uart_init();

    lcd_init();
    lcd_string("Lets go!");

    // init relays pins
    DDRD = 0b01111000;
    PORTD |= (1<<PB3);
    PORTD |= (1<<PB4);
    PORTD |= (1<<PB5);
    PORTD |= (1<<PB6);

    while(1) {
        if((UCSRA & (1<<RXC))) {
            uart_gets();

            if(mode==0) {
                if(cmd[0]=='+' && cmd[1]=='X' && cmd[2]=='+') {
                    mode = 1;
                } else {
                    lcd_clear();
                    lcd_string(cmd);
                }
            } else {
                // uart_puts("I got: ");
                // uart_puts(cmd);
                // uart_puts("\n\r");
                switch(*cmd) {
                    case 'p':
                        for(i=0; cmd[i]!='|' && cmd[i]!=0; i++);
                        cmd[i] = 0;
                        lcd_clear();
                        lcd_string(cmd + 1);
                        if(i<CMDSIZE-1) {
                            lcd_setcursor(0, 2);
                            lcd_string(cmd + i + 1);
                        }
                        break;
                    case 's':
                        if(cmd[2]>='0' && cmd[2]<'4') {
                            if(cmd[1]=='0') {
                                PORTD |= (1<<cmd[2]-'0'+3);
                            } else if(cmd[1]=='1') {
                                PORTD &= ~(1<<cmd[2]-'0'+3);
                            }
                        }
                        break;
                }
            }
        }

        if(mode==1) {
            encode_read();
            val += encode_read4();
            if(val!=0) {
                if(val>0)
                    uart_puts("right\n\r");
                if(val<0)
                    uart_puts("left\n\r");
                val = 0;
            }
            if(down==2) {
                uart_puts("click\n\r");
                down = 0;
            }
        }
    }
}
