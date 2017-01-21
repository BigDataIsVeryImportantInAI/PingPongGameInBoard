/****************************************************************/
/* display.c                                                    */
/* lee,changwoo hl2irw@kpu.ac.kr                                */
/* 2010-May-18 +82-11-726-6860                                  */
/****************************************************************/
#include "fundef.h"
#include "engmd5.h"
#include "color.h"

#define eng_font engmd5_font

#define FB_DEV_NODE 		"/dev/fb0"
#define XSIZE           	800
#define YSIZE			480
#define disp_offset		80

int tx,ty;
int max_col,max_row;
int sys_color,maxx,maxy;
int fb_fd = -1;
int fb_length = 0;
unsigned short *fbmem = NULL;
unsigned short (*frameBuffer)[XSIZE];
char buff[256],dumy_data[32];
int ax,ay,fg_color,bg_color,eng_mode,page,pi;
int s_bit[8] = {0x80,0x40,0x20,0x10,8,4,2,1};
int clipminx, clipminy, clipmaxx, clipmaxy;
extern int gr_foreground,gr_background;

     
void init_display_data (void)
{
      maxx = XSIZE;
      maxy = YSIZE;
      ax = 0;
      ay = 0;
      max_col = (maxx / 8);
      max_row = (maxy / 16);
      eng_mode = 0;
      clipminx = 0;
      clipminy = 0;
      clipmaxx = maxx - 1;
      clipmaxy = maxy - 1;
}


void SetPixel (int x, int y, unsigned short color)
{
      if ((x < 0) || (x > maxx)) return;
      if ((y < 0) || (y > maxy)) return;
      frameBuffer[y][x] = color;
}


unsigned short GetPixel (int x, int y)
{
      if ((x < 0) || (x > maxx)) return 0;
      if ((y < 0) || (y > maxy)) return 0;
      return frameBuffer[y][x];
}


void out_lcd (char Wchar)
{
      int idx;
      for (idx=0;idx<8;idx++) {
          if ((s_bit[idx] & Wchar) == 0) SetPixel(tx,ty,bg_color); else SetPixel(tx,ty,fg_color);
          tx++;
      }
}


int *fb_memcpy (unsigned int *dst, unsigned int *src, unsigned int count)
{
      while (count--) {
     	    *dst = *src;
    	    dst++;
   	    src++;
      }
      return 0;
}


void scroll_up (void)
{
      unsigned int *da,*sa;
      int i, j;
      da = (unsigned int *)frameBuffer;
      if (sys_color == 24) {
         sa = da + XSIZE * 16;
         for (i=0;i<YSIZE;i++){
             fb_memcpy(da,sa,XSIZE);
             da += XSIZE;
             sa += XSIZE;
         }
         for (i=0;i<16;i++) {
             for (j=0;j<XSIZE;j++) {
                 *da = 0;
                 da++;
             }
             da += XSIZE;
         }
      }
      if (sys_color == 16) {
         sa = da + (XSIZE/2) * 16;
         for (i=0;i<YSIZE;i++){
             fb_memcpy(da,sa,(XSIZE/2));
             da += (XSIZE/2);
             sa += (XSIZE/2);
         }
         for (i=0;i<16;i++) {
             for (j=0;j<(XSIZE/2);j++) {
                 *da = 0;
                 da++;
             }
             da += (XSIZE/2);
         }
      }
}


void put_eng (char cdata)
{
      int val;
      if (cdata < 0x20) {
         if (cdata == 0x0D) ax = 0;
         if (cdata == 0x0A) {
            ay++;
            if (ay >= max_row) {
               scroll_up();
               ay -= 1;
               ax = 0;
            }
         }
      } else {
	 val = (int)cdata;
         ty = ay * 16;
         for (pi=0;pi<16;pi++) {
             tx = ax * 8;
             if (eng_mode == 0) {
                dumy_data[pi] = eng_font[val][pi];
             } else {
                if (page == 0) dumy_data[pi] = eng_font[val*2][pi];else dumy_data[pi] = eng_font[val*2+1][pi];
             }
             out_lcd(dumy_data[pi]);
             ty++;
         }
         ax++;
         if (ax >= max_col) {
            ax = 0;
            ay++;
            if (ay >= max_row) {
               scroll_up();
               ay -= 1;
            }
         }
      }
}


void put_engxy (int x,int y,char pdata)
{
      ax = x;
      ay = y;
      put_eng(pdata);
}


