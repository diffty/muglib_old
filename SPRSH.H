#ifndef __MUG_SSH__
#define __MUG_SSH__


#include "common.h"
#include "image.h"


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


spritesheet_t far* loadSpriteSheet(char* fileName, int nbFrameX, int nbFrameY, int nbFrame);
void drawSpriteSheet(byte far* buffer, int x, int y, spritesheet_t far* sprSh, int frame, byte masked);
void freeSpriteSheet(spritesheet_t far* sprSh);

#endif