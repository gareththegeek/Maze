#include <Gamer.h>

#define TEMPO 100

#define MAX_WIDTH 22
#define MAX_HEIGHT 20

#define UPDATE_INTERVAL 20
#define PLAYER_TICKS 8

#define OFFSET_X -2
#define OFFSET_Y -2

#define SCREEN_WIDTH 8
#define SCREEN_HEIGHT 8

#define MIN_X -2
#define MIN_Y -2

#define N 0x1
#define S 0x2
#define E 0x4
#define W 0x8
#define IN 0x10
#define EDGE 0x20

#define MAIN_MENU 0
#define LOADING 1
#define MAZE 2
#define LEVEL_COMPLETE 3
#define GAME_OVER 4

Gamer gamer;

struct Cell {
  int x;
  int y;
  Cell* next;
};

byte state = MAIN_MENU;

short viewX = OFFSET_X;
short viewY = OFFSET_Y;

Cell* wallList;
short wallCount = 0;

Cell* tempCell;

byte maze[MAX_WIDTH][MAX_HEIGHT];

bool moving = false;
byte movingDirection = 0;

byte playerTicks = PLAYER_TICKS;
bool showPlayer = true;

byte exitCoordX;
byte exitCoordY;

byte level = 1;
byte width = 4;
byte height = 4;
byte imageWidth;
byte imageHeight;

/* ---- Maze Generator ---- */

void add(byte x, byte y) {
  Cell* cell = new Cell();
  cell->x = x;
  cell->y = y;
  cell->next = wallList;
  wallList = cell;
  wallCount += 1;
}

void remove(Cell* cell) {
  wallCount -= 1;

  if (wallList == cell) {
    wallList = cell->next;
    delete cell;
    return;
  }

  Cell *previous = wallList;
  while (previous->next != cell) {
    previous = previous->next;
  }
  previous->next = cell->next;
  delete cell;
}

Cell* getAt(short index) {
  Cell* cell = wallList;
  for (short i = 0; i < index; i++) {
    cell = cell->next;
  }
  return cell;
}

void markEdge(byte x, byte y) {
  if (x >= 0 && x < width && y >= 0 && y < height && maze[x][y] == 0) {
    add(x, y);
    maze[x][y] |= EDGE;
  }
}

void markIn(byte x, byte y) {
  maze[x][y] |= IN;

  markEdge(x - 1, y);
  markEdge(x + 1, y);
  markEdge(x, y - 1);
  markEdge(x, y + 1);
}

byte randomDirection() {
  switch (random(4)) {
    case 0:
      return N;
    case 1:
      return S;
    case 2:
      return E;
    case 3:
      return W;
  }
}

byte opposite(byte dir) {
  switch (dir) {
    case N:
      return S;
    case S:
      return N;
    case W:
      return E;
    case E:
      return W;
  }
}

Cell* getNeighbour(Cell* cell, byte dir) {
  byte x = cell->x;
  byte y = cell->y;

  switch (dir) {
    case N:
      y -= 1;
      break;
    case S:
      y += 1;
      break;
    case E:
      x += 1;
      break;
    case W:
      x -= 1;
      break;
  }

  tempCell->x = x;
  tempCell->y = y;

  if (x >= 0 && x < width && y >= 0 && y < height) {
    return tempCell;
  } else {
    return NULL;
  }
}

void clearMaze()
{
  if (tempCell == NULL) {
    tempCell = new Cell();
  }

  for (byte y = 0; y < height; y++)
  {
    for (byte x = 0; x < width; x++)
    {
      maze[x][y] = 0;
    }
  }
}

void generateMaze() {

  exitCoordX = imageWidth - 2;
  exitCoordY = imageHeight - 1;

  clearMaze();
  markIn(2, 0);

  while (wallList != NULL) {

    short index = random(wallCount);
    Cell* wall = getAt(index);
    byte dir = randomDirection();

    Cell* neighbour = getNeighbour(wall, dir);

    if (neighbour != NULL && (maze[neighbour->x][neighbour->y] & IN) != 0) {
      maze[wall->x][wall->y] |= dir;
      maze[neighbour->x][neighbour->y] |= opposite(dir);

      markIn(wall->x, wall->y);
      remove(wall);
    }
  }

  maze[width - 1][height - 1] |= S;
}

/* ---- Input --- */

byte getCurrentCell() {
  byte x = (viewX - OFFSET_X) / 2;
  byte y = (viewY - OFFSET_Y) / 2;

  return maze[x][y];
}

void move(byte dir) {
  switch (dir) {
    case N:
      viewY -= 1;
      break;
    case S:
      viewY += 1;
      break;
    case E:
      viewX += 1;
      break;
    case W:
      viewX -= 1;
      break;
  }
}

