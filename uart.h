#ifndef UART_H
#define UART_H

void UART0_init(void);
void UART0_TxChar(char c);
char UART0_RxChar(void);
void sendHelloWorld(void);
void sendStr(const char *str, int n);
void delayMs(int n);

#endif
