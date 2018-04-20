/* Tiny Tetris V0.94

  Copyright (C) 2016 Anthony Russell
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  Special thanks to tobozo - https://github.com/tobozo - who:
  Added sound and music
  Created Dpad.cpp for much better control interface.
  Art work for title screen
  Bug fixes and lots of other things.

  Modified by SukkoPera, March 2018:
  - Port to MyTrackr Board V1.0
  - Disable music (Temporarily)
  - Fix all warnings (under Arduino 1.8.5)
  - Fix typos
  - Other minor changes here & there

  To do:
  High score functionality.
  Decent random number generator.
  Create a letter font, create a proper system for rendering numbers and letters.
  [more to do] Tidy up code and optimize for memory, sort out the globals and types.
  Create defines for all the magic numbers but they are useful for now.

*/

#include <Wire.h>
#include "TetrisTheme.cpp"

// Keys handling is taken almost verbatim from MyTrackr
#include "buttons.h"
Buttons buttons;

#define OLED_ADDRESS	        	0x3C //you may need to change this, this is the OLED I2C address.  
#define OLED_COMMAND	                0x80
#define OLED_DATA	                0x40
#define OLED_DISPLAY_OFF	        0xAE
#define OLED_DISPLAY_ON	                0xAF
#define OLED_NORMAL_DISPLAY	    	0xA6
#define OLED_INVERSE_DISPLAY     	0xA7
#define OLED_SET_BRIGHTNESS	        0x81
#define OLED_SET_ADDRESSING	        0x20
#define OLED_HORIZONTAL_ADDRESSING	0x00
#define OLED_VERTICAL_ADDRESSING	0x01
#define OLED_PAGE_ADDRESSING	        0x02
#define OLED_SET_COLUMN                 0x21
#define OLED_SET_PAGE	                0x22

// the tetris blocks
const byte Blocks[7][2] PROGMEM = {
  { 0B01000100, 0B01000100 },
  { 0B11000000, 0B01000100 },
  { 0B01100000, 0B01000100 },
  { 0B01100000, 0B00000110 },
  { 0B11000000, 0B00000110 },
  { 0B01000000, 0B00001110 },
  { 0B01100000, 0B00001100 }
};

// the numbers for score, To do: create letter fonts

const byte NumberFont[10][8] PROGMEM = {

  { 0x00, 0x1c, 0x22, 0x26, 0x2a, 0x32, 0x22, 0x1c },
  { 0x00, 0x1c, 0x08, 0x08, 0x08, 0x08, 0x0c, 0x08 },
  { 0x00, 0x3e, 0x02, 0x04, 0x18, 0x20, 0x22, 0x1c },
  { 0x00, 0x1c, 0x22, 0x20, 0x18, 0x20, 0x22, 0x1c },
  { 0x00, 0x10, 0x10, 0x3e, 0x12, 0x14, 0x18, 0x10 },
  { 0x00, 0x1c, 0x22, 0x20, 0x20, 0x1e, 0x02, 0x3e },
  { 0x00, 0x1c, 0x22, 0x22, 0x1e, 0x02, 0x04, 0x18 },
  { 0x00, 0x04, 0x04, 0x04, 0x08, 0x10, 0x20, 0x3e },
  { 0x00, 0x1c, 0x22, 0x22, 0x1c, 0x22, 0x22, 0x1c },
  { 0x00, 0x0c, 0x10, 0x20, 0x3c, 0x22, 0x22, 0x1c }
};



// "Tiny Tetris" upside-down text binarized from http://www.dcode.fr/binary-image
const byte welcomeScreen[16][5] PROGMEM = {
  B01110011, B10100010, B00100011, B11100010, B00000000,
  B10001001, B00100010, B00100000, B00100010, B00000000,
  B10000001, B00100010, B00100000, B00100010, B00000000,
  B01110001, B00011110, B00100001, B11100010, B00000000,
  B00001001, B00100010, B00100000, B00100010, B00000000,
  B10001001, B00100010, B00100000, B00100010, B00000000,
  B01110011, B10011110, B11111011, B11101111, B10000000,
  B00000000, B00000000, B00000000, B00000000, B00000000,
  B00000000, B00000000, B00000000, B00000000, B00000000,
  B00000000, B10001000, B10111000, B10000000, B00000000,
  B00000000, B10001100, B10010000, B10000000, B00000000,
  B00000000, B10001100, B10010000, B10000000, B00000000,
  B00000001, B01001010, B10010000, B10000000, B00000000,
  B00000010, B00101001, B10010000, B10000000, B00000000,
  B00000010, B00101001, B10010000, B10000000, B00000000,
  B00000010, B00101000, B10111011, B11100000, B00000000

};

