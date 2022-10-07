/* This file written 2022 by Filip Lukowski and Duy Trung Pham
   from original code written by F Lundevall
   
   For copyright and licensing, see file COPYING */

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include <stdbool.h> /* Use of bool */
#include <stdlib.h> /* Use of rand() */
#include "mipslab.h"


int timeoutcount = 0;
extern int delayT;
int screen = 0;
int btnValue;
int currentX;
int currentY;
int seed = 0;
int seedplus = 1;
extern int score;
int highscore[3];
char name[3] = {' ', ' ', ' '};
char name1[3]= {' ', ' ', ' '};
char name2[3]= {' ', ' ', ' '};
char name3[3]= {' ', ' ', ' '};
char tempName[3]= {' ', ' ', ' '};
int currentI = 0;
int currentLetter = 1;


//declare stdout as a pointer to null to be able to use functions in stdlib
void *stdout = (void *) 0;

/* Interrupt Service Routine */
void user_isr( void )
{
  return;
}

//runs once when first initialized, starting timer, setting ports to inputs and outputs, also calling necessary functions to start the game
void init( void )
{
    
    //srand(time(NULL));
	TRISECLR = 0xff; 
	TRISDSET = 0xfe0; 
    TRISFSET = 0x2;
	T2CON = 0x70; //1/256 prescale 
	PR2 = (80000000/256)/10; //timeout period 0,1s
	TMR2 = 0;	//reset timer
	T2CONSET = 0x8000; // Timer started
    makeShapes();
    int randInt1 = rand()%7;
    int randInt2 = rand()%4;
    while (randInt1== 7) {
		randInt1= rand()&7;
	}
    while (randInt2 == 4) {
		randInt2 = rand()&4;
	}
    randShape(randInt1, randInt2);
    updateShapes();
    clearDisplay();
    return;
}

//gameover screen
void gameover(void) {
    currentI = 0;
    timeoutcount++;

	display_string(0, "GAME OVER!");
    display_string(1, "");
	display_string(2, "SCORE:");
    display_string(3, itoaconv(score));
    display_update();

    //after 4 seconds moves to either new highscore screen or menu
    if(timeoutcount == 40){
        timeoutcount = 0;
        if(score>highscore[2]){
            screen = 4;
        }else{
            screen = 0;
        }
    }
}

//new high score screen, user can input 3 initials as their name
void newHiScore(){
    int place, i;
    timeoutcount++;
    int btns= getbtns();
    if(timeoutcount == 2){
        timeoutcount = 0;
        if (btns >> 3 &0x1){
            currentLetter--;        
            name[currentI] = nextLetter(currentLetter);         //shows letter which was chosen
        }else if(btns & 0x1){
            currentLetter++;
            name[currentI] = nextLetter(currentLetter);
        }else if(btns >> 2 &0x1){                               //saves the chosen letter and moves to next initial
            currentI++;
             if(currentI > 2){                                  //check if 3 initals have already been entered
                screen = 0;                                     //move to menu
                if(score>highscore[0]){                         //changes highscore placements and the given names
                    highscore[1] = highscore[0];
                    highscore[0] = score;
                     for(i = 0; i<3;i++){
                        tempName[i] = name1[i];
                    }
                    for(i = 0; i<3;i++){
                        name3[i] = name2[i];
                    }
                    for(i = 0; i<3;i++){
                        name2[i] = tempName[i];
                    }
                     for(i = 0; i<3;i++){
                        name1[i] = name[i];
                    }
                }else if(score>highscore[1]){
                    highscore[2] = highscore[1];
                    highscore[1] = score; 
                    for(i = 0; i<3;i++){
                        name3[i] = name2[i];
                    }
                    for(i = 0; i<3;i++){
                        name2[i] = name[i];
                    }

                }else if(score>highscore[2]){
                    highscore[2] = score;
                    for(i = 0; i<3;i++){
                        name3[i] = name[i];
                    }
                }
                reset();
            }
        }
    }
    display_string(1, name);
    display_string(0, "3 INITIALS");
    display_string(2, "B3-SAVE CHAR");
    display_string(3, "B1B4-CHANGE CHAR");
    display_update();
}

