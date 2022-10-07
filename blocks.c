/* This file written 2022 by Filip Lukowski and Duy Trung Pham
   
   For copyright and licensing, see file COPYING */

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include <stdbool.h> /* Use of bool */
#include <stdlib.h> /* Use of rand() */
#include "mipslab.h"

int fallenBlocks[27][10];
int currentX;
int currentY;
const int startingX = 73;
const int startingY = 13;

//create a definition of a Shape that has a matrix of block placements and current rotation value, as well as what shape type it is
typedef struct {
	int matrix;
    int curRotation;
    int shapeType;
}Shape;

//2D array of all possible shapes (7 for the different shapes, 4 for all rotations)
Shape shapes[7][4];

Shape currentShape;
Shape nextShape;

//all possible matrix placements for shapes and their rotations
const int squareBlock = 0x0660;

const int tBlock0 = 0x4C40;
const int tBlock1 = 0x4E00;
const int tBlock2 = 0x4640;
const int tBlock3 = 0xE400;

const int IBlock0 = 0x2222;
const int IBlock1 = 0x00F0;
const int IBlock2 = 0x4444;
const int IBlock3 = 0x0F00;

const int LBlock0 = 0x6220;
const int LBlock1 = 0x1700;
const int LBlock2 = 0x2230;
const int LBlock3 = 0x7400;

const int L2Block0 = 0x2260;
const int L2Block1 = 0x4700;
const int L2Block2 = 0x3220;
const int L2Block3 = 0x7100;

const int ZBlock0 = 0x2640;
const int ZBlock1 = 0x6300;
const int ZBlock2 = 0x1320;
const int ZBlock3 = 0x0630;

const int Z2Block0 = 0x4620;
const int Z2Block1 = 0x06C0;
const int Z2Block2 = 0x8C40;
const int Z2Block3 = 0x6C00;

//assigning all shapes and their rotations
void makeShapes(void){    
    shapes[0][0].matrix = squareBlock;
    shapes[0][1].matrix = squareBlock;
    shapes[0][2].matrix = squareBlock;
    shapes[0][3].matrix = squareBlock;

    shapes[1][0].matrix = tBlock0;
    shapes[1][1].matrix = tBlock1;
    shapes[1][2].matrix = tBlock2;
    shapes[1][3].matrix = tBlock3;

    shapes[2][0].matrix = IBlock0;
    shapes[2][1].matrix = IBlock1;
    shapes[2][2].matrix = IBlock2;
    shapes[2][3].matrix = IBlock3;

    shapes[3][0].matrix = LBlock0;
    shapes[3][1].matrix = LBlock1;
    shapes[3][2].matrix = LBlock2;
    shapes[3][3].matrix = LBlock3;

    shapes[4][0].matrix = L2Block0;
    shapes[4][1].matrix = L2Block1;
    shapes[4][2].matrix = L2Block2;
    shapes[4][3].matrix = L2Block3;

    shapes[5][0].matrix = ZBlock0;
    shapes[5][1].matrix = ZBlock1;
    shapes[5][2].matrix = ZBlock2;
    shapes[5][3].matrix = ZBlock3;

    shapes[6][0].matrix = Z2Block0;
    shapes[6][1].matrix = Z2Block1;
    shapes[6][2].matrix = Z2Block2;
    shapes[6][3].matrix = Z2Block3;

    //assigning current rotation and shape type to each element
    int i,j;
    for (i = 0; i<7;i++){
        for (j = 0; j<4;j++){
            shapes[i][j].curRotation = j;
            shapes[i][j].shapeType = i;
        }
    }
}

//draw 3 by 3 pixel block given x and y coordinates
void drawBlock(int x, int y){
    int i,j;
    for (i = 0; i< 3; i++){
        for (j = 0; j< 3; j++){
            drawPixel(x+i, y+j);
        }
    }
}

