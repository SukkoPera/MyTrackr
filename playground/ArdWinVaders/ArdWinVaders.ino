/*

  OLED/ArdWinVaders
  An Arduino & OLED version of a classic.

  For info see https://github.com/tardate/LittleArduinoProjects/tree/master/playground/OLED/ArdWinVaders

  Modified by SukkoPera, March 2018:
  - Port to MyTrackr Board V1.0
  - Keep font in flash so that more RAM is free and screen buffer can be allocated.
    How could this work before???
  - Disable music (Temporarily)
 */

#include <FlexiTimer2.h>

//** control pins
#define FIRE_PIN 5		// Next
//~ #define FIRE_INTERRUPT 0
#define LEFT_PIN 7		// Back
#define RIGHT_PIN 4		//  Select

volatile int fireStatus = 0;
volatile int moveLeft = 0;
volatile int moveRight = 0;

//** pin connections
#define SPEAKER_PIN 9

#include "oled_driver.h"
#include "buffered_display.h"
#include "game_board.h"
#include "sound_system.h"

SoundSystem soundSystem(SPEAKER_PIN);
OledDriver displayDriver;
BufferedDisplay gameDisplay(&displayDriver);
GameBoard gameBoard(&gameDisplay, &soundSystem);

void setup() {
  soundSystem.init();
  gameBoard.init();

  pinMode(LEFT_PIN, INPUT_PULLUP);
  pinMode(RIGHT_PIN, INPUT_PULLUP);
  FlexiTimer2::set(20, sampleMovement);
  FlexiTimer2::start();

  pinMode(FIRE_PIN, INPUT_PULLUP);

}

void loop() {
  if(gameBoard.beginRecalc()) {
    if(moveLeft == LOW)
		gameBoard.moveLeft();
    if(moveRight == LOW)
		gameBoard.moveRight();
    if(fireStatus == LOW) {
      gameBoard.fire();
      fireStatus = 0;
    }
    gameBoard.finishRecalc();
  } else {
    if(fireStatus == LOW) {
      gameBoard.startGame();
      fireStatus = HIGH;
    }
  }
  gameDisplay.redraw();
  soundSystem.next();
}

void sampleMovement() {
  moveLeft = digitalRead(LEFT_PIN);
  moveRight = digitalRead(RIGHT_PIN);
  fireStatus = digitalRead(FIRE_PIN);
}
