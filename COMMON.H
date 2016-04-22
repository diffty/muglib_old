#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <alloc.h>
#include <dos.h>
#include <mem.h>
#include <math.h>
#include <time.h>
#include <fcntl.h>
#include <io.h>


#ifndef __MUG_CMN__
#define __MUG_CMN__


/*
----------------------------------------------------------------------
 Defines
----------------------------------------------------------------------
*/

#define VIDEO_INT           0x10      /* the BIOS video interrupt. */
#define WRITE_DOT           0x0C      /* BIOS func to plot a pixel. */
#define SET_MODE            0x00      /* BIOS func to set the video mode. */
#define VGA_256_COLOR_MODE  0x13      /* use to set 256-color mode. */
#define TEXT_MODE           0x03      /* use to set 80x25 text mode. */

#define SCREEN_WIDTH        320       /* width in pixels of mode 0x13 */
#define SCREEN_HEIGHT       200       /* height in pixels of mode 0x13 */
#define NUM_COLORS          256       /* number of colors in mode 0x13 */

#define FREAD_BUFFER_SIZE   1000


/*
----------------------------------------------------------------------
 Constants
----------------------------------------------------------------------
*/

// const char CHARS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";


/*
----------------------------------------------------------------------
 Types
----------------------------------------------------------------------
*/

typedef unsigned char  byte;
typedef unsigned short word;


/*
----------------------------------------------------------------------
 Structures
----------------------------------------------------------------------
*/

struct color_t {
  byte r;
  byte g;
  byte b;
} typedef color_t;

#endif