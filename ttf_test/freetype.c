/********************************************************************************/
/* Copyright (c) 2000, 2002, 2003 Greg Haerr <greg@censoft.com>			*/
/* Portions Copyright (c) 2002 by Koninklijke Philips Electronics N.V.		*/
/*										*/
/* Freetype TrueType routines originally contributed by Martin Jolicoeur	*/
/* Heavily modified by Greg Haerr						*/
/* 2005-10-8									*/
/* Lee Chang-woo HL2IRW (hl2irw@armstudy.com)					*/
/********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <dirent.h>
#include "inc/freetype/freetype.h"
#include "inc/freetype/ftxkern.h"
#include "inc/freetype/ftnameid.h"
#include "inc/freetype/ftxcmap.h"
#include "inc/freetype/ftxwidth.h"
#include "fundef.h"

#if TT_FREETYPE_MAJOR != 1 || TT_FREETYPE_MINOR < 3
#error "You must link with freetype lib version 1.3.x +, and not freetype 2."
#endif
#undef DEBUG
//#define DEBUG
#ifdef DEBUG
#define DBG(fmt, args...)   fprintf(stderr,"DEBUG: " __FILE__ ": " fmt, ##args)
#else
#define DBG(fmt, args...)   do { } while(0);
#endif

typedef struct FREETYPEFONT {
      PFONTPROCS	fontprocs;	
      int		fontsize;
      int		fontrotation;
      int		fontattr;		
      TT_Face 		face;		
      TT_Instance	instance;
      TT_CharMap 	char_map;
      TT_Kerning 	directory;
      TT_Matrix 	matrix;
      TT_Glyph 		glyph;
      int 		can_kern;
      short 		last_glyph_code;
      short 		last_pen_pos;
} FREETYPEFONT;

int freetype_init(void);
PFREETYPEFONT freetype_createfont(const char *name, int height, int attr);
static int freetype_getfontinfo(PFONT pfont, PFONTINFO pfontinfo);
static void freetype_gettextsize(PFONT pfont, const void *text, int cc,unsigned long flags, int *pwidth, int *pheight,int *pbase);
static void freetype_destroyfont(PFONT pfont);
static void freetype_drawtext(PFONT pfont, int x, int y, const void *text, int cc, unsigned long flags);
static void freetype_setfontsize(PFONT pfont, int fontsize);
static void freetype_setfontrotation(PFONT pfont, int tenthdegrees);
		

static FONTPROCS freetype_procs = {
      TF_UC16,				
      freetype_getfontinfo,
      freetype_gettextsize,
      NULL,				
      freetype_destroyfont,
      freetype_drawtext,
      freetype_setfontsize,
      freetype_setfontrotation,
      NULL,				
      NULL,				
};

static TT_Engine 	engine;	
static unsigned long gray_palette[5];

extern int gr_foreground;
extern int gr_background;
extern int gr_usebg;
FONTLIST **list;
extern int numfonts;
char ttf_buff[128],uni_buff[256];
				    

int GdSetFontSize (PFONT pfont, int fontsize)
{
      int oldfontsize = pfont->fontsize;
      pfont->fontsize = fontsize;
      if (pfont->fontprocs->SetFontSize) pfont->fontprocs->SetFontSize(pfont, fontsize);
      return oldfontsize;
}


int GdSetFontRotation (PFONT pfont, int tenthdegrees)
{
      int oldrotation = pfont->fontrotation;
      pfont->fontrotation = tenthdegrees;
      if (pfont->fontprocs->SetFontRotation) pfont->fontprocs->SetFontRotation(pfont, tenthdegrees);
      return oldrotation;
}


int GdSetFontAttr (PFONT pfont, int setflags, int clrflags)
{
      int oldattr = pfont->fontattr;
      pfont->fontattr &= ~clrflags;
      pfont->fontattr |= setflags;
      if (pfont->fontprocs->SetFontAttr) pfont->fontprocs->SetFontAttr(pfont, setflags, clrflags);
      return oldattr;
}


static int utf8_to_utf16 (const unsigned char *utf8, int cc, unsigned short *unicode16)
{
      int count = 0;
      unsigned char c0, c1;
      unsigned long scalar;      
      while (--cc >= 0) {
            c0 = *utf8++;
	    if (c0 < 0x80) {               
	       *unicode16++ = c0;	
	       count++;
	       continue;
	    }			
	    if ((c0 & 0xc0) == 0x80) return -1;
	    scalar = c0;
	    if(--cc < 0) return -1;
	    c1 = *utf8++;
	    if ((c1 & 0xc0) != 0x80) return -1;
	    scalar <<= 6;
	    scalar |= (c1 & 0x3f);	    
	    if (!(c0 & 0x20)) {
               if ((scalar != 0) && (scalar < 0x80)) return -1;    
	       *unicode16++ = scalar & 0x7ff;
	       count++;
	       continue;	       
	    }		
	    if (--cc < 0) return -1;
	    c1 = *utf8++;					                    
	    if ((c1 & 0xc0) != 0x80) return -1;
	    scalar <<= 6;
	    scalar |= (c1 & 0x3f);	    	
	    if (!(c0 & 0x10)) {
               if (scalar < 0x800) return -1;      
	       if (scalar >= 0xd800 && scalar < 0xe000) return -1;     
	       *unicode16++ = scalar & 0xffff;
	       count++;
	       continue;
	    }
	    c1 = *utf8++;
	    if (--cc < 0) return -1;	    
	    if ((c1 & 0xc0) != 0x80) return -1;
	    scalar <<= 6;
	    scalar |= (c1 & 0x3f);	    	
	    if (!(c0 & 0x08)) {
	       if (scalar < 0x10000) return -1;    
	       scalar -= 0x10000;	       		       
	       *unicode16++ = ((scalar >> 10) & 0x3ff) + 0xd800;
	       *unicode16++ = (scalar & 0x3ff) + 0xdc00;
	       count += 2;
	       continue;      
  	    }
	    return -1;    
      }
      return count;
}
      
	       

	    
void alphablend (unsigned long *out, unsigned short src, unsigned short dst,unsigned char *alpha, int count)
{
      unsigned int a, d;
      unsigned char r, g, b;
      while (--count >= 0) {
	    a = *alpha++;	    
	    if (a == 0) *out++ = dst;
	    else if(a == 255) *out++ = src;
	    else  {
		  d = BITS(dst, 11, 0x1f);
		  r = (unsigned char)(((BITS(src, 11, 0x1f) - d)*a)>>8) + d;
		  d = BITS(dst, 5, 0x3f);
		  g = (unsigned char)(((BITS(src, 5, 0x3f) - d)*a)>>8) + d;
		  d = BITS(dst, 0, 0x1f);
		  b = (unsigned char)(((BITS(src, 0, 0x1f) - d)*a)>>8) + d;
		  *out++ = (r << 11) | (g << 5) | b;
	    }
      }
}


int GdConvertEncoding (const void *istr, unsigned long iflags, int cc, void *ostr, unsigned long oflags)
{
      const unsigned char *istr8;
      const unsigned short *istr16;
      const unsigned long *istr32;
      unsigned char *ostr8; 
      unsigned short *ostr16;
      unsigned long *ostr32;
      unsigned int ch; 
      int icc;
      unsigned short buf16[512];      
      iflags &= TF_PACKMASK | TF_DBCSMASK;
      oflags &= TF_PACKMASK | TF_DBCSMASK;     
      if (cc == -1 && (iflags == TF_ASCII)) cc = strlen((char *)istr);           
      if (iflags == TF_UTF8) {         
	 cc = utf8_to_utf16((unsigned char *)istr, cc,oflags==TF_UC16?(unsigned short*) ostr: buf16);
	 if (oflags == TF_UC16 || cc < 0) return cc;	 	 		
	 iflags = TF_UC16;
	 istr = buf16;
      }
      icc = cc;
      cc = 0;
      istr8 = istr;
      istr16 = istr;
      istr32 = istr;
      ostr8 = ostr;
      ostr16 = ostr;
      ostr32 = ostr;
      while (--icc >= 0) {
            switch (iflags) {
	       default:
	         ch = *istr8++;
		 break;
	       case TF_UC16:
		 ch = *istr16++;
		 break;
	       case TF_XCHAR2B:
		 ch = *istr8++ << 8;
		 ch |= *istr8++;
		 break;
	       case TF_UC32:
		 ch = *istr32++;
		 break;
	       case TF_DBCS_BIG5:   
		 ch = *istr8++;
		 if (ch >= 0xA1 && ch <= 0xF9 && icc && ((*istr8 >= 0x40 && *istr8 <= 0x7E) || (*istr8 >= 0xA1 && *istr8 <= 0xFE))) {
                    ch = (ch << 8) | *istr8++;
		    --icc;
		 }
		 break;
	       case TF_DBCS_EUCCN:   /* Chinese EUCCN (GB2312+0x80)*/
		 ch = *istr8++;
		 if (ch >= 0xA1 && ch <= 0xF7 && icc && *istr8 >= 0xA1 && *istr8 <= 0xFE) {
                    ch = (ch << 8) | *istr8++;
		    --icc;
		 }
		 break;
	       case TF_DBCS_EUCKR:   /* Korean EUCKR (KSC5601+0x80)*/
		 ch = *istr8++;
		 if (ch >= 0xA1 && ch <= 0xFE && icc && *istr8 >= 0xA1 && *istr8 <= 0xFE) {
                    ch = (ch << 8) | *istr8++;
		    --icc;
		 }
		 break;
	       case TF_DBCS_EUCJP:   /* Japanese EUCJP*/
		 ch = *istr8++;
		 if (ch >= 0xA1 && ch <= 0xFE && icc && *istr8 >= 0xA1 && *istr8 <= 0xFE) {
                    ch = (ch << 8) | *istr8++;
		    --icc;
		 }
		 break;
	       case TF_DBCS_JIS:     /* Japanese JISX0213*/
		 ch = *istr8++;
		 if (icc && ((ch >= 0xA1 && ch <= 0xFE && *istr8 >= 0xA1 && *istr8 <= 0xFE) ||
                    (((ch >= 0x81 && ch <= 0x9F) || (ch >= 0xE0 && ch <= 0xEF)) && (*istr8 >= 0x40 && *istr8 <= 0xFC && *istr8 != 0x7F)))) {
                    ch = (ch << 8) | *istr8++;
		    --icc;
		 }
		 break;
	    }
	    switch (oflags) {
	       default:
		 *ostr8++ = (unsigned char)ch;
		 break;
	       case TF_UC16:
		 *ostr16++ = (unsigned short)ch;
		 break;
	       case TF_XCHAR2B:
		 *ostr8++ = (unsigned char)(ch >> 8);
		 *ostr8++ = (unsigned char)ch;
		 break;
	       case TF_UC32:
		 *ostr32++ = ch;
		 break;
	    }
	    ++cc;
      }
      return cc;
}

		 


