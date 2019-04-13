/*
 * bluetooth.h
 *
 *  Created on: Apr 12, 2019
 *      Author: colli
 */

#ifndef BLUETOOTH_H_
#define BLUETOOTH_H_

//Prototypes
void BLE_Init();
int BLE_CheckConnStatus();
void BLE_EchoBack();
void BLE_SendTimestamp();
void BLE_RunCommand();
void BLE_SetTime();

//Macros


#define END_OF_TXT      03  //Char to control the end of commands
#define END_OF_TRAN     04

#define BUFFER_SIZE 1024
#define UART_BUFFER_SIZE 24
#define MAX_CMD_SIZE 10
#define MAX_NAME_SIZE 50

#define CONNECTED       1
#define DISCONNECTED    0
#define NO_COMM         -1

//Globals
//For BLE UART Ring Buffer
uint8_t BLERxBuffer[BUFFER_SIZE];
uint8_t BLERxRead[BUFFER_SIZE];
uint8_t BLETxBuffer[BUFFER_SIZE];
int BLEFlag;
int BLERxWriteIndex;
int BLERxReadIndex;
int BLERxReadTo;
//int RxReadTo2;

#endif /* BLUETOOTH_H_ */
