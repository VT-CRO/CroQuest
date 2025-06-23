#include <TFT_eSPI.h>
#include <LittleFS.h>
#include <SD.h>
#include <Arduino.h>
#include <JPEGDecoder.h>
#include "../src2/Core/JpegDrawing.cpp"
#include "../src2/Core/JpegDrawing.hpp"


// Utilizing the TFT_eSPI library
TFT_eSPI tft = TFT_eSPI();
JpegDrawing jpegDrawer(tft);

// Centering for the test
#define CENTER MC_DATUM

// Max size of the snake
// Easily adjustable
#define SNAKE_MAX_LENGTH 200

// Button pins
#define PIN_UP    35
#define PIN_DOWN  34
#define PIN_LEFT  34
#define PIN_RIGHT 35
#define PIN_A 22
#define PIN_B 1
#define PIN_SPEAKER 21

// Game Constants
const int TILE_SIZE = 16;
const int GRID_WIDTH = 30;
const int GRID_HEIGHT = 19;

#define jpg_min(a,b) (((a)<(b))?(a):(b))

// Snake movement
unsigned long lastMoveTime = 0;
const unsigned long MOVE_INTERVAL = 110; // Can be changed

// Game state
bool gameOver = false;

// Scoring mechanics
int score = 0;
int highScore = 0;

// Snake direction enum
enum Direction { UP, DOWN, LEFT, RIGHT };

// A simple point struct
struct Point 
{
  int x;
  int y;
};

// Snake storage
Point snake[SNAKE_MAX_LENGTH];
int snakeLength = 3;
Direction direction = RIGHT;
Direction lastDirectionOnTick = RIGHT;
Point food;

// Drawing and graphics
// void drawJpegAtTile(const char* filename, int tileX, int tileY);
void drawTile(int x, int y, uint16_t color);
void jpegRender(int xpos, int ypos);
void drawBackground();
void drawSnake();
void drawFood();

// Snake logic
void spawnFood();
void moveSnake();
void handleButtonInputs();
void resetGame();

// Screens and transitions
void showGetReadyScreen();
void showCreditsScreen();
void waitForStartButton();

// void drawJpegAtTile(const char* filename, int tileX, int tileY) {
//   int xpos = tileX * TILE_SIZE;
//   int ypos = tileY * TILE_SIZE;

//   Serial.printf("Trying to draw %s at tile (%d, %d) => pixels (%d, %d)\n", filename, tileX, tileY, xpos, ypos);

//   File jpegFile = SD.open(filename, FILE_READ);
//   if (!jpegFile) {
//     Serial.print("ERROR: File not found - ");
//     Serial.println(filename);
//     drawTile(tileX, tileY, TFT_RED);  // Fallback red block
//     return;
//   }

//   if (!JpegDec.decodeSdFile(jpegFile)) {
//     Serial.println("JPEG decode failed!");
//     jpegFile.close();
//     drawTile(tileX, tileY, TFT_RED);  // Fallback red block
//     return;
//   }

//   jpegFile.close();

//   Serial.printf("JPEG dimensions: %d x %d\n", JpegDec.width, JpegDec.height);
//   jpegRender(xpos, ypos);
// }

// void jpegRender(int xpos, int ypos) {
//   uint16_t *pImg;
//   uint16_t mcu_w = JpegDec.MCUWidth;
//   uint16_t mcu_h = JpegDec.MCUHeight;
//   uint32_t max_x = xpos + JpegDec.width;
//   uint32_t max_y = ypos + JpegDec.height;

//   bool swapBytes = tft.getSwapBytes();
//   tft.setSwapBytes(true);

//   uint32_t min_w = jpg_min(mcu_w, JpegDec.width % mcu_w);
//   uint32_t min_h = jpg_min(mcu_h, JpegDec.height % mcu_h);
//   uint32_t win_w = mcu_w;
//   uint32_t win_h = mcu_h;