int freetype_init (void)
{
      static int inited = 0;
      if (inited) return 1;
      if (TT_Init_FreeType (&engine) != TT_Err_Ok) {
	 return 0;
      }
      if (TT_Init_Kerning_Extension (engine) != TT_Err_Ok) return 0;
      inited = 1;
      return 1;
}


PFREETYPEFONT freetype_createfont (const char *name, int height, int attr)
{
      PFREETYPEFONT pf;
      unsigned short i, n;
      unsigned short platform, encoding;
      TT_Face_Properties properties;
      char *p;
      char fontname[128];
      if (strchr(name, '/') != NULL) {
	 strcpy(fontname, name);
      } else {
	 strcpy(fontname, FREETYPE_FONT_DIR);
	 strcat(fontname, "/");
	 strcat(fontname, name);
      }
      if ((p = strrchr(fontname, '.')) == NULL || strcmp(p, ".ttf") != 0) {
	 strcat(fontname, ".ttf");
      }
      pf = (PFREETYPEFONT)calloc(sizeof(FREETYPEFONT), 1);
      if (!pf) return NULL;
      pf->fontprocs = &freetype_procs;
      if (TT_Open_Face (engine, fontname, &pf->face) != TT_Err_Ok) goto out;
      pf->can_kern = TRUE;
      if (TT_Load_Kerning_Table (pf->face, 0) != TT_Err_Ok) {
	 pf->can_kern = FALSE;
      } else {
	 if (TT_Get_Kerning_Directory (pf->face, &pf->directory) != TT_Err_Ok) {
	    pf->can_kern = FALSE;
	 } else {
	    if ((pf->directory.version != 0) ||	(pf->directory.nTables <= 0) || (pf->directory.tables->loaded != 1) ||
	        (pf->directory.tables->version != 0) ||(pf->directory.tables->t.kern0.nPairs <= 0)) pf->can_kern = FALSE;
	 }
      }
      TT_Get_Face_Properties (pf->face, &properties);
      if (TT_New_Glyph (pf->face, &pf->glyph) != TT_Err_Ok) goto out;
      if (TT_New_Instance (pf->face, &pf->instance) != TT_Err_Ok) goto out;
      if (TT_Set_Instance_Resolutions (pf->instance, 96, 96) != TT_Err_Ok) goto out;
      n = properties.num_CharMaps;
      for (i=0;i<n;i++) {
	  TT_Get_CharMap_ID (pf->face, i, &platform, &encoding);	
  	  if (((platform == TT_PLATFORM_MICROSOFT) && (encoding == TT_MS_ID_UNICODE_CS)) ||
	      ((platform == TT_PLATFORM_APPLE_UNICODE) && (encoding == TT_APPLE_ID_DEFAULT))) {
	     TT_Get_CharMap (pf->face, i, &pf->char_map);
	     i = n + 1;
	  }
      }
      if (i == n) {
	 DBG("freetype_createfont: no unicode map table\n");
	 goto out;
      }	
      GdSetFontSize((PFONT)pf, height);
      GdSetFontRotation((PFONT)pf, 0);
      GdSetFontAttr((PFONT)pf, attr, 0);
      return pf;
out:
      DBG(" ttf error out ... \n");
      free(pf);
      return NULL;
}


