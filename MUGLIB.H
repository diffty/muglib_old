/*
----------------------------------------------------------------------
 MUGLIB - Multi-Generation Graphic Lib
 29/03/2016 - Freddy CLEMENT (diffty@gmail.com)
 A cross-platform/cross-generation ultra-scalable (zbra)
 graphics library. 
----------------------------------------------------------------------
*/


#ifndef __MUG_HEAD__
#define __MUG_HEAD__

#include "common.h"
#include "image.h"
#include "sprsh.h"
#include "sprite.h"
#include "fonts.h"

void setVideoMode(byte mode);
void setPaletteFromImage(image_t far* image);

void plotPixel(int x, int y, byte color);
void drawLine(int x1, int y1, int x2, int y2, int color);
void drawBox(int x, int y, int w, int h, int color);

void flipBuffer();
void clearScreen();

byte getKey();

#endif