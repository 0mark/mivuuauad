#include <stdlib.h> 
#include <avr/io.h> 
// #define F_CPU 16000000
// #define BAUD 9600
#include <util/delay.h>
#include <avr/interrupt.h>
#include "uart.h"
#include "adc.h"
// #include "shift.h"
//#include "irmp/irmp.h"

#define CMDSIZE 4

// ** Amp / Speaker control
#define CHARGED 20000
#define WAIT_OFF 1000

// ** Analog Keys
#define AKEY_CHANNEL 5
#define POT_CHANNEL 1
#define ERROR_WINDOW 50  // +/- this value
#define BUTTONDELAY 20
#define BTN_SPEAKER 5
#define BTN_AMP 4
#define BTN_PREV 2
#define BTN_PLAYSE 3
#define BTN_NEXT 1

// ** Relays
#define RELAY_PORT PORTB
#define RELAY_DDR DDRB
#define RELAY_P PB0

// ** LEDS
// #define LED_AMP 5
// #define LED_SPK 7
// #define LED_WAIT 6
// #define LED_PLAY 4
#define LED_PORT PORTB
#define LED_DDR DDRB
#define LED_SPK PB5
#define LED_AMP PB3
#define LED_WAIT PB4
#define LED_PLAY PB2

// ** States
enum { AS_OFF, AS_ON, AS_WAIT_OFF, AS_WAIT4CHARGE, AS_AMP_ON };

static char cmd[CMDSIZE];
static long int ms = 0;
static uint16_t charge = 0;
static uint8_t amp_state = AS_OFF, old_state = AS_OFF;


static int read_analog_buttons(uint8_t channel) {
    uint16_t val = 0;

    val = adc_read_avg(channel, 10);

                // itoa(val, s, 10);
                // uart_puts("b: ");
                // uart_puts(s);
                // uart_puts("\n\r");

    if(val>=680-ERROR_WINDOW && val<=680+ERROR_WINDOW) return 1;
    else if(val>=515-ERROR_WINDOW && val<=515+ERROR_WINDOW) return 2;
    else if(val>=345-ERROR_WINDOW && val<=345+ERROR_WINDOW) return 3;
    else if(val>=175-ERROR_WINDOW && val<=175+ERROR_WINDOW) return 4;
    else if(val>=0 && val<=0+ERROR_WINDOW) return 5;
    else return 0;
}

ISR(TIMER0_OVF_vect) {
    TCNT0 += 6;

    ms++;
    if((amp_state==AS_WAIT4CHARGE || amp_state==AS_AMP_ON) && charge<CHARGED) charge++;
    if((amp_state==AS_OFF || amp_state==AS_WAIT_OFF) && charge>0) charge--;
}

void timer_init() {
    TIMSK |= (1 << TOIE0);
    sei();        
    TCCR0 |= (1 << CS01) | (1 << CS00);
}

