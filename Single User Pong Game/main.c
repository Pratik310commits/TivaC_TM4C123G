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
void getMappedADCValue();
void initialiaseSys();
void initialisePortsAndGpios();
void initialiseADC();
void drawPaddleAtPos(int x, int y);
void drawBallAtPos(int x,int y);
void drawWalls();
uint32_t getYCoordinate(uint32_t adcValue, uint32_t in_min, uint32_t in_max);
void calculateDestCoor(uint8_t xi, uint8_t yi, uint8_t aoi, uint8_t wallNumber, uint8_t *xf, uint8_t *yf);
void getRandomSlope(uint8_t index, int dir, int *dx, int *dy);
void drawBallAtNextPos(int *x, int *y, int dx, int dy);
uint8_t isColliding(int ballXCoor, int ballYCoor, int *dx, int *dy, uint32_t paddleXCoor, uint32_t paddleYCoor);
void createBallToStart(int *xi, int *yi);
void initializeBallStartParams(int *direction, uint8_t *index, int *dx, int *dy);

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
 * Configure the UART and its pins.  This must be called before UARTprintf().
 *
*/
void ConfigureUART(void)
{
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
}


/*
 * Main function that starts the app
 * Responsible for invoking for essential initialization-functions
 * Has the infinite while loop
 *
 */

