#include "MKL25Z4.h"  
#include "stdio.h"  
#include "UART0TXRX.h"  
#include "motor.h"  
#include "steering.h"  
  
#define CAM_SI_HIGH PTD->PSOR=(0x01<<7);  
#define CAM_SI_LOW PTD->PCOR=(0x01<<7);  
  
#define CAM_CK_HIGH PTE->PSOR=(0x01<<1);  
#define CAM_CK_LOW PTE->PCOR=(0x01<<1);  
  
#define FRQ_MCGFLLCLK 20971520  
  
short int readADC(short ChID);  
  
void camInit(void);  
void cam_ReadImage(short int *imgData);  
  
void TPM0_init(short int initMODvalue);  
void TPM0_DelayOnce(void);  
  
short int imageData[128];  
char buf [100];   // UART buffer  
int n,i;  
int PosLeft = 3300 ;  
int PosRight = 6000;  
int PosCentre = 4900;  
  
int main (void)  
{  
int leftline = 0;  
int rightline = 0;  
short unsigned int initMODValue;  
  
UART0_init();  // Initialized UART0, 57600 baud  
camInit();  
ServoInit();  
    Motor_Init();  
// initialize TPM0 for 1 us delay  
initMODValue=0.5*(float)(FRQ_MCGFLLCLK)/1000000.0; //MOD value for 1us delay  
TPM0_init(initMODValue);  
Motor_ON(1550);  
  
int counter = 0;  
  
while(1) {  
cam_ReadImage(imageData);  
for (i=0;i<128;i++)  
{  
if(imageData[i] > 500) { //sets the color recognition  
imageData[i] = 0;  
} else {  
imageData[i] = 1;  
}  
// n = sprintf(buf, "%d ", imageData[i]);  
// sendStr(buf, n);  
}  
// sendStr("\r\n",2);  
  
leftline = 0;  
rightline = 0;  
  
for (i=0;i<48;i++)  
{  
leftline = leftline + imageData[i];  
}  
  
for (i=80;i<128;i++)  
{  
rightline = rightline + imageData[i];  
}  
  
int notCenter = (rightline - leftline > 3) || (leftline - rightline > 3);  
  
if (notCenter) { //configures the steering  
int leaningRight = (rightline > leftline);  
if(leaningRight) {  
n = sprintf(buf, "Turning right\r\n");  
sendStr(buf, n);  
} else {  
n = sprintf(buf, "Turning left\r\n");  
sendStr(buf, n);  
}  
TPM1->CONTROLS[0].CnV = leaningRight ? PosLeft : PosRight;  
} else {  
TPM1->CONTROLS[0].CnV = PosCentre;  
}  
  
n = sprintf(buf, "%d %d\r\n", leftline, rightline);  
sendStr(buf, n);  
}  
}  
  
  
#define DIFF_SINGLE 0x00  
#define DIFF_DIFFERENTIAL (0x01<<5)  
  
// #define ADC_SC1_ADCH_MASK      0x1Fu    
// #define ADC_SC1_ADCH(x)     (((uint32_t)(x))&ADC_SC1_ADCH_MASK)  
  
void camInit(void)  
{    
SIM->SCGC5 |=(0x1<<12 | 0x1<<13);  
/* enable clock to Port D, E */  
  
    PORTD->PCR[7] = 0x100;     /* make PTD7 pin as GPIO */  
    PTD->PDDR |= (0x1<<7);     /* make PTD7 as output pin */  
  
    PORTE->PCR[1] = 0x100;     /* make PTE1 pin as GPIO */  
    PTE->PDDR |= (0x1<<1);     /* make PTE1 as output pin */  
PORTD->PCR[5] = 0; // PTD5.MUX[10 9 8]=000, analog input  
  
  
SIM->SCGC6 |= 0x08000000;   // enable clock to ADC0 ; 0x8000000u  
  
// Configure ADC as it will be used, but because ADC_SC1_ADCH is 31,  
    // the ADC will be inactive.  Channel 31 is just disable function.  
    // There really is no channel 31.  
// disable AIEN, Signle-ended, channel 31  
    ADC0->SC1[0] = DIFF_SINGLE|ADC_SC1_ADCH(31);    
ADC0->SC2 &= ~0x40;   // ADTRG=0, software trigger  
  
// clock div by 4, long sample time, single ended 12 bit, bus clock  
    ADC0->CFG1 =(0x1<<6 | 0x1<<4 |0x1<<2); //0b01010100;  
//  ADC0->CFG1 = 0x40 | 0x10 | 0x04 | 0x00;  
  
// select the B set of ADC input channels for PTD5 (SE6b)  
ADC0->CFG2 |=(0x1<<4); //CFG2.MUXSEL=1, ADxxb channels are selected;  
  
CAM_SI_LOW;  
CAM_CK_LOW;  
  
}    
  
void cam_ReadImage(short int *imgData)  
{    
unsigned int i;  
// SI (PTD7) Digital output, CLK (PTE1) Digitaloutput  
// AO (PTD5) Analgoue input (channel 6)  
TPM0_DelayOnce();  
TPM0_DelayOnce();  
TPM0_DelayOnce();  
  
CAM_SI_HIGH;  
TPM0_DelayOnce();  
CAM_CK_HIGH;  
TPM0_DelayOnce();  
CAM_SI_LOW;  
TPM0_DelayOnce();  
  
  
imgData[0]=(short int)readADC(6);  
CAM_CK_LOW;  
CAM_CK_LOW;  
TPM0_DelayOnce();  
TPM0_DelayOnce();  
  
for (i=0;i<128;i++)  
{  
imgData[i]=(short int)readADC(6);  
CAM_CK_HIGH;  
TPM0_DelayOnce();  
TPM0_DelayOnce();  
CAM_CK_LOW;  
TPM0_DelayOnce();  
TPM0_DelayOnce();  
}  
  
// additional one CLK to allow ??  
CAM_CK_HIGH;  
TPM0_DelayOnce();  
TPM0_DelayOnce();  
CAM_CK_LOW;  
TPM0_DelayOnce();  
TPM0_DelayOnce();  
}    
  
  
  
short int readADC(short ChID)  
{  
short int result;      
  
ADC0->SC1[0] = ChID; //software triger conversion on channel 13, SE13  
while(!(ADC0->SC1[0] & 0x80)) { } /* wait for conversion complete */  
result = ADC0->R[0];        /* read conversion result and clear COCO flag */  
return result;  
}  
  
// Initialize the TPM0 to generate a specified delay in number of MCGFLLCLK clocks  
// By default, the MCGFLLCLK set by system setup is 20.97152MHz  
void TPM0_init(short int initMODvalue)  
  
{  
  
SIM->SCGC6 |= (0x01<<24); // 0x01000000;, enable clk to TPM0  
SIM->SOPT2 |=(0x01<<24); // 0x01000000, use MCGFLLCLK as timer counter clk  
TPM0->SC = 0; // diable timer when configuring  
TPM0->MOD = initMODvalue;  
TPM0->SC|=0x80; // clear TOF  
}  
  
// Initialize the TPM0 to generate a specified delay in number of MCGFLLCLK clocks  
// By default, the MCGFLLCLK set by system setup is 20.97152MHz  
  
void TPM0_DelayOnce(void)  
{  
TPM0->SC|=0x80; // clear TOF  
TPM0->SC|=0x08; // enable timer free-rnning mode  
while((TPM0->SC & 0x80) == 0) { } // wait until the TOF is set  
TPM0->SC = 0; // diable timer whenÂ configuring  
}