// Tetris Illustration upside-down image binarized from http://www.dcode.fr/binary-image
const byte tetrisLogo[40][8] PROGMEM =  {
  B11111111, B11111111, B11111111, B11111111, B11111111, B11111111, B11111111, B11111111,
  B11101101, B10111111, B11111111, B11111111, B11111111, B01111111, B11111001, B11100111,
  B11101101, B00110100, B11111111, B11111111, B11111110, B01110011, B11110001, B11100111,
  B10111000, B01010101, B11111111, B11111111, B11111000, B01110011, B11100001, B11100111,
  B10011110, B10110011, B10110011, B11100011, B11100100, B00100011, B11100011, B11110011,
  B10001111, B00010011, B00110001, B11110001, B11110100, B00100011, B11100011, B11110011,
  B10001111, B00000111, B01110001, B11110000, B11110010, B00110011, B11100011, B11110001,
  B10001111, B00000110, B01100001, B11111000, B11111010, B00000001, B11000001, B11100001,
  B10000110, B00001110, B11100000, B11111000, B01111001, B00000001, B11000000, B11000001,
  B10000110, B00001100, B11100000, B11111100, B01111001, B00000001, B11000000, B00000001,
  B10000110, B00001100, B11110000, B11111100, B01111001, B00000000, B10000000, B00000001,
  B10000110, B00001100, B11110000, B01111100, B01111001, B00000000, B10000000, B00000001,
  B10000110, B00000110, B11110000, B01111100, B01111001, B00000000, B10000000, B00000001,
  B10000110, B00000111, B01111000, B01111000, B01110010, B00000000, B10000000, B00000001,
  B10001101, B00000011, B00111000, B01111000, B01110010, B00000000, B00000000, B00000001,
  B10011001, B10000011, B10111000, B01111000, B11110100, B00000000, B00000000, B00000001,
  B10011001, B10000001, B10011100, B01110001, B11101100, B00000000, B00000000, B00000001,
  B10001001, B00000000, B11111100, B01110001, B11011000, B00000000, B00000000, B00000001,
  B10001011, B00000000, B01111100, B01100011, B10110000, B00000000, B00000000, B00000001,
  B10000110, B00000000, B00110100, B11100111, B01100000, B00000000, B00000000, B00000001,
  B10000000, B00000000, B00011110, B11100110, B01000000, B00000000, B00000000, B00000001,
  B10000000, B00000000, B00001110, B11001100, B10000000, B00000000, B00000000, B00000001,
  B10000000, B00000000, B00000110, B11011011, B00000000, B00000000, B00000000, B00000001,
  B10000000, B00000000, B00000111, B11010010, B00000000, B00000000, B00000000, B00000001,
  B10000000, B00000000, B00000011, B10100100, B00000000, B00000000, B00000000, B00000001,
  B10000000, B00000000, B00000001, B11111000, B00000000, B00000000, B00110000, B00000001,
  B10000000, B00000000, B00000000, B11110000, B00000000, B00000000, B00110000, B00000001,
  B10000000, B00000000, B00000000, B11010000, B00000000, B00000000, B00000000, B00000001,
  B10000000, B00000000, B00000000, B01110000, B00000000, B00000000, B00000000, B00000001,
  B10000000, B00000000, B10000000, B01100000, B00000000, B00000000, B00000000, B00000001,
  B10000011, B00000000, B00000000, B01100000, B00000000, B00000000, B00000000, B00000001,
  B10000011, B00000000, B00000000, B01100000, B00000000, B00000000, B00000000, B00000001,
  B10000000, B00000000, B00000000, B01100000, B00000000, B00000000, B00000000, B00000001,
  B10000000, B00000000, B00000000, B01100000, B00000000, B00000000, B00000000, B00000001,
  B10000000, B00000000, B00000000, B11110000, B00000000, B00000000, B00000000, B00010001,
  B10000000, B00000000, B00000000, B11001000, B00000000, B00000000, B00000000, B00000001,
  B10000000, B00000000, B00000001, B10001000, B00000000, B00000000, B00000000, B00000001,
  B10000000, B00000000, B00000001, B10001000, B00000000, B00000000, B00000000, B00000001,
  B10000000, B00000000, B00000000, B10010000, B00000000, B00000000, B00000000, B00000001,
  B10000000, B00000000, B00000000, B11110000, B00001000, B00000000, B00000000, B00000001
};

// Tetris Brick upside-down image binarized from http://www.dcode.fr/binary-image
const byte brickLogo[36][8] PROGMEM = {
  B10000000, B00000000, B00000000, B00000000, B00000000, B11111111, B11111100, B00000001,
  B10000000, B00000111, B11111100, B11111111, B11111110, B11111111, B11111100, B00000001,
  B10000011, B11111111, B11111110, B11111111, B11111111, B01111111, B11111110, B00000001,
  B10000011, B11111111, B11111110, B01111111, B11111111, B00111111, B11111111, B00000001,
  B10000011, B11111111, B11111111, B01111111, B11111111, B10111111, B11111111, B10000001,
  B10001001, B11111111, B11111111, B00111111, B11111111, B10011111, B11111111, B10000001,
  B10001101, B11111111, B11111111, B10111111, B11111111, B11001111, B11111111, B11000001,
  B10001101, B11111111, B11111111, B10011111, B11111111, B11101111, B11111111, B11100001,
  B10001100, B11111111, B11111111, B11011111, B11111111, B11100111, B11111111, B11110001,
  B10001110, B11111111, B11111111, B11001111, B11111111, B11110111, B11111111, B11110001,
  B10001110, B11111111, B11111111, B11101111, B11111111, B11111011, B11111111, B00000001,
  B10001110, B01111111, B11111111, B11101111, B11111111, B11100000, B00000000, B00010001,
  B10001111, B01111111, B11111111, B11100100, B00000000, B00000001, B11111111, B11110001,
  B10001111, B00111111, B10000000, B00000000, B00111111, B11111011, B11111111, B11110001,
  B10011111, B00000000, B00000111, B11110111, B11111111, B11110011, B11111111, B11100001,
  B10001111, B00111111, B11111111, B11100111, B11111111, B11110111, B11111111, B11000001,
  B10001111, B00111111, B11111111, B11101111, B11111111, B11100111, B11111111, B11000001,
  B10001111, B01111111, B11111111, B11101111, B11111111, B11101111, B11111111, B10000001,
  B10001111, B01111111, B11111111, B11001111, B11111111, B11001111, B11111111, B10000001,
  B10000111, B01111111, B11111111, B11011111, B11111111, B11011111, B11111111, B00000001,
  B10000110, B01111111, B11111111, B11011111, B11111111, B11011111, B11111111, B00000001,
  B10000110, B01111111, B11111111, B10011111, B11111111, B10111111, B11111110, B00000001,
  B10000010, B11111111, B11111111, B10111111, B11111111, B10111111, B11111000, B00000001,
  B10000010, B11111111, B11111111, B10111111, B11111111, B00110000, B00000000, B00000001,
  B10000010, B11111111, B11111111, B00111111, B11100000, B00000000, B00000000, B00000001,
  B10000000, B11111111, B11111111, B00000000, B00000110, B00000000, B00000000, B00000001,
  B10000000, B11111111, B11000000, B00000111, B11111110, B00000000, B00000000, B00000001,
  B10000000, B10000000, B00001110, B01111111, B11111100, B00000000, B00000000, B00000001,
  B10000000, B00000000, B00111110, B11111111, B11111100, B00000000, B00000000, B00000001,
  B10000000, B00000000, B00011110, B11111111, B11111100, B00000000, B00000000, B00000001,
  B10000000, B00000000, B00011100, B11111111, B11111000, B00000000, B00000000, B00000001,
  B10000000, B00000000, B00011101, B11111111, B11111000, B00000000, B00000000, B00000001,
  B10000000, B00000000, B00001101, B11111111, B11110000, B00000000, B00000000, B00000001,
  B10000000, B00000000, B00001001, B11111111, B11110000, B00000000, B00000000, B00000001,
  B10000000, B00000000, B00000011, B11111111, B11100000, B00000000, B00000000, B00000001,
  B10000000, B00000000, B00000011, B11110000, B00000000, B00000000, B00000000, B00000001
};




