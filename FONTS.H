#ifndef __MUG_FNT__
#define __MUG_FNT__


#include "common.h"
#include "sprsh.h"


struct font_t {
  spritesheet_t far* sprSh;
  int nbCharsX;
  int nbCharsY;
} typedef font_t;


font_t far* loadFont(char* fileName, int nbCharsX, int nbCharsY);
void drawChar(byte far* buffer, char ch, font_t far* font, int x, int y);
void drawStr(byte far* buffer, char* str, font_t far* font, int x, int y);
void drawInt(byte far* buffer, int num, font_t far* font, int x, int y);
void freeFont(font_t far* font);

#endif