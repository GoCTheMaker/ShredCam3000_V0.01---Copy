/*
 * pinout.h
 *
 *  Created on: Apr 11, 2019
 *      Author: colli
 */

#ifndef PINOUT_H_
#define PINOUT_H_

//---------------
//Port Pin Macros
//---------------

//Port 1 - LED
#define LED_EN  P1->DIR |= BIT0
#define LED_ON  P1->OUT |= BIT0
#define LED_OFF P1->OUT &= ~BIT0
#define LED_TOG P1->OUT ^= BIT0

//GPIO Enable Lines
//Batt ADC Read
#define BATRD_EN    P2->DIR |= BIT1
#define BATRD_OFF   P2->OUT |= BIT1
#define BATRD_ON    P2->OUT &= ~BIT1
#define BATRD_TOG   P2->OUT ^= BIT1
//Cam Enable
#define CAM_EN  P7->DIR |= BIT5
#define CAM_OFF P7->OUT |= BIT5
#define CAM_ON  P7->OUT &= ~BIT5
#define CAM_TOG P7->OUT ^= BIT5
//BLE Enable
#define BLE_EN  P3->DIR |= BIT4
#define BLE_OFF P3->OUT |= BIT4
#define BLE_ON  P3->OUT &= ~BIT4
#define BLE_TOG P3->OUT ^= BIT4
//SD Enable
#define SD_EN   P10->DIR |= BIT0
#define SD_OFF  P10->OUT |= BIT0
#define SD_ON   P10->OUT &= ~BIT0
#define SD_TOG  P10->OUT ^= BIT0
//Sensor Enable
#define SNS_EN   P6->DIR |= BIT2
#define SNS_OFF  P6->OUT |= BIT2
#define SNS_ON   P6->OUT &= ~BIT2
#define SNS_TOG  P6->OUT ^= BIT2


#endif /* PINOUT_H_ */
