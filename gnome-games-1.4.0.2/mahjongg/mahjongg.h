/*
 * Gnome-Mahjonggg main header
 * (C) 1998-1999 the Free Software Foundation
 *
 *
 * Author: Francisco Bustamante et al.
 *
 *
 * http://www.nuclecu.unam.mx/~pancho/
 * pancho@nuclecu.unam.mx
 *
 */
#include <gnome.h>

#define MAX_TILES 144
#define MAX_TILES_STR "144"

struct _tilepos {
	int x;
	int y;
	int layer;
};
typedef struct _tilepos tilepos;     

extern tilepos *pos;


typedef struct _tile tile;

struct _tile{
  int type;
  int image;
  int visible;
  int selected;
  int sequence;
  int number;
  GdkImlibImage *current_image;
  GdkImlibImage *current_bg;
  GnomeCanvasItem *canvas_item;
  GnomeCanvasItem *image_item;
  GnomeCanvasItem *bg_item;
};

extern tile tiles[MAX_TILES];
