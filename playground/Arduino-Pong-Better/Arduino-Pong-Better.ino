/*
   Implementation of Pong with I2C OLED 128x64

   Original game retrieved from: https://github.com/ruction/arduino-pong

   Modified by SukkoPera, April 2018:
   - Port to MyTrackr Board V1.0
*/

#include <Adafruit_GFX.h>          // Adafruit GFX graphics core library
#include <Adafruit_SSD1306.h>      // Driver library for 'monochrome' 128x64 and 128x32 OLEDs


// Define the PINS you're goint to use on your Arduino
const byte KEY_BACK_PIN = 7;
const byte KEY_PREV_PIN = 6;
const byte KEY_NEXT_PIN = 5;
const byte KEY_SELECT_PIN = 4;
const byte LED_PIN = 9;

// Define variables
byte gameState = 0;                 // 0 = Home, 1 = Game, 2 = End

const byte SCREEN_H = 64;

const byte PADDLE_START_Y = SCREEN_H / 2;
const byte PADDLE_STEP = 5;         // Paddle movement step
const byte PADDLE_SIZE = 18;

int paddlePositionPlayer1 = PADDLE_START_Y;
int paddlePositionPlayer2 = PADDLE_START_Y;

int scorePlayer1 = 0;
int scorePlayer2 = 0;

int ballX = 128 / 2;
int ballY = SCREEN_H / 2;
int ballSpeedX = 2;
int ballSpeedY = 1;

Adafruit_SSD1306 display;

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

enum Key {
  KEY_NONE    = 0,
  KEY_PREV    = 1 << 0,
  KEY_NEXT    = 1 << 1,
  KEY_SELECT  = 1 << 2,
  KEY_BACK    = 1 << 3,
};

// Returns an 8-bit mask, where each button maps to bit
byte readButtons () {
  byte b = KEY_NONE;

  if (digitalRead (KEY_PREV_PIN) == LOW)
    b |= KEY_PREV;
  if (digitalRead (KEY_NEXT_PIN) == LOW)
    b |= KEY_NEXT;
  if (digitalRead (KEY_SELECT_PIN) == LOW)
    b |= KEY_SELECT;
  if (digitalRead (KEY_BACK_PIN) == LOW)
    b |= KEY_BACK;

  return b;
}


void setup() {
  // Turn on screen - Specific for MyTrackr board
  pinMode (8, OUTPUT);
  digitalWrite (8, LOW);
  delay (100);

  pinMode(LED_PIN, OUTPUT);
  pinMode (KEY_BACK_PIN, INPUT_PULLUP);
  pinMode (KEY_PREV_PIN, INPUT_PULLUP);
  pinMode (KEY_NEXT_PIN, INPUT_PULLUP);
  pinMode (KEY_SELECT_PIN, INPUT_PULLUP);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x64)
  display.clearDisplay();

  digitalWrite(LED_PIN, HIGH);                 // Status led on...
}

void loop() {

  // Read controller value and calculate paddle position
  byte buttons = readButtons();

  if (buttons != KEY_NONE && gameState == 0) {
    gameState = 1;    // Start game!
    delay(100);
  } else if (buttons != KEY_NONE && gameState == 2) {
    gameState = 0;    // Back to splash screen
    scorePlayer1 = 0;
    scorePlayer2 = 0;
    paddlePositionPlayer1 = PADDLE_START_Y;
    paddlePositionPlayer2 = PADDLE_START_Y;
    ballX = 128 / 2;
    ballY = 64 / 2;
    delay(100);
  }

  if (gameState == 0) {
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(40, 18);
    display.println("PONG");
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(26, 38);
    display.println("Press Any Key");
    display.display();
    display.clearDisplay();
  }

  if (gameState == 1) {
    // Check player 1 keys
    if (buttons & KEY_BACK)
      paddlePositionPlayer1 += PADDLE_STEP;
    else if (buttons & KEY_PREV)
      paddlePositionPlayer1 -= PADDLE_STEP;
    paddlePositionPlayer1 = constrain(paddlePositionPlayer1, 0, SCREEN_H - PADDLE_SIZE);

    // Check player 2 keys
    if (buttons & KEY_NEXT)
      paddlePositionPlayer2 += PADDLE_STEP;
    else if (buttons & KEY_SELECT)
      paddlePositionPlayer2 -= PADDLE_STEP;
    paddlePositionPlayer2 = constrain(paddlePositionPlayer2, 0, SCREEN_H - PADDLE_SIZE);
    
    drawField(scorePlayer1, scorePlayer2);

    collisionControl();
    drawBall();

    display.display();
    display.clearDisplay();
  }

  if (gameState == 2) {
    drawField(scorePlayer1, scorePlayer2);

    display.setTextSize(1);
    display.setTextColor(WHITE);

    if (scorePlayer1 == 2) {
      display.setCursor(15, 30);
    } else if (scorePlayer2 == 2) {
      display.setCursor(77, 30);
    }
    display.println("Winner!");
    display.display();
    display.clearDisplay();
  }
}

void drawField(int score1, int score2) {
  display.fillRect(0, round(paddlePositionPlayer1), 2, 18, 1);
  display.fillRect(126, round(paddlePositionPlayer2), 2, 18, 1);

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(55, 0);
  display.print(score1);
  display.print(":");
  display.print(score2);

  display.fillRect(63, 12, 1, 5, 1);
  display.fillRect(63, 22, 1, 5, 1);
  display.fillRect(63, 32, 1, 5, 1);
  display.fillRect(63, 42, 1, 5, 1);
  display.fillRect(63, 52, 1, 5, 1);
  display.fillRect(63, 62, 1, 5, 1);
}

void collisionControl() {
  //bounce from top and bottom
  if (ballY >= 64 - 2 || ballY <= 0) {
    ballSpeedY *= -1;
  }

  //score points if ball hits wall behind player
  if (ballX >= 128 - 2 || ballX <= 0) {
    if (ballSpeedX > 0) {
      scorePlayer1++;
      ballX = 128 / 4;
    }
    if (ballSpeedX < 0) {
      scorePlayer2++;
      ballX = 128 / 4 * 3;
    }
    if (scorePlayer1 == 2 || scorePlayer2 == 2) {
      gameState = 2;
    }
  }

  //bounce from player1
  if (ballX >= 0 && ballX <= 2 && ballSpeedX < 0) {
    if (ballY > round(paddlePositionPlayer1) - 2 && ballY < round(paddlePositionPlayer1) + 18) {
      ballSpeedX *= -1;
    }
  }
  //bounce from player2
  if (ballX >= 128 - 2 - 2 && ballX <= 128 - 2 && ballSpeedX > 0) {
    if (ballY > round(paddlePositionPlayer2) - 2 && ballY < round(paddlePositionPlayer2) + 18) {
      ballSpeedX *= -1;
    }

  }
}

void drawBall() {
  display.fillRect(ballX, ballY, 2, 2, 1);

  ballX += ballSpeedX;
  ballY += ballSpeedY;
}