static int compute_kernval (PFREETYPEFONT pf, short current_glyph_code)
{
      int i = 0;
      int kernval;
      int nPairs = pf->directory.tables->t.kern0.nPairs;
      TT_Kern_0_Pair *pair = pf->directory.tables->t.kern0.pairs;
      if (pf->last_glyph_code != -1) {
	 while ((pair->left != pf->last_glyph_code) && (pair->right != current_glyph_code)) {
	       pair++;
	       i++;
	       if (i == nPairs) break;
	 }
	 if (i == nPairs) {
	    kernval = 0;
	 } else {
 	    kernval = (pair->value + 32) & -64;
	 }		
      } else kernval = 0;
      return kernval;
}


static TT_UShort Get_Glyph_Width (PFREETYPEFONT pf, TT_UShort glyph_index)
{
      TT_Glyph_Metrics metrics;	
      if (TT_Load_Glyph(pf->instance,pf->glyph,TT_Char_Index(pf->char_map,glyph_index),
			      TTLOAD_SCALE_GLYPH|TTLOAD_HINT_GLYPH) != TT_Err_Ok) {
    	 if (TT_Load_Glyph ( pf->instance, pf->glyph, 0,TTLOAD_SCALE_GLYPH|TTLOAD_HINT_GLYPH) != TT_Err_Ok) return 0;
      }
      TT_Get_Glyph_Metrics (pf->glyph, &metrics);
      return((metrics.advance & 0xFFFFFFC0) >> 6);
}


