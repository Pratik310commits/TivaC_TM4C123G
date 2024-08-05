//*****************************************************************************
/*
 * Date: 12/03/2021
 * Authors: Pratik Bhogale
 * Task: Single user Pong game
 * Comments: Code is for TIVA C 123GXL, with Educational Booster Pack MKii
 */
//*****************************************************************************

// Include standard libraries
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

// Include TIVA libraries
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "driverlib/adc.h"

// Include Booster Pack libraries
#include "ST7735.h"
#include "PLL.h"

#define START_WALL_TOP_Y_COOR 14
#define START_WALL_BOTTOM_Y_COOR 122
#define START_WALL_X_COOR 118


// Functions used
void ConfigureUART(void);
void getMappedADCValue();
void initialiaseSys();
void initialisePortsAndGpios();
void initialiseADC();
uint32_t getYCoordinate(uint32_t adcValue, uint32_t in_min, uint32_t in_max);

// Ball of size 5x5
// Outer single-pixel wide border of ball is white
// Inner 3x3 is the black ball
// Outer white wall helps to clean the trail left behind (at certain angles only)
const uint16_t circle_5[]= {
    0XFFFF, 0XFFFF, 0XFFFF, 0XFFFF, 0XFFFF,
    0XFFFF, 0X0000, 0X0000, 0X0000, 0XFFFF,
    0XFFFF, 0X0000, 0X0000, 0X0000, 0XFFFF,
    0XFFFF, 0X0000, 0X0000, 0X0000, 0XFFFF,
    0XFFFF, 0XFFFF, 0XFFFF, 0XFFFF, 0XFFFF
};

// Paddle is 16x2 arrayof black pixels
// Width is 2 and height is 16 pixels
const uint16_t paddle_2[]= {
    0X0000, 0X0000,0X0000, 0X0000,0X0000, 0X0000,0X0000, 0X0000,0X0000, 0X0000,0X0000, 0X0000,0X0000, 0X0000,0X0000, 0X0000,
     0X0000, 0X0000,0X0000, 0X0000,0X0000, 0X0000,0X0000, 0X0000,0X0000, 0X0000,0X0000, 0X0000,0X0000, 0X0000,0X0000, 0X0000
};

// Brick array is used to create a wall inside a loop
// Better than having huge wall array
const uint16_t brick[]= {0X0000, 0X0000, 0X0000, 0X0000, 0X0000};

//17 different pairs of slopes to start the game with
// changing dy from +ve and -ve, makes this a 34 total pairs of dx-dy
const uint16_t slope[]= {3, 1, 2, 1, 1, 1, 1, 2, 1, 3, 2, 3, 3, 2};


//*****************************************************************************
//
//! UART0, connected to the Virtual Serial Port and running at
//! 115,200, 8-N-1, is used to display messages from this application to the console.
//
//*****************************************************************************

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif




/*
 * Main function that starts the app
 * Responsible for invoking for essential initialization-functions
 * Has the infinite while loop
 *
 */
void ConfigureUART(void);
uint32_t  yCoor;
int main(void)
{

    uint32_t ui32ADC0Value[3];
    // Initialize the system - ports, set clock, UART, gpio, etc
    initialiaseSys();
    UARTprintf("initialiaseSys()\n");


    uint8_t updateFrame = 1;
    while(1)
    {
//        getMappedADCValue(&ui32ADC0Value);
//        UARTprintf("coor:%d\n", yCoor);


        SysCtlDelay(30000);
    }
}

void getMappedADCValue(uint32_t *ui32ADC0Value)
{
    ADCProcessorTrigger(ADC0_BASE, 2);                                  // Give trigger for ADC4 , sequencer 3

    ADCSequenceDataGet(ADC0_BASE, 2, ui32ADC0Value);                   // Get the converted value by sending address of the variable

}



/*
 * This function initializes the important aspects of the system that will be used for the app, viz. Setting clock, ports, ADC, interrupts, etc
 * Input Parameter: Nothing/void
 * Output/Return Parameter: Nothing/void
 */
void initialiaseSys()
{
    ConfigureUART();
    UARTprintf("ConfigureUART()\n");
    UARTprintf("Entering PLL_Init\n");
    PLL_Init(Bus80MHz);
//    UARTprintf("Done PLL_Init\n");
    ST7735_InitR(INITR_REDTAB);

    ST7735_FillScreen(0xFFFF);
    // Set clock at 80 Mhz
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);

    // initialise ports
    initialisePortsAndGpios();
//    UARTprintf("Ports and GPIOs initialised\n");
    // initialise ADC
    initialiseADC();
//    UARTprintf("ADCs initialised\n");
    // initialise interrupts if needed

