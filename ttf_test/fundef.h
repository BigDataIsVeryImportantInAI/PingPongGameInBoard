/********************************************************************************/
/* Copyright (c) 2004, 2005, Lee,Chang-woo HL2IRW <hl2irw@armstudy.com>         */
/* fundef.h									*/
/* 2005-10-08                                                                   */
/* Lee Chang-woo HL2IRW (hl2irw@armstudy.com)                                   */
/* find . -name CVS -exec rm -rf {} \;                                          */
/********************************************************************************/
#include <sys/time.h>
#include <sys/types.h>
#include <asm/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <linux/fs.h>
#include <linux/rtc.h>
#include <linux/fb.h>
#include <pthread.h>
#include <stdarg.h>
#include <termios.h>

#define MIN(a,b)                ((a) < (b) ? (a) : (b))
#define MAX(a,b)                ((a) > (b) ? (a) : (b))

#define MAX_CHAR_HEIGHT 128                     /* maximum text bitmap height*/
#define MAX_CHAR_WIDTH  128                     /* maximum text bitmap width*/
#define TF_ASCII        0x00000000L             /* 8 bit packing, ascii*/
#define TF_UTF8         0x00000001L             /* 8 bit packing, utf8*/
#define TF_UC16         0x00000002L             /* 16 bit packing, unicode 16*/
#define TF_UC32         0x00000004L             /* 32 bit packing, unicode 32*/
#define TF_XCHAR2B      0x00000008L             /* 16 bit packing, X11 big endian PCF*/
#define TF_PACKMASK     0x0000000FL             /* packing bits mask*/
/* Text alignment flags*/
#define TF_TOP          0x01000000L     /* align on top*/
#define TF_BASELINE     0x02000000L     /* align on baseline*/
#define TF_BOTTOM       0x04000000L     /* align on bottom*/

/* SetFontAttr flags (no intersect with MWTF_ above)*/
#define TF_KERNING      0x0001          /* font kerning*/
#define TF_ANTIALIAS    0x0002          /* antialiased output*/
#define TF_UNDERLINE    0x0004          /* draw underline*/
#define TF_FREETYPE     0x1000          /* FIXME: remove*/

#define TF_DBCS_BIG5    0x00000100L     /* chinese big5*/
#define TF_DBCS_EUCCN   0x00000200L     /* chinese EUCCN (gb2312+0x80)*/
#define TF_DBCS_EUCKR   0x00000300L     /* korean EUCKR (ksc5601+0x80)*/
#define TF_DBCS_EUCJP   0x00000400L     /* japanese EUCJP*/
#define TF_DBCS_JIS     0x00000500L     /* japanese JISX0213*/
#define TF_DBCSMASK     0x00000700L     /* DBCS encodings bitmask*/

#define PF_RGB           0    /* pseudo, convert from packed 32 bit RGB*/
#define PF_PIXELVAL      1    /* pseudo, no convert from packed PIXELVAL*/
#define PF_PALETTE       2    /* pixel is packed 8 bits 1, 4 or 8 pal index*/
#define PF_TRUECOLOR0888 3    /* pixel is packed 32 bits 8/8/8 truecolor*/
#define PF_TRUECOLOR888  4    /* pixel is packed 24 bits 8/8/8 truecolor*/
#define PF_TRUECOLOR565  5    /* pixel is packed 16 bits 5/6/5 truecolor*/
#define PF_TRUECOLOR555  6    /* pixel is packed 16 bits 5/5/5 truecolor*/
#define PF_TRUECOLOR332  7    /* pixel is packed 8 bits 3/3/2 truecolor*/
#define PF_TRUECOLOR8888 8    /* pixel is packed 32 bits 8/8/8/8 truecolor with alpha */
#define FREETYPE_FONT_DIR  "/mnt/font/"
#define TRUE               1
#define FALSE              0

#define BITS(pixel,shift,mask)  (((pixel)>>shift)&(mask))
#undef DEBUG
//#define DEBUG
#ifdef DEBUG
#define DBG(fmt, args...)   fprintf(stderr,"DEBUG: " __FILE__ ": " fmt, ##args)
#else
#define DBG(fmt, args...)   do { } while(0);
#endif

/* GetFontList structure */
typedef struct {
      char *ttname;           /* TrueType name, eg "Times New Roman Bold" */
      char *name;           /* microwin name, eg "timesb" */
} FONTLIST, *PFONTLIST;

/* In-core color palette structure*/
typedef struct {
      unsigned char r;
      unsigned char g;
      unsigned char b;
      unsigned char _padding;
} PALENTRY;

typedef struct FREETYPEFONT  *PFREETYPEFONT;

typedef struct {
      int maxwidth;
      int height;
      int baseline;
      int descent;
      int maxascent;
      int maxdescent;
      int linespacing;
      int firstchar;
      int lastchar;
      int fixed;
      unsigned char widths[256];
} FONTINFO, *PFONTINFO;


