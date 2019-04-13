/*
 * ADC.c
 *
 *  Created on: Apr 13, 2019
 *      Author: colli
 */

//Includes
#include "msp.h"
#include "globals.h"
#include "pinout.h"
///-----------------------------------------------------------------------------
void ADC_Init_Read(void)
{
    P5->SEL0 |= (BIT3|BIT4);
    P5->SEL1 |= (BIT3|BIT4);

    P4->SEL0 |= BIT6;
    P4->SEL1 |= BIT6;

    //MCTL[0]=ADC1 = P5.4
    //MCTL[1]=ADC2 = P5.3
    //MCTL[2]=ADC7 = P4.6
    NVIC->ISER[0] = 1 << ((ADC14_IRQn) & 31);
    ADC14->CTL0 = ADC14_CTL0_SHT0_2 | ADC14_CTL0_SHP |ADC14_CTL0_CONSEQ_1| ADC14_CTL0_ON|ADC14_CTL0_MSC ;
    ADC14->CTL1 = ADC14_CTL1_RES_2;         // Use sampling timer, 12-bit conversion results

    ADC14->MCTL[1]&=~ ADC14_MCTLN_VRSEL_0;
    ADC14->MCTL[0] |= ADC14_MCTLN_INCH_1;   // A1 ADC input select; Vref=AVCC
    ADC14->MCTL[1] |= ADC14_MCTLN_INCH_2;   //ADC A2 Set, Vref = AVCC
    ADC14->MCTL[2] |= ADC14_MCTLN_INCH_7; ///ADC A7 set,

    ADC14->IER0 |= ADC14_IER0_IE0 ;          // Enable ADC conv complete interrupt

    SNS_ON;
    BATRD_ON;

    ADC14->CTL0 |= ADC14_CTL0_ENC | ADC14_CTL0_SC;
}
///-----------------------------------------------------------------------------