static void drawchar (PFREETYPEFONT pf, TT_Glyph glyph, int x_offset,int y_offset)
{
      TT_F26Dot6 xmin, ymin, xmax, ymax, x, y, z;
      unsigned char *src, *srcptr;
      unsigned short *dst, *dstptr;
      unsigned short *bitmap;
      int size, width, height;
      TT_Outline outline;
      TT_BBox bbox;
      TT_Raster_Map Raster;
      TT_Error error;
      TT_Get_Glyph_Outline (pf->glyph, &outline);
      TT_Get_Outline_BBox (&outline, &bbox);
      xmin = (bbox.xMin & -64) >> 6;
      ymin = (bbox.yMin & -64) >> 6;
      xmax = ((bbox.xMax + 63) & -64) >> 6;
      ymax = ((bbox.yMax + 63) & -64) >> 6;
      width = xmax - xmin;
      height = ymax - ymin;
      size = width * height;
      Raster.rows = height;
      Raster.width = width;      
      if (pf->fontattr & TF_ANTIALIAS) {
	 Raster.cols = (Raster.width + 3) & -4;		
      } else {
	 Raster.cols = (Raster.width + 7) & -8;	
      }
      Raster.flow = TT_Flow_Up;
      Raster.size = Raster.rows * Raster.cols;
      Raster.bitmap = malloc (Raster.size);
      memset (Raster.bitmap, 0, Raster.size);
      if (!(pf->fontattr & TF_ANTIALIAS)) {
	 error = TT_Get_Glyph_Bitmap (pf->glyph, &Raster,-xmin * 64, -ymin * 64);
      } else {
	 error = TT_Get_Glyph_Pixmap (pf->glyph, &Raster,-xmin * 64, -ymin * 64);
      }
      if (error) {
         free (Raster.bitmap);
	 return;
      }
      bitmap = malloc (size * sizeof (unsigned short));
      memset (bitmap, 0, size * sizeof (unsigned short));
      src = (unsigned char *)Raster.bitmap;
      dst = bitmap + (size - width);
      for (y=ymin;y<ymax;y++) {
          srcptr = src;
	  dstptr = dst;
	  for (x=xmin;x<xmax;x++) {
	      if (pf->fontattr & TF_ANTIALIAS) {
	      	 *dstptr++ = gray_palette[(int) *srcptr];
	      }	else {
		 for (z=0;z <= ((xmax-x-1) < 7 ? (xmax-x-1) : 7); z++) {
		     *dstptr++ = ((*srcptr << z) & 0x80)? gr_foreground: gr_background;
		 }
		 x += 7;
	      }
	      srcptr++;
	  }
	  src += Raster.cols;
	  dst -= width;
      }
      GdArea(x_offset + xmin, y_offset - (ymin + height), width, height,bitmap, PF_PIXELVAL);	
      free (bitmap);
      free (Raster.bitmap);
}


