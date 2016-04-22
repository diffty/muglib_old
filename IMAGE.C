#include "image.h"


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

  printf("Loading image %s.\n", fileName);

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

  image->mask = (unsigned long far *) farmalloc(2 * sizeof(unsigned long));
  
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
          image->mask = farrealloc(image->mask, 2 * (image->nbZone + 1) * sizeof(unsigned long));

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

void drawImage(byte far* buffer, int x, int y, image_t far* img, byte reversed, byte masked) {
  int i, j, xb, yb;
  unsigned int imgBufIdx, zoneSize;

  if (masked) {
    for (i=0; i < img->nbZone; i++) {
      imgBufIdx = img->mask[i*2];
      zoneSize = img->mask[i*2+1];

      xb = imgBufIdx % img->w;
      
      if (reversed)
        yb = imgBufIdx / img->w;
      else
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