//   while (JpegDec.read()) {
//     pImg = JpegDec.pImage;
//     int mcu_x = JpegDec.MCUx * mcu_w + xpos;
//     int mcu_y = JpegDec.MCUy * mcu_h + ypos;

//     if (mcu_x + mcu_w <= max_x) win_w = mcu_w;
//     else win_w = min_w;

//     if (mcu_y + mcu_h <= max_y) win_h = mcu_h;
//     else win_h = min_h;

//     if (win_w != mcu_w) {
//       uint16_t *cImg = pImg + win_w;
//       int p = 0;
//       for (int h = 1; h < win_h; h++) {
//         p += mcu_w;
//         for (int w = 0; w < win_w; w++) {
//           *cImg = *(pImg + w + p);
//           cImg++;
//         }
//       }
//     }

//     if ((mcu_x + win_w) <= tft.width() && (mcu_y + win_h) <= tft.height())
//       tft.pushImage(mcu_x, mcu_y, win_w, win_h, pImg);
//     else if ((mcu_y + win_h) >= tft.height())
//       JpegDec.abort();
//   }

//   tft.setSwapBytes(swapBytes);
// }

// Method for drawing a single tile
void drawTile(int x, int y, uint16_t color) 
{
  tft.fillRect(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE, color);
}

// Physically drawing the background to reduce latency
void drawBackground() 
{
  // Draw scoreboard background
  tft.fillRect(0, 0, tft.width(), TILE_SIZE, tft.color565(50, 50, 50));  // Dark grey banner
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("Score: " + String(score), 5, 2);
  tft.drawString("High: " + String(highScore), tft.width() - 80, 2);

  // Draw the play field (starting from y = 1)
  for (int x = 0; x < GRID_WIDTH; x++) 
  {
    for (int y = 1; y < GRID_HEIGHT + 1; y++) 
    {
      uint16_t color = (x + y) % 2 == 0 ? tft.color565(66, 176, 50) : tft.color565(111, 189, 99);
      drawTile(x, y, color);
    }
  }

  // Draw border
  tft.drawRect(0, TILE_SIZE, GRID_WIDTH * TILE_SIZE, GRID_HEIGHT * TILE_SIZE, tft.color565(0, 0, 0));
}

// Drawing the snake segments
void drawSnake() 
{
  for (int i = 0; i < snakeLength; i++) 
  {
    drawTile(snake[i].x, snake[i].y, tft.color565(0, 0, 255));
  }
}

// Drawing the food segments
void drawFood() {
  int xpos = food.x * TILE_SIZE;
  int ypos = food.y * TILE_SIZE;

  // Draw the JPEG sprite at the food location
  jpegDrawer.drawSdJpeg("/appletest.jpg", xpos, ypos);
  jpegDrawer.pushSprite(false);  // Push without using built-in transparency

  // Manually clear near-black pixels to simulate transparency
  for (int y = 0; y < TILE_SIZE; y++) {
    for (int x = 0; x < TILE_SIZE; x++) {
      uint16_t color = tft.readPixel(xpos + x, ypos + y);

      uint8_t r = (color >> 11) & 0x1F;
      uint8_t g = (color >> 5) & 0x3F;
      uint8_t b = color & 0x1F;

      if (r < 2 && g < 2 && b < 2) {
        // Use checkerboard background color
        uint16_t bgColor = ((food.x + food.y) % 2 == 0)
          ? tft.color565(66, 176, 50)
          : tft.color565(111, 189, 99);

        tft.drawPixel(xpos + x, ypos + y, bgColor);
      }
    }
  }
}


// Randomly spawning a food location
void spawnFood() 
{
  while (true) 
  {
    food.x = random(0, GRID_WIDTH);
    food.y = random(0, GRID_HEIGHT);
    food.y += 1;

    bool overlaps = false;

    for (int i = 0; i < snakeLength; i++) 
    {
      if (snake[i].x == food.x && snake[i].y == food.y) 
      {
        overlaps = true;
        break;
      }
    }
    if (!overlaps) break;
  }
  drawFood();
}

