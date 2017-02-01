/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/* Copyright (C) 1998 Redhat Software Inc. 
 * Authors: Jonathan Blandford <jrb@redhat.com>
 */
#include "crop.h"

void
clear_crop(crop_data *cd, GdkWindow *win)
{
        if ((cd->crop_w < 0) || (cd->crop_h < 0)) {
                gdk_window_clear(win);
                return;
        }
        
        if ((cd->crop_w >= 10) && (cd->crop_h >= 10))
                gdk_window_clear_area(win,
                                      cd->crop_x-1 + (cd->crop_w>>1) - 4,
                                      cd->crop_y-1 + (cd->crop_h>>1) - 4,
                                      8, 8);
        gdk_window_clear_area(win, cd->crop_x-8, cd->crop_y-8, 8, cd->crop_h+16);
        gdk_window_clear_area(win, cd->crop_x-8, cd->crop_y-8, cd->crop_w+16, 8);
        gdk_window_clear_area(win, cd->crop_x+cd->crop_w, cd->crop_y-8, 8 ,cd->crop_h+16);
        gdk_window_clear_area(win, cd->crop_x-8, cd->crop_y+cd->crop_h, cd->crop_w+16, 8);
}

void
crop_button_clicked (crop_data *cd, gint x, gint y)
{
        /* g_print (" in crop_button_clicked\n"); */
        if ((cd->crop_w>0)&&(cd->crop_h>0)) {
                if ((x>=cd->crop_x)&&(x<cd->crop_x+cd->crop_w)&&
                    (y>=cd->crop_y)&&(y<cd->crop_y+cd->crop_h)) {
                        cd->incrop=5;
                        cd->cstart=1;
                }
                else if ((x>=cd->crop_x-8)&&(x<cd->crop_x)&&
                         (y>=cd->crop_y-8)&&(y<cd->crop_y)) {
                        cd->incrop=1;
                        cd->cstart=1;
                }
                else if ((x>=cd->crop_x+cd->crop_w)&&(x<cd->crop_x+cd->crop_w+8)&&
                         (y>=cd->crop_y-8)&&(y<cd->crop_y)) {
                        cd->incrop=2;
                        cd->cstart=1;
                }
                else if ((x>=cd->crop_x+cd->crop_w)&&(x<cd->crop_x+cd->crop_w+8)&&
                         (y>=cd->crop_y+cd->crop_h)&&(y<cd->crop_y+cd->crop_h+8)) {
                        cd->incrop=3;
                        cd->cstart=1;
                }
                else if ((x>=cd->crop_x-8)&&(x<cd->crop_x)&&
                         (y>=cd->crop_y+cd->crop_h)&&(y<cd->crop_y+cd->crop_h+8)) {
                        cd->incrop=4;
                        cd->cstart=1;
                }
                else {
                        cd->incrop=0;
                        cd->crop_x=x;
                        cd->crop_y=y;
                        cd->crop_w=-1;
                        cd->crop_h=-1;
                        cd->cstart=1;
                }
        } else {
                cd->incrop=3;
                cd->crop_x=x;
                cd->crop_y=y;
                cd->crop_w=1;
                cd->crop_h=1;
                cd->cstart=1;
        }
}

