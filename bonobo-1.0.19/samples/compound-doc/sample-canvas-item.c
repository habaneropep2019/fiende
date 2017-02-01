/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include <config.h>

#include <bonobo/Bonobo.h>
#include <bonobo.h>

#define gray50_width 2
#define gray50_height 2
static char gray50_bits[] = {
  0x02, 0x01, };

#define SCALE 15.0

static GnomeCanvasItem *
make_hilbert (GnomeCanvasGroup *root)
{
	char hilbert[] = "urdrrulurulldluuruluurdrurddldrrruluurdrurddldrddlulldrdldrrurd";
	char *c;
	double *pp, *p;
	GnomeCanvasPoints *points;
	GdkBitmap *stipple;
	GnomeCanvasItem *item;

	points = gnome_canvas_points_new (strlen (hilbert) + 1);
	points->coords [0] = SCALE * 4;
	points->coords [1] = SCALE * 4;

	pp = points->coords;
	for (c = hilbert, p = points->coords + 2; *c; c++, p += 2, pp += 2)
		switch (*c) {
		case 'u':
			p[0] = pp[0];
			p[1] = pp[1] - SCALE;
			break;

		case 'd':
			p[0] = pp[0];
			p[1] = pp[1] + SCALE;
			break;

		case 'l':
			p[0] = pp[0] - SCALE;
			p[1] = pp[1];
			break;

		case 'r':
			p[0] = pp[0] + SCALE;
			p[1] = pp[1];
			break;
		}

	if (root->item.canvas->aa) {
		item = gnome_canvas_item_new (root,
					      gnome_canvas_line_get_type (),
					      "points", points,
					      "fill_color_rgba", 0x00ff0080,
					      "width_units", 8.0,
					      "cap_style", GDK_CAP_PROJECTING,
					      "join_style", GDK_JOIN_MITER,
					      NULL);
	} else {
		stipple = gdk_bitmap_create_from_data (NULL, gray50_bits, gray50_width, gray50_height);
		item = gnome_canvas_item_new (root,
					      gnome_canvas_line_get_type (),
					      "points", points,
					      "fill_color", "green",
					      "fill_stipple", stipple,
					      "width_units", 8.0,
					      "cap_style", GDK_CAP_PROJECTING,
					      "join_style", GDK_JOIN_MITER,
					      NULL);
		gdk_bitmap_unref (stipple);
	}
	
	gnome_canvas_points_free (points);

	return item;
}

static BonoboCanvasComponent *
item_factory (BonoboEmbeddable *bonobo_object, GnomeCanvas *canvas, void *data)
{
	GnomeCanvasItem *item;
	GnomeCanvasItem *group;
	
        group = gnome_canvas_item_new (
		GNOME_CANVAS_GROUP (gnome_canvas_root (canvas)),
		gnome_canvas_group_get_type (),
		"x1", 0.0,
		"y1", 0.0,
		NULL);

	item = make_hilbert (GNOME_CANVAS_GROUP (group));

/*        item = gnome_canvas_item_new (
		GNOME_CANVAS_GROUP (group),
		gnome_canvas_rect_get_type (),
		"x1", 0.0,
		"y1", 0.0,
		"x2", 40.0,
		"y2", 40.0,
		"outline_color", "red",
		"fill_color", "blue",
		NULL);*/

	return bonobo_canvas_component_new (group);
}

static BonoboObject *
bonobo_item_factory (BonoboGenericFactory *factory, void *closure)
{
	BonoboEmbeddable *server;

	server = bonobo_embeddable_new_canvas_item (item_factory, NULL);
	if (!server)
		g_error ("Can not create bonobo_embeddable");

	return (BonoboObject*) server;
}

BONOBO_OAF_FACTORY ("OAFIID:Bonobo_Sample_CanvasItem_Factory",
		    "bonobo-sample-canvas-item", VERSION, 
		    bonobo_item_factory,
		    NULL)
