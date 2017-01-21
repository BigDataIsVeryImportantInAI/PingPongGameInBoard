/********************************************************************************/
/* Copyright (c) 2004, 2005, Lee,Chang-woo HL2IRW <hl2irw@armstudy.com>         */
/* graaphics.c                                                                  */
/* 2005-10-08                                                                   */
/* Lee Chang-woo HL2IRW (hl2irw@armstudy.com)                                   */
/********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "fundef.h"

/* Line modes */
#define LINE_SOLID      	0
#define LINE_ONOFF_DASH 	1

typedef unsigned short IMAGEBITS;

/* MWIMAGEBITS macros*/
#define IMAGE_WORDS(x)		(((x)+15)/16)
#define IMAGE_BYTES(x)		(IMAGE_WORDS(x)*sizeof(IMAGEBITS))
/* size of image in words*/
#define IMAGE_SIZE(width, height)     \
        			((height) * (((width) + IMAGE_BITSPERIMAGE - 1) / IMAGE_BITSPERIMAGE))
#define IMAGE_BITSPERIMAGE	(sizeof(IMAGEBITS) * 8)
#define IMAGE_BITVALUE(n)	((IMAGEBITS) (((IMAGEBITS) 1) << (n)))
#define IMAGE_FIRSTBIT		(IMAGE_BITVALUE(IMAGE_BITSPERIMAGE - 1))
#define IMAGE_NEXTBIT(m)	((IMAGEBITS) ((m) >> 1))
#define IMAGE_TESTBIT(m)	((m) & IMAGE_FIRSTBIT)  /* use with shiftbit*/
#define IMAGE_SHIFTBIT(m)	((IMAGEBITS) ((m) << 1))  /* for testbit*/
	
#define SPITCH ((gr_stipple.width + (IMAGE_BITSPERIMAGE - 1)) / IMAGE_BITSPERIMAGE)
#define BIT_SET(data, w, h) \
 	(data[(h * SPITCH) + (w / IMAGE_BITSPERIMAGE)] & (1 << ((IMAGE_BITSPERIMAGE - 1) - (w % IMAGE_BITSPERIMAGE))))
	
/* FUTURE: MWLINE_DOUBLE_DASH */

/* Fill mode  */
#define FILL_SOLID		0  
#define FILL_STIPPLE		1  
#define FILL_OPAQUE_STIPPLE 	2  
#define FILL_TILE           	3

#define MODE_COPY             	0       /* src*/
#define MODE_XOR              	1       /* src ^ dst*/
#define MODE_OR               	2       /* src | dst*/
#define MODE_AND              	3       /* src & dst*/
#define MODE_CLEAR            	4       /* 0*/
	
#define MODE_TO_ROP(op)       	(((long)(op)) << 24)
	
//#define ROP_COPY              	MODE_TO_ROP(MODE_COPY)
//#define ROP_XOR               	MODE_TO_ROP(MODE_XOR)
#define ROP_OR                	MODE_TO_ROP(MODE_OR)
#define ROP_AND               	MODE_TO_ROP(MODE_AND)
#define ROP_CLEAR             	MODE_TO_ROP(MODE_CLEAR)
	
#define ROP_SRCCOPY           	ROP_COPY      /* obsolete*/
#define ROP_SRCAND            	ROP_AND       /* obsolete*/
#define ROP_SRCINVERT         	ROP_XOR       /* obsolete*/
#define ROP_BLACKNESS         	ROP_CLEAR     /* obsolete*/
	
typedef struct {
      int width;
      int height;
      IMAGEBITS *bitmap;
} STIPPLE;


typedef struct {
      int x;
      int y;
} POINT;


typedef struct {
      int width;
      int height;
} TILE;

	
extern int maxx,maxy,fg_color,bg_color;
extern int clipminx, clipminy, clipmaxx, clipmaxy;
int gr_foreground,gr_background,gr_usebg;
STIPPLE gr_stipple;            /* The current stipple as set by the GC */
TILE gr_tile;                  /* The current tile as set by the GC */
POINT gr_ts_offset;            /* The x and y offset of the tile / stipple */

static int ts_origin_x = 0;
static int ts_origin_y = 0;

unsigned long gr_dashcount,gr_dashmask,gr_fillmode;