//#define PIEZO_PIN   3
#define LED_PIN     9


//struct for pieces

struct PieceSpace {
  byte umBlock[4][4];
  byte Row;
  byte Col;
};

//Globals, is a mess. To do: tidy up and reduce global use if possible

byte pageArray[8] = { 0 };
byte scoreDisplayBuffer[8][6] = { { 0 }, { 0 } };
byte nextBlockBuffer[8][2] = { { 0 }, { 0 } };
bool optomizePageArray[8] = { 0 };
byte blockCol[10] = { 0 };
byte tetrisScreen[14][25] = { { 1 } , { 1 } };
PieceSpace currentPiece = { { 0 }, 0, 0 };
PieceSpace oldPiece = { { 0 }, 0, 0 };
byte nextPiece = 0;
bool gameOver = false;
unsigned long moveTime = 0;
int pageStart = 0;
int pageEnd = 0;

int score = 0;
int acceleration = 0;
int level = 0;
int levellineCount = 0;
unsigned int dropDelay = 1000;

int lastKey = 0;

// I2C

void OLEDCommand(byte command) {
  Wire.beginTransmission(OLED_ADDRESS);
  Wire.write(OLED_COMMAND);
  Wire.write(command);
  Wire.endTransmission();
}


void OLEDData(byte data) {
  Wire.beginTransmission(OLED_ADDRESS);
  Wire.write(OLED_DATA);
  Wire.write(data);
  Wire.endTransmission();
}


void setup() {
  Serial.begin(9600);
  while (!Serial)
    ;

  // Turn on screen - Specific for MyTrackr board
  pinMode (8, OUTPUT);
  digitalWrite (8, LOW);
  delay (100);

  Wire.begin();
  Wire.setClock(400000);

  //  pinMode(PIEZO_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  buttons.begin ();

  OLEDCommand(OLED_DISPLAY_OFF);
  delay(20);
  OLEDCommand(OLED_DISPLAY_ON);
  delay(20);
  OLEDCommand(OLED_NORMAL_DISPLAY);
  delay(20);
  OLEDCommand(0x8D);
  delay(20);
  OLEDCommand(0x14);
  delay(20);
  OLEDCommand(OLED_NORMAL_DISPLAY);

  fillTetrisScreen(0);

  randomSeed(analogRead(3)); /// To do: create a decent random number generator.

  // blink led
  digitalWrite(LED_PIN, HIGH);
  delay(100);
  digitalWrite(LED_PIN, LOW);
  delay(200);
  digitalWrite(LED_PIN, HIGH);
  delay(50);
  digitalWrite(LED_PIN, LOW);
}


void fillTetrisArray(byte value) {
  for (byte r = 0; r < 24; r++) {
    for (byte c = 0; c < 14; c++) {
      tetrisScreen[c][r] = value;
    }
  }
  
  for (byte r = 21; r < 24; r++) {
    for (byte c = 0; c < 14; c++) {
      tetrisScreen[c][r] = 0;
    }
  }
}


void fillTetrisScreen(byte value) {
  for (int r = 1; r < 21; r++) {
    for (int c = 2; c < 12; c++) {
      tetrisScreen[c][r] = value;
    }
  }
}


void drawTetrisScreen() {
  for (byte r = 1; r < 21; r++) {
    //loop through rows to see if there is data to be sent
    for (byte c = 2; c < 12; c++) {
      if ((tetrisScreen[c][r] == 2) | (tetrisScreen[c][r] == 3)) {
        //send line to screen
        for (byte i = 0; i < 10; i++) {
          blockCol[i] = tetrisScreen[i + 2][r];
          //clear delete block
          if (tetrisScreen[i + 2][r] == 3) tetrisScreen[i + 2][r] = 0;
        }
        drawTetrisLine((r - 1) * 6);
        break;
        break;
      }
    }
  }
}