//Tertis game screen
void startGame(void) {
    timeoutcount++;
    //speed of fall of the blocks. Every 3 tetrises/lines removed it speeds up by 0.1s. Highest speed is 0.3s per block fall
    if(timeoutcount == (20-delayT) ){
        timeoutcount = 0;
        gravity();
    }

    //user inputs and corresponding moves
    btnValue = getbtns();
    if (btnValue>0){
        if(btnValue>>3 & 0x1== 1){
            gravity();
        }
        if(btnValue>>2 &0x1 == 1){
            moveRight();
        }
        if(btnValue>>1 &0x1 == 1){
            moveLeft();
        }
        if(btnValue &0x1 == 1){
            rotate();
        }
    }

    clearDisplay();
	drawBorder();
    drawText();
    drawScore(intNOfUnits(score),score);
    drawShape();
    drawNextShape();
    drawFallenblocks();
    checkTetris();

    //chcecks if game is over and if so, moves to gameover screen
    if(checkGameOver()){
        screen = 2;
    }

    display_image(0, display);

    //checks for pause and restart switches and moves to corresponding screens
    int swValue = getsw();
    if(swValue & 0x1){
        screen = 6;
    }
    if(swValue >> 1 &0x1){
        screen = 7;
    }

}

//top 3 highscores displayed with given names
void hiScores(void) {
	display_string(0, "HIGHSCORES:");
    display_string_index(1,0,name1);
    display_string_index(1,5 ,itoaconv(highscore[0]));
    display_string_index(2,0,name2);
    display_string_index(2,5 ,itoaconv(highscore[1]));
    display_string_index(3,0,name3);
    display_string_index(3,5 ,itoaconv(highscore[2]));
    display_string_index(3, 9, "B4:BACK");
    display_update();

    //back button to menu
    int btns = getbtns();
    if(btns >> 3 & 0x1){
        screen = 0;
    }

}

//pausing game
void pause(void){
    display_string(0,"PAUSE");
    display_string(1,"");
    display_string(3,"");
    display_string(2,"UNPAUSE-SW1DOWN");
    int sw = getsw();
    display_update();
    if(!(sw&0x1)){                      //if switch is moved back to 0, game is resumed
        screen = 1;
    }
    
}

//moving to menu
void pause2(void){
    display_string(0,"");
    display_string(2,"");
    display_string(3,"");
    display_string(1, "MENU-SW2DOWN");
    int sw = getsw();
    display_update();
    if(!(sw >> 1 &0x1)){                //if switch is moved back to 0, screen changes to menu and all is reset and game is not saved
        updateShapes();
        reset();
        screen = 0;
    }
}

//menu screen
void menu(void) {
    //randomiser to give different shapes called only once
    if (seedplus == 1) {
			seedplus = 0;
			srand(seed);
	}
    btnValue = getbtns();
    if (btnValue >> 3 == 1){
        screen = 1;

    }else if(btnValue >> 2 == 1){
        screen = 3;
    }else if(btnValue>>1 == 1){
        screen = 5;
    }

    //display menu 
	display_string(0, "TETRIS");
	display_string(3, "BTN2: CONTROLS");
	display_string(1, "BTN4: START GAME");
	display_string(2, "BTN3: HIGHSCORES");
    display_update();

}

//shows controls of the tetris game
void inst(void){
    display_string(0, "B1:ROTATE");
    display_string_index(1,0,"B2:LEFT");
    display_string_index(1,8,"B3:RIGHT");
    display_string_index(2, 0, "B4:DOWN");
    display_string(3,"SW2->1>0:MENU");
    display_update();

    int btns = getbtns();
    if(btns >> 3 & 0x1){            //back button to menu
        screen = 0;
    }
}

/* This function is called repetitively from the main program */
void update( void )
{
    if((IFS(0) & 0x100) >> 8 == 1){
        IFS(0) = IFS(0) & 0xFFFFFEFF;
        if(screen == 0){
            menu();
        }else if(screen == 1){
            startGame();
        }else if(screen == 2){
            gameover();
        }else if(screen == 3){
            hiScores();
        }else if(screen == 4){
            newHiScore();
        }else if(screen == 5){
            inst();
        }else if(screen == 6){
            pause();
        }else if(screen == 7){
            pause2();
        }
        
    }
}

  