static void tile_drawrect (int x, int y, int w, int h);


void ttf_color (int color)
{
      gr_foreground = color;
}


void init_graph (void)
{
      clipminx = 0;
      clipminy = 0; 
      clipmaxx = maxx - 1; 
      clipmaxy = maxy - 1;
      gr_foreground = fg_color;
      gr_background = bg_color;      
      gr_dashcount = 0;
      gr_dashmask = 0;
      gr_fillmode = FILL_SOLID;
}	


void set_ts_origin (int x, int y)
{
      ts_origin_x = x + gr_ts_offset.x;
      ts_origin_y = y + gr_ts_offset.y;
}


void drawpoint (int x, int y)
{
      SetPixel(x, y, gr_foreground);
}


void ts_drawpoint (int x, int y)
{
      int bx = x - ts_origin_x;
      int by = y - ts_origin_y;
      if (gr_fillmode == FILL_STIPPLE || gr_fillmode == FILL_OPAQUE_STIPPLE) {
         if (!gr_stipple.bitmap || !gr_stipple.width || !gr_stipple.height) return;
      } else {
         if (!gr_tile.width || !gr_tile.height) return;
      }
      if (bx < 0 || by < 0) {
         SetPixel(x, y, gr_foreground);
         return;
      }
      switch (gr_fillmode) {
	 case FILL_OPAQUE_STIPPLE:
           if (!BIT_SET(gr_stipple.bitmap, (bx % gr_stipple.width), (by % gr_stipple.height))) SetPixel(x, y, gr_background);
         case FILL_STIPPLE:
	   if (BIT_SET(gr_stipple.bitmap, (bx % gr_stipple.width), (by % gr_stipple.height))) SetPixel(x, y, gr_foreground);
	   break;
         case FILL_TILE:
           SetPixel(x, y, gr_foreground);
	   break;
      }
}	


void ts_drawrow (int x1, int x2, int y)
{
      int x;
      int dstwidth = x2 - x1 + 1;
      switch (gr_fillmode) {
         case FILL_STIPPLE:
	 case FILL_OPAQUE_STIPPLE:
           for (x = x1; x <= x2; x++) ts_drawpoint(x, y);
           break;
         case FILL_TILE:
	   tile_drawrect(x1, y, dstwidth, 1);
      }
}


static void tile_drawrect (int x, int y, int w, int h)
{
      int sx = x, sy = y;
      int px = 0, py = 0;
      int dw = w;
      int dh = h;      
      /* This is where the tile starts */
      int tilex = x - ts_origin_x;
      int tiley = y - ts_origin_y;
      /* Sanity check */
      if (!gr_tile.width || !gr_tile.height) return;
      /* Adjust the starting point to correspond with the tile offset */
      if (tilex < 0) {
         sx += -tilex;
	 dw -= -tilex;		
	 if (sx > (x + w - 1)) return;
	 tilex = sx - ts_origin_x;
      }
      if (tiley < 0) {
         sy += -tiley;
	 dh -= -tiley;	 
	 if (sy > (y + h - 1)) return;
	 tiley = sy - ts_origin_y;
      }
      while (dh) {
            int ch = (gr_tile.height - ((tiley + py) % gr_tile.height));
            if (ch > dh) ch = dh;
	    dw = w;
	    px = 0;
	    while (dw) {
		  int cw = (gr_tile.width - ((tilex + px) % gr_tile.width));
		  if (cw > dw) cw = dw;
		  dw -= cw;
		  px += cw;
	    }
	    dh -= ch;
	    py += ch;
        }
}
									

void ts_fillrect (int x, int y, int w, int h)
{
      int x1 = x;
      int x2 = x + w - 1;
      int y1 = y;
      int y2 = y + h - 1;
      for (; y1 <= y2; y1++) ts_drawrow(x1, x2, y1);
}


void DrawHorzLine (int x1, int x2, int y, int color)
{
      int idx;
      for (idx=x1;idx<(x2+1);idx++) SetPixel(idx, y, color);
}


void DrawVertLine (int x, int y1, int y2, int color)
{
      int idx;
      for (idx=y1;idx<(y2+1);idx++) SetPixel(x, idx, color);
}	


