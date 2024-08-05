/*
 * Date: 05/11/2021
 * Authors: Pratik Bhogale
 * Task: Read inputs on pins 1.0 - 1.3 and send appropriate output on pins 2.0 - 2.7
 * Comments: This code is for MSP430G2553
 */


#include <msp430.h>
#include <stdint.h>


unsigned int getADCValue();
void initialiaseSys();
void initialisePorts();
void initialiseADC();

int main(void)
{
    initialiaseSys();
    uint8_t inputData, outputData = 0;
    while(1)
    {
        // Extract the lower 4 bits by appropriate bit mask
        inputData = P1IN & 0x0F;

        if(inputData < 8)
            outputData = 0xFF;
        else if(inputData == 8)
            outputData = 0x01;
        else if(inputData == 9)
            outputData = 0x02;
        else if(inputData == 0x0A)
            outputData = 0x04;
        else if(inputData == 11)
            outputData = 0x08;
        else if(inputData == 12)
            outputData = 0x10;
        else if(inputData == 13)
            outputData = 0x20;
        else if(inputData == 14)
            outputData = 0x40;
        else if(inputData == 15)
            outputData = 0x80;

        // Write the output data to the output port (port 2)
        P2OUT = outputData;

//        __delay_cycles(2000);

    }

    return 0;
}

/*
 * Initialise the system - watchdog timer, ports, enable gpio and interrupts if needed
 */
void initialiaseSys()
{
    //stop watch dog timer
    WDTCTL = WDTPW | WDTHOLD;

    //initialise ports
    initialisePorts();


    //initialise interrupts if needed

}


/*
 * Initialize the ports used in the system, and enable the GPIO functions of the pins used
 */
void initialisePorts()
{
    P1DIR &= ~(BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
//    P1SEL &= ~(BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);

    P2SEL &= ~(BIT6 | BIT7);
    P2DIR |= BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7;

}