static void freetype_drawtext (PFONT pfont, int ax, int ay,const void *text, int cc, unsigned long flags)
{
      PFREETYPEFONT pf = (PFREETYPEFONT)pfont;
      const unsigned short *str = text;
      TT_F26Dot6 x = ax, y = ay;
      TT_Pos vec_x, vec_y;
      int i;
      TT_F26Dot6 startx, starty;
      TT_Outline outline;
      TT_UShort curchar;
      TT_Glyph_Metrics metrics;
      TT_Face_Properties properties;
      TT_Instance_Metrics imetrics;
      TT_F26Dot6 ascent, descent;
      static unsigned char blend[5] = { 0x00, 0x44, 0x88, 0xcc, 0xff };
      static unsigned char virtual_palette[5] = { 0, 1, 2, 3, 4 };
      pf->last_glyph_code = -1;		
      pf->last_pen_pos = -32767;
      TT_Get_Face_Properties (pf->face, &properties);
      TT_Get_Instance_Metrics(pf->instance, &imetrics);	
      ascent = ((properties.horizontal->Ascender * imetrics.y_scale) / 0x10000);
      descent = ((properties.horizontal->Descender * imetrics.y_scale) / 0x10000);
      if (flags & TF_BOTTOM) {
	 vec_x = 0;
	 vec_y = descent;
	 TT_Transform_Vector(&vec_x, &vec_y,&pf->matrix);
	 x -= vec_x / 64;
	 y += vec_y / 64;
      } else if (flags & TF_TOP) {
	 vec_x = 0;
	 vec_y = ascent;
	 TT_Transform_Vector(&vec_x, &vec_y,&pf->matrix);
	 x -= vec_x / 64;
	 y += vec_y / 64;
      }
      if (pf->fontattr & TF_ANTIALIAS) {
         TT_Set_Raster_Gray_Palette (engine, virtual_palette);
	 alphablend(gray_palette, gr_foreground, gr_background,blend, 5);
      }
      startx = x;
      starty = y;
      for (i=0;i<cc;i++) {
          curchar = TT_Char_Index (pf->char_map, str[i]);
	  if (TT_Load_Glyph (pf->instance, pf->glyph, curchar,TTLOAD_DEFAULT) != TT_Err_Ok) continue;
	  if (pf->fontrotation) {
	     TT_Get_Glyph_Outline (pf->glyph, &outline);
	     TT_Transform_Outline (&outline, &pf->matrix);
	  }
	  TT_Get_Glyph_Metrics (pf->glyph, &metrics);
	  if ((pf->fontattr & TF_KERNING) && pf->can_kern) {
	     if (pf->fontrotation) {
		vec_x = compute_kernval(pf, curchar);
		vec_y = 0;
		TT_Transform_Vector(&vec_x, &vec_y,&pf->matrix);
		x += vec_x / 64;
		y -= vec_y / 64;
	     } else x += compute_kernval(pf, curchar) / 64;
	  }			
	  drawchar(pf, pf->glyph, x, y);
	  if (pf->fontrotation) {
	     vec_x = metrics.advance;
	     vec_y = 0;
	     TT_Transform_Vector (&vec_x, &vec_y, &pf->matrix);
	     x += vec_x / 64;
	     y -= vec_y / 64;
	  } else {
	     x += metrics.advance / 64;
	     if (pf->last_pen_pos > x) x = pf->last_pen_pos;
	     pf->last_pen_pos = x;
	  }	
  	  pf->last_glyph_code = curchar;
      }
      if (pf->fontattr & TF_UNDERLINE) GdLine( startx, starty, x, y, FALSE);
}


