/*
 * Lab: 05
 * Date Created: 25/10/2021
 * Authors: Pratik Bhogale
 * Task: Connecting a potentiometer to an ADC input pin, reading the analog value, then expressing that voltage on a 7-segment display (as 4 bit Hex)
 * Comments: This code is for TIVA TM4c123Gxl board, with 12-Bit ADC. Port B is used as output, Pin 3 of port E is input to the ADC from Potentiometer
 */


#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"


uint8_t getMappedADCValue();
void initialiaseSys();
void initialisePortsAndGpios();
void initialiseADC();
void displaySevenSegment(uint8_t value);

const uint8_t SEVENSEG_OUTPUT_AL[16] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90,
                                            0x88, 0x83, 0xC6, 0xA1, 0x86, 0x8E}; //7-seg LED display from "0" to "F" -- For Common Anode LED(Active Low)


#define ALL_PINS GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6



int main(void)
{
    initialiaseSys();
    uint8_t value = 0;
    while(1)
    {

        value = getMappedADCValue();
        displaySevenSegment(value);
    }
    return 0;
}


/*
 * Collects the value from the ADC output and returns the value mapped from 0 to 15
 * Sends trigger for the ADC to generate the value
 *
 * Input Parameter: Nothing/void
 * Output/Return  Parameter: Unsigned 8 bit integer
 */
uint8_t getMappedADCValue()
{
    uint32_t ui32ADC0Value;

    ADCProcessorTrigger(ADC0_BASE, 3);                                  // Give trigger for ADC0 , sequencer 3

    ADCSequenceDataGet(ADC0_BASE, 3, &ui32ADC0Value);                   // Get the converted value by sending address of the variable

    ui32ADC0Value = ui32ADC0Value & 0x0FFF;                             // Get only the least significant 12 bits

    if(ui32ADC0Value < 256)
        return 0;
    else if(ui32ADC0Value < 512)
        return 1;
    else if(ui32ADC0Value < 768)
        return 2;
    else if(ui32ADC0Value < 1024)
        return 3;
    else if(ui32ADC0Value < 1280)
        return 4;
    else if(ui32ADC0Value < 1536)
        return 5;
    else if(ui32ADC0Value < 1792)
        return 6;
    else if(ui32ADC0Value < 2048)
        return 7;
    else if(ui32ADC0Value < 2304)
        return 8;
    else if(ui32ADC0Value < 2560)
        return 9;
    else if(ui32ADC0Value < 2816)
        return 10;
    else if(ui32ADC0Value < 3072)
        return 11;
    else if(ui32ADC0Value < 3328)
        return 12;
    else if(ui32ADC0Value < 3584)
        return 13;
    else if(ui32ADC0Value < 3840)
        return 14;
    else if(ui32ADC0Value < 4096)
        return 15;

}

/*
 * This function initializes the important aspects of the system that will be used for the app, viz. Setting clock, ports, ADC, interrupts, etc
 * Input Parameter: Nothing/void
 * Output/Return Parameter: Nothing/void
 */
void initialiaseSys()
{
    // Set clock at 40 Mhz
    SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ| SYSCTL_OSC_MAIN);

    // initialise ports
    initialisePortsAndGpios();

    // initialise ADC
    initialiseADC();

    // initialise interrupts if needed
}

/*
 * Initializes ports and GPIOs for the app, waits till the peripheral is ready
 *
 * Input Parameter: Nothing/void
 * Output/Return Parameter: Nothing/void
 */
void initialisePortsAndGpios()
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);            // Enable GPIO for ADC0 Module
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE)){}

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);            // Enable GPIO for ADC0 Module
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB)){}

    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, ALL_PINS);
}

/*
 * Initializes ADCs used for the app, waits till the peripheral is ready
 * Configures the sequencer, step and enables the ADC
 *
 * Input Parameter: Nothing/void
 * Output/Return Parameter: Nothing/void
 */
void initialiseADC()
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);             // Enable ADC0 Module
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0)){}

    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);             // Set E3 for ADC Input

    ADCSequenceDisable(ADC0_BASE, 3);                        // Disable THE SEQUENCE 3 FOR ADC0

    ADCReferenceSet(ADC0_BASE, ADC_REF_INT);

    // ADC0 MODULE, TRIGGER IS PROCESSOR EVENT, SEQUENCER 3 IS CONFIGURED
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);

    // ADC0 MODULE, SEQUENCER 3 , FOR 1 SAMPLING, INPUT IS FROM CHANNEL 0 PE3
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END);

    ADCSequenceEnable(ADC0_BASE, 3);                        // Enable THE SEQUENCE 3 FOR ADC0


}

/*
 * Display the given 4 bit Hex value on the 7-segment LED
 *
 * Input Parameter: Unsigned 8 bit integer Value to be display,
 * Output/Return Parameter: Nothing/void
 */
void displaySevenSegment(uint8_t value)
{
    uint8_t displayValue = SEVENSEG_OUTPUT_AL[value];       //get the hex character to be displayed
    GPIOPinWrite(GPIO_PORTB_BASE, ALL_PINS, displayValue);  //send to output port
}
