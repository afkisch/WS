#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/fpu.h"
#include "driverlib/timer.h"

// hier noch Ihren Filterheader einbinden
#include "fdacoefs.h"

// Praeprozessor-Makros
#define SAMPLERATE 44000
#define FILTERORDER 50

// Funktionen-Deklarationen
void adcIntHandler(void);
void setup(void);

// hier nach Bedarf noch weitere Funktionsdeklarationen einfuegen

// global variables
int32_t bufferSample[FILTERORDER];
int32_t sampleIndex = 0;

// hier nach Bedarf noch weitere globale Variablen einfuegen
int32_t y = 0;

void main(void) // nicht veraendern!! Bitte Code in adcIntHandler einfuegen
{
    setup();
    while(1){}
}

void setup(void){// konfiguriert den Mikrocontroller

    // konfiguriere System-Clock
    SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
    uint32_t period = SysCtlClockGet()/SAMPLERATE;

    // aktiviere Peripherie
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

    // aktiviere Gleitkommazahlen-Modul
    FPUEnable();
    FPUStackingEnable();
    FPULazyStackingEnable();
    FPUFlushToZeroModeSet(FPU_FLUSH_TO_ZERO_EN);

    // konfiguriere GPIO
    GPIOPinTypeADC(GPIO_PORTE_BASE,GPIO_PIN_2);
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE,GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7);

    // konfiguriere Timer
    TimerConfigure(TIMER0_BASE,TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, period - 1);
    TimerControlTrigger(TIMER0_BASE,TIMER_A,true);
    TimerEnable(TIMER0_BASE,TIMER_A);

    // konfiguriere ADC
    ADCClockConfigSet(ADC0_BASE,ADC_CLOCK_RATE_FULL,1);
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_TIMER, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH1|ADC_CTL_IE|ADC_CTL_END);
    ADCSequenceEnable(ADC0_BASE, 3);
    ADCIntClear(ADC0_BASE,3);
    ADCIntRegister(ADC0_BASE,3,adcIntHandler);
    ADCIntEnable(ADC0_BASE,3);

}

void adcIntHandler(void){
   // Bitte Code hier einfuegen
  
    int i;
    uint32_t adcInputValue;
    ADCSequenceDataGet(ADC0_BASE,3,&adcInputValue);

    bufferSample[sampleIndex] = adcInputValue*adcInputValue;
    sampleIndex++;

    if(sampleIndex==FILTERORDER){
      
        for(i=0; i<FILTERORDER; i++){
            y += bufferSample[FILTERORDER-i-1]*num[i];
        }

        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7, 0x00);
      
        if(y > 5) GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, 0xFF);
        if(y > 25) GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, 0xFF);
        if(y > 125) GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, 0xFF);
        if(y > 625) GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_3, 0xFF);
        if(y > 3125) GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, 0xFF);
        if(y > 15625) GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, 0xFF);
        if(y > 78125) GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_6, 0xFF);
        if(y > 390625) GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_7, 0xFF);

        sampleIndex = 0;
        y = 0;
    }
   // am Ende von adcIntHandler, Interrupt-Flag loeschen
   ADCIntClear(ADC0_BASE,3);
}