void
draw_crop (crop_data *cd, GdkWindow *win)
{
        GdkGC *gc;
        GdkColor c1,c2;
        int r,g,b;
   
        if ((cd->crop_w < 0) || (cd->crop_h<0)) {
                gdk_window_clear(win);
                return;
        }
        gc = gdk_gc_new(win);
        gdk_gc_set_subwindow (gc, GDK_INCLUDE_INFERIORS);
        r=255,g=180,b=40;
        c1.pixel=gdk_imlib_best_color_match(&r,&g,&b);
        r=0,g=0,b=0;
        c2.pixel=gdk_imlib_best_color_match(&r,&g,&b);
        gdk_gc_set_foreground(gc,&c1);
        
        if ((cd->crop_w >= 10) && (cd->crop_h >= 10)) {
                gdk_gc_set_foreground(gc,&c2);
                gdk_draw_rectangle(win, gc, FALSE,
                                   cd->crop_x + (cd->crop_w>>1) - 5,
                                   cd->crop_y + (cd->crop_h>>1) - 5,
                                   7, 7);
                gdk_gc_set_foreground(gc,&c1);
                gdk_draw_rectangle(win, gc, TRUE,
                                   cd->crop_x + (cd->crop_w>>1) - 4,
                                   cd->crop_y + (cd->crop_h>>1) - 4,
                                   6, 6);
        }
        gdk_gc_set_foreground(gc, &c2);
        gdk_draw_rectangle(win, gc, FALSE,
                           cd->crop_x - 8,
                           cd->crop_y - 8,
                           7, 7);
        gdk_gc_set_foreground(gc, &c1);
        gdk_draw_rectangle(win, gc, TRUE,
                           cd->crop_x - 7,
                           cd->crop_y - 7,
                           6, 6);

        gdk_gc_set_foreground(gc, &c2);
        gdk_draw_rectangle(win, gc, FALSE,
                           cd->crop_x + cd->crop_w,
                           cd->crop_y - 8,
                           7, 7);
        gdk_gc_set_foreground(gc, &c1);
        gdk_draw_rectangle(win, gc, TRUE,
                           cd->crop_x + cd->crop_w + 1,
                           cd->crop_y - 7,
                           6, 6);

        gdk_gc_set_foreground(gc, &c2);
        gdk_draw_rectangle(win, gc, FALSE,
                           cd->crop_x + cd->crop_w,
                           cd->crop_y + cd->crop_h,
                           7, 7);
        gdk_gc_set_foreground(gc, &c1);
        gdk_draw_rectangle(win, gc, TRUE,
                           cd->crop_x + cd->crop_w + 1,
                           cd->crop_y + cd->crop_h + 1,
                           6, 6);

        gdk_gc_set_foreground(gc, &c2);
        gdk_draw_rectangle(win, gc, FALSE,
                           cd->crop_x - 8,
                           cd->crop_y + cd->crop_h,
                           7, 7);
        gdk_gc_set_foreground(gc, &c1);
        gdk_draw_rectangle(win, gc, TRUE,
                           cd->crop_x-7,
                           cd->crop_y + cd->crop_h + 1,
                           6, 6);
   
        gdk_gc_set_foreground(gc, &c2);
        gdk_draw_rectangle(win, gc, FALSE,
                           cd->crop_x - 3,
                           cd->crop_y-3,
                           cd->crop_w+5, 
                           cd->crop_h+5);
        gdk_draw_rectangle(win, gc, FALSE,
                           cd->crop_x - 1,
                           cd->crop_y - 1,
                           cd->crop_w + 1,
                           cd->crop_h + 1);
        gdk_gc_set_foreground(gc, &c1);
        gdk_draw_rectangle(win, gc, FALSE,
                           cd->crop_x - 2,
                           cd->crop_y - 2,
                           cd->crop_w + 3,
                           cd->crop_h + 3);
        gdk_flush();
        gdk_gc_destroy(gc);
}
void
mouse_moved (crop_data *cd, GdkWindow *win, gint x, gint y, gint width, gint height)
{
        int dx, dy;
        static int px=-999999,py=-999999;

        if (cd->cstart) {
                px=x;
                py=y;
                cd->cstart=0;
        }
        dx = x - px;
        dy = y - py;
        px = x;
        py = y;
        if (cd->incrop == 0)
                return;
        clear_crop(cd, win);
        switch(cd->incrop) {
        case 1:
                cd->crop_x+=dx;
                cd->crop_y+=dy;
                cd->crop_w-=dx;
                cd->crop_h-=dy;
                break;
        case 2:
                cd->crop_y+=dy;
                cd->crop_w+=dx;
                cd->crop_h-=dy;
                break;
        case 3:
                cd->crop_w+=dx;
                cd->crop_h+=dy;
                break;
        case 4:
                cd->crop_x+=dx;
                cd->crop_w-=dx;
                cd->crop_h+=dy;
                break;
        case 5:
                cd->crop_x+=dx;
                cd->crop_y+=dy;
                break;
        default:
                break;
        }
        if (cd->incrop>0) {
                if (cd->crop_x<0) cd->crop_x=0;
                if (cd->crop_y<0) cd->crop_y=0;
                if (cd->crop_w<1) cd->crop_w=1;
                if (cd->crop_h<1) cd->crop_h=1;
                if (cd->crop_w>width) cd->crop_w=width;
                if (cd->crop_h>height) cd->crop_h=height;
                if (cd->crop_x+cd->crop_w>width) cd->crop_x=width-cd->crop_w;
                if (cd->crop_y+cd->crop_h>height) cd->crop_y=height-cd->crop_h;
                draw_crop(cd, win);
        }
}
