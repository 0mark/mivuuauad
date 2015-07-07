#include <avr/io.h> 
// #define F_CPU 16000000
// #define BAUD 9600
#include <util/delay.h>
#include <util/interrupt.h>
#include "uart.h"
#include "adc.h"
#include "shift.h"

#define CMDSIZE 40

// ** Analog Keys
#define AKEY_CHANNEL 5
#define ERROR_WINDOW 50  // +/- this value
#define BUTTONDELAY 20

// ** Relays
#define RELAY_PORT PORTB
#define RELAY_DDR DDRB
#define RELAY_P PB0


// #define SPEAKER_TIMEOUT 20
// #define AMP_STATE PINB & _BV(PB0)
// #define AMP_ON PORTB &= ~(1<<PB0)
// #define AMP_OFF PORTB |= 1<<PB0
// #define SPEAKER_STATE PINB & _BV(PB1)
// #define SPEAKER_ON PORTB &= ~(1<<PB1)
// #define SPEAKER_OFF PORTB |= 1<<PB1
// #define SPEAKER_WAIT 20
// #define WAIT_FOR_CHARGE 1
// #define WAIT_FOR_SPEAKER_OFF 2

//#define DISCHARGE_PER_SECOND = 
//#define sbi(a, b) (a) |= (1 << (b))
//#define cbi(a, b) (a) &= ~(1 << (b))

enum { AS_OFF, AS_ON, AS_WAIT_OFF, AS_WAIT4CHARGE, AS_AMP_ON, AS_AMP_OFF, AS_SPEAKER_WAIT };
//enum { SPEAKER_OFF, SPEAKER_ON, SPEAKER_WAIT_ON, SPEAKER_WAIT_OFF }

// static uint8_t amp = AMP_OFF;
// static uint8_t charge_wait = 0;

static char cmd[CMDSIZE]; // ** CMDSIZE byte

/* state
 * 0 nothing
 * 1 speaker waiting
 * 2 power waiting
 */
static uint8_t amp_state = AS_OFF;

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
    TCNT0 = 0xF0; //16 clock cycles

    ms++;
}

//unsigned long int millis(void) { 
 //   return ms; 
//} 

void Timer0_Init(void) {
    TCCR0 = 0x07; //Init Timer0, normal, prescalar = 1024
    TCNT0 = 0xF0; //16 clock cycles
    TIMSK = 0x01; //Set TOIE bit
}


