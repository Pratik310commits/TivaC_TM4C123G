/*
 * Lab: 06
 * Date Created: 05/11/2021
 * Authors: Pratik Bhogale
 * Task: Connecting the TIVA board to the MSP430 board and testing the input and output ports of MSP board by sending appropriate signals and testing the received values on the TIVA board.
 * Comments: This code is for TIVA TM4c123Gxl board. The TIVA board is used as a test emulator to test the MSP430 board
 *
 * Following are the test cases to be examined
 * 1. Checking if Ports are configured correctly
 * 2. Checking if 4 bit input gives correct output
 * 3. Checking if 8 bit input gives correct output (upper 4 bits are being ignored by the MSP board)
 * 4. Checking if output stays consistent over multiple instances of input
 * 5. Checking if output stays stable over long period of time
 */



#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"

/*
 * Functions definitions/prototypes
 */
void configureUART(void);
void initializeSystem();
void initializePortsAndGpios();
void runTestCases();
void send4BitTestValue(uint8_t testValue);
void send8BitTestValue(uint32_t testValue);
uint32_t readResponse();
void testOutput(uint8_t sentVal, uint32_t rcvValue);
uint8_t testPorts();
uint8_t getExpectedOutput(uint8_t sentVal);
void testOutputStability();
void testOutputConsistency();

/*
 * Global variables and constants
 */
#define INPUT_PINS_PORT_A GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7
#define INPUT_PINS_PORT_B GPIO_PIN_2
#define INPUT_PINS_PORT_E GPIO_PIN_0

#define OUTPUT_PINS_PORT_B GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7
#define OUTPUT_PINS_PORT_E GPIO_PIN_4 | GPIO_PIN_5

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

int main(void)
{

    // Initialise the system - ports, clock, GPIOs, interrupts, etc, before starting the app
    initializeSystem();

    // Start running the test cases
    runTestCases();
}

/*
 * Configure the UART and its pins appropriate for UARTprintf.  This must be invoked before UARTprintf().
 * UARTprintf uses VCOM port, which only works with UART 0 - pins A0 A1 are used and other UART ports are still available for use
 * UARTprintf uses the normal console over the selected COM port, eg. COM23
 */
void configureUART(void)
{
    // Enable the GPIO Peripheral used by the UART.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // Enable UART0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    // Configure GPIO Pins for UART mode.

    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Use the internal 16MHz oscillator as the UART clock source.
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    // Initialize the UART for console I/O, for 115200 Baud rate

    UARTStdioConfig(0, 115200, 16000000);
}

/*
 * Initialize the essential components of the system
 * eg. Ports, enable GPIO, set system clock frequency and choose the oscillator
 * ++ Configure UART
 */
void initializeSystem()
{
    // Set clock at 50 Mhz
    SysCtlClockSet(SYSCTL_SYSDIV_4|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ| SYSCTL_OSC_MAIN);

    // initialize ports
    initializePortsAndGpios();

    // initialize UART
    configureUART();

    // initialize interrupts if needed
}

/*
 * Enable the GPIO ports
 * Initialize them as input or output as per the requirement of the app
 */
void initializePortsAndGpios()
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE)){}

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB)){}

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)){}

    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, OUTPUT_PINS_PORT_B);
    GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, OUTPUT_PINS_PORT_E);

    GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, INPUT_PINS_PORT_A);
    GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, INPUT_PINS_PORT_E);
    GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, INPUT_PINS_PORT_B);
}

/*
 * Run the app and run the test cases
 */
