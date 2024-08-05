#include "TM4C123.h"
#include <stdbool.h>
#include <stdint.h>
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/adc.h"
#include "C:/ti/tivaware_c_series_2_1_4_178/driverlib/uart.h"
#include "C:/ti/tivaware_c_series_2_1_4_178/utils/uartstdio.h"

#include <stdio.h>
#include "ST7735.h"
#include "PLL.h"
#include "tm4c123gh6pm.h"

#define LCDHEIGHT 128
#define LCDWIDTH 128

#define MAX_X 2240
#define MAX_Y 2236
#define MIN_X 1854
#define MIN_Y 1797

const uint16_t circle_3[]= {
     0XFFFF, 0XFFFF, 0XFFFF, 0XFFFF, 0XFFFF,
     0XFFFF, 0X0000, 0X0000, 0X0000, 0XFFFF,
     0XFFFF, 0X0000, 0X0000, 0X0000, 0XFFFF,
     0XFFFF, 0X0000, 0X0000, 0X0000, 0XFFFF,
     0XFFFF, 0XFFFF, 0XFFFF, 0XFFFF, 0XFFFF,
};

 //defining the rate of LCD cleaning
 #define FLIP_FREQ 4

 //global values definitions
 uint32_t ui32ADC0Value[8] = {0,0,0,0,0,0,0,0};//ADC values read
 uint32_t ui32ADC0Old[2] = {0,0};//ADC old values read

 int xi = 0;// initial location
 int yi = 0;
 int xf = 0;//final location
 int yf = 0;
 int dx = 0;//steps to move
 int dy = 0;

 //functions definition
 void DelayWait10ms (uint32_t n);
 void display_init(void);
 void display(void);
 void circle(int x, int y);

