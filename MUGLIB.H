/*
----------------------------------------------------------------------
 MUGLIB - Multi-Generation Lib
 29/03/2016 - Freddy CLEMENT (diffty@gmail.com)
 A cross-platform/cross-generation ultra-scalable (zbra)
 graphics library. 
----------------------------------------------------------------------
*/

#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <alloc.h>
#include <dos.h>
#include <mem.h>
#include <math.h>
#include <time.h>
#include <fcntl.h>
#include <io.h>


/*
----------------------------------------------------------------------
 Defines
----------------------------------------------------------------------
*/

#define VIDEO_INT           0x10      /* the BIOS video interrupt. */
#define WRITE_DOT           0x0C      /* BIOS func to plot a pixel. */
#define SET_MODE            0x00      /* BIOS func to set the video mode. */
#define VGA_256_COLOR_MODE  0x13      /* use to set 256-color mode. */
#define TEXT_MODE           0x03      /* use to set 80x25 text mode. */

#define SCREEN_WIDTH        320       /* width in pixels of mode 0x13 */
#define SCREEN_HEIGHT       200       /* height in pixels of mode 0x13 */
#define NUM_COLORS          256       /* number of colors in mode 0x13 */

#define FREAD_BUFFER_SIZE   1000



/*
----------------------------------------------------------------------
 Constants
----------------------------------------------------------------------
*/

// const char CHARS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";


/*
----------------------------------------------------------------------
 Types
----------------------------------------------------------------------
*/

typedef unsigned char  byte;
typedef unsigned short word;


/*
----------------------------------------------------------------------
 Structures
----------------------------------------------------------------------
*/

struct color_t {
  byte r;
  byte g;
  byte b;
} typedef color_t;


struct image_t {
  byte far* data;
  int far* mask;
  color_t far* palette;
  int w;
  int h;
  int paletteSize;
  int nbZone;
} typedef image_t;


struct spritesheet_t {
  image_t far* img;
  long far* mask;
  int frameH;
  int frameW;
  int nbFrame;
  int nbFrameX;
  int nbFrameY;
  int far* nbZone;
  long far* idxCumul;
} typedef spritesheet_t;


struct sprite_t {
  image_t far* img;
  spritesheet_t far* sprSh;
  int assetId;
  int x;
  int y;
  int frame;
  int moveToY;
  byte isMoving;
  byte type;
  int lastX;
  int lastY;
  int frozen;
} typedef sprite_t;


struct font_t {
  spritesheet_t far* sprSh;
  int nbCharsX;
  int nbCharsY;
 } typedef font_t;


/*
----------------------------------------------------------------------
 Prototypes
----------------------------------------------------------------------
*/

void setVideoMode(byte mode);
void setPaletteFromImage(image_t far* image);

void plotPixel(int x, int y, byte color);
void drawLine(int x1, int y1, int x2, int y2, int color);
void drawBox(int x, int y, int w, int h, int color);

void flipBuffer();
void clearScreen();

image_t far* loadImage(char* fileName);
spritesheet_t far* loadSpriteSheet(char* fileName, int nbFrameX, int nbFrameY, int nbFrame);
font_t far* loadFont(char* fileName, int nbCharsX, int nbCharsY);

sprite_t far* newSpriteFromImage(image_t far* img);
sprite_t far* newSpriteFromSheet(spritesheet_t far* sprSh);

void drawImage(byte far* buffer, int x, int y, image_t far* img, byte masked);
void drawSprite(byte far* buffer, int x, int y, sprite_t far* spr, byte masked);
void drawSpriteSheet(byte far* buffer, int x, int y, spritesheet_t far* sprSh, int frame, byte masked);
void drawChar(byte far* buffer, char ch, font_t far* font, int x, int y);

void freeImage(image_t far* img);
void freeSpriteSheet(spritesheet_t far* sprSh);
void freeFont(font_t far* font);

byte getKey();