/* builtin C-based proportional/fixed font structure*/
typedef struct {
      char *name;               /* font name*/
      int  maxwidth;            /* max width in pixels*/
      unsigned int height;      /* height in pixels*/
      int ascent;               /* ascent (baseline) height*/
      int firstchar;            /* first character in bitmap*/
      int size;                 /* font size in characters*/
      unsigned short *bits;     /* 16-bit right-padded bitmap data*/
      unsigned long *offset;    /* 256 offsets into bitmap data*/
      unsigned char *width;     /* 256 character widths or 0 if fixed*/
      int defaultchar;          /* default char (not glyph index)*/
      long bits_size;           /* # words of MWIMAGEBITS bits*/
} CFONT, *PCFONT;

typedef struct _font *PFONT;

/* draw procs associated with fonts.  Strings are [re]packed using defencoding*/
typedef struct {
      unsigned long encoding;   /* routines expect this encoding*/
      int     (*GetFontInfo)(PFONT pfont, PFONTINFO pfontinfo);
      void    (*GetTextSize)(PFONT pfont, const void *text, int cc,unsigned long flags, int *pwidth, int *pheight,int *pbase);
      void    (*GetTextBits)(PFONT pfont, int ch, const unsigned long **retmap,int *pwidth, int *pheight, int *pbase);
      void    (*DestroyFont)(PFONT pfont);
      void    (*DrawText)(PFONT pfont, int x, int y,const void *str, int count, unsigned long flags);
      void    (*SetFontSize)(PFONT pfont, int fontsize);
      void    (*SetFontRotation)(PFONT pfont, int tenthdegrees);
      void    (*SetFontAttr)(PFONT pfont, int setflags, int clrflags);
      PFONT   (*Duplicate) (PFONT psrcfont, int fontsize);
} FONTPROCS, *PFONTPROCS;

/* new multi-renderer font struct*/
typedef struct _font {          /* common hdr for all font structures*/
      PFONTPROCS fontprocs;     /* font-specific rendering routines*/
      int fontsize;             /* font height in pixels*/
      int fontrotation;         /* font rotation*/
      int fontattr;             /* font attributes: kerning/antialias*/
      				/* font-specific rendering data here*/
} FONT;


//
// graph.c
// 
extern void init_graph (void);
extern void drawpoint(int x, int y);	
extern void DrawHorzLine (int x1, int x2, int y, int color);
extern void DrawVertLine (int x, int y1, int y2, int color);
extern void GdLine(int x1, int y1, int x2, int y2,int bDrawLastPoint);
extern void GdArea(int x, int y, int width, int height, void *pixels, int pixtype);
extern void GdFillRect(int x1, int y1, int width, int height);
extern void GdSetDash (unsigned long *mask, unsigned long *count);
//
// freetype.c
//
extern void FreeFontList (int n);
extern void GetFontList (void);
extern void ttf_list_display (void);	
extern PFONT ttf_open (char *font_name, int height, int fontattr);
extern void ttf_close (PFONT pfont);
extern void ttf_printf (PFONT pfont, int fontattr,int x,int y,char *form,...);
extern void ttf_Size (PFONT pfont, int size);
extern void ttf_rotation (PFONT pfont, int degrees);
extern void ttf_attr (PFONT pfont, int set, int clr);
//
// serial.c
//
extern int serial_init (void);
extern void serial_close (void);
extern void serial_check (void);
//
// display.c
//
extern void UIprintf (int x,int y,char *form,...);
extern void close_display (void);
extern int init_display (void);
extern void clear_display (void);
extern void set_color (int color);
extern void set_bgcolor (int color);
extern void SetPixel (int x, int y, unsigned short color);
extern unsigned short GetPixel (int x, int y);
extern void fill_rectangle (int x1, int y1, int x2, int y2, int color);
extern void bar (int x1, int y1, int x2, int y2, int color);
extern void line (int x1, int y1, int x2, int y2 ,int color);
extern void circle (int x,int y, int radius,int fill,int color);
extern void ellipse (int x0,int y0,int a0,int b0,int fill,int color);
extern void rectangle (int x1,int y1,int x2,int y2,int color);
extern void draw_vert_line (int x, int y1, int y2, unsigned short color);
extern void draw_horz_line (int x1, int x2, int y, unsigned short color);
//
//box3.c
//
extern void norm_3d (int x1,int y1,int x2,int y2,int color);
extern void box_3d (int x1, int y1, int x2, int y2, int out_color, int in_color);
extern void vert_3d (int x1, int y1, int x2, int y2, int color);
extern void two_line_3d (int x1, int y1, int x2, int y2, int in_color, int out_color);
extern void double_3d (int x1, int y1, int x2, int y2, int color);
extern void round_3d (int x1, int y1, int x2, int y2, int color);
extern void inline_3d (int x1, int y1, int x2, int y2, int color);
extern void line_3d (int x1, int y1, int x2, int y2);
extern void shade (int x1, int y1, int x2, int y2, int color);