void drawTetrisTitle(bool blank = false) {
  byte byteval;

  //set Vertical addressing mode and column - page start end
  OLEDCommand(OLED_SET_ADDRESSING);
  OLEDCommand(OLED_VERTICAL_ADDRESSING);

  OLEDCommand(OLED_SET_COLUMN);
  OLEDCommand( 50 );                //Set column start
  OLEDCommand( 66 );              //Set column end

  OLEDCommand(OLED_SET_PAGE);
  OLEDCommand( 1 );               //Set page start
  OLEDCommand( 5 );               //Set page end

  for (int r = 0; r < 16; r++) {
    for (int c = 4; c >= 0; c--) {
      if (blank) {
        OLEDData(0);
      } else {
        byteval = pgm_read_byte(&welcomeScreen[r][c]);
        OLEDData(byteval);
      }
    }
  }

  OLEDCommand(OLED_SET_COLUMN);
  OLEDCommand( 1 );                //Set column start
  OLEDCommand( 42 );              //Set column end

  OLEDCommand(OLED_SET_PAGE);
  OLEDCommand( 0 );               //Set page start
  OLEDCommand( 7 );               //Set page end

  for (int r = 0; r < 40; r++) {
    for (int c = 7; c >= 0; c--) {
      if (blank) {
        OLEDData(0);
      } else {
        byteval = pgm_read_byte(&tetrisLogo[r][c]);
        OLEDData(byteval);
      }
    }
  }

  OLEDCommand(OLED_SET_COLUMN);
  OLEDCommand( 75 );                //Set column start
  OLEDCommand( 116 );              //Set column end

  OLEDCommand(OLED_SET_PAGE);
  OLEDCommand( 0 );               //Set page start
  OLEDCommand( 7 );               //Set page end

  for (int r = 0; r < 36; r++) {
    for (int c = 7; c >= 0; c--) {
      if (blank) {
        OLEDData(0);
      } else {
        byteval = pgm_read_byte(&brickLogo[r][c]);
        OLEDData(byteval);
      }
    }
  }
  //brickLogo[36][8]
}


void drawTetrisLine(byte x) {
  //fill array with blocks based on blockRow

  //clear page and Optimize array
  memset(optomizePageArray, 0, 8);   ///review this... declare them here? interesting question...
  memset(pageArray, 0, 8);

  x++; // up one
  //*********Column 0***********
  //draw block
  if (blockCol[0] == 2 || blockCol[0] == 1) {
    pageArray[0] = pageArray[0] | B11111001;
    optomizePageArray[0] = 1;
  }
  //delete block
  if (blockCol[0] == 3) {
    pageArray[0] = pageArray[0] | B00000001; //create side wall
    pageArray[0] = pageArray[0] & B00000111;
    optomizePageArray[0] = 1;
  }
  //*********Column 1***********
  if (blockCol[1] == 2 || blockCol[1] == 1) {
    pageArray[1] = pageArray[1] | B00111110;
    optomizePageArray[1] = 1;
  }
  //delete block
  if (blockCol[1] == 3) {
    pageArray[1] = pageArray[1] & B11000001;
    optomizePageArray[1] = 1;
  }
  //*********Column 2***********
  if (blockCol[2] == 2 || blockCol[2] == 1) {
    pageArray[1] = pageArray[1] | B10000000;
    optomizePageArray[1] = 1;
    pageArray[2] = pageArray[2] | B00001111;
    optomizePageArray[2] = 1;
  }
  //delete block
  if (blockCol[2] == 3) {
    pageArray[1] = pageArray[1] & B01111111;
    optomizePageArray[1] = 1;
    pageArray[2] = pageArray[2] & B11110000;
    optomizePageArray[2] = 1;
  }
  //*********Column 3***********
  if (blockCol[3] == 2 || blockCol[3] == 1) {
    pageArray[2] = pageArray[2] | B11100000;
    optomizePageArray[2] = 1;
    pageArray[3] = pageArray[3] | B00000011;
    optomizePageArray[3] = 1;
  }
  //delete block
  if (blockCol[3] == 3) {
    pageArray[2] = pageArray[2] & B00011111;
    optomizePageArray[2] = 1;
    pageArray[3] = pageArray[3] & B11111100;
    optomizePageArray[3] = 1;
  }
  //*********Column 4***********
  if (blockCol[4] == 2 || blockCol[4] == 1) {
    pageArray[3] = pageArray[3] | B11111000;
    optomizePageArray[3] = 1;
  }
  //delete block
  if (blockCol[4] == 3) {
    pageArray[3] = pageArray[3] & B00000111;
    optomizePageArray[3] = 1;
  }
  //*********Column 5***********
  if (blockCol[5] == 2 || blockCol[5] == 1) {
    pageArray[4] = pageArray[4] | B00111110;
    optomizePageArray[4] = 1;
  }

  //delete block
  if (blockCol[5] == 3) {
    pageArray[4] = pageArray[4] & B11000001;
    optomizePageArray[4] = 1;
  }
  //*********Column 6***********
  if (blockCol[6] == 2 || blockCol[6] == 1) {
    pageArray[4] = pageArray[4] | B10000000;
    optomizePageArray[4] = 1;
    pageArray[5] = pageArray[5] | B00001111;
    optomizePageArray[5] = 1;
  }
  //delete block
  if (blockCol[6] == 3) {
    pageArray[4] = pageArray[4] & B01111111;
    optomizePageArray[4] = 1;
    pageArray[5] = pageArray[5] & B11110000;
    optomizePageArray[5] = 1;
  }
  //*********Column 7***********
  if (blockCol[7] == 2 || blockCol[7] == 1) {
    pageArray[5] = pageArray[5] | B11100000;
    optomizePageArray[5] = 1;
    pageArray[6] = pageArray[6] | B00000011;
    optomizePageArray[6] = 1;
  }
  if (blockCol[7] == 3) {
    pageArray[5] = pageArray[5] & B00011111;
    optomizePageArray[5] = 1;
    pageArray[6] = pageArray[6] & B11111100;
    optomizePageArray[6] = 1;
  }
  //*********Column 8***********
  if (blockCol[8] == 2 || blockCol[8] == 1) {
    pageArray[6] = pageArray[6] | B11111000;
    optomizePageArray[6] = 1;
  }
  //delete block
  if (blockCol[8] == 3) {
    pageArray[6] = pageArray[6] & B00000111;
    optomizePageArray[6] = 1;
  }
  //*********Column 9***********
  if (blockCol[9] == 2 || blockCol[9] == 1) {
    pageArray[7] = pageArray[7] | B10111110;
    optomizePageArray[7] = 1;
  }
  if (blockCol[9] == 3) {
    pageArray[7] = pageArray[7] | B10000000;//create side wall
    pageArray[7] = pageArray[7] & B11000001;
    optomizePageArray[7] = 1;
  }
  //Optimize - figure out what page array has data
  for (int page = 0; page < 8; page++) {
    if (optomizePageArray[page]) {
      //block found set page start
      pageStart = page;
      break;
    }
  }
  for (int page = 7; page >= 0; page--) {
    if (optomizePageArray[page]) {
      //block found set page end
      pageEnd = page;
      break;
    }
  }

  //set Vertical addressing mode and column - page start end
  OLEDCommand(OLED_SET_ADDRESSING);
  OLEDCommand(OLED_VERTICAL_ADDRESSING);
  OLEDCommand(OLED_SET_COLUMN);
  OLEDCommand(x);
  OLEDCommand(x + 4);
  OLEDCommand(OLED_SET_PAGE);
  OLEDCommand(pageStart);
  OLEDCommand(pageEnd);

  //send the array 5 times
  for (int c = 0; c < 5; c++) {
    for (int p = pageStart; p <= pageEnd; p++) {
      OLEDData(pageArray[p]);
    }
  }
}


