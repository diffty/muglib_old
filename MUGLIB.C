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

mask_t far* makeMaskFromBuffer(byte far* buffer, int w, int h) {
  int i, j, k;
  unsigned int nbPixels;
  mask_t far* newMask;

  nbPixels = w * h;

  if ((newMask = (mask_t far*) farmalloc(sizeof(mask_t))) == NULL) {
    printf("Not enough memory for new mask allocation.\n");
    exit(1);
  }

  if ((newMask->mask = (unsigned long far *) farmalloc(1000 * sizeof(unsigned long))) == NULL) {
    printf("Not enough memory for mask data allocation.\n");
    exit(1);
  }

  j = 0;
  k = 0;
  newMask->nbZone = 0;

  for (i = 0; i < nbPixels; i++) {
    if (buffer[i] != buffer[0]) {
      j++;
    }

    if (i < nbPixels - 1) {
      if (buffer[i] != buffer[i+1]) {
        if (buffer[i+1] == buffer[0] || i+1 == (nbPixels) - 1 || (i > 0 && i % w == 0)) {

          newMask->mask[k] = i-j+1;
          newMask->mask[k+1] = j;

          k += 2;
          j = 0;

          newMask->nbZone++;
        }
      }
    }
  }
  // newMask->mask = farrealloc(newMask->mask, 2 * (newMask->nbZone + 1) * sizeof(int));

  return newMask;
}

image_t far* makeImageFromBuffer(byte far* buffer, int w, int h) {
  int i, j, k;
  unsigned int nbPixels;
  image_t far* newImage;
  mask_t far* maskInfo;

  nbPixels = w * h;

  if ((newImage = (image_t far*) farmalloc(sizeof(image_t))) == NULL) {
    printf("Not enough memory for new image allocation.\n");
    exit(1);
  }

  newImage->data = (byte far *) farmalloc(nbPixels * sizeof(byte));
  _fmemcpy(newImage->data, buffer, nbPixels);

  maskInfo = makeMaskFromBuffer(newImage->data, w, h);

  newImage->w = w;
  newImage->h = h;
  newImage->mask = maskInfo->mask;
  newImage->nbZone = maskInfo->nbZone;

  farfree(maskInfo);

  return newImage;
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