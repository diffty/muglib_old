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

image_t far* loadImage(char* fileName) {
  FILE* fp;
  int fh;
  int nBytes, nBytesToRead;
  int i, j, k;

  int* imgW = (int *) malloc(4);
  int* imgH = (int *) malloc(4);
  int* imgPalSize = (int *) malloc(4);

  unsigned long* dataSize = (unsigned long *) malloc(4);
  int* startOffset = (int *) malloc(4);
  int* headerSize  = (int *) malloc(4);
  unsigned long nbPixels;

  image_t far* image;
  byte* fileBuf = (byte *) malloc(FREAD_BUFFER_SIZE);
  long seekPtr;

  byte* paletteData;


  if ((image = (image_t far*) farmalloc(sizeof(image_t))) == NULL) {
    printf("Not enough memory for new image allocation.\n");
    exit(1);
  }

  if ((fp = fopen(fileName, "rb")) == NULL) {
    printf("Can't read image file %s. Aborting.\n", fileName);
    exit(1);
  }

  // Reading header
  fseek(fp, 0x0002, SEEK_SET);
  fread(dataSize, 4, 1, fp);
  fseek(fp, 0x000A, SEEK_SET);
  fread(startOffset, 4, 1, fp);
  fseek(fp, 0x000E, SEEK_SET);
  fread(headerSize, 4, 1, fp);
  fseek(fp, 0x0012, SEEK_SET);
  fread(imgW, 4, 1, fp);
  fseek(fp, 0x0016, SEEK_SET);
  fread(imgH, 4, 1, fp);
  fseek(fp, 0x002E, SEEK_SET);
  fread(imgPalSize, 4, 1, fp);

  image->w = *imgW;
  image->h = *imgH;
  image->paletteSize = *imgPalSize;

  if (image->paletteSize == 0) image->paletteSize = 256;

  printf("Size : %ld\n", *dataSize);
  printf("Start offset : %d\n", *startOffset);
  printf("Header size : %d\n", *headerSize);
  printf("Image width : %d\n", image->w);
  printf("Image height : %d\n", image->h);
  printf("Palette size : %d\n", image->paletteSize);

  nbPixels = (long) image->h * (long) image->w;

  // Reading palette
  paletteData = (byte *) malloc(image->paletteSize*4);

  fseek(fp, (*headerSize)+14, SEEK_SET);
  fread(paletteData, 1, image->paletteSize*4, fp);

  image->palette = (color_t far *) farmalloc(sizeof(color_t) * image->paletteSize);

  for (i = 0; i < image->paletteSize; i++) {
    image->palette[i].b = paletteData[i*4];
    image->palette[i].g = paletteData[i*4+1];
    image->palette[i].r = paletteData[i*4+2];
  }

  // Reading pixel array
  image->data = (byte far *) farmalloc(nbPixels);

  seekPtr = *startOffset;

  while (seekPtr < (nbPixels + (*startOffset))) {
    nBytesToRead = min((nbPixels - (seekPtr - (*startOffset))), FREAD_BUFFER_SIZE);
    fseek(fp, seekPtr, SEEK_SET);
    fread(fileBuf, 1, nBytesToRead, fp);
    _fmemcpy(image->data + (seekPtr - *startOffset), fileBuf, nBytesToRead);
    seekPtr += nBytesToRead;
  }

  fclose(fp);

  image->mask = (int far *) farmalloc(2 * sizeof(int));
  
  // Building transparency info
  j = 0;
  k = 0;
  image->nbZone = 0;

  for (i = 0; i < nbPixels; i++) {
    if (image->data[i] != image->data[0]) {
      j++;
    }

    if (i < (nbPixels) - 1) {
      if (image->data[i] != image->data[i+1]) {
        if (image->data[i+1] == image->data[0] || i+1 == (nbPixels) - 1 || (i > 0 && i%image->w == 0)) {
          image->mask = farrealloc(image->mask, 2 * (image->nbZone + 1) * sizeof(int));

          image->mask[k] = i-j+1;
          image->mask[k+1] = j;

          k += 2;
          j = 0;

          image->nbZone++;
        }
      }
    }
  }

  free(fp);
  free(dataSize);
  free(startOffset);
  free(headerSize);
  free(fileBuf);
  free(paletteData);
  free(imgW);
  free(imgH);
  free(imgPalSize);

  return image;
}

