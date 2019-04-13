/*
 * bluetooth.c
 *
 *  Created on: Apr 12, 2019
 *      Author: colli
 */

#include "msp.h"
#include "flash.h"
#include <string.h>
#include "bluetooth.h"
#include "globals.h"

void BLE_Init()
{
    /*ONLY WORKS FOR BAUD OF 19200*/
    EUSCI_A0->CTLW0 |=1; //Reset register=1 to allow config
    EUSCI_A0->MCTLW = 0;// Disable oversampling
    EUSCI_A0->CTLW0 = 0x0081; //1 S bit, no parity, SMCLK, 8-bit data
    EUSCI_A0->BRW = 26;  /*3MHz SMCLK/115200 = 26.0416 */

    P1->SEL0 |=  0x0C;  //Setup pins 1.3 & 1.2 for UART communication
    P1->SEL1 &=~ 0x0C;

    EUSCI_A0->CTLW0 &=~ 1; //End config of EUSCI
    EUSCI_A0->IE |= 0x01; //Interrupt enable

    NVIC_SetPriority(EUSCIA0_IRQn, 2);
    NVIC_EnableIRQ(EUSCIA0_IRQn);
}

int BLE_CheckConnStatus()
{
    int i, tempIndex, readFrom;
    uint8_t temp[8];

    readFrom = BLERxWriteIndex - 7;
    for(i = 0; i < 7; i++)
    {
        tempIndex = readFrom + i;
        if(tempIndex < 0)
        {
            tempIndex = tempIndex + BUFFER_SIZE;
        }
        temp[i] = BLERxBuffer[tempIndex];
    }
    temp[7] = 0;

    if (!strcmp(temp, "OK+CONN"))
    {
        BLEFlag = 1;
        return CONNECTED;
    }
    else if (!strcmp(temp, "OK+LOST"))
    {
        BLEFlag = 1;
        return DISCONNECTED;
    }
    else return NO_COMM;
}

void BLE_EchoBack()
{
    int i,j = 0;
    uint8_t temp[BUFFER_SIZE];


    //Check for flag
    //Read to index
    //Write to index
    if(BLEFlag)
    {
        //Read data from buffer
        for(i=BLERxReadIndex; i!=BLERxWriteIndex; i = (i+1)%BUFFER_SIZE)
        {
            temp[j] = BLERxBuffer[i];   //Read buffer to string
            j++;                        //Increment temp index
        }

        BLERxReadIndex = BLERxWriteIndex;

        for(i = 0; i < j; i++)
        {
            while((EUSCI_A0->IFG&0x02) == 0);
            EUSCI_A0->TXBUF=temp[i];
        }

        BLEFlag = 0;//Clear Flag
    }
    return; //If no data, return

}

void BLE_RunCommand()
{
    int i, tempIndex, readFrom;
    uint8_t temp[8];
    if(BLEFlag)
    {
        readFrom = BLERxWriteIndex - 7;
        for(i = 0; i < 7; i++)
        {
            tempIndex = readFrom + i;
            if(tempIndex < 0)
            {
                tempIndex = tempIndex + BUFFER_SIZE;
            }
            temp[i] = BLERxBuffer[tempIndex];
        }
        temp[7] = 0;

        if (!strcmp(temp, "OK+TIME"))
        {
            BLE_SendTimestamp();
            BLEFlag = 0;
            return;
        }
        else if (!strcmp(temp, "OK+PICT"))
        {
            //Take a pic or run that code
        }
        else if (!strcmp(temp, "OK+TSET"))
        {
            BLE_SetTime();
            BLEFlag = 0;
            return;
        }
        //add addtl commands here
        else return;
    }//BLEFlag
}

void BLE_SendTimestamp()
{
    char str[500];
    uint8_t len, i;

    memset(str,0,sizeof(str));
    Flash_GetTimestamp(str);


    Flash_ReadLastTimeStamps(str);
    Flash_WriteNewTimeStamp();
    len = strlen(str);

    for(i = 0; i <= len; i++)
    {
        while((EUSCI_A0->IFG&0x02) == 0);
        EUSCI_A0->TXBUF=str[i];
    }
    return;
}

void BLE_SetTime()
{
    int flag = 0;

    int i, tempIndex, readFrom, len;
    int h,mi,s,mo,d,y;

    uint8_t * ptr;
    uint8_t temp[15];
    uint8_t str[100];
    uint64_t buffer;

    sprintf(str, "Send in form\nhhmmssmmddyyyy:\n");
    len = strlen(str);
    for(i = 0; i < len; i++)
    {
        while((EUSCI_A0->IFG&0x02) == 0);
        EUSCI_A0->TXBUF=str[i];
    }

    ConnTimeout = 0;
    TIMER_A0->CTL |= TACLR;
    while(!ConnTimeout)
    {
        if(BLERxBuffer[BLERxWriteIndex - 1] == ':')
        {
            flag = 1;
            break;
        }
    }

    if (flag = 0)
    {
        return; //timeout
    }
    else
    {
        //hhmmssmmddyyyy = 14 chars
            readFrom = BLERxWriteIndex - 15;
            for(i = 0; i < 14; i++)
            {
                tempIndex = readFrom + i;
                if(tempIndex < 0)
                {
                    tempIndex = tempIndex + BUFFER_SIZE;
                }
                temp[i] = BLERxBuffer[tempIndex];
            }
            temp[15] = 0;

            h = ((temp[0] - 48) << 4) | (temp[1] - 48);
            mi = ((temp[2] - 48) << 4) | (temp[3] - 48);
            s = ((temp[4] - 48) << 4) | (temp[5] - 48);
            mo = ((temp[6] - 48) << 4) | (temp[7] - 48);
            d = ((temp[8] - 48) << 4) | (temp[9] - 48);
            y = ((temp[10] - 48) << 12) | ((temp[11] - 48) << 8) | ((temp[12] - 48) << 4) | ((temp[13] - 48));

            // Unlock RTC key protected registers
            // RTC enable, BCD mode, RTC hold
            // enable RTC read ready interrupt
            RTC_C->CTL0 = RTC_C_KEY | RTC_C_CTL0_TEVIE;
            RTC_C->CTL13 = RTC_C_CTL13_HOLD |
                    RTC_C_CTL13_MODE |
                    RTC_C_CTL13_BCD;

            RTC_C->YEAR = y;                   // Year = 0x2016
            RTC_C->DATE = (mo << RTC_C_DATE_MON_OFS) | // Month = 0x04 = April
                    (d | RTC_C_DATE_DAY_OFS);    // Day = 0x05 = 5th
            RTC_C->TIM1 = (0x06 << RTC_C_TIM1_DOW_OFS) | // Day of week = 0x01 = Monday
                    (h << RTC_C_TIM1_HOUR_OFS);  // Hour = 0x10
            RTC_C->TIM0 = (mi << RTC_C_TIM0_MIN_OFS) | // Minute = 0x32
                    (s << RTC_C_TIM0_SEC_OFS);   // Seconds = 0x45

            // Start RTC calendar mode
            RTC_C->CTL13 = RTC_C->CTL13 & ~(RTC_C_CTL13_HOLD);

            // Lock the RTC registers
            RTC_C->CTL0 = RTC_C->CTL0 & ~(RTC_C_CTL0_KEY_MASK);


            //buffer = strtol(temp, &ptr, 16);
            return;

    }














}