void updateInput() {

  byte current = getCurrentCell();
  byte dir;

  if (gamer.isPressed(UP) || gamer.isHeld(UP)) {
    dir = N;
  } else if (gamer.isPressed(DOWN) || gamer.isHeld(DOWN)) {
    dir = S;
  } else if (gamer.isPressed(LEFT) || gamer.isHeld(LEFT)) {
    dir = W;
  } else if (gamer.isPressed(RIGHT) || gamer.isHeld(RIGHT)) {
    dir = E;
  }

  if ((current & dir) == 0) return;
  move(dir);

  moving = true;
  movingDirection = dir;
}

/* ---- Renderer ---- */

byte getWorldCoordPixel(short wx, short wy) {
  byte x = wx / 2;
  byte y = wy / 2;

  byte wxmod2 = wx % 2;
  byte wymod2 = wy % 2;

  if (wxmod2 == 0 && wymod2 == 0) {
    return 1;
  }
  if (wx == exitCoordX && wy == exitCoordY) {
    return 0;
  }
  if (wxmod2 == 1 && wymod2 == 0) {
    if ((maze[x][y] & N) == 0) {
      return 1;
    }
  }
  if (wxmod2 == 0 && wymod2 == 1) {
    if ((maze[x][y] & W) == 0) {
      return 1;
    }
  }

  return 0;
}

void renderMaze() {

  for (short j = 0; j < SCREEN_HEIGHT; j++) {
    for (short i = 0; i < SCREEN_WIDTH; i++) {
      short wx = i + viewX;
      short wy = j + viewY;

      if (wx >= 0 && wx < imageWidth && wy >= 0 && wy < imageHeight) {
        gamer.display[i][j] = getWorldCoordPixel(wx, wy);
      } else {
        gamer.display[i][j] = 0;
      }
    }
  }
}

void renderCursor() {
  gamer.display[3][3] = 1;
}

/* ---- Menu State ---- */

void newGame() {
  state = LOADING;
}

void menuState() {
  gamer.printString(F("Maze"));
  delay(100);
  if (gamer.isPressed(START)) {
    newGame();
    return;
  }
  gamer.printString(F("Press Start"));
  delay(100);
  if (gamer.isPressed(START)) {
    newGame();
    return;
  }
}

/* ---- Loading State ---- */

void loadingState() {

  gamer.printString(F("Level"));

  viewX = OFFSET_X;
  viewY = OFFSET_Y;
  imageWidth = width * 2 + 1;
  imageHeight = height * 2 + 1;

  generateMaze();

  gamer.showScore(level);
  delay(500);

  gamer.playTone(220);
  delay(TEMPO>>1);
  gamer.stopTone();
  delay(TEMPO>>1);

  gamer.playTone(260);
  delay(TEMPO>>1);
  gamer.stopTone();
  delay(TEMPO>>1);

  state = MAZE;
}

/* ---- Maze State ---- */

void mazeState() {
  playerTicks -= 1;

  if (playerTicks == 0) {
    playerTicks = PLAYER_TICKS;
    showPlayer = !showPlayer;

    if (moving) {
      move(movingDirection);
      moving = false;
      movingDirection = 0;
    }
  }

  if (!moving) {
    updateInput();
  }

  renderMaze();
  if (showPlayer) {
    renderCursor();
  }
  gamer.updateDisplay();

  delay(UPDATE_INTERVAL);
  Serial.println(imageHeight);
  if (viewY + 3 >= imageHeight) {
    state = LEVEL_COMPLETE;
  }
}

void randomise() {
  int a = analogRead(A5);
  randomSeed(a);
}

void debugWait() {
  while (!Serial.available()) {}
  while (Serial.available()) {
    Serial.read();
  }
}

/* ---- Level Complete State ---- */

void levelCompleteState() {

  gamer.playTone(220);
  delay(TEMPO);
  gamer.stopTone();
  delay(TEMPO);

  gamer.playTone(220);
  delay(TEMPO >> 1);
  gamer.stopTone();
  delay(TEMPO >> 1);

  gamer.playTone(220);
  delay(TEMPO >> 1);
  gamer.stopTone();
  delay(TEMPO >> 1);

  gamer.playTone(260);
  delay(TEMPO);
  gamer.stopTone();
  delay(TEMPO >> 1);

  gamer.playTone(220);
  delay(TEMPO);
  gamer.stopTone();
  delay(TEMPO);

  gamer.playTone(260);
  delay(TEMPO);
  gamer.stopTone();
  delay(TEMPO);

  level++;
  if (width < MAX_WIDTH) {
    width++;
  }
  if (height < MAX_HEIGHT) {
    height++;
  }

  state = LOADING;

  if (level % 10 == 0) {
    for (int i = 0; i < 5; i++) {
      gamer.printString("Eye Break");
      delay(1000);
    }
  }
}

void setup() {

  Serial.begin(9600);

  gamer.begin();
  randomise();
}

void loop() {

  switch (state) {
    case MAIN_MENU:
      menuState();
      break;
    case LOADING:
      loadingState();
      break;
    case MAZE:
      mazeState();
      break;
    case LEVEL_COMPLETE:
      levelCompleteState();
      break;
    case GAME_OVER:
      break;
  }
}