static int freetype_getfontinfo (PFONT pfont, PFONTINFO pfontinfo)
{
      int i;
      PFREETYPEFONT pf = (PFREETYPEFONT)pfont;
      TT_Face_Properties properties;
      TT_Instance_Metrics imetrics;  
      TT_UShort last_glyph_index;
      TT_Get_Face_Properties (pf->face, &properties);
      TT_Get_Instance_Metrics(pf->instance, &imetrics);
      pfontinfo->height = (((properties.horizontal->Ascender * imetrics.y_scale)/ 0x10000) >> 6) -
	                    (((properties.horizontal->Descender * imetrics.y_scale)/ 0x10000) >> 6);
      pfontinfo->maxwidth = ((properties.horizontal->xMax_Extent * imetrics.x_scale)/ 0x10000) >> 6;
      pfontinfo->baseline = ((properties.horizontal->Ascender * imetrics.y_scale)/ 0x10000) >> 6;
      pfontinfo->firstchar = TT_CharMap_First(pf->char_map, NULL);
      pfontinfo->lastchar = TT_CharMap_Last(pf->char_map, NULL);
      pfontinfo->fixed = properties.postscript->isFixedPitch;      
      pfontinfo->linespacing = pfontinfo->height + 2;
      pfontinfo->descent = pfontinfo->height - pfontinfo->baseline;
      pfontinfo->maxascent = pfontinfo->baseline;
      pfontinfo->maxdescent = pfontinfo->descent;		
      last_glyph_index = properties.num_Glyphs > 255 ? 255: properties.num_Glyphs-1;
      for (i=0;i<=last_glyph_index;i++)	pfontinfo->widths[i] = Get_Glyph_Width(pf, i);
      return TRUE;
}


static void freetype_gettextsize (PFONT pfont, const void *text, int cc, unsigned long flags,int *pwidth, int *pheight, int *pbase)
{
      PFREETYPEFONT pf = (PFREETYPEFONT)pfont;
      const unsigned short *str = text;
      TT_F26Dot6 x = 0;
      int i;
      TT_UShort curchar;
      TT_Glyph_Metrics metrics;
      TT_Face_Properties properties;
      TT_Instance_Metrics imetrics;
      TT_Get_Face_Properties (pf->face, &properties);
      TT_Get_Instance_Metrics(pf->instance, &imetrics);	
      pf->last_glyph_code = -1;		
      pf->last_pen_pos = -32767;	
      for (i=0;i<cc;i++) {
	  curchar = TT_Char_Index (pf->char_map, str[i]);
	  if (TT_Load_Glyph (pf->instance, pf->glyph, curchar,TTLOAD_SCALE_GLYPH|TTLOAD_HINT_GLYPH) != TT_Err_Ok) continue;
	  TT_Get_Glyph_Metrics (pf->glyph, &metrics);
	  if ((pf->fontattr & TF_KERNING) && pf->can_kern) {
	     x += compute_kernval(pf, curchar) / 64;
	  }		
	  x += metrics.advance / 64;
	  if (pf->last_pen_pos > x) x = pf->last_pen_pos;
	  pf->last_pen_pos = x;
	  pf->last_glyph_code = curchar;
      }
      *pwidth = x;
      *pheight = (((properties.horizontal->Ascender * imetrics.y_scale)/ 0x10000) >> 6) -
	            (((properties.horizontal->Descender * imetrics.y_scale)/ 0x10000) >> 6);
      *pbase = (((-properties.horizontal->Descender) * imetrics.y_scale)/ 0x10000) >> 6;
}


static void freetype_destroyfont (PFONT pfont)
{
      PFREETYPEFONT pf = (PFREETYPEFONT)pfont;
      TT_Close_Face(pf->face);
      free(pf);
}


static void freetype_setfontsize (PFONT pfont, int fontsize)
{
      PFREETYPEFONT pf = (PFREETYPEFONT)pfont;	
      pf->fontsize = fontsize;
      if (!fontsize) return;	
      TT_Set_Instance_PixelSizes( pf->instance, pf->fontsize,pf->fontsize, pf->fontsize * 64 );
}

static void freetype_setfontrotation (PFONT pfont, int tenthdegrees)
{
      PFREETYPEFONT pf = (PFREETYPEFONT)pfont;
      float angle;
      pf->fontrotation = tenthdegrees;
      TT_Set_Instance_Transform_Flags (pf->instance, TRUE, FALSE);
      angle = pf->fontrotation * M_PI / 1800;
      pf->matrix.yy = (TT_Fixed) (cos (angle) * (1 << 16));
      pf->matrix.yx = (TT_Fixed) (sin (angle) * (1 << 16));
      pf->matrix.xx = pf->matrix.yy;
      pf->matrix.xy = -pf->matrix.yx;
}