int main()
{
    //Var declaration section
    int flip = FLIP_FREQ;
    int i = 0;
    int j = 0;
    //Set init value at the middle of LCD
    //LCD is 128x128, my circle image is 12x12
    //x index starts at the left edge
    //y index starts at the lower edge of image
    //128/2 - 12/2
    int x = 58;
    //y range is inverted, so I invert it before using
    //52 is 128/2 = 64 and 64 - 12 = 52
    int y = 128 - 52;
    //variable to get the movement direction with smooth movement
    int slope = 0;
    long int difference [2] = {0,0};
    //setting system clock to 80 MHz
    PLL_Init(Bus80MHz);
        // iniltiaze LCD
    ST7735_InitR(INITR_REDTAB);
    //Startup screen with lab # 4 and authors
    while (i<1)
    {
    i++;
    ST7735_DrawCharS(0, 0, 'L', ST7735_Color565 (255, 0, 0), 0, 4);
    ST7735_DrawCharS(30, 0, 'A', ST7735_Color565 (255, 128, 0), 0, 4);
    ST7735_DrawCharS(64, 0, 'B', ST7735_Color565 (255, 255, 9), 0, 4);
    ST7735_DrawCharS(102, 0, '4', ST7735_Color565(128, 255, 0), 0, 4);

    ST7735_DrawCharS (18, 40, 'S', ST7735_Color565(0, 0, 255), 0, 3);
    ST7735_DrawCharS (36, 40, 'U', ST7735_Color565(0, 0, 255), 0, 3);
    ST7735_DrawCharS (54, 40, 'N', ST7735_Color565(0, 0, 255), 0, 3);
    ST7735_DrawCharS (72, 40, 'N', ST7735_Color565(0, 0, 255), 0, 3);
    ST7735_DrawCharS (90, 40, 'Y', ST7735_Color565(0, 0, 255), 0, 3);

    ST7735_DrawCharS(18, 80, 'K', ST7735_Color565(0, 255, 0), 0, 3);
    ST7735_DrawCharS(36, 80, 'A', ST7735_Color565(0, 255, 0), 0, 3);
    ST7735_DrawCharS(54, 80, 'R', ST7735_Color565(0, 255, 0), 0, 3);
    ST7735_DrawCharS(72, 80, 'T', ST7735_Color565(0, 255, 0), 0, 3);
    ST7735_DrawCharS(90, 80, 'M', ST7735_Color565(0, 255, 0), 0, 3);

    DelayWait10ms(500); //add delay
    ST7735_FillScreen(0x0000);
      DelayWait10ms(500);
      }

    //start of [1], [3]
    //set system clock
    SysCtlClockSet (SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

    //initialize ADCO for PD0 and PD1
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0); // ENABLE ADCO MODULE
    // SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1); // ENABLE ADC1 MODULE
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    // setting GPIO for ADCO MODULE
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    // ENABLE ANI6,7 OF ADCO MODULE
    GPIOPinTypeADC (GPIO_PORTD_BASE, 0x03);

    ADCSequenceDisable(ADC0_BASE, 0);
    // ADCO MODULE, TRIGGER IS PROCESSOR EVENT, SEQUENCER O IS CONFIGURED
    ADCSequenceConfigure (ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);
    // ADCO MODULE, SEQUENCER O , FOR 0 SAMPLING, INPUT IS FROM CHANNEL 7 PDB
    ADCSequenceStepConfigure (ADC0_BASE, 0, 0, ADC_CTL_CH7);
    // ADCO MODULE, SEQUENCER 0 , FOR 1 SAMPLING, INPUT IS FROM CHANNEL 6 PD1
    ADCSequenceStepConfigure (ADC0_BASE, 0, 1, ADC_CTL_CH6 | ADC_CTL_IE | ADC_CTL_END);
    // ENABLE THE SEQUENCE 1 FOR ADCO
    ADCSequenceEnable(ADC0_BASE, 0);

    ST7735_FillScreen(0xFFFF);

    xi = 59;
    yi = 54;

    dx = 1;
    dy = 1;

    x = xi;
    y = yi;
    xf = xi;
    yf = yi;
    circle(xi, yi);

while(1)
{

    if (flip == 0)
    {
    ST7735_FillScreen(0xFFFF);
    flip = FLIP_FREQ;
    }
    flip--;

    ADCIntClear(ADC0_BASE, 0);

    ADCProcessorTrigger(ADC0_BASE, 0);

    while (!ADCIntStatus(ADC0_BASE,0,0)){}

    ADCSequenceDataGet(ADC0_BASE, 0, ui32ADC0Value);

    if(ui32ADC0Value [0] > MAX_X - 20)
    {
        ui32ADC0Value [0] = MAX_X - 20;
    }
    else if (ui32ADC0Value [0] <MIN_X)
    {
        ui32ADC0Value [0] = MIN_X + 20;
    }

    if(ui32ADC0Value [1] > MAX_Y - 20)
    {
        ui32ADC0Value [1] = MAX_Y - 20;
    }
    else if (ui32ADC0Value [1] <MIN_Y + 20)
    {
        ui32ADC0Value [1] = MIN_Y + 20;
    }

    xf = (ui32ADC0Value [0] - MIN_X ) / 2;
    yf = (ui32ADC0Value [1] - MIN_Y ) / 2;
    yf = 128 - yf;

    if ((xf == xi) && (yf == yi))
    {
        ST7735_DrawBitmap(xf, yf, circle_3 ,5, 5);
    }
    else if (xf == xi)
    {
        if(yf > yi)
    {
            dy = 1;
            for (i = 0; i < (yf - y) ; i++)
            {
                y = y + dy;
                circle(x, y);
                DelayWait10ms(5);
                }
     }
    else
    {
        dy = -1;
        for (i = 0; i < (y - yf) ; i++)
        {
            y = y + dy;
            circle(x, y);
            DelayWait10ms(5);
        }
     }
   }
    else if (yf == yi)
    {
        if(xf > xi)
        {
            dx = 1;
            for (i = 0; i < (xf - x) ; i++)
            {
                x = x + dx;
                circle(x, y);
                DelayWait10ms(5);
            }
        }
        else
        {
            dx = -1;
            for (i = 0; i < (x - xf) ; i++)
            {
                x = x + dx;
                circle(x, y);
                DelayWait10ms(5);
            }
        }
    }



    else if (xf > xi)
    {
        dx = 1;
        if (yf > yi)
        {
            dy = 1;

            if ((xf - xi) > ( yf - yi))
            {
                slope = (int)((xf - xi) / (yf - yi));
                for (i = 0; i < (yf - y); i++)
                {
                    y=y+dy;
                    for( j =0; j<slope;j++)
                    {
                        x+=dx;
                        circle(x,y);
                         DelayWait10ms(5);
                      }
                 }
                for(i=0;i<((xf-xi)-(slope*(yf-yi)));i++)
                {
                    x+=dx;
                    circle(x,y);
                    DelayWait10ms(5);
                }
            }
            else {
                   slope = (int) ((yf - yi) / (xf - xi));
                   for (i = 0; i < (xf - x); i++)
                       {
                          x +=dx;
                           for ( j= 0; j < slope; j++)
                           {
                               y = y + dy;
                               circle(x, y);
                               DelayWait10ms(5);
                           }
                       }
                   for (i = 0; i < ((yf - yi) - (slope* (xf - xi))); i++)
                   {
                       y+=dy;
                       circle(x, y);
                           DelayWait10ms(5);
                   }
            }
        }
        else{
            dy=-1;
            if ((xf - xi) > (yi - yf))
            {
                slope = (int) ((xf- xi) / (yi - yf));
                for (i = 0; i < (yi - yf); i++)
                {
                    y+= dy;
                    for (j=0;j<slope;j++)
                    {
                        x+=dx;
                        circle(x,y);
                        DelayWait10ms(5);
                    }
                }
                for (i = 0; i < ((xf - xi) - (slope* (yi - yf))); i++)
                {
                    x+=dx;
                    circle(x, y);
                        DelayWait10ms(5);
                }
            }
            else{
                slope = (int) ((yi- yf) / (xf - xi));
                for (i = 0; i < (xf - xi); i++)
                {
                    x+= dx;
                    for (j=0;j<slope;j++){
                        y+=dy;
                        circle(x,y);
                        DelayWait10ms(5);
                    }
                }
                for (i = 0; i < ((yi - yf) - (slope* (xf - xi))); i++)
                {
                    y+=dy;
                    circle(x, y);
                        DelayWait10ms(5);
                }
            }
        }
    }
//--------------------------------------------------------------------------------------------------
    else
    {
        dx = -1;
        if (yf > yi)
        {
            dy = 1;

            if ((xi - xf) > ( yf - yi))
            {
                slope = (int)((xi - xf) / (yf - yi));
                for (i = 0; i < (yf - yi); i++)
                {
                    y=y+dy;
                    for(j =0; j<slope;j++)
                    {
                        x+=dx;
                        circle(x,y);
                        DelayWait10ms(5);
                    }
                }
                for(i=0;i<((xi-xf)-(slope*(yf-yi)));i++)
                {
                    x+=dx;
                    circle(x,y);
                    DelayWait10ms(5);
                }
            }
            else if ( (xi-xf) < (yf-yi) )
            {
                slope = (int) ((xi-xf) / (yf - yi));
                for (i = 0; i < (xi - xf); i++)
                {
                    x +=dx;
                            for ( j= 0; j < slope; j++)
                            {
                                y = y + dy;
                                circle(x, y);
                                DelayWait10ms(5);
                            }
              }
                for (i = 0; i < ((xi-xf) - (slope* (yf-yi))); i++)
                {
                    y+=dy;
                    circle(x, y);
                        DelayWait10ms(5);
                }
            }
        }
        else
        {
            dy=-1;
            if((xi-xf)>(yi-yf))
            {
                slope = (int) ((xi - xf) / (yi-yf));
                for (i = 0; i < (difference[1]*-1); i++)
                {
                    y+= dy;
                    for (j=0;j<slope;j++)
                    {
                        x+=dx;
                        circle(x,y);
                        DelayWait10ms(5);
                    }
                }
                for (i = 0; i < ((xi-xf) - (slope* (yi - yf))); i++)
                {
                    x+=dx;
                    circle(x, y);
                        DelayWait10ms(5);
                }
            }
            else
            {
                   slope = (int) ((yi- yf) / (xi - xf));
                   for (i = 0; i < (xi-xf); i++)
                   {
                       x+= dx;
                       for (j=0;j<slope;j++)
                       {
                       y+=dy;
                       circle(x,y);
                       DelayWait10ms(5);
                       }
                   }
                   for (i = 0; i < ((yi - yf) - (slope* (xi - xf))); i++)
                   {
                       y+=dy;
                       circle(x, y);
                       DelayWait10ms(5);
                   }
            }
        }
    }
    ui32ADC0Old[0]=ui32ADC0Value[0];
    ui32ADC0Old[1]=ui32ADC0Value[1];
    xi=x;
    yi=y;
    }
    return 0;
  }
void circle(int x,int y){
    ST7735_DrawBitmap(x,y,circle_3,5,5);
}
void DelayWait10ms(uint32_t n){
    uint32_t volatile time;
    while(n){
        time=727240*2/91;
        while(time){
            time--;
        }
        n--;
    }
}

void
  ConfigureUART(void)
  {

      //
      // Enable UART0
      //
      SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

      //
      // Configure GPIO Pins for UART mode.
      //
      GPIOPinConfigure(GPIO_PA0_U0RX);
      GPIOPinConfigure(GPIO_PA1_U0TX);
      GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

      //
      // Use the internal 16MHz oscillator as the UART clock source.
      //
      UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

      //
      // Initialize the UART for console I/O.
      //
      UARTStdioConfig(0, 115200, 16000000);
  }
