void uitoa(long int i){
    char const digit[] = "0123456789";
    char b[20];
    char* p = b;
    if(i<0){
        *p++ = '-';
        i *= -1;
    }
    long int shifter = i;
    do{ //Move to where representation ends
        ++p;
        shifter = shifter/10;
    }while(shifter);
    *p = '\0';
    do{ //Move back, inserting digits as u go
        *--p = digit[i%10];
        i = i/10;
    }while(i);
    uart_puts(b);
    uart_puts("\n\r");
}

            if(buttNum && buttNum!=last) {
                uart_puts("Button ");
                uart_putc(buttNum + '0');
                uart_putc(last + '0');
                uart_puts(" was pushed.\n\r");
            } /*else {
                uart_putc(buttNum+'0');
                uart_puts("\n\r");
            }*/

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



        // if(buttonLastChecked > F_CPU/1000) {
        //     buttNum = read_analog_buttons(AKEY_CHANNEL);
        //     if(buttNum && buttNum!=last) {
        //         uart_puts("Button ");
        //         uart_putc(buttNum + '0');
        //         uart_putc(last + '0');
        //         uart_puts(" was pushed.\n\r");
        //     }
        //     if(buttNum==1 && buttNum!=last) { // SPEAKER BUTTON
        //         switch(amp_state) {
        //             case AS_OFF: case AS_AMP_ON: amp_state = AS_WAIT4CHARGE; break;
        //             case AS_WAIT4CHARGE: amp_state = AS_OFF; break;
        //             case AS_ON: amp_state = AS_AMP_ON; break;
        //             case AS_WAIT_OFF: amp_state = AS_ON; break;
        //         }
        //     } else if(buttNum==2 && buttNum!=last) { // AMP BUTTON
        //         switch(amp_state) {
        //             case AS_OFF: amp_state = AS_AMP_ON; break;
        //             case AS_AMP_ON:
        //             case AS_WAIT4CHARGE: amp_state = AS_OFF; break;
        //             case AS_ON: amp_state = AS_WAIT_OFF; break;
        //             case AS_SPEAKER_WAIT: amp_state = AS_WAIT_OFF; break;
        //             case AS_WAIT_OFF: amp_state = AS_ON; break;
        //         }

        //     }
        //     last = buttNum;
        //     buttonLastChecked = 0;
        // } else
        //     buttonLastChecked++;

// #define F_CPU 16000000
// #define BAUD 9600
