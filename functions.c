/* This file written 2015 by F Lundevall
   Some parts are original code written by Axel Isaksson
   
   Modified 2022 by Filip Lukowski and Duy Trung Pham

   For copyright and licensing, see file COPYING */

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include <stdbool.h> /* Use of bool */
#include <stdlib.h>  // use of rand()
#include "mipslab.h"

/* Declare a helper function which is local to this file */
static void num32asc( char * s, int );

#define DISPLAY_CHANGE_TO_COMMAND_MODE (PORTFCLR = 0x10)
#define DISPLAY_CHANGE_TO_DATA_MODE (PORTFSET = 0x10)

#define DISPLAY_ACTIVATE_RESET (PORTGCLR = 0x200)
#define DISPLAY_DO_NOT_RESET (PORTGSET = 0x200)

#define DISPLAY_ACTIVATE_VDD (PORTFCLR = 0x40)
#define DISPLAY_ACTIVATE_VBAT (PORTFCLR = 0x20)

#define DISPLAY_TURN_OFF_VDD (PORTFSET = 0x40)
#define DISPLAY_TURN_OFF_VBAT (PORTFSET = 0x20)


extern int fallenBlocks[27][10]; //creating the array that represents the tetris board
int nOfTetris = 0;
int delayT = 0;

/* quicksleep:
   A simple function to create a small delay.
   Very inefficient use of computing resources,
   but very handy in some special cases. */
void quicksleep(int cyc) {
	int i;
	for(i = cyc; i > 0; i--);
}

uint8_t spi_send_recv(uint8_t data) {
	while(!(SPI2STAT & 0x08));
	SPI2BUF = data;
	while(!(SPI2STAT & 1));
	return SPI2BUF;
}

void display_init(void) {
        DISPLAY_CHANGE_TO_COMMAND_MODE;
	quicksleep(10);
	DISPLAY_ACTIVATE_VDD;
	quicksleep(1000000);
	
	spi_send_recv(0xAE);
	DISPLAY_ACTIVATE_RESET;
	quicksleep(10);
	DISPLAY_DO_NOT_RESET;
	quicksleep(10);
	
	spi_send_recv(0x8D);
	spi_send_recv(0x14);
	
	spi_send_recv(0xD9);
	spi_send_recv(0xF1);
	
	DISPLAY_ACTIVATE_VBAT;
	quicksleep(10000000);
	
	spi_send_recv(0xA1);
	spi_send_recv(0xC8);
	
	spi_send_recv(0xDA);
	spi_send_recv(0x20);
	
	spi_send_recv(0xAF);
}



void display_string(int line, char *s) {
	int i;
	if(line < 0 || line >= 4)
		return;
	if(!s)
		return;
	
	for(i = 0; i < 16; i++)
		if(*s) {
			textbuffer[line][i] = *s;
			s++;
		} else
			textbuffer[line][i] = ' ';
}

/* a funtion derived from the original display_string function so that and index can also be inputted.
This allowed us to be able to intput more than stuff in one line*/
void display_string_index(int line, int index, char *s) {
    int i;
    if(line < 0 || line >= 4)
        return;
    if(!s)
        return;

    for(i = 0; i < 16; i++)
        if(*s) {
            textbuffer[line][i + index] = *s; // index + i shifts the outputted word i spaces to the right (16 places overall on the screen)
            s++;
        } else
            textbuffer[line][i + index] = ' ';
}

//this function gives the number or digits in a string of integers. It will be used to display the current score
int intNOfUnits (int n) {
    if (n < 10) return 1;
    if (n < 100) return 2;
    if (n < 1000) return 3;
    if (n < 10000) return 4;
    if (n < 100000) return 5;
    if (n < 1000000) return 6;
    if (n < 10000000) return 7;
    if (n < 100000000) return 8;
    if (n < 1000000000) return 9;
    return 10;
}

// a power function. It return n^(n1)
int pow(int n, int n1){
	int i;
	int result = n;
	if(n1 == 0){
		return 1;
	}
	for (i = 1; i<n1;i++){
		result = result * n;
	}
	return result;
}

//it makes all the pixels go black, which clears the display allowing other things to be displayed
void clearDisplay(void)
{
	int i;
  	for (i = 0; i < 512; i++)
  	{
		display[i] = 255; // display array is declared in data.c
  	}
}

