// ** Config
#define SHIFT_PORT PORTB
#define SHIFT_DDR DDRB
#define SHIFT_PCLOCK PB4
#define SHIFT_PDATA PB3

void shift_init();
void shift_bit(uint8_t v);
void shift_byte(uint8_t v);