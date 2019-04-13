/*
 * spi.h
 *
 *  Created on: Jan 20, 2019
 *      Author: colli
 */

#ifndef SPI_H_
#define SPI_H_

//Prototypes
void SPI_PortInit();
int SPI_SendByte(uint8_t txData);
int SPI_SendCommand(uint8_t cmd);
int SPI_SendData(uint16_t byteCount, uint8_t * dataPointer);
int SPI_ReadByte(uint8_t * rxData);
int SPI_ReadData(uint8_t rxData[]);

//Macros
#define CLOCK_IN 0x00

#define END_OF_TXT      03  //Char to control the end of commands
#define END_OF_TRAN     04

#define BUFFER_SIZE 1024
#define UART_BUFFER_SIZE 24
#define MAX_CMD_SIZE 10
#define MAX_NAME_SIZE 50

//Globals
uint8_t SPI_TXFlag; //Flag set to alert program that data is ready to be sent
uint8_t SPI_RXFlag; //Flag set to alert program that data is ready to be parsed

uint8_t SPI_TXBuff [BUFFER_SIZE]; //Buffers for both received and sent data
uint8_t SPI_RXBuff [BUFFER_SIZE];

uint16_t SPI_TXWrtIndex; //Indexes for buffer operation
uint16_t SPI_TXReadIndex;
uint16_t SPI_RXWrtIndex;
uint16_t SPI_RXReadIndex;



#endif /* SPI_H_ */