//the function draw a pixel at coordinate (x,y)
void drawPixel(int x, int y) {
	display[y/8 * 128 + x] &= (~(1 << (y-(y/8)*8)));	//draws pixel: y/8*128 is offset per page. Using bitwise NAND allows us to change specific pixels
}

// draws the border using the drawPixel function
void drawBorder(void){

	int i;

	//Long sides
	for(i = 0; i < 128; i++){
		drawPixel(i, 0);
		drawPixel(i, 31);
	}
	//Short sides
	for(i = 0; i < 31; i++){
		drawPixel(0, i);
		drawPixel(127, i);
		drawPixel(110, i);
		drawPixel(82, i);
	}
	return;
}

void drawText(void) {
	//Draw the word "NEXT" pixel by pixel.
	
	//N
	drawPixel(105,4);
	drawPixel(104,4);
	drawPixel(103,4);
	drawPixel(102,4);
	drawPixel(101,4);
	drawPixel(104,5);
	drawPixel(103,6);
	drawPixel(102,7);
	drawPixel(101,8);
	drawPixel(102,8);
	drawPixel(103,8);
	drawPixel(104,8);
	drawPixel(105,8);
	
	//E
	drawPixel(105,10);
	drawPixel(104,10);
	drawPixel(103,10);
	drawPixel(102,10);
	drawPixel(101,10);
	drawPixel(105,11);
	drawPixel(105,12);
	drawPixel(105,13);
	drawPixel(103,11);
	drawPixel(103,12);
	drawPixel(103,13);
	drawPixel(101,11);
	drawPixel(101,12);
	drawPixel(101,13);

	//X
	drawPixel(105,15);
	drawPixel(104,16);
	drawPixel(103,17);
	drawPixel(102,16);
	drawPixel(101,15);
	drawPixel(101,19);
	drawPixel(102,18);
	drawPixel(104,18);
	drawPixel(105,19);
	
	
	//T
	drawPixel(105,21);
	drawPixel(105,22);
	drawPixel(105,23);
	drawPixel(105,24);
	drawPixel(105,25);
	drawPixel(104,23);
	drawPixel(103,23);
	drawPixel(102,23);
	drawPixel(101,23);

	//draws the word SCORE pixel by pixel.

	//S
	drawPixel(125,5);
	drawPixel(124,4);
	drawPixel(123,5);
	drawPixel(123,6);
	drawPixel(125,6);
	drawPixel(122,7);
	drawPixel(121,6);
	drawPixel(121,5);

	//C
	drawPixel(125,10);
	drawPixel(125,11);
	drawPixel(124,9);
	drawPixel(123,9);
	drawPixel(122,9);
	drawPixel(121,10);
	drawPixel(121,11);

	//O
	drawPixel(125,14);
	drawPixel(125,15);
	drawPixel(124,13);
	drawPixel(123,13);
	drawPixel(122,13);
	drawPixel(121,14);
	drawPixel(121,15);
	drawPixel(124,16);
	drawPixel(123,16);
	drawPixel(122,16);

	//R
	drawPixel(125,18);
	drawPixel(124,18);
	drawPixel(123,18);
	drawPixel(122,18);
	drawPixel(121,18);
	drawPixel(125,19);
	drawPixel(123,19);
	drawPixel(124,20);
	drawPixel(122,20);
	drawPixel(121,20);

	//E
	drawPixel(125,22);
	drawPixel(124,22);
	drawPixel(123,22);
	drawPixel(122,22);
	drawPixel(121,22);
	drawPixel(125,23);
	drawPixel(125,24);
	drawPixel(123,23);
	drawPixel(123,24);
	drawPixel(121,23);
	drawPixel(121,24);

	//:
	drawPixel(124,26);
	drawPixel(122,26);
}