int main (void) {
    // ** buttons
    uint16_t button_timer = 0, pot = 0, pot_last = 0;
    uint8_t button = 0, button_last = 0, last_pressed = 0;
    char s[8];

    // ** init stuff
    uart_init();
    adc_init();
    timer_init();

    // ** init relays (switched off)
    RELAY_DDR |= (1<<RELAY_P);
    RELAY_DDR |= (1<<(RELAY_P+1));
    RELAY_PORT |= (1<<RELAY_P);
    RELAY_PORT |= (1<<(RELAY_P+1));

    // ** init leds (switched off)
    LED_DDR |= (1<<LED_SPK);
    LED_DDR |= (1<<LED_WAIT);
    LED_DDR |= (1<<LED_AMP);
    LED_DDR |= (1<<LED_PLAY);
    LED_PORT &= ~(1<<LED_SPK);
    LED_PORT &= ~(1<<LED_WAIT);
    LED_PORT &= ~(1<<LED_AMP);
    LED_PORT &= ~(1<<LED_PLAY);

    // ** ready!
    uart_puts("\n\rready\n\r");

    while(1) {
        // ** handle serial commands
        if((UCSRA & (1<<RXC))) {
            uart_gets(cmd, CMDSIZE);
            uart_puts(cmd);
            uart_puts("\n\r");

            switch(*cmd) {
                case 's': goto handle_speaker;
                case 'a': goto handle_amp;
                case 'p':
                    if(cmd[1]=='0')
                        LED_PORT &= ~(1<<LED_PLAY);
                    else if(cmd[1]=='1')
                        LED_PORT |= (1<<LED_PLAY);
                    break;
                case 'i':
                    uart_puts("State: ");
                    switch(amp_state) {
                        case AS_OFF: uart_puts("OFF"); break;
                        case AS_ON: uart_puts("ON"); break;
                        case AS_AMP_ON: uart_puts("AMP ON"); break;
                        case AS_WAIT4CHARGE:
                            uart_puts("WAIT ON (");
                            itoa((CHARGED - charge) / 1000, s, 10);
                            uart_puts(s);
                            uart_putc(')');
                            break;
                        case AS_WAIT_OFF: uart_puts("WAIT OFF"); break;
                        default: uart_puts(""); break;
                    }
                    uart_puts(", Vol: ");
                    itoa(pot/50, s, 10);
                    uart_puts(s);
                    uart_puts("\n\r");
                    break;
            }
        }

        // ** handle buttons
        if(ms - button_timer>BUTTONDELAY) {
            button = read_analog_buttons(AKEY_CHANNEL);
            if(button==button_last && button!=last_pressed) {
                switch(button) {
                    case BTN_SPEAKER:
                        handle_speaker:
                        switch(amp_state) {
                            case AS_OFF: case AS_AMP_ON: amp_state = AS_WAIT4CHARGE; break;
                            case AS_WAIT4CHARGE: amp_state = AS_OFF; break;
                            case AS_ON: amp_state = AS_AMP_ON; break;
                            case AS_WAIT_OFF: amp_state = AS_ON; break;
                        }
                        break;
                    case BTN_AMP:
                        handle_amp:
                        switch(amp_state) {
                            case AS_OFF: amp_state = AS_AMP_ON; break;
                            case AS_AMP_ON: case AS_WAIT4CHARGE: amp_state = AS_OFF; break;
                            case AS_ON: amp_state = AS_WAIT_OFF; break;
                            case AS_WAIT_OFF: amp_state = AS_ON; break;
                        }
                        break;
                    case BTN_PREV: uart_puts("prev\n\r"); break;
                    case BTN_PLAYSE: uart_puts("playse\n\r"); break;
                    case BTN_NEXT: uart_puts("next\n\r"); break;
                }
                last_pressed = button;
            }

            button_last = button;
            button_timer = ms;

            pot = adc_read_avg(POT_CHANNEL,10);
            if( (pot>50 && pot-50>pot_last) || pot+50<pot_last ) {
                itoa(pot/50, s, 10);
                uart_puts("vol:");
                uart_puts(s);
                uart_puts("\n\r");
                pot_last = pot;
            }
        }

        // ** act on state changes
        if(amp_state != old_state) {
            switch(amp_state) {
                case AS_OFF:
                    uart_puts("OFF\n\r");
                    RELAY_PORT |= 1<<(RELAY_P + 0); // AMP off if on
                    RELAY_PORT |= 1<<(RELAY_P + 1); // SPEAKER off if on
                    LED_PORT &= ~(1<<LED_AMP);
                    LED_PORT &= ~(1<<LED_SPK);
                    LED_PORT &= ~(1<<LED_WAIT);
                    break;
                case AS_WAIT4CHARGE:
                    uart_puts("WAIT ON (");
                    itoa((CHARGED - charge) / 1000, s, 10);
                    uart_puts(s);
                    uart_puts(")\n\r");
                    RELAY_PORT &= ~(1<<(RELAY_P + 0)); // AMP on if off
                    RELAY_PORT |= 1<<(RELAY_P + 1); // SPEAKER off if on
                    LED_PORT |= (1<<LED_AMP);
                    LED_PORT |= (1<<LED_WAIT);
                    LED_PORT &= ~(1<<LED_SPK);
                    break;
                case AS_AMP_ON:
                    uart_puts("AMP ON\n\r");
                    RELAY_PORT &= ~(1<<(RELAY_P + 0)); // AMP on if off
                    RELAY_PORT |= 1<<(RELAY_P + 1); // SPEAKER off if on
                    LED_PORT |= (1<<LED_AMP);
                    LED_PORT &= ~(1<<LED_SPK);
                    LED_PORT &= ~(1<<LED_SPK);
                    break;
                case AS_ON:
                    uart_puts("ON\n\r");
                    RELAY_PORT &= ~(1<<(RELAY_P + 0)); // AMP on if off
                    RELAY_PORT &= ~(1<<(RELAY_P + 1)); // SPEAKER on if off
                    LED_PORT |= (1<<LED_AMP);
                    LED_PORT &= ~(1<<LED_WAIT);
                    LED_PORT |= (1<<LED_SPK);
                    break;
                case AS_WAIT_OFF:
                    uart_puts("WAIT OFF\n\r");
                    RELAY_PORT &= ~(1<<(RELAY_P + 0)); // AMP on if off
                    RELAY_PORT |= 1<<(RELAY_P + 1); // SPEAKER off if on
                    LED_PORT |= (1<<LED_AMP);
                    LED_PORT |= (1<<LED_WAIT);
                    LED_PORT |= (1<<LED_SPK);
                    break;
            }
            old_state = amp_state;
        }

        // ** handle timeouts
        switch(amp_state) {
            case AS_WAIT4CHARGE: if(charge>=CHARGED) amp_state = AS_ON; break;
            case AS_WAIT_OFF: if(charge<=(CHARGED-WAIT_OFF)) amp_state = AS_OFF; break;
        }
    }

}

