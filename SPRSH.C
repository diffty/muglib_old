#include "sprsh.h"


spritesheet_t far* loadSpriteSheet(char* fileName, int nbFrameX, int nbFrameY, int nbFrame) {
  int i, j, k, f, frmMaskIdx;
  long offsetX, offsetY, offset;
  long xf, yf;
  long nbPixels;
  long far* tempMask;
  spritesheet_t far* newSprSh = (spritesheet_t far *) malloc(sizeof(spritesheet_t));

  printf("Loading sprite sheet %s.\n", fileName);
  
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