//draws current shape using the given matrix, each 4 bits represent a line in the y axis
void drawShape(void) {
    int i, j;
	int line1 = (currentShape.matrix & 0xF000)>>12;
    int line2 = (currentShape.matrix & 0x0F00)>>8;
    int line3 = (currentShape.matrix & 0x00F0)>>4;
    int line4 = currentShape.matrix & 0x000F;
    for(j = 0; j<4;j++){
        if((line1>>(3-j))&0x1){
            drawBlock(currentX+3*j, currentY);
        }   
    }
    for(j = 0; j<4;j++){
        if((line2>>(3-j))&0x1){
            drawBlock(currentX+3*j, currentY+3);
        }   
    }
    for(j = 0; j<4;j++){
        if((line3>>(3-j))&0x1){
            drawBlock(currentX+3*j, currentY+6);
        }   
    }
    for(j = 0; j<4;j++){
        if((line4>>(3-j))&0x1){
            drawBlock(currentX+3*j, currentY+9);
        }   
    }

}

//draws next shape same way as drawShape function, but does this in different x and y coordinates
void drawNextShape(void){
    int i, j;
	int line1 = (nextShape.matrix & 0xF000)>>12;
    int line2 = (nextShape.matrix & 0x0F00)>>8;
    int line3 = (nextShape.matrix & 0x00F0)>>4;
    int line4 = nextShape.matrix & 0x000F;
    for(j = 0; j<4;j++){
        if((line1>>(3-j))&0x1){
            drawBlock(86+3*j, 11);
        }   
    }
    for(j = 0; j<4;j++){
        if((line2>>(3-j))&0x1){
            drawBlock(86+3*j, 14);
        }   
    }
    for(j = 0; j<4;j++){
        if((line3>>(3-j))&0x1){
            drawBlock(86+3*j, 17);
        }   
    }
    for(j = 0; j<4;j++){
        if((line4>>(3-j))&0x1){
            drawBlock(86+3*j, 20);
        }   
    }
}

//given two ints (randomised before), assigns the next shape to the corresponding shape and rotation
void randShape(int n1, int n2){
    nextShape =  shapes[n1][n2];
}

//current shape becomes the next shape, next shape becomes a new random shape. Also resets current X and Y coordinates to the starting position
void updateShapes(void){
    currentShape = nextShape;
    int randInt1 = rand()%7;
    int randInt2 = rand()%4;
    while (randInt1== 7) {
		randInt1= rand()&7;
	}
    while (randInt2 == 4) {
		randInt2 = rand()&4;
	}
    randShape(randInt1, randInt2);
    currentX = startingX;
    currentY = startingY;
}

//when a shape has impacted with a floor or block, stores the current shapes block position in fallenBlocks array so they are dispalyed later. Then updates shapes
void impact(void){
    int i,j;
    int val = 0;
    int valY = 0;
    if(currentX<0){
        val = 1;
    }
    if(currentY<0){
        valY = 1;
    }
    for(i = 0; i<4;i++){
        for(j = 0;j<4;j++){
            int temp = 4*i+j;
            //goes through each bit of the matrix
            if(currentShape.matrix & pow(2, temp)){
                fallenBlocks[currentX/3 + 3 - j-val][currentY/3+3-i-valY] = 1;
            }
        }
    }
    updateShapes();
}