spritesheet_t far* loadSpriteSheet(char* fileName, int nbFrameX, int nbFrameY, int nbFrame) {
  int i, j, k, f, frmMaskIdx;
  long offsetX, offsetY, offset;
  long xf, yf;
  long nbPixels;
  long far* tempMask;
  spritesheet_t far* newSprSh = (spritesheet_t far *) malloc(sizeof(spritesheet_t));

  newSprSh->img = loadImage(fileName);
  newSprSh->nbFrameX = nbFrameX;
  newSprSh->nbFrameY = nbFrameY;
  newSprSh->nbFrame = nbFrame;
  newSprSh->frameW = newSprSh->img->w / nbFrameX;
  newSprSh->frameH = newSprSh->img->h / nbFrameY;

  nbPixels = (long) newSprSh->frameH * (long) newSprSh->frameW;

  newSprSh->nbZone   = (int far *) farmalloc(nbFrame * sizeof(int));
  newSprSh->mask     = (long far *) farmalloc(10000 * sizeof(long));
  newSprSh->idxCumul = (long far *) farmalloc(nbFrame * sizeof(long));
  
  k = 0;

  for (f=0; f < nbFrame; f++) {
    newSprSh->idxCumul[f] = 0;
    newSprSh->nbZone[f] = 0;
  }

  for (f=0; f < nbFrame; f++) {
    j = 0;

    xf = ((f % newSprSh->nbFrameX) * newSprSh->frameW);
    yf = ((newSprSh->nbFrameY - (f / newSprSh->nbFrameX) - 1) * newSprSh->frameH * newSprSh->img->w);

    for (i=0; i < newSprSh->frameH * newSprSh->frameW; i++) {
      // offsetX = ((f % (long) newSprSh->nbFrameX) * (long) newSprSh->frameW) + (i % (long) newSprSh->frameW);
      // offsetY = ((nbFrameY - (f / (long) newSprSh->nbFrameX) - 1) * ((long) newSprSh->img->w * (long) newSprSh->frameH)) + (i / (long) newSprSh->frameW) * (long) newSprSh->img->w;
      // offset = offsetX + offsetY;

      offsetX = xf + (i % (long) newSprSh->frameW);
      offsetY = yf + (i / (long) newSprSh->frameW) * (long) newSprSh->img->w;
      offset = offsetX + offsetY;

      if (newSprSh->img->data[offset] != newSprSh->img->data[0]) {
        j++;
      }

      if (i < nbPixels - 1) {
        if (newSprSh->img->data[offset] != newSprSh->img->data[offset+1]) {
          if (newSprSh->img->data[offset+1] == newSprSh->img->data[0] || i+1 == nbPixels || ((i+1) % newSprSh->frameW == 0)) {
            newSprSh->mask[k] = offset-j+1;
            newSprSh->mask[k+1] = j;

            k += 2;
            j = 0;

            newSprSh->nbZone[f]++;
          }
        }
      }
    }

    if (f < nbFrame - 1) {
      newSprSh->idxCumul[f+1] = k;
    }
  }

  return newSprSh;
}

font_t far* loadFont(char* fileName, int nbCharsX, int nbCharsY) {
  font_t far* newFont = (font_t far*) malloc(sizeof(font_t));
  spritesheet_t far* fontSprSh = loadSpriteSheet(fileName, nbCharsX, nbCharsY, nbCharsX * nbCharsY);

  newFont->sprSh = fontSprSh;
  newFont->nbCharsX = nbCharsX;
  newFont->nbCharsY = nbCharsY;

  return newFont;
}

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