int GdGetTextSizeEx (PFONT pfont,const void *str,int cc,int nMaxExtent,int* lpnFit,int* alpDx,int *pwidth,int *pheight,int *pbase,unsigned long flags)
{
      unsigned short buf[256];
      unsigned short *text;
      PFREETYPEFONT pf = (PFREETYPEFONT)pfont;
      unsigned long defencoding = pf->fontprocs->encoding;
      int x = 0;
      int i;
      TT_UShort curchar;
      TT_Glyph_Metrics metrics;
      TT_Face_Properties properties;
      TT_Instance_Metrics imetrics;
      if ((cc<0) || (!str)) {
	 *pwidth = *pheight = *pbase = 0;	
 	 return 0;
      }
      if ((flags & (TF_PACKMASK | TF_DBCSMASK)) != defencoding) { 
         cc = GdConvertEncoding(str, flags, cc, buf, defencoding);
	 flags &= ~(TF_PACKMASK | TF_DBCSMASK);
	 flags |= defencoding;
	 text = buf;
      } else text =(unsigned short*)str;
      if (cc <= 0) {
         *pwidth = *pheight = *pbase = 0;
	 return 0;
      }
      TT_Get_Face_Properties (pf->face, &properties);
      TT_Get_Instance_Metrics(pf->instance, &imetrics);	
      pf->last_glyph_code = -1;		
      pf->last_pen_pos = -32767;
      if (lpnFit) *lpnFit=-1;
      for (i=0;i<cc;i++) {
	  curchar = TT_Char_Index (pf->char_map,text[i]);		
	  if (TT_Load_Glyph (pf->instance, pf->glyph, curchar,TTLOAD_SCALE_GLYPH|TTLOAD_HINT_GLYPH) != TT_Err_Ok) {		
	     DBG("Unable to load glyph with index=%d\n",curchar);    	
	     return 0;
	  }
	  TT_Get_Glyph_Metrics (pf->glyph, &metrics);
	  if ((pf->fontattr & TF_KERNING) && pf->can_kern) {
	     x += compute_kernval(pf, curchar) / 64;	
  	  }
	  x += metrics.advance / 64;
	  if ((lpnFit)&&(alpDx)) {
	     if (x<=nMaxExtent) alpDx[i]=x;else if (*lpnFit==-1) (*lpnFit)=i;               		     
	  }
	  if (pf->last_pen_pos > x) x = pf->last_pen_pos;
	  pf->last_pen_pos = x;
	  pf->last_glyph_code = curchar;
      }
      if ((lpnFit) && (*lpnFit==-1)) *lpnFit=cc;
      *pwidth = x;
      *pheight = (((properties.horizontal->Ascender * imetrics.y_scale)/ 0x10000) >> 6) -
	      (((properties.horizontal->Descender * imetrics.y_scale)/ 0x10000) >> 6);
      if (pbase) *pbase = (((-properties.horizontal->Descender) * imetrics.y_scale)/ 0x10000) >> 6;
      return 1;
}


static char *tt_lookup_name (TT_Face face)
{
      TT_Face_Properties prop;
      unsigned short i, n;
      unsigned short platform, encoding, language, id;
      char *string;
      char *name_buffer;
      unsigned short string_len;
      int j, found;
      int index = 4;
      int name_len;
      TT_Get_Face_Properties(face, &prop);
      n = prop.num_Names;
      for (i=0;i<n;i++) {
	  TT_Get_Name_ID( face, i, &platform, &encoding, &language, &id );	
  	  TT_Get_Name_String( face, i, &string, &string_len );
	  if (id == index) {
	     found = 0;
	     if (platform == 3) {
		for (j=1;j>=0;j--) {
		    if (encoding == j) { 
		       if ((language & 0x3FF) == 0x009) {
			  found = 1;
			  break;
		       }
                    }
		}    
	     }	
     	     if (!found && platform == 0 && language == 0) found = 1;
	     if (found) {
	        if (string_len > 512) string_len = 512;
		name_len = 0;
		name_buffer = (char*)malloc((string_len / 2) + 1);		
		for (i=1;i<string_len;i+=2) name_buffer[name_len++] = string[i];
		name_buffer[name_len] = '\0';
		return name_buffer;
	     }
	  }
      }
      return NULL;
}


static char *get_tt_name (char *p)
{
      TT_Face face;
      char *ret;
      if (TT_Open_Face(engine, p, &face) != TT_Err_Ok) {
	 DBG("Error opening font: %s\n", p);
	 return NULL;
      }
      ret = tt_lookup_name(face);
      TT_Close_Face(face);
      return ret;
}


void FreeFontList (int n)
{
      int i;
      FONTLIST *g;
      for (i=0;i<n;i++) {
	  g = list[i];	
  	  if (g) {
	     if (g->name) free(g->name);
	     if (g->ttname) free(g->ttname);
	     free(g);
	  }
      }
      free(list);
      numfonts = 0;
}