void runTestCases()
{
    uint32_t rcvValue = 0;
    // UARTprintf("Clock: %d\n",SysCtlClockGet());
    UARTprintf("Starting test cases...\n");



    UARTprintf("1. Test case 1: Checking if Ports are configured correctly.\n");
    testPorts();


    initializePortsAndGpios();

    UARTprintf("2. Test case 2: Checking if 4 bit input gives correct output.\n");
    uint32_t val = 0;
    // Send data bits from 0 to 15, to check if the MSP board replies with correct values
    for(; val < 16; ++val)
    {
        send4BitTestValue((uint8_t)val);
        SysCtlDelay(SysCtlClockGet()/100);
        rcvValue = readResponse();
        testOutput(val, rcvValue);
    }

    UARTprintf("3. Test case 3: Checking for 8 bit input.\n");
    val = 16;
    // Send data bits from 16 to 255, to check if the MSP board ignores the upper 4 bits on its port 1
    for(; val <= 255; ++val)
    {
        send8BitTestValue((uint8_t)val);
        SysCtlDelay(SysCtlClockGet()/100);

        rcvValue = readResponse();
        testOutput(val, rcvValue);
    }

    UARTprintf("4. Checking if output remains consistent.\n");
    testOutputConsistency();

    UARTprintf("5. Checking if output remains stable over long period of time.\n");
    testOutputStability();

}

/*
 * Send 4 but values to pins 1.0 - 1.3 on MSP board
 */
void send4BitTestValue(uint8_t testValue)
{
    testValue = testValue & 0x0F;
    uint8_t msb = (testValue & 0x08) >> 3;
    msb = msb == 0 ? 0 :  GPIO_PIN_4;

    uint8_t bit2 = (testValue & 0x04) >> 2;
    bit2 = bit2 == 0 ? 0 :  GPIO_PIN_1;

    uint8_t bit1 = (testValue & 0x02) >> 1;
    bit1 = bit1 == 0 ? 0 :  GPIO_PIN_0;

    uint8_t lsb = testValue & 0x01;
    lsb = lsb == 0 ? 0 :  GPIO_PIN_5;


    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, lsb);  //send to output port
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, bit1);  //send to output port
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, bit2);  //send to output port
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_4, msb);  //send to output port
}

/*
* Send 8 but values to Port 1 on MSP board
*/
void send8BitTestValue(uint32_t testValue)
{
//    Send the lower 4 bits first, since the functions is ready to use
    send4BitTestValue(testValue);

    testValue = testValue & 0xF0;

    // Using bit mask, extract the upper 4 bits, shift them to the right accordingly and write the values on the respective pins
    uint32_t lsb = (testValue & 0x10) >> 4;
    lsb = lsb == 0 ? 0 :  GPIO_PIN_5;

    uint32_t bit1 = (testValue & 0x20) >> 5;
    bit1 = bit1 == 0 ? 0 :  GPIO_PIN_4;

    uint32_t bit2 = (testValue & 0x40) >> 6;
    bit2 = bit2 == 0 ? 0 :  GPIO_PIN_6;

    uint32_t msb = (testValue & 0x80) >> 7;
    msb = msb == 0 ? 0 :  GPIO_PIN_7;

    // Write the remaining data to pins 1.4 to 1.7 on MSP board
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_5, lsb);  //send to output port
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, bit1);  //send to output port
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_6, bit2);  //send to output port
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_7, msb);  //send to output port
}

/*
* Read the 8 bit response from MSP 430 board
* Shift the bits accordingly based on the pin position in the 8 bit number
* Create the 8 bit number read from the
*/
uint32_t readResponse()
{
    uint32_t rcvByte = 0;
    uint32_t bitRcv = 0;
    bitRcv = GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_5);
    bitRcv = bitRcv == GPIO_PIN_5 ? 1 : 0;
    rcvByte = bitRcv;

    bitRcv = GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_6);
    bitRcv = bitRcv == GPIO_PIN_6 ? 1 : 0;
    bitRcv = bitRcv << 1;
    rcvByte |= bitRcv;

    bitRcv = GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_7);
    bitRcv = bitRcv == GPIO_PIN_7 ? 1 : 0;
    bitRcv = bitRcv << 2;
    rcvByte |= bitRcv;


    bitRcv = GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_2);
    bitRcv = bitRcv == GPIO_PIN_2 ? 1 : 0;
    bitRcv = bitRcv << 3;
    rcvByte |= bitRcv;


    bitRcv = GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_3);
    bitRcv = bitRcv == GPIO_PIN_3 ? 1 : 0;
    bitRcv = bitRcv << 4;
    rcvByte |= bitRcv;


    bitRcv = GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_4);
    bitRcv = bitRcv == GPIO_PIN_4 ? 1 : 0;
    bitRcv = bitRcv << 5;
    rcvByte |= bitRcv;


    bitRcv = GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_2);
    bitRcv = bitRcv == GPIO_PIN_2 ? 1 : 0;
    bitRcv = bitRcv << 6;
    rcvByte |= bitRcv;


    bitRcv = GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_0);
    bitRcv = bitRcv == GPIO_PIN_0 ? 1 : 0;
    bitRcv = bitRcv << 7;
    rcvByte |= bitRcv;


    return rcvByte;

}