void drawImage(byte far* buffer, int x, int y, image_t far* img, byte masked) {
  int i, j, xb, yb;
  int imgBufIdx, zoneSize;

  if (masked) {
    for (i=0; i<img->nbZone; i++) {
      imgBufIdx = img->mask[i*2];
      zoneSize = img->mask[i*2+1];

      xb = imgBufIdx % img->w;
      yb = img->h - (imgBufIdx / img->w) - 1;

      // Clipping
      if (x + xb + zoneSize < 0 || x + xb >= SCREEN_WIDTH || y + yb < 0 || y + yb >= SCREEN_HEIGHT) {
        continue;
      }
      else if (x + xb + zoneSize > SCREEN_WIDTH) {
        zoneSize = zoneSize - ((x + xb + zoneSize) % SCREEN_WIDTH);
      }
      else if (x + xb < 0) {
        imgBufIdx += -(x + xb);
        zoneSize = zoneSize + (x + xb);
        xb = imgBufIdx % img->w;
      }

      // ...Then current line copy
      _fmemcpy(buffer + (x + xb) + ((y * SCREEN_WIDTH) + (yb * SCREEN_WIDTH)),
               img->data + imgBufIdx,
               zoneSize);
    }
  }
  else {
    for (i=0; i<img->h; i++) {
      _fmemcpy(buffer + x + (y * SCREEN_WIDTH) + (i * SCREEN_WIDTH),
               img->data + ((img->h - i - 1) * img->w),
               img->w);
    }
  }
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

void drawSpriteSheet(byte far* buffer, int x, int y, spritesheet_t far* sprSh, int frame, byte masked) {
  int i, j, xb, yb, xf, yf;
  long imgBufIdx;
  int zoneSize;

  // printf("%ld, %ld\n", sprSh->mask[sprSh->idxCumul[frame] + 0], sprSh->mask[sprSh->idxCumul[frame] + 1]);
  // printf("%ld, %ld\n", sprSh->mask[sprSh->idxCumul[frame] + 2], sprSh->mask[sprSh->idxCumul[frame] + 3]);

  if (masked) {
    for (i=0; i<sprSh->nbZone[frame]; i++) {
      imgBufIdx = sprSh->mask[sprSh->idxCumul[frame] + i*2];
      zoneSize = sprSh->mask[sprSh->idxCumul[frame] + i*2+1];

      xb = imgBufIdx % sprSh->frameW;
      yb = sprSh->frameH - ((imgBufIdx / sprSh->img->w) % sprSh->frameH);

      // Clipping (-> faire fonction?)
      if (x + xb + zoneSize < 0 || x + xb >= SCREEN_WIDTH || y + yb < 0 || y + yb >= SCREEN_HEIGHT) {
        continue;
      }
      else if (x + xb + zoneSize > SCREEN_WIDTH) {
        zoneSize = zoneSize - ((x + xb + zoneSize) % SCREEN_WIDTH);
      }
      else if (x + xb < 0) {
        imgBufIdx += -(x + xb);
        zoneSize = zoneSize + (x + xb);
        xb = imgBufIdx % sprSh->img->w;
      }

      _fmemcpy(buffer + x + xb + (y * SCREEN_WIDTH) + (yb * SCREEN_WIDTH),
               sprSh->img->data + imgBufIdx,
               zoneSize);
    }
  }
  else {
    for (i=0; i<sprSh->frameH; i++) {
      xf = ((frame % sprSh->nbFrameX) * sprSh->frameW);
      yf = ((sprSh->nbFrameY - (frame / sprSh->nbFrameX) - 1) * sprSh->frameH * sprSh->img->w);

      _fmemcpy(buffer + x + ((y+i) * SCREEN_WIDTH),
               sprSh->img->data + xf + (sprSh->frameH - 1 - i) * sprSh->img->w + yf, //sprSh->img->data + xf + ((sprSh->frameH - 1 - i) * sprSh->img->w) + yf,
               sprSh->frameW);
    }
  }
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

void drawSprite(byte far* buffer, int x, int y, sprite_t far* spr, byte masked) {
  int i;

  /*if (spr->lastX != NULL && spr->lastY != NULL) {
    for(i=0; i<img->h; i++) {
      _fmemcpy(dBuffer + spr->lastX + (spr->lastY * SCREEN_WIDTH) + (i * SCREEN_WIDTH),
               spr->backBuf + ((img->h - i - 1) * img->w),
               img->w);
    }
  }*/

  /*for (i=0; i<img->h; i++) {
    _fmemcpy(spr->backBuf + (i * img->w),
             dBuffer + x + (y * SCREEN_WIDTH) + (img->h - i - 1) * SCREEN_WIDTH,
             img->w);
  }*/

  if (spr->sprSh != NULL) {
    drawSpriteSheet(buffer, x, y, spr->sprSh, spr->frame, masked);
  }
  else {
    drawImage(buffer, x, y, spr->img, masked);
  }

  spr->lastX = x;
  spr->lastY = y;
}

void freezeSprite(byte far* fBuffer, sprite_t* spr) {
  /*_fmemcpy(dBuffer + x + ((y+i) * SCREEN_WIDTH),
           sprSh->img->data + xf + (i * sprSh->img->w) + yf,
           sprSh->frameW);*/
}

void freeImage(image_t far* img) {
  int i;

  farfree(img->data);
  farfree(img->mask);
  farfree(img->palette);
  farfree(img);

  img->data = NULL;
  img->mask = NULL;
  img->palette = NULL;
  img = NULL;
}

void freeSpriteSheet(spritesheet_t far* sprSh) {
  int i, j, f;

  freeImage(sprSh->img);
  sprSh->img = NULL;

  /*for (f=0; f<sprSh->nbFrame; f++) {
    farfree(sprSh->mask[f]);
    sprSh->mask[f] = NULL;
  }*/
  farfree(sprSh->mask);
  farfree(sprSh->nbZone);
  farfree(sprSh);

  sprSh->mask = NULL;
  sprSh->nbZone = NULL;
  sprSh = NULL;
}

void freeSprite(sprite_t far* sprite) {
  farfree(sprite);
  sprite = NULL;
}

void freeFont(font_t far* font) {
  freeSpriteSheet(font->sprSh);
  farfree(font);
  font = NULL;
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