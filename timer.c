#include <avr/interrupt.h>
#include "timer.h"

void timer_init() {
    TIMSK |= (1 << TOIE0);
    sei();        
    TCCR0 |= (1 << CS01) | (1 << CS00);
}
