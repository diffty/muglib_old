#ifndef __MUG_IMG__
#define __MUG_IMG__


#include "common.h"


struct image_t {
  byte far* data;
  unsigned long far* mask;
  color_t far* palette;
  int w;
  int h;
  int paletteSize;
  int nbZone;
} typedef image_t;

struct mask_t {
	unsigned long far* mask;
	int nbZone;
} typedef mask_t;


image_t far* loadImage(char* fileName);
void drawImage(byte far* buffer, int x, int y, image_t far* img, byte reversed, byte masked);
void freeImage(image_t far* img);

#endif