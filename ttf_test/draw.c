/****************************************************************/
/* display.c                                                    */
/* lee,changwoo hl2irw@kpu.ac.kr                                */
/* 2010-May-18 +82-11-726-6860                                  */
/****************************************************************/
#include "fundef.h"
#include "color.h"

extern int maxx,maxy;
extern int han_ttfattr,han_ttfsize,eng_ttfattr,eng_ttfsize;
extern PFONT han_ttffont;
extern PFONT eng_ttffont;


void draw_status (void)
{
      norm_3d(  0,278,194,479,LIGHTGRAY);
      norm_3d(202,278,396,479,LIGHTGRAY);
      norm_3d(404,278,598,479,LIGHTGRAY);
      norm_3d(606,278,799,479,LIGHTGRAY);
}


void draw_page (void)
{
      norm_3d(0,0,maxx,34,LIGHTGRAY);
      set_color(BLACK); 
      set_bgcolor(LIGHTGRAY);
      two_line_3d(0,35,500,270,LIGHTBLUE,LIGHTGRAY);
      two_line_3d(500,35,maxx,270,BLACK,LIGHTGRAY);
      norm_3d(  0,278,194,479,LIGHTGRAY);
      norm_3d(202,278,396,479,LIGHTGRAY);
      norm_3d(404,278,598,479,LIGHTGRAY);
      norm_3d(606,278,799,479,LIGHTGRAY);
}

