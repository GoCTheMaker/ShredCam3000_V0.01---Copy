/*
 * usb_uart.h
 *
 *  Created on: Apr 12, 2019
 *      Author: colli
 */

#ifndef USB_UART_H_
#define USB_UART_H_

//Prototypes
void UU_InitBridge();
int UU_CheckFullCommand();
void UU_ReadFromBuffer();
void UU_EchoBack();

//Macros
#define YES 1
#define NO  0


#define END_OF_TXT      03  //Char to control the end of commands
#define END_OF_TRAN     04

#define BUFFER_SIZE 1024
#define UART_BUFFER_SIZE 24
#define MAX_CMD_SIZE 10
#define MAX_NAME_SIZE 50


//Globals
//For USB UART Ring Buffer
uint8_t USBRxBuffer[BUFFER_SIZE];
uint8_t USBRxRead[BUFFER_SIZE];
uint8_t USBTxBuffer[BUFFER_SIZE];
int USBFlag;
int USBRxWriteIndex;
int USBRxReadIndex;
int USBRxReadTo;
//int RxReadTo2;




#endif /* USB_UART_H_ */