//function that draws a digit on x and y value of the screen
void drawNumber(int x, int y, int number) {
	switch(number) {
		case 0:
			drawPixel(x,y);
			drawPixel(x,y+1);
			drawPixel(x,y+2);
			drawPixel(x-1,y);
			drawPixel(x-2,y);
			drawPixel(x-3,y);
			drawPixel(x-4,y);
			drawPixel(x-4,y+1);
			drawPixel(x-4,y+2);
			drawPixel(x-1,y+2);
			drawPixel(x-2,y+2);
			drawPixel(x-3,y+2);
			drawPixel(x-4,y+2);
		break;
		case 1:
			drawPixel(x-1,y);
			drawPixel(x-4,y);
			drawPixel(x,y+1);
			drawPixel(x-1,y+1);
			drawPixel(x-2,y+1);
			drawPixel(x-3,y+1);
			drawPixel(x-4,y+1);
			drawPixel(x-4,y+2);
		break;
		case 2:
			drawPixel(x,y);
			drawPixel(x,y+1);
			drawPixel(x,y+2);
			drawPixel(x-1,y+2);
			drawPixel(x-2,y+2);
			drawPixel(x-2,y+1);
			drawPixel(x-2,y);
			drawPixel(x-3,y);
			drawPixel(x-4,y);
			drawPixel(x-4,y+1);
			drawPixel(x-4,y+2);
		break;
		case 3:
			drawPixel(x,y);
			drawPixel(x,y+1);
			drawPixel(x,y+2);
			drawPixel(x-1,y+2);
			drawPixel(x-2,y+2);
			drawPixel(x-2,y+1);
			drawPixel(x-2,y);
			drawPixel(x-3,y+2);
			drawPixel(x-4,y+2);
			drawPixel(x-4,y+1);
			drawPixel(x-4,y);
		break;
		case 4:
			drawPixel(x,y);
			drawPixel(x,y+2);
			drawPixel(x-1,y+2);
			drawPixel(x-2,y+2);
			drawPixel(x-2,y+1);
			drawPixel(x-2,y);
			drawPixel(x-3,y+2);
			drawPixel(x-4,y+2);
			drawPixel(x-1,y);
		break;
		
		case 5:
			drawPixel(x,y);
			drawPixel(x,y+1);
			drawPixel(x,y+2);
			drawPixel(x-2,y+2);
			drawPixel(x-2,y+1);
			drawPixel(x-2,y);
			drawPixel(x-3,y+2);
			drawPixel(x-4,y+2);
			drawPixel(x-4,y+1);
			drawPixel(x-4,y);
			drawPixel(x-1,y);
		break;
		
		case 6:
			drawPixel(x,y);
			drawPixel(x,y+1);
			drawPixel(x,y+2);
			drawPixel(x-2,y+2);
			drawPixel(x-2,y+1);
			drawPixel(x-2,y);
			drawPixel(x-3,y+2);
			drawPixel(x-4,y+2);
			drawPixel(x-4,y+1);
			drawPixel(x-4,y);
			drawPixel(x-1,y);
			drawPixel(x-3,y);
		break;
		
		case 7:
			drawPixel(x,y);
			drawPixel(x,y+1);
			drawPixel(x,y+2);
			drawPixel(x-1,y+2);
			drawPixel(x-2,y+2);
			drawPixel(x-3,y+2);
			drawPixel(x-4,y+2);
		break;
		
		case 8:
			drawPixel(x,y);
			drawPixel(x,y+1);
			drawPixel(x,y+2);
			drawPixel(x-1,y+2);
			drawPixel(x-2,y+2);
			drawPixel(x-2,y+1);
			drawPixel(x-2,y);
			drawPixel(x-3,y+2);
			drawPixel(x-4,y+2);
			drawPixel(x-4,y+1);
			drawPixel(x-4,y);
			drawPixel(x-1,y);
			drawPixel(x-3,y);
		break;
		
		case 9:
			drawPixel(x,y);
			drawPixel(x,y+1);
			drawPixel(x,y+2);
			drawPixel(x-1,y+2);
			drawPixel(x-2,y+2);
			drawPixel(x-2,y+1);
			drawPixel(x-2,y);
			drawPixel(x-3,y+2);
			drawPixel(x-4,y+2);
			drawPixel(x-1,y);
		break;
		
	}
}

//displayes the score 
void drawScore(int n, int score){
	int i;
	for (i=n;i>0;i--) { // start from the last digit
		int x = 118; //the x value where we want to draw the score
		int y = 12 + (i-1)*5; //the y value is depending on which digit of the score we want to draw
		drawNumber(x, y, score%10); //draw the score, %10 gets the last digit
		score = score / 10; //divide score by ten
	}
}

