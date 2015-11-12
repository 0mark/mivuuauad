#include <avr/io.h> 
#include "shift.h"
#include <util/delay.h>


void shift_init() {
    SHIFT_DDR |= (1<<4);
    SHIFT_DDR |= (1<<3);

    shift_byte(0);
}

void shift_bit(uint8_t v) {
    if(v & 0b00000001) {
        SHIFT_PORT |= (1<<SHIFT_PDATA);
    } else {
        SHIFT_PORT &= ~(1<<SHIFT_PDATA);
    }
    SHIFT_PORT |= (1<<SHIFT_PCLOCK);
    _delay_ms(10);
    SHIFT_PORT &= ~(1<<SHIFT_PCLOCK);
    SHIFT_PORT &= ~(1<<SHIFT_PDATA);
}

void shift_byte(uint8_t v) {
    for(int i=0; i<8; i++) {
        shift_bit(v);
        v = v>>1;
    }
}