//    UARTprintf("Clock speed: %d\n", SysCtlClockGet());
}

/*
 * Initializes ports and GPIOs for the app, waits till the peripheral is ready
 *
 * Input Parameter: Nothing/void
 * Output/Return Parameter: Nothing/void
 */
void initialisePortsAndGpios()
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);            // Enable GPIO for ADC0 Module and for random number using Acclr
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD)){}

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);            // Enable GPIO for ADC0 Module
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB)){}


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

    GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_3);             // Set D3 for ADC Input, Y axis of joystick
    GPIOPinTypeADC(GPIO_PORTB_BASE, GPIO_PIN_5);             // Set B5 for ADC Input, X axis of joystick

    // ENABLE ANI7 OF ADCO MODULE. for accelerometer (channel 7)
    // AIN7 = PD0 = 0x01
    GPIOPinTypeADC (GPIO_PORTD_BASE, GPIO_PIN_0);

    ADCSequenceDisable(ADC0_BASE, 2);                        // Disable THE SEQUENCE 2 FOR ADC0
//    UARTprintf("ADC disabled\n");

    ADCReferenceSet(ADC0_BASE, ADC_REF_INT);
//    UARTprintf("ADC Reference Set\n");
    // ADC0 MODULE, TRIGGER IS PROCESSOR EVENT, SEQUENCER 2 IS CONFIGURED
    ADCSequenceConfigure(ADC0_BASE, 2, ADC_TRIGGER_PROCESSOR, 0);
//    UARTprintf("ADC Sequence Configured\n");
    // ADC0 MODULE, SEQUENCER 2
    ADCSequenceStepConfigure(ADC0_BASE, 2, 0, ADC_CTL_CH11);
    ADCSequenceStepConfigure(ADC0_BASE, 2, 1, ADC_CTL_CH4);
    ADCSequenceStepConfigure(ADC0_BASE, 2, 2, ADC_CTL_CH7 | ADC_CTL_END);
//    UARTprintf("ADC Sequence Step Configured\n");
    ADCSequenceEnable(ADC0_BASE, 2);                        // Enable THE SEQUENCER 2 FOR ADC0


}

uint32_t getYCoordinate(uint32_t adcValue, uint32_t in_min, uint32_t in_max)
{
    adcValue = adcValue > in_max ? in_max : adcValue;
    adcValue = adcValue < in_min ? in_min : adcValue;
    uint32_t out_min = 1, out_max = 112, value;
    value = ((adcValue - in_min) * (out_max - out_min) / (in_max - in_min)) + out_min;
//    return value > 64 ? value - 64 : 128 - value;
    return 128 - value;
}

/*
 * Configure the UART and its pins.  This must be called before UARTprintf().
 *
*/
void ConfigureUART(void)
{
    // ******************************* UART 0 ***************************
    // Enable the GPIO Peripheral used by the UART.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // Enable UART0 (used for COM port)
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    // Configure GPIO Pins for UART mode.
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Use the internal 16MHz oscillator as the UART clock source.
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    // Initialize the UART for console I/O.
    // Baud rate is 115200 and 16MHz speed
    UARTStdioConfig(0, 115200, 16000000);

    UARTprintf("ConfigureUART()\n");
    // ******************************* UART 5 ***************************
//    UARTIntDisable(UART5_BASE, UART_INT_RX);
//    // Enable the GPIO Peripheral used by the UART.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    UARTprintf("1\n");
    // Enable UART0 (used for COM port)
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART5);
    UARTprintf("2\n");
    // Configure GPIO Pins for UART mode.
    GPIOPinConfigure(GPIO_PE4_U5RX);
    GPIOPinConfigure(GPIO_PE5_U5TX);
    GPIOPinTypeUART(GPIO_PORTE_BASE, GPIO_PIN_4 | GPIO_PIN_5);
    UARTprintf("3\n");
    // Use the internal 16MHz oscillator as the UART clock source.
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    // Initialize the UART for console I/O.
    // Baud rate is 115200 and 16MHz speed
    UARTConfigSetExpClk(UART5_BASE, 16000000, 115200,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));
    UARTprintf("3\n");
//    UARTIntRegister(UART5_BASE, UART5Handler);
    UARTFIFOEnable(UART5_BASE);
    UARTIntEnable(UART5_BASE, UART_INT_RX);
    UARTprintf("4\n");
}

void UART5Handler(void)
{
    //
    // Go into an infinite loop.
    //
    UARTprintf("Int\n");
    if(UARTCharsAvail(UART5_BASE))
    {
        yCoor = UARTCharGetNonBlocking(UART5_BASE);
//        UARTIntClear(UART5_BASE, UART_INT_RX);
    }
}
