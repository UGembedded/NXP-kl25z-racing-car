#include "MKL25Z4.h"
#include <stdio.h>
#include "uart.h"
#include "motor.h"
#include "steering.h"

#define CAM_SI_HIGH (PTD->PSOR = (1UL << 7))
#define CAM_SI_LOW  (PTD->PCOR = (1UL << 7))
#define CAM_CK_HIGH (PTE->PSOR = (1UL << 1))
#define CAM_CK_LOW  (PTE->PCOR = (1UL << 1))
#define FRQ_MCGFLLCLK 20971520UL

short int readADC(short ChID);
void camInit(void);
void cam_ReadImage(short int *imgData);
void TPM2_init(unsigned short initMODvalue);
void TPM2_DelayOnce(void);

short int imageData[128];
char buf[100];
int n, i;
int PosLeft = 3300;
int PosRight = 6000;
int PosCentre = 4900;

int main(void)
{
    int leftline, rightline, notCenter, leaningRight;
    unsigned short initMODValue;

    /*assumes a 20.97152 MHz FLL. */
    SIM->SOPT2 = (SIM->SOPT2 & ~((3UL << 24) | (1UL << 16)))
                | (1UL << 24);

    UART0_init();
    camInit();
    ServoInit();
    Motor_Init();

    initMODValue = (unsigned short)((FRQ_MCGFLLCLK + 999999UL)
                                   / 1000000UL - 1UL);
    TPM2_init(initMODValue);
    Motor_ON(1550);

    while (1) {
        cam_ReadImage(imageData);

        for (i = 0; i < 128; i++) {
            imageData[i] = (imageData[i] > 500) ? 0 : 1;
        }

        leftline = 0;
        rightline = 0;
        for (i = 0; i < 48; i++) {
            leftline += imageData[i];
        }
        for (i = 80; i < 128; i++) {
            rightline += imageData[i];
        }

        notCenter = (rightline - leftline > 3)
                    || (leftline - rightline > 3);

        if (notCenter) {
            leaningRight = (rightline > leftline);
            if (leaningRight) {
                n = sprintf(buf, "Turning right\r\n");
            } else {
                n = sprintf(buf, "Turning left\r\n");
            }
            sendStr(buf, n);
            TPM1->CONTROLS[0].CnV = leaningRight ? PosLeft : PosRight;
        } else {
            TPM1->CONTROLS[0].CnV = PosCentre;
        }

        n = sprintf(buf, "%d %d\r\n", leftline, rightline);
        sendStr(buf, n);
    }
}

void camInit(void)
{
    SIM->SCGC5 |= (1UL << 12) | (1UL << 13);

    PORTD->PCR[7] = 0x100;
    CAM_SI_LOW;
    PTD->PDDR |= (1UL << 7);

    PORTE->PCR[1] = 0x100;
    CAM_CK_LOW;
    PTE->PDDR |= (1UL << 1);

    PORTD->PCR[5] = 0;
    SIM->SCGC6 |= (1UL << 27);
    ADC0->SC1[0] = ADC_SC1_ADCH(31);
    ADC0->SC2 &= ~0x40UL;
    ADC0->CFG1 = (1UL << 6) | (1UL << 4) | (1UL << 2);
    ADC0->CFG2 |= (1UL << 4);
}

void cam_ReadImage(short int *imgData)
{
    unsigned int pixel;

    TPM2_DelayOnce();
    TPM2_DelayOnce();
    TPM2_DelayOnce();

    CAM_SI_HIGH;
    TPM2_DelayOnce();
    CAM_CK_HIGH;
    TPM2_DelayOnce();
    CAM_SI_LOW;
    TPM2_DelayOnce();

    CAM_CK_LOW;
    TPM2_DelayOnce();
    TPM2_DelayOnce();


    for (pixel = 0; pixel < 128; pixel++) {
        imgData[pixel] = readADC(6);
        CAM_CK_HIGH;
        TPM2_DelayOnce();
        TPM2_DelayOnce();
        CAM_CK_LOW;
        TPM2_DelayOnce();
        TPM2_DelayOnce();
    }

    CAM_CK_HIGH;
    TPM2_DelayOnce();
    TPM2_DelayOnce();
    CAM_CK_LOW;
    TPM2_DelayOnce();
    TPM2_DelayOnce();
}

short int readADC(short ChID)
{
    ADC0->SC1[0] = ADC_SC1_ADCH(ChID);
    while (!(ADC0->SC1[0] & 0x80U)) { }
    return (short int)ADC0->R[0];
}

void TPM2_init(unsigned short initMODvalue)
{
    SIM->SCGC6 |= (1UL << 26);
    TPM2->SC = 0;
    TPM2->CONF = 0;
    TPM2->CNT = 0;
    TPM2->MOD = initMODvalue;
    TPM2->SC = TPM_SC_TOF_MASK;
}

void TPM2_DelayOnce(void)
{
    TPM2->SC = TPM_SC_TOF_MASK;
    TPM2->CNT = 0;
    TPM2->SC = TPM_SC_CMOD(1);
    while (!(TPM2->SC & TPM_SC_TOF_MASK)) { }
    TPM2->SC = TPM_SC_TOF_MASK;
}
