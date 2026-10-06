#include "MKL25Z4.h"
#include "uart.h"

void UART0_init(void)
{
    SIM->SCGC4 |= 0x0400UL;
    SIM->SOPT2 = (SIM->SOPT2 & ~((3UL << 26) | (1UL << 16)))
                | (1UL << 26);

    UART0->C2 = 0;
    UART0->BDH = 0x00;
    UART0->BDL = 0x17;  /* Approximately 57600 baud at 20.97152 MHz. */
    UART0->C4 = 0x0F;
    UART0->C1 = 0x00;
    UART0->C3 = 0x00;

    SIM->SCGC5 |= 0x0200UL;
    PORTA->PCR[2] = PORT_PCR_MUX(2);
    PORTA->PCR[1] = PORT_PCR_MUX(2);
    UART0->C2 = 0x0C;
}

void UART0_TxChar(char c)
{
    while (!(UART0->S1 & 0x80U)) { }
    UART0->D = (unsigned char)c;
}

char UART0_RxChar(void)
{
    while (!(UART0->S1 & 0x20U)) { }
    return (char)UART0->D;
}

void sendStr(const char *str, int n)
{
    int i;
    for (i = 0; i < n; i++) {
        UART0_TxChar(str[i]);
    }
}

void sendHelloWorld(void)
{
    sendStr("Hello World!\r\n", 14);
}

/* Original approximate software delay; not used by the camera. */
void delayMs(int n)
{
    int i;
    volatile int j;
    for (i = 0; i < n; i++) {
        for (j = 0; j < 7000; j++) { }
    }
}
