/*
 * spi.c
 *
 *  Created on: Jan 20, 2019
 *      Author: colli
 */

//Includes
#include "msp.h"
#include "globals.h"
#include "spi.h"


void SPI_PortInit()//Potentially make this more configurable and less hard coded....?
{
    SPI_TXWrtIndex = 0; //Resetting indexes for buffer operation
    SPI_TXReadIndex = 0;
    SPI_RXWrtIndex = 0;
    SPI_RXReadIndex = 0;

    EUSCI_A3->CTLW0 = 0x01; //Disable UCA1 during config

    //EUSCI_A3->CTLW0 = 0xEDC3;

    EUSCI_A3->CTLW0 = EUSCI_A_CTLW0_SWRST | //bit1 0, bit6,7 10, bit8 1, bit9,10 10, bit11 1, bit12 0, bit13 1, bit14, 15 00
            //EUSCI_A_CTLW0_TXBRK |           //STE used for enable, SMCLK, Synchronous, 4-pin SPI active low,
            EUSCI_A_CTLW0_SSEL__SMCLK |     //Master mode, 8-bit data, MSB first, SPI mode 0
            EUSCI_A_CTLW0_SYNC|
            //EUSCI_A_CTLW0_MODE_2 |
            EUSCI_A_CTLW0_MST |
            EUSCI_A_CTLW0_MSB|
            //EUSCI_A_CTLW0_CKPL|
            EUSCI_A_CTLW0_CKPH
            ;


    EUSCI_A3->BRW = 1;      //3MHz
    EUSCI_A3->CTLW0 &= ~0x01;   //Re-enable UCA3 after config

    P9->SEL1 &= ~0xE0;
    P9->SEL0 |=  0xE0; //Configure UCA3 pins in SPI3 for SPI communication
    P9->DIR |= 0x10;
    P9->OUT |= 0x10;   //Configure chip select pin

    //Need to configure pins for rest of chip control?

    EUSCI_A3->IE |= 0x03;   //Enable interrupt control of SPI
    EUSCI_A3->IFG &= ~(EUSCI_A_IFG_TXIFG | EUSCI_A_IFG_RXIFG);
    NVIC_EnableIRQ(EUSCIA3_IRQn);
}

int SPI_SendByte(uint8_t txData)
{
    //Check to see if SPI is busy
    //If so return busy code
    //P1->OUT ^= BIT0; //Toggle LED for debugging
    EUSCI_A3->TXBUF = txData;
    while(!SPI_TXFlag);
    SPI_TXFlag = 0;
    return 0;
}

int SPI_SendCommand(uint8_t cmd)
{
    if(EUSCI_A3->TXBUF)
    {
        return 1; //Register already full, wait for completion
    }

    P1->OUT ^= BIT0; //Toggle LED for debugging
    P9->OUT &= ~BIT4;

    EUSCI_A3->TXBUF = cmd; //Should send a single byte

    while(!(EUSCI_A3->IFG & EUSCI_A_IFG_TXIFG));
    //P9->OUT |= BIT4;

    return 0;
}

int SPI_SendData(uint16_t byteCount, uint8_t * dataPointer)
{
    uint16_t i;
    //Should be able to be called even while data transfer in progress
    //Set TX flag to alert program fresh data is in buffer
    //write data for full byte count into buffer

    SPI_TXFlag = YES; //Resets flag to alert interrupt to send data

    //Circular buffer should take data from input data pointer to byteCount
    for(i = SPI_TXWrtIndex; i < byteCount; i = (i + 1) % BUFFER_SIZE)
    {
        SPI_TXBuff[i] = * dataPointer;
        dataPointer++;
    }

    SPI_TXWrtIndex = i; //Set up for next send data command

    return 0; //No error reporting yet
}

//Needs to be fixed
int SPI_ReadByte(uint8_t * rxData)
{

    *rxData = EUSCI_A3->RXBUF;
    SPI_RXFlag = NO;
    return 0;
}

//Needs to be fixed
int SPI_ReadData(uint8_t rxData[])
{
    if(SPI_RXFlag)
    {
        return 1;  //Waiting for buffer to finish receiving
    }

    int i = 0;
    for(SPI_RXReadIndex; SPI_RXReadIndex < SPI_RXWrtIndex; SPI_RXReadIndex = (SPI_RXReadIndex + 1) % BUFFER_SIZE)
    {
        rxData[i++] = SPI_RXBuff[SPI_RXReadIndex];
    }

    return 0;

}


