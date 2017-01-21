/****************************************************************/
/* fundef.h                                                     */
/* lee,changwoo hl2irw@kpu.ac.kr                                */
/* 2010-May-18 +82-11-726-6860                                  */
/****************************************************************/
#include "fundef.h"
#include "color.h"

void norm_3d (int x1,int y1,int x2,int y2,int color)
{
      bar(x1,y1,x2,y2,color);
      
      line(x1,y2,x1,y1,WHITE);
      line(x1 + 1,y2 - 1,x1 + 1,y1,WHITE);
      line(x1 + 2,y2 - 2,x1 + 2,y1,WHITE);
      line(x1,y1,x2,y1,WHITE);
      line(x1,y1 + 1,x2 - 1,y1 + 1,WHITE);
      line(x1,y1 + 2,x2 - 2,y1 + 2,WHITE);
      line(x1,y2,x2,y2,DARKGRAY);
      line(x1 + 1,y2 - 1,x2,y2 - 1,DARKGRAY);
      line(x1 + 2,y2 - 2,x2,y2 - 2,DARKGRAY);
      line(x2,y2,x2,y1,DARKGRAY);
      line(x2 - 1,y2,x2 - 1,y1 + 1,DARKGRAY);
      line(x2 - 2,y2,x2 - 2,y1 + 2,DARKGRAY);
       
}


void box_3d (int x1, int y1, int x2, int y2, int out_color, int in_color)
{
      bar(x1,y1,x2,y2,out_color);
      line(x1,y2,x1,y1,WHITE);
      line(x1 + 1,y2 - 1,x1 + 1,y1,WHITE);
      line(x1 + 2,y2 - 2,x1 + 2,y1,WHITE);
      line(x1,y1,x2,y1,WHITE);
      line(x1,y1 + 1,x2 - 1,y1 + 1,WHITE);
      line(x1,y1 + 2,x2 - 2,y1 + 2,WHITE);
      line(x1, y2,x2,y2,DARKGRAY);
      line(x1 + 1,y2 - 1,x2,y2 - 1,DARKGRAY);
      line(x1 + 2,y2 - 2,x2,y2 - 2,DARKGRAY);
      line(x2,y2,x2,y1,DARKGRAY);
      line(x2 - 1,y2,x2 - 1,y1 + 1,DARKGRAY);
      line(x2 - 2,y2,x2 - 2,y1 + 2,DARKGRAY);
      if ((y2 - y1) >= 40) {
         line(x1 + 7,y2 - 15,x1 + 7,y1 + 6,DARKGRAY);
         line(x1 + 8,y2 - 16,x1 + 8,y1 + 6,DARKGRAY);
         line(x1 + 7,y1 + 6,x2 - 7,y1 + 6,DARKGRAY);
         line(x1 + 7,y1 + 7,x2 - 8,y1 + 7,DARKGRAY);
         line(x1 + 7,y2 - 15,x2 - 7,y2 - 15,WHITE);
         line(x1 + 8,y2 - 16,x2 - 7,y2 - 16,WHITE);
         line(x2 - 7,y2 - 15,x2 - 7,y1 + 6,WHITE);
         line(x2 - 8,y2 - 15,x2 - 8,y1 + 7,WHITE);
         bar(x1 + 9,y1 + 7,x2 - 9,y2 - 17,in_color);
      }
}


void vert_3d (int x1, int y1, int x2, int y2, int color)
{
      bar(x1,y1,x2,y2,color);
      line(x1,y2,x1,y1,DARKGRAY);
      line(x1 + 1,y2 - 1,x1 + 1,y1,DARKGRAY);
      line(x1 + 2,y2 - 2,x1 + 2,y1,DARKGRAY);
      line(x1,y1,x2,y1,DARKGRAY);
      line(x1,y1 + 1,x2 - 1,y1 + 1,DARKGRAY);
      line(x1,y1 + 2,x2 - 2,y1 + 2,DARKGRAY);
      line(x1,y2,x2,y2,WHITE);
      line(x1 + 1,y2 - 1,x2,y2 - 1,WHITE);
      line(x1 + 2,y2 - 2,x2,y2 - 2,WHITE);
      line(x2,y2,x2,y1,WHITE);
      line(x2 - 1,y2,x2 - 1,y1 + 1,WHITE);
      line(x2 - 2,y2,x2 - 2,y1 + 2,WHITE);
}


void shade (int x1, int y1, int x2, int y2, int color)
{
      bar(x1 + 8,y1 + 6,x2 + 8,y2 + 6,BLACK);
      bar(x1,y1,x2,y2,color);
      rectangle(x1,y1,x2,y2,WHITE);
}


void two_line_3d (int x1, int y1, int x2, int y2, int in_color, int out_color)
{
      norm_3d(x1,y1,x2,y2,out_color);
      line(x1 + 7,y1 + 7,x1 + 7,y2 - 7,DARKGRAY);
      line(x1 + 7,y1 + 7,x2 - 7,y1 + 7,DARKGRAY);
      line(x1 + 9,y1 + 9,x1 + 9,y2 - 9,DARKGRAY);
      line(x1 + 9,y1 + 9,x2 - 9,y1 + 9,DARKGRAY);
      line(x1 + 7,y2 - 7,x2 - 7,y2 - 7,WHITE);
      line(x2 - 7,y2 - 7,x2 - 7,y1 + 7,WHITE);
      line(x1 + 9,y2 - 9,x2 - 9,y2 - 9,WHITE);
      line(x2 - 9,y2 - 9,x2 - 9,y1 + 9,WHITE);
      bar(x1 + 10,y1 + 10,x2 - 10,y2 - 10,in_color);

}