int main(void)
{
    // Initialize the system - ports, set clock, UART, gpio, etc
    initialiaseSys();
//    UARTprintf("initialiaseSys()\n");
    int i = 0;
    uint32_t seed = 0;
    uint32_t ui32ADC0Value[3];
    uint32_t xCoor, yCoor;
    int xi = 0; // initial location
    int yi = 0;
    int dx = 0; //steps to in x-dir
    int dy = 0; //steps to in y-dir
    int direction;
    uint8_t index;

    ST7735_FillScreen(0xFFFF);

    // generate multiple initial couple of values with some delay to settle the ADC
    // use the ADC for seed, for random number generation
    for(i = 0; i < 5; ++i)
    {
        getMappedADCValue(&ui32ADC0Value);
        SysCtlDelay(300000);
//        UARTprintf("ui32ADC0Value[2] : %d");
    }
    seed = ui32ADC0Value[2];
    ST7735_FillScreen(0xFFFF);

    srand(seed);
    UARTprintf("seed: %d\n", seed);
    createBallToStart(&xi, &yi);

    initializeBallStartParams(&direction, &index, &dx, &dy);


//    UARTprintf("index: %d\n", index);
//    UARTprintf("direction: %d\n", direction);
//    UARTprintf("dx: %d\n", dx);
//    UARTprintf("dy: %d\n", dy);
//    UARTprintf("xi, yi: %d, %d\n", xi, yi);

    drawBallAtPos(xi, yi);


    while(getYCoordinate(ui32ADC0Value[1], 0, 3800) > 30)
    {
        getMappedADCValue(&ui32ADC0Value);
//        UARTprintf("js position: %d\n", getYCoordinate(ui32ADC0Value[1], 0, 3800));
        SysCtlDelay(300);
    }

    uint8_t updateFrame = 1;
    while(1)
    {
        if(updateFrame == 0)
        {
            updateFrame = 1;
            ST7735_FillScreen(0xFFFF);
        }
        updateFrame--;
        getMappedADCValue(&ui32ADC0Value);

//        UARTprintf("xADC0Value: %d    yADC0Value: %d\n", ui32ADC0Value[0], ui32ADC0Value[1]);

        yCoor = getYCoordinate(ui32ADC0Value[1], 0, 3800);
//        UARTprintf("xCoor: %d    yCoor: %d\n", xCoor, yCoor);
        drawPaddleAtPos(5, yCoor);
        if(isColliding(xi, yi, &dx, &dy, 5, yCoor) == 1)
        {
            drawBallAtNextPos(&xi, &yi, dx, dy);
        }
        else
        {

            SysCtlDelay(30000000);
            ST7735_FillScreen(0xFFFF);
            ST7735_DrawCharS (60, 60, '3', ST7735_Color565(255, 0, 0), 0, 3);
            SysCtlDelay(30000000);

            ST7735_FillScreen(0xFFFF);
            ST7735_DrawCharS (60, 60, '2', ST7735_Color565(255, 0, 0), 0, 3);
            SysCtlDelay(30000000);

            ST7735_FillScreen(0xFFFF);
            ST7735_DrawCharS (60, 60, '1', ST7735_Color565(255, 0, 0), 0, 3);
            SysCtlDelay(30000000);

            createBallToStart(&xi, &yi);
            initializeBallStartParams(&direction, &index, &dx, &dy);


            drawBallAtPos(xi, yi);
        }

        SysCtlDelay(500000);
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
//    UARTprintf("ConfigureUART()\n");
//    UARTprintf("Entering PLL_Init\n");
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

void drawWalls()
{
    // loop is better since array size is stupidly big

    // top
    uint8_t i = 0;
    for(i = 0; i < 128; ++i)
    {
        ST7735_DrawBitmap(i, 9, brick, 1, 5);
    }

    // right
    for(i = 5; i < 123; ++i)
    {
        ST7735_DrawBitmap(123, i, brick, 5, 1);
    }

    // bottom
    for(i = 0; i < 128; ++i)
    {
        ST7735_DrawBitmap(i, 127, brick, 1, 5);
    }

    // left
//    for(i = 5; i < 123; ++i)
//    {
//        ST7735_DrawBitmap(0, i, brick, 5, 1);
//    }
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

void drawBallAtPos(int x, int y)
{
//    UARTprintf("x, y: %d, %d\n", x, y);
    ST7735_DrawBitmap(x, y, circle_5, 5, 5);
}


void getRandomSlope(uint8_t index, int dir, int *dx, int *dy)
{
    index = index * 2;
//    UARTprintf("slope[%d] = %d\n", index, slope[index]);
    *dx = slope[index] * (-1);
    dir = dir == 0 ? -1 : dir;
    *dy = slope[index + 1] * dir;
}

void drawBallAtNextPos(int *x, int *y, int dx, int dy)
{
    *x = *x + dx;
    *y = *y + dy;
    drawBallAtPos(*x, *y);
}

uint8_t isColliding(int ballXCoor, int ballYCoor, int *dx, int *dy, uint32_t paddleXCoor, uint32_t paddleYCoor)
{
    int nextX, nextY;
    nextX = ballXCoor + *dx;
    nextY = ballYCoor + *dy;
//int xr = 118, yr = 66, xb = 61, yb = 122, xl = 5, yl = 66, xt = 61, yt = 14;

    if(ballXCoor < (paddleXCoor + 2) && (ballYCoor <= paddleYCoor || (ballYCoor >= paddleYCoor - 16)))
    {
        // if balls passes the left wall/paddle area
        return 0;
    }

    if(nextY >= 122 || nextY <= 9)
    {
        // bouncing off the top and bottom walls
        *dy = (-1) * (*dy);
    }

    if(nextX >= 118)
    {
        // bouncing off the right wall
        *dx = (-1) * (*dx);
    }

    if(((nextY <= (paddleYCoor)  && nextY > paddleYCoor - 16) && (nextX <= paddleXCoor + 2)) || ((ballYCoor <= (paddleYCoor)  && ballYCoor > paddleYCoor - 16) && (ballXCoor <= paddleXCoor + 2)))
    {
        //bouncing off the paddle
        *dx = (-1) * (*dx);
    }
    return 1;
}

void drawPaddleAtPos(int x,int y)
{
    ST7735_DrawBitmap(x, y, paddle_2, 2, 16);
}

/*
 * Created a Ball at front (right) wall at a random height
 * 20 pixels from top and bottom are not considered for creating ball's starting height
 *
 * Input Parameter: Addresses of the ball's x and y coordinates
 * Data type "int"
 * Output/Return Parameter: Nothing/void
 */
void createBallToStart(int *xi, int *yi)
{
    // generate height for the ball randomly between given range
    *yi = (rand() % ((START_WALL_BOTTOM_Y_COOR - 20) - (START_WALL_TOP_Y_COOR + 20) + 1)) + START_WALL_TOP_Y_COOR; // 20 pixels buffer from top and bottom walls, for random y-coordinate
    *xi = START_WALL_X_COOR;
}

void initializeBallStartParams(int *direction, uint8_t *index, int *dx, int *dy)
{
    *direction = (rand() % (1 + 1)); // generate direction either 0 or 1
    *index = (rand() % (6 + 1)); // create random index for the dx-dy slop array, between 0 and 16 (17 pairs used)
    getRandomSlope(*index, *direction, dx, dy);
}