void lput_char (char *str)
{
      int len,n;
      len = strlen(str);
      for (n=0;n<len;n++) {
          put_eng(str[n]);
      }
}


void UIprintf (int x,int y,char *form,...)
{
      va_list argptr;
      ax = x;
      ay = y;
      va_start(argptr,form);
      vsprintf(buff,form,argptr);
      lput_char(buff);
      va_end(argptr);
}


unsigned int fb_grab (int fd, unsigned short **fbmem)
{
      struct fb_var_screeninfo modeinfo;
      unsigned int length;
      if (ioctl(fd, FBIOGET_VSCREENINFO, &modeinfo) < 0) {
         perror("FBIOGET_VSCREENINFO");
         exit (EXIT_FAILURE);
      }
      length = modeinfo.xres * modeinfo.yres * (modeinfo.bits_per_pixel >> 3);
      printf("  %d x %d, %d bpp\n\r",modeinfo.xres, modeinfo.yres, modeinfo.bits_per_pixel);
      maxx = modeinfo.xres;
      maxy = modeinfo.yres;
      sys_color = modeinfo.bits_per_pixel;
      *fbmem = mmap(0, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
      if (*fbmem < 0) {
         perror("mmap()");
         length = 0;
      }
      return length;
}


void fb_ungrab (unsigned short **fbmem, unsigned int length)
{
      if (*fbmem) munmap(*fbmem, length);
}


void clear_display (void)
{
      int *fb = (int *)fbmem;
      int idx;
      for (idx=0;idx<(XSIZE*YSIZE)/2;idx++) {
          *fb++ = (bg_color << 16) | bg_color;
      }
}


void clear_area (int y1, int y2)
{
      int *fb = (int *)fbmem;
      int idx;
      fb += (y1*(maxx/2));
      for (idx=(y1*(maxx/2));idx<(y2*(maxx/2));idx++) {
          *fb++ = (bg_color << 16) | bg_color;
      }
}


void set_color (int color)
{
      fg_color = color;
      gr_foreground = color;
}


void set_bgcolor (int color)
{
      bg_color = color;
      gr_background = color;
}


void draw_pixel (int x, int y, unsigned short color)
{
      SetPixel(x,y,color);
}


void draw_horz_line (int x1, int x2, int y, unsigned short color)
{
      int idx;
      for (idx=x1;idx<(x2+1);idx++) SetPixel(idx,y,color);
}


void draw_vert_line (int x, int y1, int y2, unsigned short color)
{
      int idx;
      for (idx=y1;idx<(y2+1);idx++) SetPixel(x,idx,color);
}


void fill_rectangle (int x1, int y1, int x2, int y2, int color)
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
          draw_horz_line(x1,x2,idx,color);
      }
}


void bar (int x1, int y1, int x2, int y2, int color)
{
      fill_rectangle(x1,y1,x2,y2,color);
}


void drawrow (int x1, int x2, int y, int color)
{
      int temp;
      if (x1 > x2) {
         temp = x1;
         x1 = x2;
         x2 = temp;
      }
      if (x1 < 0) x1 = 0;
      if (x2 >= maxx) x2 = maxx - 1;
      while (x1 <= x2) {
            temp = MIN(clipmaxx, x2);
            draw_horz_line(x1, temp, y, color);
            x1 = temp + 1;
      }
}


void drawcol (int x,int y1,int y2,int color)
{
      int temp;
      if (y1 > y2) {
         temp = y1;
         y1 = y2;
         y2 = temp;
      }
      if (y1 < 0) y1 = 0;
      if (y2 >= maxy) y2 = maxy - 1;
      while (y1 <= y2) {
            temp = MIN(clipmaxy, y2);
            draw_vert_line(x, y1, temp, color);
            y1 = temp + 1;
      }
}


