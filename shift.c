#include <avr/io.h> 
#include "shift.h"

void shift_init() {
    DDRB |= (1<<4);
    DDRB |= (1<<3);

    shift_byte(0);
}

void shift_bit(uint8_t v) {
    if(v & 0b00000001) {
        PORTB |= (1<<PB3);
    } else {
        PORTB &= ~(1<<PB3);
    }
    PORTB |= (1<<PB4);
    PORTB &= ~(1<<PB4);
}

void shift_byte(uint8_t v) {
    for(int i=0; i<8; i++) {
        shift_bit(v);
        v = v>>1;
    }
}
