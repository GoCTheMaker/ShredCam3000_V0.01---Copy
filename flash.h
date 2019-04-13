/*
 * flash.h
 *
 *  Created on: Jan 21, 2019
 *      Author: Collin
 */

#ifndef FLASH_H_
#define FLASH_H_

//Macros
//Flash memory commands
#define WREN    0b00000110  //Set write enable latch
#define WRDI    0b00000100  //Reset write enable latch
#define RDSR    0b00000101  //Read status register
#define WRSR    0b00000001  //Write status register
#define READ    0b00000011  //Read memory code
#define WRITE   0b00000010  //Write memory code
#define RDID    0b10011111  //Read device ID

#define INDEX_SIZE 100
#define FILE_SLOTS 20
#define INDEX_START_ADDR 0x0000
#define  MIN_ADDR         0x0065
#define  MAX_ADDR         0x1FFF
#define  MAX_INDEX        19

/*******************************************************************
 * When passed pointer will return device ID, mostly used for
 *  debugging
 *  id: Value of register
 ******************************************************************/
int Flash_GetDeviceID(uint32_t * id);
/*******************************************************************
 * Writes data to the flash device directly
 *  addr: 16 Bit address (13 bits used)
 *  data: Pointer to data
 *  length: Amount of bytes sent
 ******************************************************************/
int Flash_WriteData(uint16_t addr, uint8_t * data, uint16_t length);
/*******************************************************************
 * Reads data from flash device directly
 *  addr: 16 Bit address (13 bits used)
 *  data: Pointer to data returned
 *  length: Amount of bytes recieved
 ******************************************************************/
int Flash_ReadData(uint16_t addr, uint8_t * data, uint16_t length);
/*******************************************************************
 * Reads status register of flash device
 *  status: Returns encoded flash status register
 ******************************************************************/
int Flash_GetStatus(uint8_t * status);
/*******************************************************************
 * Formats the flash device by zeroing the index except for the
 *  first data write addr
 ******************************************************************/
int Flash_FormatDevice();
/*******************************************************************
 * Gets highest value index from index
 *  index: Value of highest index
 ******************************************************************/
int Flash_GetMaxIndex(uint8_t * index);
/*******************************************************************
 * Finds an open address to write new data to
 *  addr: Returns possible address
 *  num: Index new data would take in index table
 ******************************************************************/
int Flash_FindOpenAddr(uint16_t * addr, uint8_t * num);
/*******************************************************************
 * Stores a new file, handles all index and pointer updates
 *  data: data to write
 *  len: size of data to write
 ******************************************************************/
int Flash_StoreFile(uint8_t * data, uint16_t len);
/*******************************************************************
 * Reads file from flash based on index
 *  index: Val of index to read
 *  data: Returned file pointer
 *  len: Size of file
 ******************************************************************/
int Flash_ReadFile(uint8_t index, uint8_t * data, uint16_t * len);
/*******************************************************************
 * Deletes a file, handles all defrag and index table updates
 *  index: Index val of file to be deleted
 ******************************************************************/
int Flash_DeleteFile(uint8_t index);
/*******************************************************************
 * Gets the amount of storage used and how much is left
 *  free: Space free
 *  total: Total Space
 ******************************************************************/
int Flash_GetMemSize(uint16_t * free, uint16_t * total);
/*******************************************************************
 * Parses titles from common input text files
 *  inString: File input
 *  outString: Output title string pointer
 ******************************************************************/
int Flash_ParseTitle(char * inString, char * outString);
/*******************************************************************
 * Returns a string of formatted table data
 ******************************************************************/
int Flash_DisplayIndex(uint8_t * data);
/*******************************************************************
 * Initializes RTC for flash timestamps
 ******************************************************************/
void Flash_RTCInit();
/*******************************************************************
 * Returns a string of formatted timestamp data
 ******************************************************************/
void Flash_GetTimestamp(uint8_t * str);
void Flash_ReadLastTimeStamps(uint8_t * str);
void Flash_WriteNewTimeStamp();

#endif /* FLASH_H_ */