/*
* Test the output from MSP received on TIVA, for each input sent to MSP
* Test it based the expected value for each input
*/
void testOutput(uint8_t sentVal, uint32_t rcvValue)
{
    uint8_t expectedVal = getExpectedOutput(sentVal);

    if(rcvValue == expectedVal)
    {
//         UARTprintf("Sent 0x%X, received 0x%X correctly\n\n", sentVal, rcvValue);
    }
    else
    {
         UARTprintf("ERROR! Sent 0x%X, but received 0x%X, expected was 0x%X\n\n", sentVal, rcvValue, expectedVal);
    }

}

/*
 * Returns the expected value for each input value given/sent to MSP 430 board
 */
uint8_t getExpectedOutput(uint8_t sentVal)
{
    uint8_t expectedVal = 0;
    sentVal = sentVal & 0x0F;
    if(sentVal < 8)
    {
        expectedVal = 0xFF;
    }

    else if(sentVal == 8)
    {
        expectedVal = 0x01;
    }
    else if(sentVal == 9)
    {
        expectedVal = 0x02;
    }

    else if(sentVal == 10)
    {
        expectedVal = 0x04;
    }

    else if(sentVal == 11)
    {
        expectedVal = 0x08;
    }

    else if(sentVal == 12)
    {
        expectedVal = 0x10;
    }

    else if(sentVal == 13)
    {
        expectedVal = 0x20;
    }

    else if(sentVal == 14)
    {
        expectedVal = 0x40;
    }

    else if(sentVal == 15)
    {
        expectedVal = 0x80;
    }

    return expectedVal;
}


/*
 * Test if the input and output ports on the MSP 430 board are configured as per the requirement - Port 1 (1.0 to 1.3) as input, Port 2 (2.0 to 2.7) as output
 */
