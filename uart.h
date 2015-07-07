void uart_init();
int uart_putc(unsigned char c);
void uart_puts (char *s);
uint8_t uart_getc();
void uart_gets(char *buffer, uint8_t size);