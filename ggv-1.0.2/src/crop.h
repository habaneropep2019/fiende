/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/* Copyright (C) 1998 Redhat Software Inc. 
 * Authors: Jonathan Blandford <jrb@redhat.com>
 */
#ifndef __CROP_H__
#define __CROP_H__

#include <gnome.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
        
typedef struct _crop_data crop_data;
struct _crop_data {
        gint crop_x;
        gint crop_y;
        gint crop_w;
        gint crop_h;
        gint incrop;
        gint cstart;
};

void clear_crop(crop_data *cd, GdkWindow *win);
void crop_button_clicked (crop_data *cd, gint x, gint y);
void draw_crop (crop_data *cd, GdkWindow *win);
void mouse_moved (crop_data *cd, GdkWindow *win, gint x, gint y, gint width, gint height);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