uint8_t testPorts()
{
    // First test if sending values to port B, gives output at port A
    // Test if changes correct values sent to port A, gives some output at port B

    GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, OUTPUT_PINS_PORT_B);
    GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, OUTPUT_PINS_PORT_E);

    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, INPUT_PINS_PORT_A);
    GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, INPUT_PINS_PORT_E);
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, INPUT_PINS_PORT_B);

    // Send 4 bit data to Port 2 -- A2 A7 A6 A5
    // Receive lower 4 bit data from Port 1
    // Do it multiple times and check if values at port 1 change
    // If the response changes, it means ports on MSO aren't configured according to the requirement


    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2 | GPIO_PIN_7 | GPIO_PIN_6 | GPIO_PIN_5, GPIO_PIN_2); // Sending 8
    SysCtlDelay(SysCtlClockGet()/4);
    uint32_t byte0 = 0, byte1 = 0, byte2 = 0, byte3 = 0;

    byte0 = GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_5) >> 5;
    byte0 |= GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_0) << 1;
    byte0 |= GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_1) << 1;


    byte1 = GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_5) >> 5;
    byte1 |= GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_0) << 1;
    byte1 |= GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_1) << 1;

    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2 | GPIO_PIN_7 | GPIO_PIN_6 | GPIO_PIN_5, GPIO_PIN_2 | GPIO_PIN_6); // Sending 10
    SysCtlDelay(SysCtlClockGet()/4);

    byte2 = GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_5) >> 5;
    byte2 |= GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_0) << 1;
    byte2 |= GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_1) << 1;

    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2 | GPIO_PIN_7 | GPIO_PIN_6 | GPIO_PIN_5, GPIO_PIN_2 | GPIO_PIN_7); // Sending 12
    SysCtlDelay(SysCtlClockGet()/4);
    byte3 = GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_5) >> 5;
    byte3 |= GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_0) << 1;
    byte3 |= GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_1) << 1;

    // Check the test signals' respective responses
    // If they are same, it means that the input didnt change the response so the ports are configured correctly
    // Otherwise the ports on MSP are not configured as desired

    if(!(byte0 == byte1 && byte1 == byte2 && byte1 == byte3))
    {
        UARTprintf("Port 2 configured as Input and Port 1 configured as output\n");
        return 0;
    }
    else
    {
        UARTprintf("\nTesting Port 1 to be as input and Port 2 as output...\n");
        initializePortsAndGpios();
        byte0 = readResponse();
        send4BitTestValue(8);
        SysCtlDelay(SysCtlClockGet()/4);
        byte1 = readResponse();

        send4BitTestValue(9);
        SysCtlDelay(SysCtlClockGet()/4);
        byte2 = readResponse();

        send4BitTestValue(10);
        SysCtlDelay(SysCtlClockGet()/4);
        byte3 = readResponse();

        if(!(byte0 == byte1 && byte1 == byte2 && byte1 == byte3))
        {
            UARTprintf("Port 1 configured as Input and Port 2 configured as output.\nRunning further test cases...\n\n");
            return 1;
        }
        else
        {
            UARTprintf("Port 1 not configured as Input and/or Port 2 not configured as output\n");
            return 0;
        }
    }

}

/*
 * Checking if output stays consistent over multiple instances of input
 * Send same input data multiple times with certain delay and check if the response is consistent
 */
void testOutputConsistency()
{
    uint32_t byte1 = 0, byte2 = 0, byte3 = 0, byte4 = 0;

    send4BitTestValue(8);
    SysCtlDelay(SysCtlClockGet()/4);
    byte1 = readResponse();

    send4BitTestValue(8);
    SysCtlDelay(SysCtlClockGet()/4);
    byte2 = readResponse();

    send4BitTestValue(11);
    SysCtlDelay(SysCtlClockGet()/4);
    byte3 = readResponse();

    send4BitTestValue(11);
    SysCtlDelay(SysCtlClockGet()/4);
    byte4 = readResponse();

    if(byte1 == byte2 && byte3 == byte4)
    {
        UARTprintf("Output is consistent over multiple iterations of same input.\n");
    }
    else
    {
        UARTprintf("Output is not consistent over multiple iterations of same input.\n");
    }

}

/*
 * Checking if output stays stable over long period of time
 * Send same input data multiple times with certain delay and check if the response is stable
 */
void testOutputStability()
{
    uint32_t byte1 = 0, byte2 = 0, byte3 = 0, byte4 = 0;

    send4BitTestValue(8);
    SysCtlDelay(SysCtlClockGet()/4);
    byte1 = readResponse();

    SysCtlDelay(SysCtlClockGet());
    SysCtlDelay(SysCtlClockGet());
    byte2 = readResponse();

    send4BitTestValue(11);
    SysCtlDelay(SysCtlClockGet()/4);
    byte3 = readResponse();

    SysCtlDelay(SysCtlClockGet());
    SysCtlDelay(SysCtlClockGet());
    byte4 = readResponse();

    if(byte1 == byte2 && byte3 == byte4)
    {
        UARTprintf("Output is stable over long period of time.\n");
    }
    else
    {
        UARTprintf("Output is not stable over long period of time.\n");
    }
}