int main (void) { // 3 byte
    // int8_t i;
    long buttonLastChecked = 0; // variable to limit the button getting checked every cycle
    int buttNum, last = 0;
    // uint8_t shiftleds = 0;

    uart_init();
    adc_init();
    shift_init();
    // millis_init();

    // init relays pins
    RELAY_DDR |= (1<<RELAY_P);
    RELAY_DDR |= (1<<(RELAY_P+1));
    RELAY_PORT |= (1<<RELAY_P);
    RELAY_PORT |= (1<<(RELAY_P+1));

    uart_puts("\n\rGo!\n\r");

    while(1) {
        // shift_byte(0b11111111);
        if((UCSRA & (1<<RXC))) {
            uart_gets(cmd, CMDSIZE);
            uart_puts(cmd);
            uart_puts("\n\r");
            // i = uart_getc();
            // uart_putc(i);

            switch(*cmd) {
                case 's':
                    if(cmd[2]>='0' && cmd[2]<'2') {
                        if(cmd[2]>='0' && cmd[2]<'2') {
                            if(cmd[1]=='0') {
                                uart_puts("switch off ");
                                uart_putc(cmd[2]);
                                uart_puts("\n\r");
                                RELAY_PORT |= 1<<(RELAY_P+cmd[2]-'0');
                            } else if(cmd[1]=='1') {
                                uart_puts("switch on ");
                                uart_putc(cmd[2]);
                                uart_puts("\n\r");
                                RELAY_PORT &= ~(1<<(RELAY_P+cmd[2]-'0'));
                            }
                        }
                    }
                    break;
            }
        }

        if(buttonLastChecked > F_CPU/1000) {
            buttNum = read_analog_buttons(AKEY_CHANNEL);
            // if(buttNum) {
            //     uart_puts("Button ");
            //     uart_putc(buttNum + '0');
            //     uart_putc(last + '0');
            //     uart_puts(" was pushed.\n\r");
            // }
            if(buttNum==0 && buttNum!=last) {
                // shift_byte(0b00000000);
            } else if(buttNum==1 && buttNum!=last) { // SPEAKER BUTTON
                switch(amp_state) {
                    case AS_OFF: case AS_AMP_ON: amp_state = AS_WAIT4CHARGE; break;
                    case AS_WAIT4CHARGE: amp_state = AS_OFF; break;
                    case AS_ON: amp_state = AS_SPEAKER_WAIT; break;
                    case AS_SPEAKER_WAIT: amp_state = AS_ON; break;
                    case AS_WAIT_OFF: amp_state = AS_ON; break;
                }
            } else if(buttNum==2 && buttNum!=last) { // AMP BUTTON
                switch(amp_state) {
                    case AS_OFF: amp_state = AS_AMP_ON; break;
                    case AS_AMP_ON: case AS_WAIT4CHARGE: amp_state = AS_AMP_OFF; break;
                    case AS_ON: amp_state = AS_WAIT_OFF; break;
                    case AS_SPEAKER_WAIT: amp_state = AS_WAIT_OFF; break;
                    case AS_WAIT_OFF: amp_state = AS_ON; break;

                }

            } else if(buttNum==3 && buttNum!=last) shift_byte(0b00000100);
            else if(buttNum==4 && buttNum!=last) shift_byte(0b00001000);
            else if(buttNum==5 && buttNum!=last) shift_byte(0b00010000);
            last = buttNum;
            // shift_byte(shiftleds);
            buttonLastChecked = 0;
        } else
            buttonLastChecked++;

        switch(amp_state) {
            case AS_OFF:
                break;
            case AS_WAIT4CHARGE:

                break;
            case AS_AMP_ON:
                break;
            case AS_ON:
                break;
            case AS_SPEAKER_WAIT:
                break;
            case AS_WAIT_OFF:
                break;
        }

    }
}


/* 
* Read 5( or more or less ) buttons using one analog pin and an equal number of
* resistors configured in series across +5 and ground.
* The concept is to create a voltage divider and the switches connect to one
* analog intput pin the voltage at some fixed position in the series circuit.
* The software just checks within a wide range of values and returns a button
* number if a button was determined to have been pushed.
*
* The internal 20K resistor is enabled so we can't just divide the 1023 by the
* number of resistors to get the range for each switch. With the 20K int R, the
* circuit looks like intR is in parallel with the switch resistors above the
* switch pushed. For example, SW2 would be 20K in parallel with 1K resulting in
* an equivalent R of .566K ohms. This then looks almost like there is only 4
* resistors in series and 1023 / 4 = ~255 and 1023-255=~767. Someone could probably
* spend the time to calculate this but I just used imperical measurements and a
* +/- 50 step range.
*
*
*   analogPin                 +5 V
*      |                         |
*      |                         \
*      ----------------          /   
*                     |          \    .5K
*                     |          /
*                     |          \
*                     |____ \____|
*                     |   SW1    |
*                     |          \
*                     |          /   
*                     |          \    .5K
*                     |          /
*                     |          \
*                     |____ \____|
*                     |   SW2    |
*                     |          |
*                     |          \
*                     |          /   
*                     |          \    .5K
*                     |          /
*                     |          \
*                     |____ \____|
*                     |   SW3    |
*                     |          |
*                     |          \
*                     |          /   
*                     |          \    .5K
*                     |          /
*                     |          \
*                     |____ \____|
*                     |   SW4    |
*                     |          |
*                     |          \
*                     |          /   
*                     |          \    .5K
*                     |          /
*                     |          \
*                     |          |
*                     |____ \____|
*                         SW5    |
*                                |
*                                |
*                              _____   
*                               ___     ground
*                                _
*
*/