void loadPiece(byte pieceNumber, byte row, byte col, bool loadScreen) {
  //load the piece from piece array to screen
  byte pieceRow = 0;
  byte pieceCol = 0;
  byte c = 0;

  // load piece from progmem
  byte byte_in;
  bool piece_out[4][4];
  byte piece_bit[2] = {0, 0};

  for (int i = 0; i < 2; i++) {

    byte_in = pgm_read_byte(&Blocks[pieceNumber - 1][i]);

    for ( byte mask = 1; mask; mask <<= 1) {
      if (mask & byte_in) {
        piece_out[piece_bit[0]][piece_bit[1]] = 1;
      } else {
        piece_out[piece_bit[0]][piece_bit[1]] = 0;
      }
      piece_bit[1]++;
      if (piece_bit[1] >= 4) {
        piece_bit[1] = 0;
        piece_bit[0]++;
      }
    }
  }

  memcpy(currentPiece.umBlock, piece_out, 16);

  currentPiece.Row = row;
  currentPiece.Col = col;

  if (loadScreen) {
    oldPiece = currentPiece;

    for (c = col; c < col + 4; c++) {
      for (int r = row; r < row + 4; r++) {
        if (currentPiece.umBlock[pieceCol][pieceRow]) tetrisScreen[c][r] = 2;
        pieceRow++;
      }
      pieceRow = 0;
      pieceCol++;
    }
  }
}


void drawPiece() {
  byte col;
  byte row;
  byte pieceRow = 0;
  byte pieceCol = 0;
  byte c = 0;

  // delete blocks first

  col = oldPiece.Col;
  row = oldPiece.Row;

  for (c = col; c < col + 4; c++) {
    for (byte r = row; r < row + 4; r++) {
      if (oldPiece.umBlock[pieceCol][pieceRow]) tetrisScreen[c][r] = 3;
      pieceRow++;
    }
    pieceRow = 0;
    pieceCol++;
  }

  //draw new blocks
  pieceRow = 0;
  pieceCol = 0;
  c = 0;

  col = currentPiece.Col;
  row = currentPiece.Row;

  for (c = col; c < col + 4; c++) {
    for (byte r = row; r < row + 4; r++) {
      if (currentPiece.umBlock[pieceCol][pieceRow]) tetrisScreen[c][r] = 2;
      pieceRow++;
    }
    pieceRow = 0;
    pieceCol++;
  }
}


void drawLandedPiece() {
  byte pieceRow = 0;
  byte pieceCol = 0;

  // Landed pieces are 1

  byte col = currentPiece.Col;
  byte row = currentPiece.Row;

  for (byte c = col; c < col + 4; c++) {
    for (byte r = row; r < row + 4; r++) {
      if (currentPiece.umBlock[pieceCol][pieceRow]) tetrisScreen[c][r] = 1;
      pieceRow++;
    }
    pieceRow = 0;
    pieceCol++;
  }
  processCompletedLines();
}


bool led = true;


void RotatePiece() {
  byte i, j;

  byte umFig[4][4] = { 0 };

  memcpy(oldPiece.umBlock, currentPiece.umBlock, 16);
  oldPiece.Row = currentPiece.Row;
  oldPiece.Col = currentPiece.Col;

  for (i = 0; i < 4; ++i) {
    for (j = 0; j < 4; ++j) {
      umFig[j][i] = currentPiece.umBlock[4 - i - 1][j];
    }
  }

  oldPiece = currentPiece;
  memcpy(currentPiece.umBlock, umFig, 16);

  if (checkCollision()) currentPiece = oldPiece;

  // no need for this...
  if (led) {
    digitalWrite(LED_PIN, HIGH);
    led = false;
  }
  delay(1);
  digitalWrite(LED_PIN, LOW);
  if (led == false) {
    digitalWrite(LED_PIN, LOW);
    led = true;
  }
}