// Moving the snake
void moveSnake()
{
  bool ateFood = (snake[0].x == food.x && snake[0].y == food.y);

  // Checking if we collided with food
  if (ateFood && snakeLength < SNAKE_MAX_LENGTH) 
  {
    for (int i = snakeLength; i > 1; --i) snake[i] = snake[i - 1];
    snake[1] = snake[0];
    snakeLength++;
    score++;

    tft.fillRect(0, 0, tft.width(), TILE_SIZE, tft.color565(50, 50, 50));  // Clear top row
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(1);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("Score: " + String(score), 5, 2);
    tft.drawString("High: " + String(highScore), tft.width() - 80, 2);
  } 

  else 
  {
    Point tail = snake[snakeLength - 1];
    uint16_t bg = (tail.x + tail.y) & 1 ? tft.color565(111,189,99) : tft.color565( 66,176,50);
    drawTile(tail.x, tail.y, bg);
    for (int i = snakeLength - 1; i > 0; --i) snake[i] = snake[i - 1];
  }

  // Changing direction
  switch (direction) 
  {
    case UP:    --snake[0].y; break;
    case DOWN:  ++snake[0].y; break;
    case LEFT:  --snake[0].x; break;
    case RIGHT: ++snake[0].x; break;
  }
  lastDirectionOnTick = direction;

  // Checking for border hit
  if (snake[0].x < 0 || snake[0].x >= GRID_WIDTH || snake[0].y < 1 || snake[0].y >= GRID_HEIGHT + 1) 
  {
    gameOver = true;
  }

  // Self collision
  for (int i = 1; !gameOver && i < snakeLength; i++) 
  {
    if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) 
    {
      gameOver = true;
    }
  }

  // Calling game over if any collision occurs
  if (gameOver) 
  {
    // if (score > highScore) 
    // {
    //   highScore = score;
    //   File file = LittleFS.open("/highscore.txt", "w");
    //   if (file) 
    //   {
    //     file.println(highScore);
    //     file.close();
    //   }
    // }

    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(3);
    tft.setTextDatum(CENTER);
    tft.drawString("GAME OVER!", tft.width() / 2, tft.height() / 2);

    tft.setTextSize(2);
    tft.drawString("Score: " + String(score), tft.width() / 2, tft.height() / 2 + 40);
    //tft.drawString("High Score: " + String(highScore), tft.width() / 2, tft.height() / 2 + 80);

    delay(2000);

    //waitForStartButton();
    resetGame();
    return;
  }

  // draw new head
  drawTile(snake[0].x, snake[0].y, tft.color565(0, 0, 255));

  // if food was eaten, respawn it
  if (ateFood)
  {
    spawnFood();
  }
}

// Reading the input from the pins
void handleButtonInputs() {
  int val35 = analogRead(35);  // Up / Right combo pin
  int val34 = analogRead(34);  // Left / Down combo pin

  // ----------- GPIO 35 (UP / RIGHT) ----------
  if (val35 > 3900 && val35 < 4200) {  // Tune this range for RIGHT
    if (lastDirectionOnTick != LEFT) direction = RIGHT;
  } 
  else if (val35 > 3000 && val35 < 3400) {  // Tune this range for UP
    if (lastDirectionOnTick != DOWN) direction = UP;
  }

  // ----------- GPIO 34 (DOWN / LEFT) ----------
  if (val34 > 3900 && val34 < 4200) {  // Tune this range for DOWN
    if (lastDirectionOnTick != UP) direction = DOWN;
  } 
  else if (val34 > 3000 && val34 < 3400) {  // Tune this range for LEFT
    if (lastDirectionOnTick != RIGHT) direction = LEFT;
  }

  delay(20);
}

