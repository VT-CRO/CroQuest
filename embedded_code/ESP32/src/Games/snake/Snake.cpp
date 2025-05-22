#include "snake.hpp"

// Centering for the test
#define CENTER MC_DATUM

// Max size of the snake
// Easily adjustable
#define SNAKE_MAX_LENGTH 200

// Game Constants
const int TILE_SIZE = 16;
const int GRID_WIDTH = 30;
const int GRID_HEIGHT = 19;

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
  Direction direction;
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
  drawing.drawSdJpeg("/snake/assets/closed_right.jpg", snake[0].x * TILE_SIZE, snake[0].y * TILE_SIZE);
  drawing.addToCache("/snake/assets/closed_right.jpg");
  drawing.pushSprite(false, true, 0x1F);

  for (int i = 1; i < snakeLength - 1; i++) 
  {
    drawTile(snake[i].x, snake[i].y, tft.color565(77, 134, 214));
  }
  drawing.drawSdJpeg("/snake/assets/tail_right_light.jpg", snake[snakeLength - 1].x * TILE_SIZE, snake[snakeLength - 1].y * TILE_SIZE);
  drawing.addToCache("/snake/assets/tail_right_light.jpg");
  drawing.pushSprite();
}

// Drawing the food segments
void drawFood() {
  int xpos = food.x * TILE_SIZE;
  int ypos = food.y * TILE_SIZE;

  // Draw the JPEG sprite at the food location
  drawing.drawSdJpeg("/snake/assets/apple.jpg", xpos, ypos);
  drawing.addToCache("/snake/assets/apple.jpg");
  drawing.pushSprite(false, true, 0xFFFF);  // Push without using built-in transparency

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
    //Old tail position
    Point prev_tail = snake[snakeLength - 1];
    uint16_t bg = (prev_tail.x + prev_tail.y) & 1 ? tft.color565(111,189,99) : tft.color565( 66,176,50);
    for (int i = snakeLength - 1; i > 0; --i) snake[i] = snake[i - 1];
    
    //The new tail position
    Point new_tail = snake[snakeLength - 1];

    std::string tailPath;

    //The second to last segment right above the tail
    switch (snake[snakeLength - 2].direction) 
    {
        case UP: tailPath = "/snake/assets/tail_up"; break;
        case DOWN: tailPath = "/snake/assets/tail_down"; break;
        case LEFT: tailPath = "/snake/assets/tail_left"; break;
        case RIGHT: tailPath = "/snake/assets/tail_right"; break;
    }
    tailPath = (new_tail.x + new_tail.y) & 1 ? tailPath + "_light.jpg" : tailPath + "_dark.jpg";

    // Draw a new tail
    drawing.drawSdJpeg(tailPath.c_str(), new_tail.x * TILE_SIZE, new_tail.y * TILE_SIZE);
    drawing.addToCache(tailPath.c_str());
    drawing.pushSprite();

    // Remove the old tail
    drawTile(prev_tail.x, prev_tail.y, bg);
  }

  // Changing direction
  std::string headPath;
  bool foodAhead = false;
  const int sightRange = 3;
  switch (direction) 
  {
    case UP:
        for (int dy = 1; dy <= sightRange; dy++) {
            if (snake[0].y - dy == food.y && snake[0].x == food.x) {
                foodAhead = true;
                break;
            }
        }
        --snake[0].y;
        snake[0].direction = UP;
        headPath = foodAhead ? "/snake/assets/open_up.jpg" : "/snake/assets/closed_up.jpg";
        break;

    case DOWN:
        for (int dy = 1; dy <= sightRange; dy++) {
            if (snake[0].y + dy == food.y && snake[0].x == food.x) {
                foodAhead = true;
                break;
            }
        }
        ++snake[0].y;
        snake[0].direction = DOWN;
        headPath = foodAhead ? "/snake/assets/open_down.jpg" : "/snake/assets/closed_down.jpg";
        break;

    case LEFT:
        for (int dx = 1; dx <= sightRange; dx++) {
            if (snake[0].x - dx == food.x && snake[0].y == food.y) {
                foodAhead = true;
                break;
            }
        }
        --snake[0].x;
        snake[0].direction = LEFT;
        headPath = foodAhead ? "/snake/assets/open_left.jpg" : "/snake/assets/closed_left.jpg";
        break;

    case RIGHT:
        for (int dx = 1; dx <= sightRange; dx++) {
            if (snake[0].x + dx == food.x && snake[0].y == food.y) {
                foodAhead = true;
                break;
            }
        }
        ++snake[0].x;
        snake[0].direction = RIGHT;
        headPath = foodAhead ? "/snake/assets/open_right.jpg" : "/snake/assets/closed_right.jpg";
        break;
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

  //Drawing new head
  drawing.drawSdJpeg(headPath.c_str(), snake[0].x * TILE_SIZE, snake[0].y * TILE_SIZE);
  drawing.addToCache(headPath.c_str());
  drawing.pushSprite(false, true, 0x1F);

  //Draw Body part (previous location of the head)
  // drawTile(snake[1].x, snake[1].y, tft.color565(77, 134, 214));
  Direction fromDir = snake[1].direction;
  Direction toDir = snake[0].direction;
  std::string turnPath;
  if(fromDir != toDir){
    if ((fromDir == UP && toDir == RIGHT) || (fromDir == LEFT && toDir == DOWN))
      turnPath = "/snake/assets/turn_up_right_or_left_down";
    else if ((fromDir == UP && toDir == LEFT) || (fromDir == RIGHT && toDir == DOWN))
      turnPath = "/snake/assets/turn_up_left_or_right_down";
    else if ((fromDir == DOWN && toDir == RIGHT) || (fromDir == LEFT && toDir == UP))
      turnPath = "/snake/assets/turn_left_up_or_down_right";
    else if ((fromDir == DOWN && toDir == LEFT) || (fromDir == RIGHT && toDir == UP))
      turnPath = "/snake/assets/turn_right_up_or_down_left";
    
    turnPath = (snake[1].x + snake[1].y) & 1 ? turnPath + "_light.jpg" : turnPath + "_dark.jpg";

    drawing.drawSdJpeg(turnPath.c_str(), snake[1].x * TILE_SIZE, snake[1].y * TILE_SIZE);
    drawing.addToCache(turnPath.c_str());
    drawing.pushSprite();

  }else{
    drawTile(snake[1].x, snake[1].y, tft.color565(77, 134, 214));
  }

  // if food was eaten, respawn it
  if (ateFood)
  {
    spawnFood();
  }
}

