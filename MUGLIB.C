#include "muglib.h"


extern byte far *sBuffer;        /* this points to video memory. */
extern byte far *dBuffer;
extern word far *my_clock;    /* this points to the 18.2hz system clock. */


void setVideoMode(byte mode) {
  union REGS regs;

  regs.h.ah = SET_MODE;
  regs.h.al = mode;
  int86(VIDEO_INT, &regs, &regs);
}

void plotPixel(int x, int y, byte color) {
  //dBuffer[(y<<8) + (y<<6) + x]=color;
  dBuffer[y*SCREEN_WIDTH+x]=color;      // Voir en conditions réelles quel est le plus rapide.
}

void drawLine(int x1, int y1, int x2, int y2, int color) {
  unsigned int i;
  int lineW = x2-x1;
  float lineSlope = (y2 - y1) / (x2 - x1);

  for (i=0; i < lineW; i++) {
    plotPixel(x1 + i, y1 + i*lineSlope, color);
  }
}

void drawBox(int x, int y, int w, int h, int color) {
  unsigned int i, j;

  for (i=0; i < h; i++) {
    _fmemset(dBuffer + x + (SCREEN_WIDTH * (y + i)),
             color,
             w);
  }
}

void flipBuffer() {
  _fmemcpy(sBuffer, dBuffer, SCREEN_WIDTH * SCREEN_HEIGHT);
}

void clearScreen() {
  _fmemset(dBuffer, 0, SCREEN_WIDTH * SCREEN_HEIGHT);
}

image_t* makeImageFromBuffer() {

}

void writeDebug(spritesheet_t far* sprSh) {
  int i, f;
  long imgBufIdx;
  int zoneSize;

  FILE* fp = fopen("debug.txt", "w");

  fprintf(fp, "%d\n", sprSh->nbFrame);
  fprintf(fp, "f, sprSh->idxCumul[f], imgBufIdx, zoneSize\n");

  for (f = 0; f < sprSh->nbFrame; f++) {
    fprintf(fp, "%d\n", sprSh->nbZone[f]);
    for (i = 0; i < sprSh->nbZone[f]; i++) {
      imgBufIdx = sprSh->mask[sprSh->idxCumul[f] + i*2];
      zoneSize = sprSh->mask[sprSh->idxCumul[f] + i*2+1];
      fprintf(fp, "%d, %ld, %ld, %d\n", f, sprSh->idxCumul[f], imgBufIdx, zoneSize);
    }
  }

  fclose(fp);
}

char* intToStr(int num) {
  unsigned int i, j;
  char tmp;
  char* newStr = (char *) malloc(16 * sizeof(char));

  for (i = 0; num > 0; i++) {
    newStr[i] = (num % 10) + 48;

    num /= 10;
  }

  newStr[i] = '\0';

  for (j = 0; j < i*0.5; j++) {
    tmp = newStr[i-1-j];
    newStr[i-1-j] = newStr[j];
    newStr[j] = tmp;
  }
  //newStr = realloc(newStr, (i+1) * sizeof(char));

  return newStr;
}

void setPaletteFromImage(image_t far* img) {
  int i;
  unsigned int palR, palG, palB;

  outp(0x03c8, 0);

  for (i = 0; i < img->paletteSize; i++) {
    palR = (unsigned int) ((float) img->palette[i].r / 255 * 63);
    palG = (unsigned int) ((float) img->palette[i].g / 255 * 63);
    palB = (unsigned int) ((float) img->palette[i].b / 255 * 63);

    outp(0x03c9, palR);
    outp(0x03c9, palG);
    outp(0x03c9, palB);
  }
}

byte getKey() {
  byte key = inportb(0x60); // Get the scan code
  return key;
}

double getTimestamp() {
  double currentTime;
  struct time t_struc; 
  gettime(&t_struc);
  currentTime = ((double) time(NULL) + ((double) t_struc.ti_hund)*0.01);
  return currentTime;
}