void double_3d (int x1, int y1, int x2, int y2, int color)
{
      line(x1,y2,x1,y1,DARKGRAY);
      line(x1 + 1,y2 - 1,x1 + 1,y1,DARKGRAY);
      line(x1 + 2,y2 - 2,x1 + 2,y1,DARKGRAY);
      line(x1,y1,x2,y1,DARKGRAY);
      line(x1,y1 + 1,x2 - 1,y1 + 1,DARKGRAY);
      line(x1,y1 + 2,x2 - 2,y1 + 2,DARKGRAY);
      line(x1,y2,x2,y2,WHITE);
      line(x1 + 1,y2 - 1,x2,y2 - 1,WHITE);
      line(x1 + 2,y2 - 2,x2,y2 - 2,WHITE);
      line(x2,y2,x2,y1,WHITE);
      line(x2 - 1,y2,x2 - 1,y1 + 1,WHITE);
      line(x2 - 2,y2,x2 - 2,y1 + 2,WHITE);
      bar(x1 + 3,y1 + 3,x2 - 3,y2 - 3,color);
      line(x1 + 3,y2 - 3,x1 + 3,y1 + 3,WHITE);
      line(x1 + 4,y2 - 4,x1 + 4,y1 + 3,WHITE);
      line(x1 + 3,y1 + 3,x2 - 3,y1 + 3,WHITE);
      line(x1 + 3,y1 + 4,x2 - 4,y1 + 4,WHITE);
      line(x1 + 3,y2 - 3,x2 - 3,y2 - 3,DARKGRAY);
      line(x1 + 4,y2 - 4,x2 - 3,y2 - 4,DARKGRAY);
      line(x2 - 3,y2 - 3,x2 - 3,y1 + 3,DARKGRAY);
      line(x2 - 4,y2 - 3,x2 - 4,y1 + 4,DARKGRAY);
}


void round_3d (int x1, int y1, int x2, int y2, int color)
{
      bar(x1,y1,x2,y2,color);
      line(x1,y2,x1,y1,WHITE);
      line(x1 + 1,y2 - 1,x1 + 1,y1,WHITE);
      line(x1,y1,x2,y1,WHITE);
      line(x1,y1 + 1,x2 - 1,y1 + 1,WHITE);
      line(x1,y2,x2,y2,DARKGRAY);
      line(x1 + 1,y2 - 1,x2,y2 - 1,DARKGRAY);
      line(x2,y2,x2,y1,DARKGRAY);
      line(x2 - 1,y2,x2 - 1,y1 + 1,DARKGRAY);
      line(x1 + 2,y1 + 15,x1 + 7,y1 + 15,WHITE);
      line(x1 + 15,y1 + 2,x1 + 15,y1 + 7,WHITE);
      line(x2 - 2,y1 + 15,x2 - 7,y1 + 15,WHITE);
      line(x2 - 15,y1 + 2,x2 - 15,y1 + 7,WHITE);
      line(x1 + 2,y2 - 15,x1 + 7,y2 - 15,WHITE);
      line(x1 + 15,y2 - 2,x1 + 15,y2 - 7,WHITE);
      line(x2 - 2,y2 - 15,x2 - 7,y2 - 15,WHITE);
      line(x2 - 15,y2 - 2,x2 - 15,y2 - 7,WHITE);
      line(x2 - 8,y1 + 8, x2 - 8, y2 - 8,WHITE);
      line(x2 - 8,y2 - 8, x1 + 8, y2 - 8,WHITE);
      line(x1 + 2,y1 + 14,x1 + 7,y1 + 14,DARKGRAY);
      line(x1 + 14,y1 + 2,x1 + 14,y1 + 7,DARKGRAY);
      line(x2 - 2,y1 + 14,x2 - 7,y1 + 14,DARKGRAY);
      line(x2 - 14,y1 + 2,x2 - 14,y1 + 7,DARKGRAY);
      line(x1 + 2,y2 - 14,x1 + 7,y2 - 14,DARKGRAY);
      line(x1 + 14,y2 - 2,x1 + 14,y2 - 7,DARKGRAY);
      line(x2 - 2,y2 - 14,x2 - 7,y2 - 14,DARKGRAY);
      line(x2 - 14,y2 - 2,x2 - 14,y2 - 7,DARKGRAY);
      line(x1 + 8,y1 + 8,x2 - 8,y1 + 8,DARKGRAY);
      line(x1 + 8,y1 + 8,x1 + 8,y2 - 8,DARKGRAY);
}


void inline_3d (int x1, int y1, int x2, int y2, int color)
{
      line(x1,y1,x1,y2,DARKGRAY);
      line(x1,y1,x2,y1,DARKGRAY);
      line(x1 + 2,y1 + 2,x1 + 2,y2 - 2,DARKGRAY);
      line(x1 + 2,y1 + 2,x2 - 2,y1 + 2,DARKGRAY);
      line(x1,y2,x2,y2,WHITE);
      line(x2,y2,x2,y1,WHITE);
      line(x1 + 2,y2 - 2,x2 - 2,y2 - 2,WHITE);
      line(x2 - 2,y2 - 2,x2 - 2,y1 + 2,WHITE);
      bar(x1 +3,y1 + 3,x2 - 3,y2 - 3,color);
}


void line_3d (int x1, int y1, int x2, int y2)
{
      line(x1,y1,x1,y2,DARKGRAY);
      line(x1,y1,x2,y1,DARKGRAY);
      line(x1,y2,x2,y2,WHITE);
      line(x2,y2,x2,y1,WHITE);
}

