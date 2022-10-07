/* This file written 2018 by Filip Lukowski and Duy Trung Pham */

#include <stdint.h>
#include <pic32mx.h>

// gets the input from the switches
int getsw( void )
{
    return ((PORTD >> 8) & 0xf);   // bits 11-8 in PORTD tell the input of the 4 switches 
}

int getbtns(void)
{
    return ((PORTD >> 4) & 0xe) | ((PORTF>>1)&0x1);  //bits 8-5 in PORTD tell the input of BTN4, 3 and 2. bit 1 in PORTF tell the input of BTN1 
}