// checks if the blocks in the top 2 rows are occupied. If yes, ends the game
bool checkGameOver(void){
	if(fallenBlocks[26][4] || fallenBlocks[26][5] || fallenBlocks[26][6]){ // the 3 blocks in the middle of the top row. 
		return 1;
	}
	return 0;
}

// draws the blocks that have already fallen
void drawFallenblocks(void){
	int i,j;
	for (i = 0; i<27; i++){
		for (j = 0; j<10; j++){
			if(fallenBlocks[i][j]){
				drawBlock(i*3+1,j*3+1 );
			}
		}
	}
}

int line[27];
int score = 0;
//deletes a line at the given index and then drops the blocks to take its place
void removeLine(index){
	int i,j;
	nOfTetris++;
	if(nOfTetris >=18){
		nOfTetris = 18;		//so it does not become too fast - max speed
	}
	delayT = nOfTetris/3; // checks the number of tetrices. Everytime 3 tetrices are achieved, difficulty increases (speed increases)
	for(i = 0; i<10;i++){
		fallenBlocks[index][i] = 0; // makes all blocks in the line at given index 0, which deletes them
	}
	for(i = index+1;i<27;i++ ){
		for(j = 0; j<10;j++){
			if(fallenBlocks[i][j]){ // checks if the blocks in the current line is 1 (if there is a block there)
				fallenBlocks[i][j] = 0;  // if yes, delete them and place them in the line below
				fallenBlocks[i-1][j] = 1;
			}
		}
	}
}

// checks if there's a tetris
void checkTetris(void){
	int i,j;
	bool lineTrue;
	for(i = 0; i<27;i++){
		line[i] = 0; // makes all the elements in the array 0
	}
	for (i = 0; i<27; i++){
		lineTrue = 1;
		for (j = 0; j<10; j++){
			if(fallenBlocks[i][j]){ // checks each block in the line to see if theres any gaps
				lineTrue = 1;
			}else{
				lineTrue = 0; // if there's a gap in the line (not full), updated to 0
				break;
			}
		}
		if(lineTrue){   //if lineTrue is still 1, meaning that there were no gaps (full line)
			line[i] = 1; //the array is updated with the full line 
		}

	}
	for(i = 0; i<27;i++){
		if(line[i]){ // checks which lines are full and calls the remove function
			score = score+10;  //score increased by 10 as tetris is achieved
			removeLine(i);
		}
	}
}

//selects the next letter
char nextLetter(int n){
	int index = n;
	if(n == -1 || n == 27){
		index = 0;
	}
	switch(index){
		case 0:
		return ' ';
		break;
		case 1:
		return 'A';
		break;
		case 2:
		return 'B';
		break;
		case 3:
		return 'C';
		break;
		case 4:
		return 'D';
		break;
		case 5:
		return 'E';
		break;
		case 6:
		return 'F';
		break;
		case 7:
		return 'G';
		break;
		case 8:
		return 'H';
		break;
		case 9:
		return 'I';
		break;
		case 10:
		return 'J';
		break;
		case 11:
		return 'K';
		break;
		case 12:
		return 'L';
		break;
		case 13:
		return 'M';
		break;
		case 14:
		return 'N';
		break;
		case 15:
		return 'O';
		break;
		case 16:
		return 'P';
		break;
		case 17:
		return 'Q';
		break;
		case 18:
		return 'R';
		break;
		case 19:
		return 'S';
		break;
		case 20:
		return 'T';
		break;
		case 21:
		return 'U';
		break;
		case 22:
		return 'V';
		break;
		case 23:
		return 'W';
		break;
		case 24:
		return 'X';
		break;
		case 25:
		return 'Y';
		break;
		case 26:
		return 'Z';
		break;
	}
}

// starts the game from beginning, clearing the screen and resetting the score and difficulty
void reset(void){
	int i,j;
	for(i = 0; i < 27; i++){
		for(j = 0; j < 10; j++){
			fallenBlocks[i][j] = 0;
		}
	}
	score = 0;
	delayT = 0;
}