//Checks if the any of the blocks below the current shape are already taken (are the floor or previously dropped blocks), and calls impact if so
void impactCheck(void){
    int i, j;
    int check = 1;
    int val = 0;
    int valY = 0;
    if(currentX<0){
        val = 1;
    }
    if(currentY<0){
        valY = 1;
    }
    //floor check
    if(currentX == 1){
        if(currentShape.matrix & pow(2,3) || currentShape.matrix & pow(2,7) || currentShape.matrix & pow(2,11) || currentShape.matrix & pow(2,15)){
            check = 0;
            impact();
        }
    }
    else if(currentX == -2){
        if(currentShape.matrix & pow(2,2) || currentShape.matrix & pow(2,6) || currentShape.matrix & pow(2,10) || currentShape.matrix & pow(2,14)){
            check = 0;
            impact();
        }
    }else if(currentX == -5){
        if(currentShape.matrix & pow(2,1) || currentShape.matrix & pow(2,5) || currentShape.matrix & pow(2,9) || currentShape.matrix & pow(2,13)){
            check = 0;
            impact();
        }
    }
    //does not check if the block has hit the floor
    if (check){
        //hit block from bottom check
        if (currentX > 0){
            for(i = 0; i <4; i++){
                for(j = 0; j<4;j++){
                    int power = 15-4*i-j;
                    if(currentShape.matrix & pow(2,power)){
                        if(fallenBlocks[currentX/3+j-1][currentY/3+i-valY]){
                            impact();
                        }
                    }
                }
            }
        }else{
            for(i = 0; i <4; i++){
                for(j = 0; j<4;j++){
                    int power = 15-4*i-j;
                    if(currentShape.matrix & pow(2,power)){
                        if(fallenBlocks[currentX/3+j-2][currentY/3+i-valY])
                            impact();
                    }
                }
            }
        }
    }
}


//checks when the block has been moved left or right if it overlaps the wall or other blocks, if so returns 1 so movement is reverted
bool wallCheck(void){
    int i,j;
    int val = 1;
    if(currentY<0){
        val = 0;
    }
    //left wall check
    if(currentY < 1 ){
        for(i = 0; i<16; i++){
            if(currentShape.matrix & pow(2, 15-i)){
                if(currentY/3 + i/4<= 0){
                    return 1;
                }
            }
        }
    }
    //right wall check
    if(currentY >21){
        for(i = 0; i<16; i++){
            if(currentShape.matrix & pow(2, 15-i)){
                if(currentY + 3+ i/4*3 >= 32){
                    return 1;
                }
            }
        }
    }

    
    int val2 = 0;
    if(currentY<0){
        val2 = 1;
    }
    //htting block from the side check
    if(currentX>0){
    for(i = 0; i <4; i++){
        for(j = 0; j<4;j++){
            int power = 15-4*i-j;
            if(currentShape.matrix & pow(2,power)){
                if(fallenBlocks[currentX/3+j][currentY/3+i-val2]){
                   return 1;                   
                }
            }
        }
    }
    }else if(currentX < 0){
            for(i = 0; i <4; i++){
                for(j = 0; j<4;j++){
                    int power = 15-4*i-j;
                    if(currentShape.matrix & pow(2,power)){
                        if(fallenBlocks[currentX/3+j-1][currentY/3+i-val])
                            return 1;
                    }
                }
            }
    }else{
    for(i = 0; i <4; i++){
        for(j = 0; j<4;j++){
            int power = 15-4*i-j;
            if(currentShape.matrix & pow(2,power)){
                if(fallenBlocks[currentX/3+j][currentY/3+i-val]){
                   return 1;                   
                }
            }
        }
    }
    }
    return 0;
}


//make shape fall one block
void gravity(void) {
    //check for collision below
    impactCheck();
	currentX = currentX - 3;
}

//make shape move one block to the left
void moveLeft(void) {
	currentY = currentY - 3;
    //check for side collisions
    if(wallCheck()){
        currentY = currentY + 3;
    }	
    //check for collsion below
    impactCheck();
}

//make shape move one block to the right
void moveRight(void) {
	currentY = currentY + 3;
    //check for side collision
    if(wallCheck()){
        currentY = currentY - 3;
    }
    //check for collision below
    impactCheck();
}

//rotates shape 90 degress clockwise. Uses the preset definitions of shape rotation matrices
void rotate(void){
    currentShape.curRotation = currentShape.curRotation+1;
    //checks fot illegal/erroneous numbers
    if (currentShape.curRotation == 4){
        currentShape.curRotation = 0;
    }
    currentShape = shapes[currentShape.shapeType][currentShape.curRotation];
    if(wallCheck()){
        currentShape.curRotation = currentShape.curRotation-1;
        if (currentShape.curRotation == -1){
        currentShape.curRotation = 0;
        }
        //updates current shape
        currentShape = shapes[currentShape.shapeType][currentShape.curRotation];
    }
}


