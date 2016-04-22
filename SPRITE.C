#include "sprite.h"

sprite_t far* newSpriteFromImage(image_t far* img) {
  sprite_t far* newSpr;
  
  if ((newSpr = (sprite_t far*) farmalloc(sizeof(sprite_t))) == NULL) {
    printf("Not enough memory for a new sprite. Aborting\n");
    exit(1);
  }

  newSpr->img = img;
  newSpr->sprSh = NULL;
  newSpr->isMoving = 0;
  newSpr->x = 0;
  newSpr->y = 0;
  newSpr->frame = 0;
  newSpr->type = 0;
  newSpr->frozen = 0;
  newSpr->assetId = -1;

  return newSpr;
}

sprite_t far* newSpriteFromSheet(spritesheet_t far* sprSh) {
  sprite_t far* newSpr;
  
  if ((newSpr = (sprite_t far*) farmalloc(sizeof(sprite_t))) == NULL) {
    printf("Not enough memory for a new sprite. Aborting\n");
    exit(1);
  }

  newSpr->img = NULL;
  newSpr->sprSh = sprSh;
  newSpr->isMoving = 0;
  newSpr->x = 0;
  newSpr->y = 0;
  newSpr->frame = 0;
  newSpr->type = 1;
  newSpr->frozen = 0;

  return newSpr;
}


void drawSprite(byte far* buffer, int x, int y, sprite_t far* spr, byte masked) {
  int i;

  if (spr->sprSh != NULL) {
    drawSpriteSheet(buffer, x, y, spr->sprSh, spr->frame, masked);
  }
  else {
    drawImage(buffer, x, y, spr->img, masked);
  }

  spr->lastX = x;
  spr->lastY = y;
}

void freeSprite(sprite_t far* sprite) {
  farfree(sprite);
  sprite = NULL;
}

