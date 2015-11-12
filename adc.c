#include <avr/io.h> 
#include "adc.h"

void adc_init() {
    // DDRC  = 0x00; // DDRC.1  = 0 = input 
    ADC_DDR &= ~(1<<5);
    ADC_PORT = 0xFF; // PORTC.1 = 1 = internal pullup enabled

    // die Versorgungsspannung AVcc als Referenz wählen:
    ADMUX = (1<<REFS0);    

    // Bit ADFR ("free running") in ADCSRA steht beim Einschalten
    // schon auf 0, also single conversion
    ADCSRA = (1<<ADPS1) | (1<<ADPS0);     // Frequenzvorteiler
    ADCSRA |= (1<<ADEN);                  // ADC aktivieren

    // nach Aktivieren des ADC wird ein "Dummy-Readout" empfohlen, man liest
    // also einen Wert und verwirft diesen, um den ADC "warmlaufen zu lassen"
    ADCSRA |= (1<<ADSC);                  // eine ADC-Wandlung 
    while(ADCSRA & (1<<ADSC) ) {}

    // ADCW muss einmal gelesen werden, sonst wird Ergebnis der nächsten
    // Wandlung nicht übernommen.
    (void)ADCW;
}
 
uint16_t adc_read(uint8_t channel) {
    // Kanal waehlen, ohne andere Bits zu beeinflußen
    ADMUX = (ADMUX & ~(0x1F)) | (channel & 0x1F);
    ADCSRA |= (1<<ADSC);            // eine Wandlung "single conversion"
    while(ADCSRA & (1<<ADSC)) {}
    return ADCW;                    // ADC auslesen und zurückgeben
}
