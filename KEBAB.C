#include "muglib.h"
#include "test2.h"

/*
----------------------------------------------------------------------
 Global vars
----------------------------------------------------------------------
*/

#define FALLSPEED 3


byte far *sBuffer = (byte far *) 0xA0000000L;        /* this points to video memory. */
byte far *dBuffer = NULL;

word far *my_clock = (word far *) 0x0000046C;    /* this points to the 18.2hz system clock. */




/*
----------------------------------------------------------------------
 Main
----------------------------------------------------------------------
*/

void drawProgressBar(int x, int y, int w, int h, int value, int color) {
  // drawLine(x, y, x+w, y, 14);
  // drawLine(x+w, y, x+w, y+h, 14);
  // drawLine(x+w, y+h, x, y+h, 14);
  // drawLine(x, y+h, x, y, 14);
  drawBox(x+1, y+1, (int) ((float) (w-1) / 100 * value), h-1, color);
  // drawBox(x+1, y+1, 10, h-1, 13);
}

/*
----------------------------------------------------------------------
 Main
----------------------------------------------------------------------
*/

int main() {
  // Init vars
  int x;
  int y;
  int timeByFrame = 1000 / 60;
  int kebabPosX;
  int kebabPosY;
  int frame = 0;
  int nbSprite = 0;
  int inputCooldown = 0;
  unsigned int nFrame = 0;

  int currentSec, lastSec, nbFrameSec, lastFrameSec;

  float deltaTime = 0.;
  double prevTime;
  double currentTime;

  word i, j, start;
  word beginTime = *my_clock;

  byte cont = 1;
  byte key;

  byte far *fBuffer = NULL;    // Frozen background buffer

  // Assets init
  sprite_t far **spriteList;
  sprite_t far *kebabFrontSpr;
  sprite_t far *kebabBackSpr;
  image_t far *kebabFront;
  image_t far *kebabBack;
  image_t far *tomato;
  image_t far *salad;
  image_t far *meat;
  image_t far *bg;
  spritesheet_t far *onion;
  font_t far *font;

  sprite_t far *test;

  struct time tt;

  // Gameplay vars init
  int ingredients[4];

  srand(*my_clock);                   /* seed the number generator. */

  // Loading assets
  kebabFront = loadImage("kebabfr.bmp");
  kebabBack = loadImage("kebabba.bmp");
  salad = loadImage("salad.bmp");
  tomato = loadImage("tomato.bmp");
  onion = loadSpriteSheet("onion.bmp", 2, 2, 4);
  meat = loadImage("meat.bmp");

  font = loadFont("font.bmp", 16, 16);

  kebabFrontSpr = newSpriteFromImage(kebabFront);
  kebabBackSpr = newSpriteFromImage(kebabBack);
  kebabBackSpr->frozen = 1;
  
  
  // Initing kebab position
  kebabPosX = SCREEN_WIDTH / 2 - kebabFront->w / 2;
  kebabPosY = SCREEN_HEIGHT / 2;

  // Double buffer init
  if ((dBuffer = (byte far *) farmalloc((unsigned int) SCREEN_WIDTH * SCREEN_HEIGHT)) == NULL) {
    printf("Not enough memory for back buffer allocation.");
    return 1;
  }

  // Frozen buffer init
  if ((fBuffer = (byte far *) farmalloc((unsigned int) SCREEN_WIDTH * SCREEN_HEIGHT)) == NULL) {
    printf("Not enough memory for back buffer allocation.");
    return 1;
  }

  // Clearing screen
  clearScreen();

  // Initing frozen buffer
  _fmemset(fBuffer, 0, SCREEN_WIDTH * SCREEN_HEIGHT);

  // Video init
  setVideoMode(VGA_256_COLOR_MODE);       /* set the video mode. */

  // Setting palette
  setPaletteFromImage(kebabBack);

  // Getting current time
  gettime(&tt);
  currentTime = getTimestamp();

  // drawImage(0, 0, bg, 0);

  if ((spriteList = (sprite_t far**) farmalloc(64)) == NULL) {
    printf("Not enough memory. Aborting\n");
    exit(1);
  }

  nFrame = 0;

  // Draw background on frozen layer
  drawSprite(fBuffer, kebabPosX, kebabPosY, kebabBackSpr, 1);

  lastSec = 0;
  currentSec = 0;
  nbFrameSec = 0;
  lastFrameSec = 0;

  for (i = 0; i < 4; i++)
    ingredients[i] = 0;

  // ---- GAME LOOP ----
  while(cont) {
    currentSec = (int) ((float) *my_clock) / CLOCKS_PER_SEC;

    start = *my_clock;                    /* record the starting time. */
    
    // clearScreen();

    key = getKey();


    // ------------------
    // ---- UPDATING ----
    // ------------------
    for (i=0; i<nbSprite; i++) {
      if (spriteList[i]->isMoving == 1) {
        if (spriteList[i]->moveToY == spriteList[i]->y) {
          spriteList[i]->isMoving = 0;
          spriteList[i]->frozen = 1;

          ingredients[spriteList[i]->assetId]++;

          drawSprite(fBuffer, spriteList[i]->x, spriteList[i]->y, spriteList[i], 1);
          freeSprite(spriteList[i]);

          for (j=i; j<nbSprite-1; j++) {
            spriteList[j] = spriteList[j+1];
          }

          nbSprite--;
          i--;
        }
        else {
          spriteList[i]->y += FALLSPEED;
        }
      }
    }

    if (key) {
      if (inputCooldown <= 0) {
        if (key == 18) {
          cont = 0;
        }
        else if (key == 44) {   // W
          // if ((spriteList = farrealloc(spriteList, (nbSprite+1))) == NULL) {
          //   printf("Not enough memory for extending the sprite list. Aborting\n");
          //   exit(1);
          // }

          spriteList[nbSprite] = newSpriteFromImage(tomato);
          spriteList[nbSprite]->x = 100 + (int) rand() % 60;
          spriteList[nbSprite]->y = -spriteList[nbSprite]->img->h;
          spriteList[nbSprite]->moveToY = 110;
          spriteList[nbSprite]->isMoving = 1;
          spriteList[nbSprite]->assetId = 0;

          nbSprite++;
        }
        else if (key == 45) {   // X
          spriteList[nbSprite] = newSpriteFromImage(salad);
          spriteList[nbSprite]->x = 100 + (int) rand() % 100;
          spriteList[nbSprite]->y = -spriteList[nbSprite]->img->h;
          spriteList[nbSprite]->moveToY = 130;
          spriteList[nbSprite]->isMoving = 1;
          spriteList[nbSprite]->assetId = 1;

          nbSprite++;
        }
        else if (key == 46) {   // C
          spriteList[nbSprite] = newSpriteFromSheet(onion);
          spriteList[nbSprite]->x = 100 + (int) rand() % 100;
          spriteList[nbSprite]->y = -spriteList[nbSprite]->sprSh->frameH;
          spriteList[nbSprite]->moveToY = 130;
          spriteList[nbSprite]->isMoving = 1;
          spriteList[nbSprite]->frame = nbSprite % 4;
          spriteList[nbSprite]->assetId = 2;

          nbSprite++;
        }
        else if (key == 47) {   // V
          spriteList[nbSprite] = newSpriteFromImage(meat);
          spriteList[nbSprite]->x = 100 + (int) rand() % 100;
          spriteList[nbSprite]->y = -spriteList[nbSprite]->img->h;
          spriteList[nbSprite]->moveToY = 130;
          spriteList[nbSprite]->isMoving = 1;
          spriteList[nbSprite]->assetId = 3;

          nbSprite++;
        }

        inputCooldown = 5;
      }
    }

    // -----------------
    // ---- DRAWING ----
    // -----------------

    // Draw frozen buffer as background
    _fmemcpy(dBuffer, fBuffer, SCREEN_WIDTH * SCREEN_HEIGHT);

    // Drawing sprites
    for (i=0; i<nbSprite; i++) {
      if (!spriteList[i]->frozen)
        drawSprite(dBuffer, spriteList[i]->x, spriteList[i]->y, spriteList[i], 1);
    }

    drawSprite(dBuffer, kebabPosX, kebabPosY, kebabFrontSpr, 1);

    // Drawing FPS
    drawInt(dBuffer, lastFrameSec, font, 0, 0);

    // Draw progression bars
    drawStr(dBuffer, "Tomato", font, kebabPosX - 50, kebabPosY);
    if (ingredients[0] >= 6)
      drawProgressBar(kebabPosX - 50, kebabPosY + 10, 48, 5, 100, 40);
    else if (ingredients[0] >= 5)
      drawProgressBar(kebabPosX - 50, kebabPosY + 10, 48, 5, 100, 17);
    else
      drawProgressBar(kebabPosX - 50, kebabPosY + 10, 48, 5, ingredients[0]*100/5, 14);

    drawStr(dBuffer, "Salad", font, kebabPosX - 50, kebabPosY + 25);
    if (ingredients[1] >= 10)
      drawProgressBar(kebabPosX - 50, kebabPosY + 35, 48, 5, 100, 40);
    else if (ingredients[1] >= 8)
      drawProgressBar(kebabPosX - 50, kebabPosY + 35, 48, 5, 100, 17);
    else
      drawProgressBar(kebabPosX - 50, kebabPosY + 35, 48, 5, ingredients[1]*100/10, 14);

    drawStr(dBuffer, "Onion", font, kebabPosX - 50, kebabPosY + 50);
    if (ingredients[2] >= 10)
      drawProgressBar(kebabPosX - 50, kebabPosY + 60, 48, 5, 100, 40);
    else if (ingredients[2] >= 8)
      drawProgressBar(kebabPosX - 50, kebabPosY + 60, 48, 5, 100, 17);
    else
      drawProgressBar(kebabPosX - 50, kebabPosY + 60, 48, 5, ingredients[2]*100/12, 14);

    drawStr(dBuffer, "Kebab", font, kebabPosX - 50, kebabPosY + 75);
    if (ingredients[3] >= 12)
      drawProgressBar(kebabPosX - 50, kebabPosY + 85, 48, 5, 100, 40);
    else if (ingredients[3] >= 10)
      drawProgressBar(kebabPosX - 50, kebabPosY + 85, 48, 5, 100, 17);
    else
      drawProgressBar(kebabPosX - 50, kebabPosY + 85, 48, 5, ingredients[3]*100/15, 14);

    // Copying back buffer to screen
    flipBuffer(dBuffer);

    /*prevTime = currentTime;
    currentTime = getTimestamp();

    deltaTime = currentTime - prevTime;*/
    
    // Delta time calculation
    // deltaTime = (((float) (*my_clock)) - start) / 18.2;  /* calculate how long it took. */

    nFrame += 1;

    if (inputCooldown > 0) {
      inputCooldown -= 1;
    }


    if (currentSec != lastSec) {
      lastSec = currentSec;
      lastFrameSec = nbFrameSec;
      nbFrameSec = 0;
    }

    nbFrameSec++;


    // printf("%d", key);
  }
  setVideoMode(TEXT_MODE);  /* set the video mode back to text mode. */

  if (nbSprite > 0)
    writeDebug(spriteList[nbSprite-1]->sprSh);

  printf("StartTime %f, EndTime %f\n, Diff %f, NFrame %d\n", (float) beginTime, (float) *my_clock, (float) (*my_clock - beginTime), nFrame);
  printf("Avg FPS : %f\n", (nFrame / (((float) (*my_clock - beginTime)) / 18.2)));

  printf("Freeing buffer.\n");
  farfree(dBuffer);
  dBuffer = NULL;

  printf("Freeing image.\n");
  freeImage(bg);
  freeImage(kebabFront);
  freeImage(kebabBack);
  freeImage(tomato);
  freeSpriteSheet(onion);
  freeImage(meat);
  bg = NULL;
  kebabFront = NULL;
  kebabBack = NULL;
  tomato = NULL;
  onion = NULL;
  meat = NULL;

  printf("Freeing sprite sheet.\n");

  return 0;
}