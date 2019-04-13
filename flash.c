/*
 * flash.c
 *
 *  Created on: Jan 21, 2019
 *      Author: Collin Maker
 *
 *  File contains all source code for operating the the Flash chip with SPI commands
 */

//Includes
#include "msp.h"
#include "globals.h"
#include "spi.h"
#include "flash.h"
#include <string.h>

///-----------------------------------------------------------------------------
int Flash_GetDeviceID(uint32_t * id)
{
    uint8_t i;
    uint8_t temp;
    *id = 0;

    SPI_TXFlag = NO;
    SPI_RXFlag = NO; //Expecting data

    P9->OUT &= ~BIT4; //Pull STE Low

    SPI_SendByte(RDID); //Send command
    //while(EUSCI_A3->STATW & UCBUSY);
    for(i = 0; i <4; i++)
    {
        SPI_SendByte(CLOCK_IN);
        SPI_ReadByte(&temp);
        *id |= (temp<<(i*8));
        SPI_RXFlag = NO;
    }
    P9->OUT |= BIT4;
    return 0;
}
//-----------------------------------------------------------------------------
int Flash_WriteData(uint16_t addr, uint8_t * data, uint16_t length)
{
    uint16_t i;
    uint8_t status;

    //Set TX flag low
    SPI_TXFlag = 0;
    //STE low
    P9->OUT &= ~BIT4;

    //WREN or write enable
    SPI_SendByte(WREN);
    P9->OUT |= BIT4;

    //Seems to need this deselect between commands
    P9->OUT &= ~BIT4;

    //Send Write Command
    SPI_SendByte(WRITE);

    //Mask and send address
    SPI_SendByte((addr & 0xFF00)>>8); //Bit 15 to 8
    SPI_SendByte(addr & 0xFF); //Bit 7 to 0

    //Write all data in array
    for(i = 0; i < length; i++)
    {
        SPI_SendByte(*data);
        data++;
    }
    SPI_SendByte(WRDI); //Reset Write Enable Latch
    P9->OUT |= BIT4;

    Flash_GetStatus(&status);
    P9->OUT &= ~BIT4;

    P9->OUT |= BIT4; //STE High
    return 0;
}
//-----------------------------------------------------------------------------
int Flash_ReadData(uint16_t addr, uint8_t * data, uint16_t length)
{
    uint16_t i;
    SPI_TXFlag = 0;
    SPI_RXFlag = 0;

    //STE low
    P9->OUT &= ~BIT4;

    //Send Read Command
    SPI_SendByte(READ);

    //Mask and send address
    SPI_SendByte((addr & 0xFF00)>>8); //Bit 15 to 8
    SPI_SendByte(addr & 0xFF); //Bit 7 to 0

    for(i = 0; i < length; i++)
    {
        SPI_SendByte(CLOCK_IN);
        SPI_ReadByte(data);
        data++;
    }

    P9->OUT |= BIT4;
    return 0;
}
//-----------------------------------------------------------------------------
int Flash_GetStatus(uint8_t * status)
{
    //Set TX flag low
    SPI_TXFlag = 0;
    //STE low
    P9->OUT &= ~BIT4;

    //WREN or write enable
    SPI_SendByte(RDSR);

    //Read in
    SPI_SendByte(CLOCK_IN);
    SPI_ReadByte(status);
    P9->OUT |= BIT4;
    return 0;
}
//-----------------------------------------------------------------------------
int Flash_FormatDevice()
{
    uint8_t index[INDEX_SIZE];
    uint8_t ret;

    memset(index, 0, sizeof(index));
    index[1] = MIN_ADDR;//sets first write address past the initial index all rest of index is 0

    ret = Flash_WriteData(INDEX_START_ADDR, index, INDEX_SIZE);
    return ret;
}
//-----------------------------------------------------------------------------
int Flash_GetMaxIndex(uint8_t * index)
{
    uint8_t indexData[INDEX_SIZE];
    int i;
    Flash_ReadData(INDEX_START_ADDR, indexData, INDEX_SIZE);

    for(i = 4; i < INDEX_SIZE; i = i + 5)
    {
        if(!indexData[i])
        {
            *index = (i / 5);
            return 0;
        }
        //printf("Index %d: Addresses: %X - %X Activated %d\n", i / 5 , (index[i-4]<<8) | index[i-3], (index[i-2]<<8) | index[i-1], index[i]);
    }
    index = 19; //max index size
    return 0;
}
//-----------------------------------------------------------------------------
int Flash_FindOpenAddr(uint16_t * addr, uint8_t * num)
{
    uint8_t index[INDEX_SIZE];
        int i;
        Flash_ReadData(INDEX_START_ADDR, index, INDEX_SIZE);
        for(i = 4; i < INDEX_SIZE; i = i + 5)
        {
            //Search for empty index
            if(!index[i])
            {
                *addr = (index[i-4]<<8) | index[i-3];
                *num = i / 5; //Should round down to proper val
                return 0;
            }
        }
        return -1; //No more space. Write will fail
}
//-----------------------------------------------------------------------------
int Flash_StoreFile(uint8_t * data, uint16_t len)
{
    uint16_t srtAddr, endAddr;
    uint8_t indexUpdate[5];
    uint8_t index;


    if(Flash_FindOpenAddr(&srtAddr,&index))
    {
        return -1; //Failed to write file to storage, no index
    }
    else if((len + srtAddr) > MAX_ADDR)
    {
        return -2; //Failed to write, not enough space
    }

    Flash_WriteData(srtAddr, data, len);

    endAddr = srtAddr + len;

    indexUpdate[0] = (srtAddr & 0xFF00)>>8;
    indexUpdate[1] = srtAddr & 0xFF;
    indexUpdate[2] = (endAddr & 0xFF00)>>8;
    indexUpdate[3] = endAddr & 0xFF;
    indexUpdate[4] = 1;
    Flash_WriteData(index * 5, indexUpdate, 5);
    if(index == MAX_INDEX)
    {
        return 1; //Index is now full
    }
    else //Set next fill address
    {
        srtAddr = endAddr + 1;
        indexUpdate[0] = (srtAddr & 0xFF00)>>8;
        indexUpdate[1] = srtAddr & 0xFF;
        Flash_WriteData((index * 5) + 5, indexUpdate, 2);
        return 0;
    }
}
//-----------------------------------------------------------------------------
int Flash_ReadFile(uint8_t index, uint8_t * data, uint16_t * len)
{
    uint8_t indexData[INDEX_SIZE], i;
    uint16_t srtAddr, endAddr;

    Flash_ReadData(INDEX_START_ADDR, indexData, INDEX_SIZE);

    i = (index * 5) + 4;

    if(!indexData[i])
    {
        return -1; //No file stored in directory
    }
    srtAddr = (indexData[i - 4] << 8) | indexData[i - 3];
    endAddr = (indexData[i - 2] << 8) | indexData[i - 1];
    *len = endAddr - srtAddr;

    /*if(sizeof(data) < *len)
    {
        return -2; //Memory allocated for data out is too small
    }*/

    Flash_ReadData(srtAddr, data, *len);

    return 0;

}
//-----------------------------------------------------------------------------
int Flash_DeleteFile(uint8_t index)
{
    uint8_t maxIndex, i;
    uint8_t currIndex[INDEX_SIZE], localData[MAX_ADDR];
    uint16_t len;


    Flash_GetMaxIndex(&maxIndex);
    if(index >= maxIndex)
    {
        return -1; //Unused index
    }

    //Updating index to reflect delete change then defrag
    Flash_ReadData(INDEX_START_ADDR, currIndex, INDEX_SIZE);
    currIndex[(index * 5) + 4] = 0;
    Flash_WriteData(INDEX_START_ADDR, currIndex, INDEX_SIZE);

    Flash_ReadData(INDEX_START_ADDR, currIndex, INDEX_SIZE);//Debug check

    for(i = index + 1; i < maxIndex; i++)
    {
        Flash_ReadFile(i, localData, &len);
        Flash_StoreFile(localData, len);

        Flash_ReadData(INDEX_START_ADDR, currIndex, INDEX_SIZE);
        currIndex[(i * 5) + 4] = 0;
        Flash_WriteData(INDEX_START_ADDR, currIndex, INDEX_SIZE);
    }
    Flash_ReadData(INDEX_START_ADDR, currIndex, INDEX_SIZE);
    currIndex[(i * 5) + 4] = 0;
    Flash_WriteData(INDEX_START_ADDR, currIndex, INDEX_SIZE);

    return 0;
}
//-----------------------------------------------------------------------------
int Flash_GetMemSize(uint16_t * free, uint16_t * total)
{
    uint8_t localData[INDEX_SIZE];
    uint8_t maxIndex, i;
    uint16_t used;

    Flash_GetMaxIndex(&maxIndex);
    Flash_ReadData(INDEX_START_ADDR, localData, INDEX_SIZE);

    *total = MAX_ADDR - MIN_ADDR;

    maxIndex = ((maxIndex - 1) * 5) + 4;

    used = (localData[maxIndex - 2] << 8) | localData[maxIndex - 1]; //Last addr from initial used

    *free = *total - used;

    return 0;
}
//-----------------------------------------------------------------------------
int Flash_ParseTitle(char * inString, char * outString)
{
    char searchString[2] = "\n\n";
    char * fNameStart;
    char * fNameEnd;
    uint16_t fLen;

    fNameStart = inString;
    fNameEnd = strstr(inString, searchString);
    fLen = (fNameEnd) - fNameStart + 1;  //+1 is room for newline char
    strncpy(outString, fNameStart, fLen);
    outString[fLen] = '\0';

    if (strlen(outString) > 2)
    {
        return 0 ;
    }

    else
    {
        return -1;
    }
}
//-----------------------------------------------------------------------------
int Flash_DisplayIndex(uint8_t * data)
{
    uint8_t maxIndex, i;
    uint8_t parsedTable[MAX_ADDR], parsedTitle[INDEX_SIZE], localData[MAX_ADDR], temp[100];
    uint16_t len;

    memset(parsedTable, 0, sizeof(parsedTable));


    Flash_GetMaxIndex(&maxIndex);
    for(i = 0; i < maxIndex; i++)
    {
        Flash_ReadFile(i, localData, &len);
        Flash_ParseTitle(localData, parsedTitle);

        sprintf(temp, "Index %d, Size %dB : %s\n",i, len, parsedTitle);
        strcat(parsedTable, temp);

    }
    if(i == 0)
    {
        sprintf(data, "Nothing stored in Flash\n");
    }
    memcpy(data, parsedTable, strlen(parsedTable));
    return 0;
}
//-----------------------------------------------------------------------------
void Flash_RTCInit()
{
    PJ->SEL0 |= BIT0 | BIT1;                // set LFXT pin as second function

    CS->KEY = CS_KEY_VAL ;                  // Unlock CS module for register access

    CS->CTL2 |= CS_CTL2_LFXT_EN;            // LFXT on

    // Loop until XT1, XT2 & DCO fault flag is cleared
    do
    {
        // Clear XT2,XT1,DCO fault flags
        CS->CLRIFG |= CS_CLRIFG_CLR_DCOR_OPNIFG | CS_CLRIFG_CLR_HFXTIFG |
                CS_CLRIFG_CLR_LFXTIFG | CS_CLRIFG_CLR_FCNTLFIFG;
        SYSCTL->NMI_CTLSTAT &= ~ SYSCTL_NMI_CTLSTAT_CS_SRC;
    } while ((SYSCTL->NMI_CTLSTAT | SYSCTL_NMI_CTLSTAT_CS_FLG)
            && (CS->IFG & CS_IFG_LFXTIFG)); // Test oscillator fault flag

    // Select ACLK as LFXTCLK
    CS->CTL1 &= ~(CS_CTL1_SELA_MASK) | CS_CTL1_SELA_0;
    CS->KEY = 0;                            // Lock CS module from unintended accesses

    // Configure RTC

    // Unlock RTC key protected registers
    // RTC enable, BCD mode, RTC hold
    // enable RTC read ready interrupt
    RTC_C->CTL0 = RTC_C_KEY | RTC_C_CTL0_TEVIE;
    RTC_C->CTL13 = RTC_C_CTL13_HOLD |
            RTC_C_CTL13_MODE |
            RTC_C_CTL13_BCD;

    RTC_C->YEAR = 0x2019;                   // Year = 0x2016
    RTC_C->DATE = (0x4 << RTC_C_DATE_MON_OFS) | // Month = 0x04 = April
            (0x13 | RTC_C_DATE_DAY_OFS);    // Day = 0x05 = 5th
    RTC_C->TIM1 = (0x06 << RTC_C_TIM1_DOW_OFS) | // Day of week = 0x01 = Monday
            (0x13 << RTC_C_TIM1_HOUR_OFS);  // Hour = 0x10
    RTC_C->TIM0 = (0x21 << RTC_C_TIM0_MIN_OFS) | // Minute = 0x32
            (0x45 << RTC_C_TIM0_SEC_OFS);   // Seconds = 0x45

    // Start RTC calendar mode
    RTC_C->CTL13 = RTC_C->CTL13 & ~(RTC_C_CTL13_HOLD);

    // Lock the RTC registers
    RTC_C->CTL0 = RTC_C->CTL0 & ~(RTC_C_CTL0_KEY_MASK);

    // Enable all SRAM bank retentions prior to going to LPM3 (Deep-sleep)
    SYSCTL->SRAM_BANKRET |= SYSCTL_SRAM_BANKRET_BNK7_RET;
}
//-----------------------------------------------------------------------------
void Flash_GetTimestamp(uint8_t * str)
{
    char temp[100];
    int y, mo, d, h, mi, s;
    y = RTC_C->YEAR;
    mo = RTC_C->DATE >> 8;
    d = RTC_C->DATE & 0xFF;
    h = RTC_C->TIM1 & 0xFF;
    mi = RTC_C->TIM0 >> 8;
    s = RTC_C->TIM0 & 0xFF;
    sprintf(str,"%2.x:%2.x:%2.x %2.x/%2.x/%4.x\n",h,mi,s,mo,d,y);

    return;
}
//-----------------------------------------------------------------------------
void Flash_ReadLastTimeStamps(uint8_t * str)
{
    //hh:mm:ss mm/dd/yyyy\n = len of 20
    //20x10 = 200 addresses
    //start at 0 split every 20
    int i,j;
    uint8_t data[500], outStr[500];
    Flash_ReadData(0x0000, str, 20*10);
    str[(20*10) + 1] = 0;
    //memcpy(str,data,strlen(data));
    return;
}
void Flash_WriteNewTimeStamp()
{
    uint8_t currTime[100],data[500],temp[500];
    memset(currTime,0,sizeof(currTime));

    Flash_ReadData(0x0000, temp, 20*11);
    Flash_GetTimestamp(currTime);
    memcpy(data,currTime,strlen(currTime));
    memcpy(&data[20], temp, 20*10);
    Flash_WriteData(0x0000, data, 20*11);
    return;
}

