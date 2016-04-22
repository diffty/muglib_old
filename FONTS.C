#include "fonts.h"


font_t far* loadFont(char* fileName, int nbCharsX, int nbCharsY) {
  font_t far* newFont;
  spritesheet_t far* fontSprSh;

  printf("Loading font %s.\n", fileName);

  if ((newFont = (font_t far*) farmalloc(sizeof(font_t))) == NULL) {
    printf("Not enough memory for new font allocation.\n");
    exit(1);
  }

  fontSprSh = loadSpriteSheet(fileName, nbCharsX, nbCharsY, nbCharsX * nbCharsY);
  
  newFont->sprSh = fontSprSh;
  newFont->nbCharsX = nbCharsX;
  newFont->nbCharsY = nbCharsY;

  return newFont;
}

void drawChar(byte far* buffer, char ch, font_t far* font, int x, int y) {
  drawSpriteSheet(buffer, x, y, font->sprSh, ch-32, 1);
}

void drawStr(byte far* buffer, char* str, font_t far* font, int x, int y) {
  int i = 0;

  while (str[i] != '\0') {
    drawChar(buffer, str[i], font, x + (i*8), y);
    i++;
  }
}

void drawInt(byte far* buffer, int num, font_t far* font, int x, int y) {
  char* intStr = intToStr(num);
  drawStr(buffer, intStr, font, 20, 20);
  free(intStr);
}

void freeFont(font_t far* font) {
  freeSpriteSheet(font->sprSh);
  farfree(font);
  font = NULL;
}

