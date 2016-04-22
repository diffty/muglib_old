#ifndef __MUG_SPR__
#define __MUG_SPR__


#include "common.h"
#include "image.h"
#include "sprsh.h"


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


sprite_t far* newSpriteFromImage(image_t far* img);
sprite_t far* newSpriteFromSheet(spritesheet_t far* sprSh);

void drawSprite(byte far* buffer, int x, int y, sprite_t far* spr, byte masked);
void freeSprite(sprite_t far* sprite);

#endif