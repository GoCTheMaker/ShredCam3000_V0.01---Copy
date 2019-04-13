/********************************************************
 * EGR 436 Final Project Code "Shred Cam 3000"
 * Written By: Collin Maker and Tom Quist
 * Dr. Brakora
 * System will operate a motion sensor camera device
 *  with bluetooth interfacing
 *******************************************************/



//Library includes
//General
#include "msp.h"
#include <math.h>
//Custom
#include "pinout.h"
#include "usb_uart.h"
#include "globals.h"
#include "bluetooth.h"
#include "spi.h"
#include "flash.h"


//Local Prototypes
void InitHardware();
void InitSoftware();
void InitRTC();
void TimerAInit();

//------------------------------------------------------------------------
//Main Program
//------------------------------------------------------------------------
void main(void)
{
    volatile double i;
    uint32_t id;
    uint8_t data[1000];
    int searchForConn; //Flag to control connection status
    //--------------------------------------------------------------------
	//-------Setup--------------------------------------------------------
    //--------------------------------------------------------------------
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer

    InitHardware();
    //InitSoftware();
    Flash_GetDeviceID(&id);
    Flash_DisplayIndex(data);

    //--------------------------------------------------------------------
    //----Program Loop----------------------------------------------------
    //--------------------------------------------------------------------
    for(;;)
    {

        //Should return here upon timer ISR wakeup after sleep


        //If current mode is to try and establish BLE connection
        if(searchForConn)
        {
            //Clear timer and time out flag
            ConnTimeout = 0;
            TIMER_A0->CTL |= TACLR;

            for(;;)
            {
                BLE_ON; //Enable BLE Module
                if(BLE_CheckConnStatus() == CONNECTED) //Checks for established connection with device
                {
                    searchForConn = 0;  //Connection established, update flag
                    break;
                }
                else if(ConnTimeout)    //Connection timed out
                {
                    BLE_OFF;
                    break;
                }
            }//Exits on connection or timeout
        }//search for connection mode


        //If BLE connection has been established
        if(!searchForConn)
        {
            for(;;)
            {
                if(BLE_CheckConnStatus() == DISCONNECTED)
                {
                    BLE_OFF;
                    searchForConn = 1;
                    break;
                }
                else
                {
                    //BLE_EchoBack();
                    BLE_RunCommand();
                }
            }//Exits loop on disconnect
        }//Connection established mode

        //Sleep when not performing other actions
        __sleep();

    }//Main Loop
    return;
}


//--------------------------------------------------------------------
//-------Support------------------------------------------------------
//--------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------
//Support functions
//-----------------------------------------------------------------------------------------------------------------
//Initialize Hardware
void InitHardware()
{
    __disable_irq();
    LED_EN;
    LED_OFF;
    BLE_EN;
    BLE_ON;
    BLE_Init();
    SPI_PortInit();
    UU_InitBridge();
    TimerAInit();
    Flash_RTCInit();
    __enable_irq();

}
//Initialize Software
void InitSoftware()
{
    //ActiveFlag = 1;
}

void TimerAInit()
{
    TIMER_A0->CTL &= ~0x3F7;//TACL for 0
    TIMER_A0->CTL |= 0x01D2;//ISR EN, Upmode, Div 8, ACLK
    TIMER_A0->EX0 = 0;//1/1 div
    TIMER_A0->CCR[0] = 40960;
    TIMER_A0->CCTL[0] |= 0x10;
    TIMER_A0->CTL |= TACLR;

    NVIC_SetPriority(TA0_0_IRQn, 3);
    NVIC_EnableIRQ(TA0_0_IRQn);
}

void RTC_C_IRQHandler(void)
{
    // Toggles P1.0 every second
    if (RTC_C->PS1CTL & RTC_C_PS1CTL_RT1PSIFG)
    {
        P1->OUT ^= BIT0;

        // Clear the pre-scalar timer interrupt flag
        RTC_C->PS1CTL &= ~(RTC_C_PS1CTL_RT1PSIFG);
    }
   // ActiveFlag = !ActiveFlag;

}
//-----------------------------------------------------------------------------------------------------------------
//Interrupt Service Routines
//-----------------------------------------------------------------------------------------------------------------
//Timer capture compare interrupt
//Currently set to run every 10 seconds
void TA0_0_IRQHandler(void)
{
    P1->OUT ^= BIT0;
    TIMER_A0->CCTL[0] &= ~0x01;
    TIMER_A0->CTL &= ~TAIFG;
    TIMER_A0->CTL |= TACLR;
    ConnTimeout = 1;
}

//Interrupt vector for the UART Communications
//This routine handles data from rx and writes to BLE ring buffer
void EUSCIA0_IRQHandler(void)
{
    BLERxBuffer[BLERxWriteIndex] = EUSCI_A0->RXBUF;   //Takes char from buffer and puts writes to serial buffer
    BLERxWriteIndex = (BLERxWriteIndex + 1) % BUFFER_SIZE;    //Increments the circular buffer write index
    LED_TOG;   //Toggles led for debug
    BLEFlag = 1;   //Set UART flag to begin parsing of the buffer
}

//Interrupt vector for the UART Communications
//This routine handles data from rx and writes to USB ring buffer
void EUSCIA2_IRQHandler(void)
{

    USBRxBuffer[USBRxWriteIndex] = EUSCI_A2->RXBUF;   //Takes char from buffer and puts writes to serial buffer
    USBRxWriteIndex = (USBRxWriteIndex + 1) % BUFFER_SIZE;    //Increments the circular buffer write index
    LED_TOG;   //Toggles led for debug
    USBFlag = 1;   //Set UART flag to begin parsing of the buffer
}

//Interrupt vector for the SPI comm for flash memory
//Handles data to and from registers into buffer
void EUSCIA3_IRQHandler(void)//Does not operate as intended at the moment
{
    if(EUSCI_A3->IFG & UCTXIFG) //Set flag if data transfer is still operating
    {
        SPI_TXFlag = 1; //TX successful
        EUSCI_A3->IFG &= ~UCTXIFG;
    }
    if(EUSCI_A3->IFG & UCRXIFG)
    {
        SPI_RXFlag = 1; //RX successful
        EUSCI_A3->IFG &= ~(UCTXIFG|UCRXIFG);
    }
    return;

}
//Interrupt for ADC which triggers upon finished ADC read.
/*******************************************************************
 * Reads ADC values for the 3 memories
 * Temporarily assigns 3 ints of strength/4096.
 ******************************************************************/
void ADC14_IRQHandler(void)
{
    uint16_t temp1,temp2,temp3;

    ProxInt = ADC14->MEM[0];
    BattInt= ADC14->MEM[1];
    PanelInt = ADC14->MEM[2];


    ADC14->CTL0 &=~ (ADC14_CTL0_ON|ADC14_CTL0_ENC);
    SNS_OFF;
    BATRD_OFF;
}