// Reading the input
void handleButtonInputs() {

  // ----------- (UP / RIGHT) ----------
  if (right.isPressed()) {  // Tune this range for RIGHT
    if (lastDirectionOnTick != LEFT) direction = RIGHT;
  } 
  else if (up.isPressed()) {  // Tune this range for UP
    if (lastDirectionOnTick != DOWN) direction = UP;
  }

  // ----------- (DOWN / LEFT) ----------
  if (down.isPressed()) {  // Tune this range for DOWN
    if (lastDirectionOnTick != UP) direction = DOWN;
  } 
  else if (left.isPressed()) {  // Tune this range for LEFT
    if (lastDirectionOnTick != RIGHT) direction = LEFT;
  }

  // delay(20);
}

// Resetting the game after death
// Logic will be updated with buttons
void resetGame() 
{
  gameOver = false;
  snakeLength = 3;
  score = 0;
  direction = RIGHT;
  snake[0] = {5, 6, RIGHT};
  snake[1] = {4, 6, RIGHT};
  snake[2] = {3, 6, RIGHT};

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

  while (A.wasJustPressed()) 
  {
    delay(10);
  }

  while (A.wasJustPressed()) 
  {
    delay(10);
  }
}

// Initializing game
void runSnake() 
{
  Serial.begin(115200);
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);

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
  
  // Loop through and play game
  for(;;) 
  {
    unsigned long now = millis();
    handleButtonInputs();
  
    if (!gameOver && (now - lastMoveTime >= MOVE_INTERVAL)) 
    {
      moveSnake();  
      lastMoveTime = now;
    }
  }
}