void GetFontList (void)
{
      DIR *dir;
      struct dirent *dent;
      char *p, *ftmp;
      int pl, idx = 0;
      if (TT_Err_Ok != TT_Init_FreeType(&engine)) {
         DBG("Unable to initialize freetype\n");
	 numfonts = -1;
	 return ;
      }
      dir = opendir(FREETYPE_FONT_DIR);
      if (dir <= 0) {
	 DBG("Error opening font directory\n");
	 numfonts = -1;
	 return ;
      }
      while ((dent = readdir(dir)) != NULL) {
	    p = strrchr(dent->d_name, '.');
	    if (strcasecmp(p, ".ttf") == 0) idx++;
      }
      numfonts = idx;
      rewinddir(dir);
      list = (FONTLIST**)malloc(idx * sizeof(FONTLIST*));
      for (pl=0;pl<idx;pl++) list[pl] = (FONTLIST*)malloc(sizeof(FONTLIST));
      idx = 0;
      while ((dent = readdir(dir)) != NULL) {
      	    p = strrchr(dent->d_name, '.');
	    if (strcasecmp(p, ".ttf") == 0) {			
    	       p = 0;	
	       pl = strlen(FREETYPE_FONT_DIR) + strlen(dent->d_name) *	sizeof(char) + 2;
	       p = (char*)malloc(pl);
	       p = (char*)memset(p, '\0', pl);
	       p = (char*)strcat(p, FREETYPE_FONT_DIR);
	       p = (char*)strcat(p, "/");
	       p = (char*)strcat(p, dent->d_name);
	       if ((ftmp = get_tt_name(p)) != NULL) {		       
		  list[idx]->ttname = ftmp;		
  		  list[idx]->name = malloc(strlen(dent->d_name) + 1);
		  list[idx]->name = strcpy(list[idx]->name, dent->d_name);
		  idx++;
	       } 	       
	       free(p);
	    }
      }	
      closedir(dir);
}


void ttf_list_display (void)
{	
      int idx;	
      if (!numfonts) return;
      DBG("-------ttf--Font-list-----------\n");
      for (idx=0;idx<numfonts;idx++) {
          DBG(" %d: %s \n",idx, list[idx]->name);
      }
}


PFONT ttf_open (char *font_name, int height, int fontattr)
{
      int idx; 
      PFONT pfont;      
      if (!numfonts) {
	 return 0;
      }	      
      if ((height == 0) || (height > 128)) return 0;
      for (idx=0;idx<numfonts;idx++) {
          if (strcmp(list[idx]->name,font_name) == 0) {
	     break; 
	  }   
      }	      
      if (idx == numfonts) {
         DBG(" font %s not found \n",font_name);	 
         return 0;
      }	      
      if (freetype_init()) {
         pfont = (PFONT)freetype_createfont(font_name, height,fontattr);
         if (pfont) {
            pfont->fontattr |= TF_FREETYPE;
	    return pfont;
         } else {
            DBG("freetype_createfont: %s,%d not found\n",font_name, height);
            return 0;
         }
      } else {
         DBG("freetype_init: Error \n");
         return 0;
      }                                                                                                                
}	


void ttf_close (PFONT pfont)
{
      if (numfonts == 0) return;
      freetype_destroyfont(pfont);
      if (pfont) free(pfont);
}


int kor_len (char *str,int len)
{	
      int idx,jdx;
      jdx = 0;
      for (idx=0;idx<len;idx++) {
          if (str[idx] >= 0xB0) {
	     if (str[idx+1] >= 0xA0) {		     
                idx++;	    
	     }	     
	  } 	  
	  jdx++;
      }	      
      return jdx;
}


void ttf_printf (PFONT pfont, int fontattr, int x,int y, char *form,...)
{
      int len,tlen;	
      va_list argptr;	       		         
      va_start(argptr,form);
      memset(ttf_buff,0,128);
      memset(uni_buff,0,256);      
      vsprintf(ttf_buff,form,argptr);
      len = strlen(ttf_buff);           
      tlen = GdConvertEncoding(ttf_buff,TF_UTF8,len,uni_buff,TF_UC16);
      freetype_drawtext(pfont, x, y, uni_buff, tlen, fontattr);
      va_end(argptr);
}


void ttf_Size (PFONT pfont, int size)
{       
      GdSetFontSize(pfont,size);
}


void ttf_rotation (PFONT pfont, int degrees)
{	    
      GdSetFontRotation(pfont,degrees);
}


void ttf_attr (PFONT pfont, int set, int clr)
{	     
      GdSetFontAttr(pfont,set,clr);
}	
