/***********************************************************
  File name: Adeept_snakeGame.ino
  Website: www.adeept.com
  E-mail: support@adeept.com
  Author: Tom
  Date: 2017/01/07

  Modified by SukkoPera, March 2018:
  - Port to MyTrackr Board V1.0
  - Actually generate food in random positions
  - Other minor changes here & there
***********************************************************/
#include <U8glib.h>

#define RIGHT 0
#define UP    1
#define LEFT  2
#define DOWN  3

#define KEY_BACK_PIN 7
#define KEY_PREV_PIN 6
#define KEY_NEXT_PIN 5
#define KEY_SELECT_PIN 4

U8GLIB_SSD1306_128X64 u8g (U8G_I2C_OPT_DEV_0 | U8G_I2C_OPT_NO_ACK | U8G_I2C_OPT_FAST);

int score = 0;
int level = 1;
int gamespeed = 100;
int i;

//Constitute the unit, 4x4 pixels in the box.
const uint8_t ele[] PROGMEM = {
  0xf0, //B1111000
  0xb0, //B1011000
  0xd0, //B1101000
  0xf0, //B1111000
};

//snake
void Aelement(int x, int y) {
  u8g.drawBitmapP(x, y, 1, 4, ele);
}

struct FOOD {
  int x;
  int y;
  int yes;
};

FOOD food = {0, 0, 0};

struct SNAKE {
  int x[200];
  int y[200];
  int node;
  int dir;
  int lefe;
};

SNAKE snake = {{9, 5}, {30, 30}, 2, RIGHT, 0};
void UI() {   //Basic game interface
  u8g.drawFrame(0, 1, 102, 62); //Internal boundary
  u8g.drawFrame(0, 0, 102, 64); //External boundary
  u8g.setFont(u8g_font_5x7);    //Set the font
  u8g.drawStr(104, 12, "LEVEL"); //Level of tips
  u8g.drawStr(104, 40, "SCORE"); //Score tips
}

void printScore(int x, int y, int s) {
  u8g.setFont(u8g_font_6x10);
  u8g.setPrintPos(x, y);
  u8g.print(s);
}

void key() {
  if (LOW == digitalRead(KEY_NEXT_PIN)) {
    snake.dir = DOWN;
  }
  if (LOW == digitalRead(KEY_SELECT_PIN)) {
    snake.dir = RIGHT;
  }
  if (LOW == digitalRead(KEY_BACK_PIN)) {
    snake.dir = LEFT;
  }
  if (LOW == digitalRead(KEY_PREV_PIN)) {
    snake.dir = UP;
  }
}

void newFood() {
  food.x = random(0, 25) * 4 + 1;
  food.y = random(0, 15) * 4 + 2;
}

void snakeGame() {
  switch (snake.dir) {
    case RIGHT:
      snake.x[0] += 4;
      if (snake.x[0] >= 101) {
        snake.x[0] = 1;
      } break;
    case UP:
      snake.y[0] -= 4;
      if (snake.y[0] <= 1) {
        snake.y[0] = 58;
      } break;
    case LEFT:
      snake.x[0] -= 4;
      if (snake.x[0] <= 0) {
        snake.x[0] = 97;
      } break;
    case DOWN:
      snake.y[0] += 4;
      if (snake.y[0] >= 62) {
        snake.y[0] = 2;
      } break;
  }
  if ((snake.x[0] == food.x) && (snake.y[0] == food.y)) {
    snake.x[0] = food.x;
    snake.y[0] = food.y;
    snake.node++;
    food.yes = 1;
    score += 2;
    level = score / 10 + 1; //FOOD food = {random(1, 20), 50, 1};
    newFood();
  }
  for (i = snake.node - 1; i > 0; i--) {
    snake.x[i] = snake.x[i - 1];
    snake.y[i] = snake.y[i - 1];
  }
}

void setup() {
  // Turn on screen - Specific for MyTrackr board
  pinMode (8, OUTPUT);
  digitalWrite (8, LOW);
  delay (100);
  
  pinMode(KEY_BACK_PIN, INPUT_PULLUP);
  pinMode(KEY_PREV_PIN, INPUT_PULLUP);
  pinMode(KEY_NEXT_PIN, INPUT_PULLUP);
  pinMode(KEY_SELECT_PIN, INPUT_PULLUP);

  randomSeed(analogRead(A3));

  newFood();
}

void loop() {
  u8g.firstPage();
  do {
    UI();
    for (i = 0; i < snake.node; i++)
      Aelement(snake.x[i], snake.y[i]);

    Aelement(food.x, food.y);
    printScore(103, 62, food.x);
    printScore(116, 62, food.y);
    printScore(109, 22, level);
    printScore(109, 50, score);
  } while (u8g.nextPage());
  key();
  snakeGame();
  delay(gamespeed);
}