// Resetting the game after death
// Logic will be updated with buttons
void resetGame() 
{
  gameOver = false;
  snakeLength = 3;
  score = 0;
  direction = RIGHT;
  snake[0] = {5, 6};
  snake[1] = {4, 6};
  snake[2] = {3, 6};

  tft.fillScreen(TFT_BLACK);
  drawBackground();
  drawSnake();
  spawnFood();
}

// Countdown screen
void showGetReadyScreen() 
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(5);
  tft.setTextDatum(CENTER);

  for (int i = 3; i > 0; i--) 
  {
    tft.fillScreen(TFT_BLACK);
    tft.drawString(String(i), tft.width() / 2, tft.height() / 2);
    delay(800);
  }

  tft.fillScreen(TFT_BLACK);
  tft.drawString("GO!", tft.width() / 2, tft.height() / 2);
  delay(800);

  drawBackground();
  drawSnake();
  drawFood();
}

// Startup screen
void showCreditsScreen() 
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(3);
  tft.setTextDatum(MC_DATUM);

  String title = "Snake";
  int charSpacingTitle = 18;
  int titleXStart = (tft.width() / 2) - (title.length() * charSpacingTitle) / 2;

  for (int i = 0; i < title.length(); i++) {
    int x = titleXStart + i * charSpacingTitle;
    int y = tft.height() / 2 - 20;

    tft.setCursor(x, y);
    tft.print(title[i]);
    delay(80);
  }

  delay(400);

  tft.setTextSize(2);
  String author = "Designed by CroQuest";
  int charSpacingAuthor = 14;
  int authorXStart = (tft.width() / 2) - (author.length() * charSpacingAuthor) / 2;

  for (int i = 0; i < author.length(); i++) 
  {
    int x = authorXStart + i * charSpacingAuthor;
    int y = tft.height() / 2 + 30;

    tft.setCursor(x, y);
    tft.print(author[i]);
    delay(60);
  }
  delay(1500);
}

void waitForStartButton() 
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setTextDatum(CENTER);
  tft.drawString("Press 'placeholder' to Start", tft.width() / 2, tft.height() / 2);

  while (digitalRead(PIN_A) == HIGH) 
  {
    delay(10);
  }

  while (digitalRead(PIN_A) == LOW) 
  {
    delay(10);
  }
}

// Initializing game
void setup() 
{

  Serial.begin(115200);

  if (!SD.begin(5)) 
  {
    tft.setTextColor(TFT_RED);
    tft.drawString("SD Mount Failed", 10, 30);
  } 
  else 
  {
    Serial.println("SD Mounted.");
  }

  Serial.println("SD Card Files:");
  File root = SD.open("/");
  while (true) 
  {
    File file = root.openNextFile();
    if (!file) break;
    Serial.println(file.name());
    file.close();
  }

  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);

  pinMode(PIN_UP, INPUT_PULLUP);
  pinMode(PIN_DOWN, INPUT_PULLUP);
  pinMode(PIN_LEFT, INPUT_PULLUP);
  pinMode(PIN_RIGHT, INPUT_PULLUP);
  pinMode(PIN_A, INPUT_PULLUP);

  // Mount LittleFS and read high score
  // if (!LittleFS.begin()) 
  // {
  //   tft.drawString("FS Mount Failed", 10, 10);
  // } 
  // else 
  // {
  //   File file = LittleFS.open("/highscore.txt", "r");
  //   if (file) 
  //   {
  //     highScore = file.parseInt();
  //     file.close();
  //   }
  // }

  showCreditsScreen();
  //waitForStartButton();
  resetGame();
}

// Loop through and play game
void loop() 
{
  unsigned long now = millis();
  handleButtonInputs();

  if (!gameOver && (now - lastMoveTime >= MOVE_INTERVAL)) 
  {
    moveSnake();  
    lastMoveTime = now;
  }
}