void movePieceDown() {
  bool pieceLanded = false;

  oldPiece = currentPiece;
  currentPiece.Row = currentPiece.Row - 1;

  //check collision
  if (checkCollision()) {
    // its at the bottom make it a landed piece and start new piece
    currentPiece = oldPiece; // back to where it was
    drawLandedPiece();
    pieceLanded = true;
  }

  if (pieceLanded) {
    loadPiece(nextPiece, 19, 4, false);
    acceleration = 0;

    if (checkCollision()) {
      gameOver = true;
    } else {
      loadPiece(nextPiece, 19, 4, true);
      acceleration = 0;//reset acceleration as there is a new piece
    }

    nextPiece = random(1, 8);
    setNextBlock(nextPiece);
  }
}


void movePieceLeft() {
  oldPiece = currentPiece;
  currentPiece.Col = currentPiece.Col - 1;
  //check collision
  if (checkCollision()) 	{
    currentPiece = oldPiece; // back to where it was
  }
}


void movePieceRight() {
  oldPiece = currentPiece;
  currentPiece.Col = currentPiece.Col + 1;
  //check collision
  if (checkCollision()) 	{
    currentPiece = oldPiece; // back to where it was
  }
}


bool checkCollision() {
  byte pieceRow = 0;
  byte pieceCol = 0;
  byte c = 0;
  byte col = currentPiece.Col;
  byte row = currentPiece.Row;

  //scan across piece and translate to Tetris array and check Collisions.
  for (c = col; c < col + 4; c++) {
    for (byte r = row; r < row + 4; r++) {
      if (currentPiece.umBlock[pieceCol][pieceRow]) {
        if (tetrisScreen[c][r] == 1) return true; //is it on landed blocks?
      }
      pieceRow++;
    }
    pieceRow = 0;
    pieceCol++;
  }
  return false;
}


void processCompletedLines() {
  byte colCheck = 0;
  bool noLine = true;
  byte linesProcessed = 0;
  byte clearedLines = 0;
  byte topRow = 0;
  byte bottomRow = 0;
  byte currentRow = 0;
  int amountScored = 0;

  if (currentPiece.Row < 1)
    bottomRow = 1;
  else
    bottomRow = currentPiece.Row;

  for (int rowCheck = bottomRow; rowCheck < currentPiece.Row + 4; rowCheck++) {
    bool fullLine = true;
    
    for (colCheck = 2; colCheck < 12; colCheck++) {
      if (tetrisScreen[colCheck][rowCheck] == 0) {
        fullLine = false;
        break;
      }
    }

    if (fullLine) {
      //make line values 3's and render
      for (byte c = 2; c < 12; c++) {
        tetrisScreen[c][rowCheck] = 3;
      }

      bottomRow = rowCheck + 1;
      //line is now all 0's
      linesProcessed++;
      delay(77); // animation :)
    }

    drawTetrisScreen();
  }

  //******all lines are 0's and have been removed from the screen
  if (linesProcessed) {

    clearedLines = linesProcessed;

    while (clearedLines) {
      for (currentRow = 1; currentRow < 20; currentRow++) {
        noLine = true;
        for (byte c = 2; c < 12; c++) {
          if (tetrisScreen[c][currentRow])  noLine = false;
        }
        if (noLine) {
          //move all lines down
          for (byte r = currentRow + 1; r < 20; r++) {
            for (byte c = 2; c < 12; c++) {
              if (tetrisScreen[c][r]) tetrisScreen[c][r - 1] = 2;
              else tetrisScreen[c][r - 1] = 3;
            }
          }
        }
      }

      //make the 2's 1's
      for (byte r = 1; r < 24; r++) {
        for (byte c = 2; c < 12; c++) {
          if (tetrisScreen[c][r] == 2)tetrisScreen[c][r] = 1;
        }
      }
      clearedLines--;
      drawTetrisScreen();
      //      tone(PIEZO_PIN, 1000, 50);
      delay(60);
      //      tone(PIEZO_PIN, 2000, 50);
      delay(50);
      //      tone(PIEZO_PIN, 500, 50);
      delay(60);
    }
  }

  // ************** process score *******************
  switch (linesProcessed) {
    case 1: 	amountScored = 40 * (level + 1); break;
    case 2: 	amountScored = 100 * (level + 1); break;
    case 3: 	amountScored = 300 * (level + 1); break;
    case 4: 	amountScored = 1200 * (level + 1);
      //do 4 line affect
      OLEDCommand(OLED_INVERSE_DISPLAY);
      delay(20);
      OLEDCommand(OLED_NORMAL_DISPLAY);
      break;
  }

  //score animation
  for (long s = score; s < score + amountScored; s = s + (5 * (level + 1))) {
    setScore(s, false);
  }

  score = score + amountScored;
  setScore(score, false);

  //****update level line count
  levellineCount = levellineCount + linesProcessed;
  if (levellineCount > 10) {
    level++;
    levellineCount = 0;
    //do level up affect
    OLEDCommand(OLED_INVERSE_DISPLAY);
    delay(100);
    OLEDCommand(OLED_NORMAL_DISPLAY);
    for (int i = 250; i < 2500; i += 100) {
      //      tone(PIEZO_PIN, i, 5);
      delay(5);
      //      tone(PIEZO_PIN, i / 2, 5);
      delay(10);
    }
    OLEDCommand(OLED_INVERSE_DISPLAY);
    delay(100);
    OLEDCommand(OLED_NORMAL_DISPLAY);
  }

  //make the 2's 1's
  for (byte r = bottomRow; r <= topRow; r++) {
    for (byte c = 2; c < 12; c++) {
      if (tetrisScreen[c][r]) {
        tetrisScreen[c][r] = 1;
      }
    }
  }
}