void GdSetDash (unsigned long *mask, unsigned long *count)
{
      int oldm = gr_dashmask;
      int oldc = gr_dashcount;
      if (!mask || !count) return;
      gr_dashmask = *mask;
      gr_dashcount = *count;
      *mask = oldm;
      *count = oldc;
}


static void drawrow (int x1, int x2, int y)
{
      int temp;
      /* reverse endpoints if necessary */
      if (x1 > x2) {
         temp = x1;
	 x1 = x2;
	 x2 = temp;
      }
      /* clip to physical device */
      if (x1 < 0) x1 = 0;
      if (x2 >= maxx) x2 = maxx - 1;
      /* If aren't trying to draw a dash, then head for the speed */
      if (!gr_dashcount) {
	 while (x1 <= x2) {
               temp = MIN(clipmaxx, x2);
	       DrawHorzLine(x1, temp, y, gr_foreground);
	       x1 = temp + 1;
	 }
      } else {
	 unsigned int p, bit = 0;
	 /* We want to draw a dashed line instead */
	 for (p=x1;p<=x2;p++) {
	     if (gr_dashmask & (1 << bit)) SetPixel(p, y, gr_foreground);
	     bit = (bit + 1) % gr_dashcount;
	 }
      }
}


static void drawcol (int x,int y1,int y2)
{
      int temp;
      /* reverse endpoints if necessary */
      if (y1 > y2) {
         temp = y1;
	 y1 = y2;
	 y2 = temp;
      }
      /* clip to physical device */
      if (y1 < 0) y1 = 0;
      if (y2 >= maxy) y2 = maxy - 1;
      /* check cursor intersect once for whole line */     
      if (!gr_dashcount) {
	 while (y1 <= y2) {	       
	       temp = MIN(clipmaxy, y2);
	       DrawVertLine(x, y1, temp, gr_foreground);	     
	       y1 = temp + 1;
	 }
      } else {
	 unsigned int p, bit = 0;
	 /* We want to draw a dashed line instead */	 	
	 for (p=y1;p<=y2;p++) {										     
	     if (gr_dashmask & (1<<bit)) SetPixel(x, p, gr_foreground);
	     bit = (bit + 1) % gr_dashcount;
	 }
      }
}


void FillRect (int x1, int y1, int x2, int y2, int color)
{
      int idx;
      if (x1 > x2) {
         idx = x2;
	 x2 = x1;
	 x1 = idx;
      }	      
      if (y1 > y2) {
         idx = y2;
	 y2 = y1;
	 y1 = idx;
      }
      for (idx=y1;idx<(y2+1);idx++) {
          DrawHorzLine(x1,x2,idx,color); 	      
      }
}	


void GdFillRect (int x1, int y1, int width, int height)
{
      unsigned long dm = 0, dc = 0;
      int x2 = x1 + width - 1;
      int y2 = y1 + height - 1;
      if (width <= 0 || height <= 0) return;
      /* Stipples and tiles have their own drawing routines */
      if (gr_fillmode != FILL_SOLID) {
         set_ts_origin(x1, y1);
         ts_fillrect(x1, y1, width, height);
         return;
      }
      FillRect(x1, y1, x2, y2, gr_foreground);      
      GdSetDash(&dm,&dc);
      /* The rectangle may be partially obstructed. So do it line by line. */
      while (y1 <= y2) drawrow(x1, x2, y1++);
      /* Restore the dash settings */
      GdSetDash(&dm,&dc);
}


