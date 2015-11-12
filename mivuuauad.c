#include <avr/io.h> 
// #define F_CPU 16000000
// #define BAUD 9600
#include <util/delay.h>
#include <avr/interrupt.h>
#include "uart.h"
#include "adc.h"
#include "shift.h"
//#include "irmp/irmp.h"

#define CMDSIZE 4

// ** Amp / Speaker control
#define CHARGED 20000
#define WAIT_OFF 1000

// ** Analog Keys
#define AKEY_CHANNEL 5
#define POT_CHANNEL 4
#define ERROR_WINDOW 100  // +/- this value
#define BUTTONDELAY 20
#define BTN_SPEAKER 5
#define BTN_AMP 4
#define BTN_PREV 2
#define BTN_PLAYSE 3
#define BTN_NEXT 1
#define LED_AMP 5
#define LED_SPK 7
#define LED_WAIT 6
#define LED_PLAY 4

// ** Relays
#define RELAY_PORT PORTB
#define RELAY_DDR DDRB
#define RELAY_P PB0

// ** States
enum { AS_OFF, AS_ON, AS_WAIT_OFF, AS_WAIT4CHARGE, AS_AMP_ON, AS_SPEAKER_WAIT };

static char cmd[CMDSIZE];
static long int ms = 0;
static uint16_t charge = 0;
static uint8_t amp_state = AS_OFF, old_state = AS_OFF;


