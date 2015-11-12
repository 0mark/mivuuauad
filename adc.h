#define ADC_PORT PORTC
#define ADC_DDR DDRC

void adc_init();
uint16_t adc_read(uint8_t channel);