void line (int x1, int y1, int x2, int y2 ,int color)
{
      int xdelta;
      int ydelta;
      int xinc;
      int yinc;
      int rem;
      int temp;
      if (y1 == y2) {
         if (x1 > x2) {
            temp = x1;
            x1 = x2 + 1;
            x2 = temp;
         } else --x2;
         drawrow(x1, x2, y1, color);
         return;
      }
      if (x1 == x2) {
         if (y1 > y2) {
            temp = y1;
            y1 = y2 + 1;
            y2 = temp;
         } else --y2;
         drawcol(x1, y1, y2, color);
         return;
      }
      xdelta = x2 - x1;
      ydelta = y2 - y1;
      if (xdelta < 0) xdelta = -xdelta;
      if (ydelta < 0) ydelta = -ydelta;
      xinc = (x2 > x1)? 1 : -1;
      yinc = (y2 > y1)? 1 : -1;
      draw_pixel(x1, y1, color);
      if (xdelta >= ydelta) {
         rem = xdelta / 2;
         for (;;) {
             if (x1 == x2) break;
             x1 += xinc;
             rem += ydelta;
             if (rem >= xdelta) {
                rem -= xdelta;
                y1 += yinc;
             }
             draw_pixel(x1, y1, color);
             if (x1 == x2) break;
         }
      } else {
         rem = ydelta / 2;
         for (;;) {
             if (y1 == y2) break;
             y1 += yinc;
             rem += xdelta;
             if (rem >= ydelta) {
                rem -= ydelta;
                x1 += xinc;
             }
             draw_pixel(x1, y1, color);
             if (y1 == y2) break;
         }
      }
}


void circle (int x,int y, int radius,int fill,int color)
{
      int a,b,P;
      a = 0;
      b = radius;
      P = 1 - radius;
      do {
          if (fill) {
             line(x-a,y+b,x+a,y+b,color);
             line(x-a,y-b,x+a,y-b,color);
             line(x-b,y+a,x+b,y+a,color);
             line(x-b,y-a,x+b,y-a,color);
          } else {
             draw_pixel(a+x,b+y,color);
             draw_pixel(b+x,a+y,color);
             draw_pixel(x-a,b+y,color);
             draw_pixel(x-b,a+y,color);
             draw_pixel(b+x,y-a,color);
             draw_pixel(a+x,y-b,color);
             draw_pixel(x-a,y-b,color);
             draw_pixel(x-b,y-a,color);
          }
          if (P < 0) P += (3 + 2 * a++);else P += (5 + 2 * (a++ - b--));
      } while (a <= b);
}


void ellipse (int x0,int y0,int a0,int b0,int fill,int color)
{
      int x = 0,y = b0;
      int a = a0,b = b0;
      int a_squ = a * a;
      int two_a_squ = a_squ << 1;
      int b_squ = b * b;
      int two_b_squ = b_squ << 1;
      int d,dx,dy;
      d = b_squ - a_squ * b + (a_squ >> 2);
      dx = 0;
      dy = two_a_squ * b;
      while (dx < dy) {
            if (fill) {
               line(x0-x,y0-y,x0+x,y0-y,color);
               line(x0-x,y0+y,x0+x,y0+y,color);
            } else {
               draw_pixel(x0+x,y0+y,color);
               draw_pixel(x0-x,y0+y,color);
               draw_pixel(x0+x,y0-y,color);
               draw_pixel(x0-x,y0-y,color);
            }
            if (d > 0) {
               y--;
               dy -= two_a_squ;
               d -= dy;
            }
            x++;
            dx += two_b_squ;
            d += b_squ + dx;
      }
      d += (3 * (a_squ - b_squ) / 2 - (dx + dy) / 2);
      while (y >= 0) {
            if (fill) {
               line(x0-x,y0-y,x0+x,y0-y,color);
               line(x0-x,y0+y,x0+x,y0+y,color);
            } else {
               draw_pixel(x0+x,y0+y,color);
               draw_pixel(x0-x,y0+y,color);
               draw_pixel(x0+x,y0-y,color);
               draw_pixel(x0-x,y0-y,color);
            }
            if (d < 0) {
               x++;
               dx += two_b_squ;
               d += dx;
            }
            y--;
            dy -= two_a_squ;
            d += a_squ - dy;
      }
}


void rectangle (int x1,int y1,int x2,int y2,int color)
{
      line(x1,y1,x2,y1,color);
      line(x2,y1,x2,y2,color);
      line(x1,y2,x2,y2,color);
      line(x1,y1,x1,y2,color);
}


void close_display (void)
{
      if (fb_fd) {
         fb_ungrab(&fbmem, fb_length);
         close(fb_fd);
      }
}


int init_display (void)
{
      fb_fd = open(FB_DEV_NODE, O_RDWR);
      if (fb_fd < 0) {
         printf(" FrameBuffer open fail....\n");
         return -1;
      }
      if ((fb_length = fb_grab(fb_fd, &fbmem)) == 0) return -2;
      printf(" Framebuffer memory address 0x%X  Size %d \n\r",(int)fbmem,fb_length);
      frameBuffer = (unsigned short (*)[XSIZE]) fbmem;
      init_display_data();
      fg_color = WHITE;
      bg_color = BLACK;
      clear_display();
      return 0;
}

