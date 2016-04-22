#ifndef __MUG_IMG__
#define __MUG_IMG__


#include "common.h"


struct image_t {
  byte far* data;
  int far* mask;
  color_t far* palette;
  int w;
  int h;
  int paletteSize;
  int nbZone;
} typedef image_t;


image_t far* loadImage(char* fileName);
void drawImage(byte far* buffer, int x, int y, image_t far* img, byte masked);
void freeImage(image_t far* img);

#endif