void tetrisScreenToSerial() {
  //for debug
  for (byte r = 0; r < 24; r++) {
    for (byte c = 0; c < 14; c++) {
      Serial.print(tetrisScreen[c][r], DEC);
    }
    Serial.println();
  }
  Serial.println();
}


void processKeys () {
  bool keypressed = true;

  byte keys = buttons.read ();
  if (keys & Buttons::KEY_LEFT) {
    movePieceLeft();
  } else if (keys & Buttons::KEY_RIGHT) {
    movePieceRight();
  } else if (keys & Buttons::KEY_DOWN) {
    movePieceDown();
  } else if (keys & Buttons::KEY_ROTATE) {
    RotatePiece();
  } else {
    keypressed = false;
  }

  if (keypressed) {
    drawPiece ();
    drawTetrisScreen ();
  }
}


void setScore(long score, bool blank)
{
  // this is a kludge. To do: create a proper system for rendering numbers and letters.


  long ones = (score % 10);
  long tens = ((score / 10) % 10);
  long hundreds = ((score / 100) % 10);
  long thousands = ((score / 1000) % 10);
  long tenthousands = ((score / 10000) % 10);
  long hunderedthousands = ((score / 100000) % 10);


  //create the score in upper left part of the screen
  char bytes_out[8];
  memset(scoreDisplayBuffer, 0, sizeof scoreDisplayBuffer);

  //****************score digit 6****************

  for (int v = 0; v < 8; v++) bytes_out[v] = pgm_read_byte(&NumberFont[hunderedthousands][v]);

  //write the number to the Score buffer
  for (int i = 0; i < 8; i++)
  {
    scoreDisplayBuffer[i][0] = scoreDisplayBuffer[i][0] | bytes_out[i] >> 1;
  }

  //****************score digit 5****************

  for (int v = 0; v < 8; v++) bytes_out[v] = pgm_read_byte(&NumberFont[tenthousands][v]);

  //write the number to the Score buffer
  for (int i = 0; i < 8; i++)
  {
    scoreDisplayBuffer[i][0] = scoreDisplayBuffer[i][0] | (bytes_out[i] << 6);
  }

  //write the number to the Score buffer
  for (int i = 0; i < 8; i++)
  {
    scoreDisplayBuffer[i][1] = scoreDisplayBuffer[i][1] | bytes_out[i] >> 1;
  }

  //****************score digit 4****************

  for (int v = 0; v < 8; v++) bytes_out[v] = pgm_read_byte(&NumberFont[thousands][v]);


  //write the number to the Score buffer
  for (int i = 0; i < 8; i++)
  {
    scoreDisplayBuffer[i][1] = scoreDisplayBuffer[i][1] | (bytes_out[i] << 6);
  }

  //write the number to the Score buffer
  for (int i = 0; i < 8; i++)
  {
    scoreDisplayBuffer[i][2] = scoreDisplayBuffer[i][2] | bytes_out[i] >> 1;
  }

  //****************score digit 3****************

  for (int v = 0; v < 8; v++) bytes_out[v] = pgm_read_byte(&NumberFont[hundreds][v]);

  //write the number to the Score buffer
  for (int i = 0; i < 8; i++)
  {
    scoreDisplayBuffer[i][2] = scoreDisplayBuffer[i][2] | (bytes_out[i] << 6);
  }

  //write the number to the Score buffer
  for (int i = 0; i < 8; i++)
  {
    scoreDisplayBuffer[i][3] = scoreDisplayBuffer[i][3] | bytes_out[i] >> 1;
  }


  //****************score digit 2****************

  for (int v = 0; v < 8; v++) bytes_out[v] = pgm_read_byte(&NumberFont[tens][v]);

  //write the number to the Score buffer
  for (int i = 0; i < 8; i++)
  {
    scoreDisplayBuffer[i][3] = scoreDisplayBuffer[i][3] | (bytes_out[i] << 6);
  }

  //write the number to the Score buffer
  for (int i = 0; i < 8; i++)
  {
    scoreDisplayBuffer[i][4] = scoreDisplayBuffer[i][4] | bytes_out[i] >> 1;
  }


  //****************score digit 1****************

  for (int v = 0; v < 8; v++) bytes_out[v] = pgm_read_byte(&NumberFont[ones][v]);

  //write the number to the Score buffer
  for (int i = 0; i < 8; i++)
  {
    scoreDisplayBuffer[i][4] = scoreDisplayBuffer[i][4] | (bytes_out[i] << 6);
  }

  //write the number to the Score buffer
  for (int i = 0; i < 8; i++)
  {
    scoreDisplayBuffer[i][5] = scoreDisplayBuffer[i][5] | bytes_out[i] >> 1;

  }

  //set Vertical addressing mode and column - page start end
  OLEDCommand(OLED_SET_ADDRESSING);
  OLEDCommand(OLED_VERTICAL_ADDRESSING);

  OLEDCommand(OLED_SET_COLUMN);
  OLEDCommand(120);                 //Set column start
  OLEDCommand(127);                 //Set column end

  OLEDCommand(OLED_SET_PAGE);
  OLEDCommand(0);                  //Set page start
  OLEDCommand(5);                  //Set page end

  for (int p = 0; p < 8; p++)
  {
    for (int c = 0; c < 6; c++)
    {
      if (blank) OLEDData(0);
      else OLEDData(scoreDisplayBuffer[p][c]);
    }

  }
}