static int read_analog_buttons(uint8_t channel) {
    uint16_t val = 0;

    val = adc_read(channel);

    if(val>=890-ERROR_WINDOW && val<=890+ERROR_WINDOW) return 1;
    else if(val>=680-ERROR_WINDOW && val<=680+ERROR_WINDOW) return 2;
    else if(val>=460-ERROR_WINDOW && val<=460+ERROR_WINDOW) return 3;
    else if(val>=240-ERROR_WINDOW && val<=240+ERROR_WINDOW) return 4;
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
    uint16_t button_timer = 0;
    uint8_t button = 0, button_last = 0, last_pressed = 0, pot = 0;//, a, b, c, d;
    // ** leds
    uint8_t leds = 0b00000000, old_leds = 0b00000000;

    // ** init stuff
    uart_init();
    adc_init();
    shift_init();
    timer_init();

    // ** all leds off
    // shift_byte(leds);

    // ** init relays (switched off)
    RELAY_DDR |= (1<<RELAY_P);
    RELAY_DDR |= (1<<(RELAY_P+1));
    RELAY_PORT |= (1<<RELAY_P);
    RELAY_PORT |= (1<<(RELAY_P+1));

    // ** ready!
    uart_puts("\n\rready\n\r");
//play4,amp3,wait2
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
                        leds &= ~(1 << LED_PLAY); // PLAY LED OFF
                    else if(cmd[1]=='1')
                        leds |= 1 << LED_PLAY; // PLAY LED ON
                    break;
                case 'i':
                    uart_putc(amp_state + '0');
                    uart_puts("\n\r");
            }
        }

        // ** handle buttons
        if(ms - button_timer>BUTTONDELAY) {
            button = read_analog_buttons(AKEY_CHANNEL);
            // uart_putc(button+'0');
            // uart_putc('\n');
            // uart_putc('\r');

            // pot = adc_read(POT_CHANNEL);
            // a = pot/1000;
            // b = (pot-(a*1000))/100;
            // c = (pot-(a*1000+b*100))/10;
            // d = pot-(a*1000+b*100+c*10);
            // uart_putc(a+'0');
            // uart_putc(b+'0');
            // uart_putc(c+'0');
            // uart_putc(d+'0');
            // uart_putc('\n');
            // uart_putc('\r');
            // _delay_ms(200);

/*                uart_putc(button+'0');
                uart_putc('\n');
                uart_putc('\r');
*/            if(button==button_last && button!=last_pressed) {
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
        }//leds |= 1 << LED_PLAY;

        // ** act on state changes
        if(amp_state != old_state) {
            switch(amp_state) {
                case AS_OFF:
                    uart_puts("off\n\r");
                    RELAY_PORT |= 1<<(RELAY_P + 0); // AMP off if on
                    RELAY_PORT |= 1<<(RELAY_P + 1); // SPEAKER off if on
                    leds &= ~(1 << LED_AMP); // AMP LED OFF
                    leds &= ~(1 << LED_WAIT); // WAIT LED OFF
                    leds &= ~(1 << LED_SPK); // SPEAKER LED OFF
                    break;
                case AS_WAIT4CHARGE:
                    uart_puts("wait\n\r");
                    RELAY_PORT &= ~(1<<(RELAY_P + 0)); // AMP on if off
                    RELAY_PORT |= 1<<(RELAY_P + 1); // SPEAKER off if on
                    leds |= 1 << LED_AMP; // AMP LED ON
                    leds |= 1 << LED_WAIT; // WAIT LED ON
                    leds &= ~(1 << LED_SPK); // SPEAKER LED OFF
                    break;
                case AS_AMP_ON:
                    uart_puts("amp\n\r");
                    RELAY_PORT &= ~(1<<(RELAY_P + 0)); // AMP on if off
                    RELAY_PORT |= 1<<(RELAY_P + 1); // SPEAKER off if on
                    leds |= 1 << LED_AMP; // AMP LED ON
                    leds &= ~(1 << LED_WAIT); // WAIT LED OFF
                    leds &= ~(1 << LED_SPK); // SPEAKER LED OFF
                    //charge = CHARGED;
                    break;
                case AS_ON:
                    uart_puts("on\n\r");
                    RELAY_PORT &= ~(1<<(RELAY_P + 0)); // AMP on if off
                    RELAY_PORT &= ~(1<<(RELAY_P + 1)); // SPEAKER on if off
                    leds |= 1 << LED_AMP; // AMP LED ON
                    leds &= ~(1 << LED_WAIT); // WAIT LED OFF
                    leds |= 1 << LED_SPK; // SPEAKER LED ON
                    break;
                case AS_WAIT_OFF:
                    uart_puts("offwait\n\r");
                    RELAY_PORT &= ~(1<<(RELAY_P + 0)); // AMP on if off
                    RELAY_PORT |= 1<<(RELAY_P + 1); // SPEAKER off if on
                    leds |= 1 << LED_AMP; // AMP LED ON
                    leds |= 1 << LED_WAIT; // WAIT LED ON
                    leds |= 1 << LED_SPK; // SPEAKER LED ON
                    break;
            }
            old_state = amp_state;
        }

        // ** handle timeouts
        switch(amp_state) {
            case AS_WAIT4CHARGE: if(charge>=CHARGED) amp_state = AS_ON; break;
            case AS_WAIT_OFF: if(charge<=(CHARGED-WAIT_OFF)) amp_state = AS_OFF; break;
        }

        // ** set leds
/*        if(leds!=old_leds) {
            uart_puts("leds: ");
            int v=leds;
            for(int i=0; i<8; i++) {
                if(v & 0b00000001) {
                    uart_putc('1');
                } else {
                    uart_putc('0');
                }
                v = v>>1;
            }
            uart_puts("\n\r");
            shift_byte(leds);
            old_leds = leds;
        }
*/        // leds |= 1 << LED_AMP; // AMP LED ON
        // leds &= ~(1 << LED_SPK); // WAIT LED OFF
        // leds &= ~(1 << LED_WAIT); // SPEAKER LED OFF
        // shift_byte(leds);
        // _delay_ms(100);
        // shift_byte(0);
        // _delay_ms(100);

        // leds |= 1 << LED_SPK; // AMP LED ON
        // leds &= ~(1 << LED_WAIT); // WAIT LED OFF
        // leds &= ~(1 << LED_AMP); // SPEAKER LED OFF
        // shift_byte(leds);
        // _delay_ms(100);

        // leds |= 1 << LED_AMP; // AMP LED ON
        // leds &= ~(1 << LED_SPK); // WAIT LED OFF
        // leds &= ~(1 << LED_WAIT); // SPEAKER LED OFF
        // shift_byte(leds);
        // _delay_ms(100);

        shift_byte(0b10000000);
        _delay_ms(100);
        shift_byte(0b01000000);
        _delay_ms(100);
        shift_byte(0b00100000);
        _delay_ms(100);
        shift_byte(0b00010000);
        _delay_ms(100);
        shift_byte(0b00001000);
        _delay_ms(100);
        shift_byte(0b00000100);
        _delay_ms(100);
        shift_byte(0b00000010);
        _delay_ms(100);
        shift_byte(0b00000001);
        _delay_ms(100);

    }

}