void display_image(int x, const uint8_t *data) {
	int i, j;

	for(i = 0; i < 4; i++) {
		DISPLAY_CHANGE_TO_COMMAND_MODE;

		spi_send_recv(0x22);
		spi_send_recv(i);

		spi_send_recv(x & 0xF);
		spi_send_recv(0x10 | ((x >> 4) & 0xF));

		DISPLAY_CHANGE_TO_DATA_MODE;

		// Adding multiplier of 4 so we calculate with the whole width (4 "pages")
		for(j = 0; j < 32*4; j++)
			spi_send_recv(~data[i*32*4 + j]);
	}
}



void display_update(void) {
	int i, j, k;
	int c;
	for(i = 0; i < 4; i++) {
		DISPLAY_CHANGE_TO_COMMAND_MODE;
		spi_send_recv(0x22);
		spi_send_recv(i);
		
		spi_send_recv(0x0);
		spi_send_recv(0x10);
		
		DISPLAY_CHANGE_TO_DATA_MODE;
		
		for(j = 0; j < 16; j++) {
			c = textbuffer[i][j];
			if(c & 0x80)
				continue;
			
			for(k = 0; k < 8; k++)
				spi_send_recv(font[c*8 + k]);
		}
	}
}


/*
 * itoa
 * 
 * Simple conversion routine
 * Converts binary to decimal numbers
 * Returns pointer to (static) char array
 * 
 * The integer argument is converted to a string
 * of digits representing the integer in decimal format.
 * The integer is considered signed, and a minus-sign
 * precedes the string of digits if the number is
 * negative.
 * 
 * This routine will return a varying number of digits, from
 * one digit (for integers in the range 0 through 9) and up to
 * 10 digits and a leading minus-sign (for the largest negative
 * 32-bit integers).
 * 
 * If the integer has the special value
 * 100000...0 (that's 31 zeros), the number cannot be
 * negated. We check for this, and treat this as a special case.
 * If the integer has any other value, the sign is saved separately.
 * 
 * If the integer is negative, it is then converted to
 * its positive counterpart. We then use the positive
 * absolute value for conversion.
 * 
 * Conversion produces the least-significant digits first,
 * which is the reverse of the order in which we wish to
 * print the digits. We therefore store all digits in a buffer,
 * in ASCII form.
 * 
 * To avoid a separate step for reversing the contents of the buffer,
 * the buffer is initialized with an end-of-string marker at the
 * very end of the buffer. The digits produced by conversion are then
 * stored right-to-left in the buffer: starting with the position
 * immediately before the end-of-string marker and proceeding towards
 * the beginning of the buffer.
 * 
 * For this to work, the buffer size must of course be big enough
 * to hold the decimal representation of the largest possible integer,
 * and the minus sign, and the trailing end-of-string marker.
 * The value 24 for ITOA_BUFSIZ was selected to allow conversion of
 * 64-bit quantities; however, the size of an int on your current compiler
 * may not allow this straight away.
 */
#define ITOA_BUFSIZ ( 24 )
char * itoaconv( int num )
{
  register int i, sign;
  static char itoa_buffer[ ITOA_BUFSIZ ];
  static const char maxneg[] = "-2147483648";
  
  itoa_buffer[ ITOA_BUFSIZ - 1 ] = 0;   /* Insert the end-of-string marker. */
  sign = num;                           /* Save sign. */
  if( num < 0 && num - 1 > 0 )          /* Check for most negative integer */
  {
    for( i = 0; i < sizeof( maxneg ); i += 1 )
    itoa_buffer[ i + 1 ] = maxneg[ i ];
    i = 0;
  }
  else
  {
    if( num < 0 ) num = -num;           /* Make number positive. */
    i = ITOA_BUFSIZ - 2;                /* Location for first ASCII digit. */
    do {
      itoa_buffer[ i ] = num % 10 + '0';/* Insert next digit. */
      num = num / 10;                   /* Remove digit from number. */
      i -= 1;                           /* Move index to next empty position. */
    } while( num > 0 );
    if( sign < 0 )
    {
      itoa_buffer[ i ] = '-';
      i -= 1;
    }
  }
  /* Since the loop always sets the index i to the next empty position,
   * we must add 1 in order to return a pointer to the first occupied position. */
  return( &itoa_buffer[ i + 1 ] );
}