void setNextBlock(byte pieceNumber) {
  memset(nextBlockBuffer, 0, sizeof nextBlockBuffer); //clear buffer
  switch (pieceNumber) {
    case 1:
      //************l piece - 1 *************
      for (int k = 2; k < 6; k++) {
        nextBlockBuffer[k][0] = B01110111;
        nextBlockBuffer[k][1] = B01110111;
      }
      break;
    case 2:
      //************J piece - 2 *************
      for (int k = 0; k < 3; k++) {
        nextBlockBuffer[k][0] = B01110000;
        nextBlockBuffer[k][1] = B01110111;
      }
      for (int k = 4; k < 7; k++) {
        nextBlockBuffer[k][0] = B01110000;
      }
      break;
    case 3:
      //************L piece - 3 *************
      for (int k = 0; k < 3; k++) {
        nextBlockBuffer[k][0] = B01110000;
      }
      for (int k = 4; k < 7; k++) {
        nextBlockBuffer[k][0] = B01110000;
        nextBlockBuffer[k][1] = B01110111;
      }
      break;
    case 4:
      //************O piece - 4 *************
      for (int k = 0; k < 3; k++) {
        nextBlockBuffer[k][0] = B01110000;
        nextBlockBuffer[k][1] = B00000111;
      }
      for (int k = 4; k < 7; k++) {
        nextBlockBuffer[k][0] = B01110000;
        nextBlockBuffer[k][1] = B00000111;
      }
      break;
    case 5:
      //************S piece - 5 *************
      for (int k = 0; k < 3; k++) {
        nextBlockBuffer[k][0] = B01110000;
        nextBlockBuffer[k][1] = B00000111;
      }
      for (int k = 4; k < 7; k++) {
        nextBlockBuffer[k][0] = B00000000;
        nextBlockBuffer[k][1] = B11101110;
      }
      break;
    case 6:
      //************T piece - 6 *************
      for (int k = 0; k < 3; k++) {
        nextBlockBuffer[k][0] = B01110000;
        nextBlockBuffer[k][1] = B01110111;
      }
      for (int k = 4; k < 7; k++) {
        nextBlockBuffer[k][0] = B00000000;
        nextBlockBuffer[k][1] = B00001110;
      }
      break;
    case 7:
      //************Z piece - 7 *************
      for (int k = 0; k < 3; k++) {
        nextBlockBuffer[k][0] = B01110000;
        nextBlockBuffer[k][1] = B00000111;
      }
      for (int k = 4; k < 7; k++) {
        nextBlockBuffer[k][0] = B11101110;
        nextBlockBuffer[k][1] = B00000000;
      }
      break;
  }

  //set Vertical addressing mode and column - page start end
  OLEDCommand(OLED_SET_ADDRESSING);
  OLEDCommand(OLED_VERTICAL_ADDRESSING);

  OLEDCommand(OLED_SET_COLUMN);
  OLEDCommand(120);                 //Set column start
  OLEDCommand(127);                 //Set column end

  OLEDCommand(OLED_SET_PAGE);
  OLEDCommand(6);                  //Set page start
  OLEDCommand(7);                  //Set page end

  for (int p = 0; p < 8; p++) {
    for (int c = 0; c < 2; c++) {
      OLEDData(nextBlockBuffer[p][c]);
    }
  }
}


void drawBottom() {

  //set Vertical addressing mode and column - page start end
  OLEDCommand(OLED_SET_ADDRESSING);
  OLEDCommand(OLED_VERTICAL_ADDRESSING);

  OLEDCommand(OLED_SET_COLUMN);
  OLEDCommand(0);              //Set column start
  OLEDCommand(0);              //Set column end

  OLEDCommand(OLED_SET_PAGE);
  OLEDCommand(0);              //Set page start
  OLEDCommand(7);              //Set page end

  for (int c = 0; c < 8; c++) {
    OLEDData(255);
  }
}


void drawSides() {

  //set Vertical addressing mode and column - page start end
  OLEDCommand(OLED_SET_ADDRESSING);
  OLEDCommand(OLED_VERTICAL_ADDRESSING);

  OLEDCommand(OLED_SET_COLUMN);
  OLEDCommand(0);                //Set column start
  OLEDCommand(127);              //Set column end

  OLEDCommand(OLED_SET_PAGE);
  OLEDCommand(0);               //Set page start
  OLEDCommand(7);               //Set page end

  for (int r = 0; r < 128; r++) {
    for (int c = 0; c < 8; c++) {
      if (c == 0) OLEDData(1);
      else if (c == 7) OLEDData(128);
      else OLEDData(0);
    }
  }
}


void loop() {
  //main loop code
  //To do: create high score system that savees to EEprom
  gameOver = false;
  score = 0;
  fillTetrisArray(1); //fill with 1's to make border
  fillTetrisScreen(2);
  drawTetrisScreen();
  delay(200);
  fillTetrisScreen(3);
  drawTetrisScreen();
  delay(200);
  drawSides();
  drawBottom();

  // tetrisScreenToSerial();

  OLEDCommand(OLED_INVERSE_DISPLAY);
  delay(200);
  OLEDCommand(OLED_NORMAL_DISPLAY);

  loadPiece(random(1, 8), 20, 5, true);
  drawTetrisScreen();
  nextPiece = random(1, 8);
  setNextBlock(nextPiece);

  setScore(0, false);
  delay(300);
  setScore(0, true);
  delay(300);
  setScore(0, false);

  drawTetrisTitle(false);

  TetrisTheme::start();
  while (songOn)
    TetrisTheme::tetrisThemePlay();

  // Wait for keypress
  while (buttons.read () == Buttons::KEY_NONE)
    ;

  drawTetrisTitle(true);
  drawSides();
  drawBottom();
  setScore(0, false);

  for (byte i = 1; i < 10; i++) {
    nextPiece = random(1, 8);
    setNextBlock(nextPiece);
    delay(100);
  }

  while (!gameOver) {
    movePieceDown();
    drawPiece();
    drawTetrisScreen();
    moveTime = millis();
    while (millis() - moveTime < (dropDelay - (level * 50))) {
      processKeys();
    }
  }
}