void GdLine (int x1, int y1, int x2, int y2,int bDrawLastPoint)
{
      int xdelta;               /* width of rectangle around line */
      int ydelta;               /* height of rectangle around line */
      int xinc;                 /* increment for moving x coordinate */
      int yinc;                 /* increment for moving y coordinate */
      int rem;                  /* current remainder */
      unsigned int bit = 0;     /* used for dashed lines */
      int temp;
      /* See if the line is horizontal or vertical. If so, then call special routines. */
      if (y1 == y2) {
	 /* Adjust coordinates if not drawing last point.  Tricky. */
	 if (!bDrawLastPoint) {
	    if (x1 > x2) {              
	       temp = x1;
	       x1 = x2 + 1;
	       x2 = temp;
	    } else --x2;
	 }
	 /* call faster line drawing routine */
	 drawrow(x1, x2, y1);
	 return;
      }
      if (x1 == x2) {
	 /* Adjust coordinates if not drawing last point.  Tricky. */
	 if (!bDrawLastPoint) {
	    if (y1 > y2) {               
	       temp = y1;
	       y1 = y2 + 1;
	       y2 = temp;
	    } else --y2;
	 }
	 /* call faster line drawing routine */
	 drawcol(x1, y1, y2);
	 return;
      }
      /* The line may be partially obscured. Do the draw line algorithm checking each point against the clipping regions. */
      xdelta = x2 - x1;
      ydelta = y2 - y1;
      if (xdelta < 0) xdelta = -xdelta;
      if (ydelta < 0) ydelta = -ydelta;
      xinc = (x2 > x1)? 1 : -1;
      yinc = (y2 > y1)? 1 : -1;
      /* draw first point*/
      SetPixel(x1, y1, gr_foreground);
      if (xdelta >= ydelta) {
	 rem = xdelta / 2;
	 for (;;) {
	     if (!bDrawLastPoint && x1 == x2) break;
	     x1 += xinc;
	     rem += ydelta;
	     if (rem >= xdelta) {
     		rem -= xdelta;
		y1 += yinc;
	     }
             if (gr_dashcount) {
                if (gr_dashmask & (1 << bit))  SetPixel(x1, y1, gr_foreground);
                bit = (bit + 1) % gr_dashcount;
             } else {   /* No dashes */
                SetPixel(x1, y1, gr_foreground);
             }
             if (bDrawLastPoint && x1 == x2) break;
         }
      } else {
         rem = ydelta / 2;
         for (;;) {
             if (!bDrawLastPoint && y1 == y2) break;
             y1 += yinc;
             rem += xdelta;
             if (rem >= ydelta) {
	        rem -= ydelta;
	        x1 += xinc;
	     }
             /* If we are trying to draw to a dash mask */
             if (gr_dashcount) {
                if (gr_dashmask & (1 << bit)) SetPixel(x1, y1, gr_foreground);
		bit = (bit + 1) % gr_dashcount;
	     } else {   /* No dashes */
                SetPixel( x1, y1, gr_foreground);
	     }
             if (bDrawLastPoint && y1 == y2) break;
         }
      }
}


void GdArea (int x, int y, int width, int height, void *pixels, int pixtype)
{
      unsigned char *PIXELS = pixels;   /* for ANSI compilers, can't use void*/
      long cellstodo;                   /* remaining number of cells */
      long count;                       /* number of cells of same color */
      long cc;      		        /* current cell count */
      long rows;                        /* number of complete rows */
      int minx;                 	/* minimum x value */
      int maxx;                     	/* maximum x value */
      unsigned short savecolor;         /* saved foreground color */
      int dodraw;                    	/* TRUE if draw these points */
      minx = x;
      maxx = x + width - 1;
      savecolor = gr_foreground;
      cellstodo = (long)width * height;
      while (cellstodo > 0) {
            gr_foreground = *(unsigned short *)PIXELS;
            PIXELS += sizeof(unsigned short);
            dodraw = (gr_usebg || (gr_foreground != gr_background));
            count = 1;
            --cellstodo;
            while (cellstodo > 0) {
                  if (gr_foreground != *(unsigned short *)PIXELS) goto breakwhile;
                  PIXELS += sizeof(unsigned short);
                  ++count;
                  --cellstodo;		     
            }
breakwhile:
            /* If there is only one point with this color, then draw it by itself. */
            if (count == 1) {
               if (dodraw) drawpoint(x, y);
               if (++x > maxx) {
                  x = minx;
                  y++;
               }
               continue;
            }
            if (x != minx) {
               cc = count;
               if (x + cc - 1 > maxx) cc = maxx - x + 1;
	       if (dodraw) drawrow(x, x + cc - 1, y);
	       count -= cc;
	       x += cc;
               if (x > maxx) {
   	          x = minx;
                  y++;
               }
            }
            rows = count / width;
            if (rows > 0) {
               if (dodraw) {
                  GdFillRect(x, y, maxx - x + 1, rows);
               }
               count %= width;
               y += rows;
            }
            if (count > 0) {
               if (dodraw) drawrow(x, x + count - 1, y);
               x += count;
	    }
      }
      gr_foreground = savecolor;      
}

