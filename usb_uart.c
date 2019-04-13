/*
 * usb_uart.c
 *
 *  Created on: Apr 12, 2019
 *      Author: colli
 */
#include "msp.h"
#include "usb_uart.h"
#include "pinout.h"


//Definitions
//================================================================================
//Initialize the UART Port associated with the USB to UART bridge
//Requires the SMCLK to be set up properly with the DCO
//================================================================================
void UU_InitBridge()
{
    EUSCI_A2->CTLW0 |=1; //Reset register=1 to allow config
    EUSCI_A2->MCTLW = 0;// Disable oversampling
    EUSCI_A2->CTLW0 = 0x0081; //1 S bit, no parity, SMCLK, 8-bit data
    EUSCI_A2->BRW = 26;  /*3MHz SMCLK/115200 = 26.0416 */

    P3->SEL0 |=  0x0C;  //Setup pins 3.3 & 3.2 for UART communication
    P3->SEL1 &=~ 0x0C;

    EUSCI_A2->CTLW0 &=~ 1; //End config of EUSCI
    EUSCI_A2->IE |= 0x01; //Interrupt enable

    NVIC_SetPriority(EUSCIA2_IRQn, 4);
    NVIC_EnableIRQ(EUSCIA2_IRQn);
}

void UU_EchoBack() //Just for test
{
    int i,j = 0;
    uint8_t temp[BUFFER_SIZE];


    //Check for flag
    //Read to index
    //Write to index
    if(USBFlag)
    {
        //Read data from buffer
        for(i=USBRxReadIndex; i!=USBRxWriteIndex; i = (i+1)%BUFFER_SIZE)
        {
            temp[j] = USBRxBuffer[i];   //Read buffer to string
            j++;                        //Increment temp index
        }

        USBRxReadIndex = USBRxWriteIndex;

        for(i = 0; i < j; i++)
        {
            while((EUSCI_A2->IFG&0x02) == 0);
            EUSCI_A2->TXBUF=temp[i];
        }

        USBFlag = 0;//Clear Flag
    }
    return; //If no data, return
}
