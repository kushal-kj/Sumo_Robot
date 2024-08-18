#ifndef UART_H_
#define UART_H_

// This will initialize the UART peripheral
void uart_init(void);

// This function will send a single character through polling method
void uart_putchar_polling(char c);

// This function will send characters through Interrupt method
void uart_putchar_interrupt(char c);

// To print an entire string of characters
void uart_print_interrupt(const char *string);

#endif /* UART_H_ */
