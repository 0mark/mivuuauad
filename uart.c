#include <avr/io.h> 
#include <util/setbaud.h>
#include "uart.h"

void uart_init() {
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
    UCSRC &= (1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0);
}

int uart_putc(unsigned char c) {
    /* wait until sending is possible */
    while(!(UCSRA & (1<<UDRE))) {}
 
    /* send char */
    UDR = c;

    return 0;
}
 
void uart_puts (char *s) {
    while(*s) {
        uart_putc(*s);
        s++;
    }
}

uint8_t uart_getc() {
    /* wait until char is available */
    while (!(UCSRA & (1<<RXC)));
    
    /* receive char */
    return UDR;
}
 
void uart_gets(char *buffer, uint8_t size) {
    uint8_t c, i;

    for(i=0; i<size; i++) buffer[i] = '\0';

    c = uart_getc();

    /* read chars until size, or CR or LF */
    while((c != '\n' && c != '\r') && i < 1024) {
        if(i < size - 1) {
            buffer[i] = c;
            c = uart_getc();
        }
        i++;
    }

    buffer[c] = '\0';
}
