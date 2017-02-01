/**
 * bonobo-print-client.c: a print client interface for compound documents.
 *
 * Author:
 *    Michael Meeks (mmeeks@gnu.org)
 *
 * Copyright 2000, Helix Code, Inc.
 */
#include <config.h>
#include <stdarg.h>
#include <libart_lgpl/art_affine.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-print-client.h>
#include <libgnomeprint/gnome-print-meta.h>
#include <bonobo/bonobo-stream-client.h>

#undef PRINT_DEBUG

/**
 * bonobo_print_client_render:
 * @client: the print client.
 * @pd: the intialized print data
 * 
 * This routine is used to encourage a remote print client
 * to print itself. The BonoboPrintData specifies the size
 * information for the remote client to render itself to.
 * After render the BonoboPrintData contains the meta data
 * for the rendered page. This interface is baroque.
 **/
void
bonobo_print_client_render (BonoboPrintClient   *client,
			    BonoboPrintData     *pd)
{
	Bonobo_Stream           stream;
	Bonobo_PrintDimensions *p_dim;
	Bonobo_PrintScissor    *p_scissor;
	CORBA_Environment       ev;
	CORBA_long              len;
	guint8                 *data;

	g_return_if_fail (pd != NULL);
	g_return_if_fail (client != NULL);
	g_return_if_fail (BONOBO_IS_PRINT_CLIENT (client));

	CORBA_exception_init (&ev);

	p_scissor = Bonobo_PrintScissor__alloc ();
	p_scissor->width_first_page  = pd->width_first_page;
	p_scissor->width_per_page    = pd->width_per_page;
	p_scissor->height_first_page = pd->height_first_page;
	p_scissor->height_per_page   = pd->height_per_page;

	p_dim = Bonobo_PrintDimensions__alloc ();
	p_dim->width  = pd->width;
	p_dim->height = pd->height;

	stream = Bonobo_Print_render (client->corba_print, p_dim,
				      p_scissor, &ev);

#ifdef PRINT_DEBUG
	{
		int i;

		printf ("Read data %d %d\n", data->_length, data->_maximum);
		for (i = 0; i < data->_length; i++)
			printf ("0x%x ", data->_buffer [i]);
		printf ("\n");
	}
#endif

	CORBA_free (p_dim);
	CORBA_free (p_scissor);

	if (BONOBO_EX (&ev)) {
		g_warning ("Component print exception");
		return;
	}
	if (stream == CORBA_OBJECT_NIL) {
		g_warning ("Component print returns no data");
		return;
	}

	data = bonobo_stream_client_read (stream, -1, &len, &ev);
	if (BONOBO_EX (&ev) || !data)
		g_warning ("Failed to read print data from stream");
	else {
		pd->meta_data = gnome_print_meta_new_from (data);

		g_free (data);
	}

	bonobo_object_release_unref (stream, &ev);

	CORBA_exception_free (&ev);
}

/**
 * bonobo_print_data_get_meta:
 * @pd: the print data
 * 
 * Return value: the meta_data from @pd
 **/
GnomePrintMeta *
bonobo_print_data_get_meta (BonoboPrintData *pd)
{
	g_return_val_if_fail (pd != NULL, NULL);

	if (!pd->meta_data)
		pd->meta_data = gnome_print_meta_new ();

	return pd->meta_data;
}

/**
 * bonobo_print_data_new_full:
 * @width: the width in pts of the component to render
 * @height: the height in pts of the component to render
 * @width_first_page: the clear width available on the first page
 * @width_per_page: the width available on subsequent pages
 * @height_first_page: the clear height available on the first page
 * @height_per_page: the height available on subsequent pages
 * 
 * This initializes a BonoboPrintData to contain the above
 * parameters so that it can be used by #bonobo_print_client_render
 * 
 * Return value: a new BonoboPrintData object.
 **/
BonoboPrintData *
bonobo_print_data_new_full (double               width,
			    double               height,
			    double               width_first_page,
			    double               width_per_page,
			    double               height_first_page,
			    double               height_per_page)
{
	BonoboPrintData *pd = g_new (BonoboPrintData, 1);

	pd->width  = width;
	pd->height = height;
	
	pd->width_first_page  = width_first_page;
	pd->width_per_page    = width_per_page;
	pd->height_first_page = height_first_page;
	pd->height_per_page   = height_per_page;

	pd->meta_data = NULL;

	return pd;
}

/**
 * bonobo_print_data_new:
 * @width: the width in pts of the component to render
 * @height: the height in pts of the component to render
 * 
 * This constructs a BonoboPrintData with default scissor data.
 * 
 * Return value: a new BonoboPrintData object.
 **/
BonoboPrintData *
bonobo_print_data_new (double               width,
		       double               height)
{
	return bonobo_print_data_new_full (width, height, width, 0.0, height, 0.0);
}

/**
 * bonobo_print_data_render:
 * @ctx: the context
 * @x: the tlc bbox x
 * @y: the ltc bbox y
 * @pd: the print data to render
 * @meta_x: the offset into the print data x
 * @meta_y: the offset into the print data y
 * 
 * This is used to render the print data in @pd
 * onto a GnomePrintContext in @ctx.
 **/
void
bonobo_print_data_render (GnomePrintContext   *ctx,
			  double               x,
			  double               y,
			  BonoboPrintData     *pd,
			  double               meta_x,
			  double               meta_y)
{
	double matrix[6];
	double w, h;

	g_return_if_fail (pd != NULL);
	g_return_if_fail (GNOME_IS_PRINT_CONTEXT (ctx));

	if (!pd->meta_data)
		return;

	g_return_if_fail (GNOME_IS_PRINT_META (pd->meta_data));

	if (meta_x == 0.0)
		w = pd->width_first_page;
	else
		w = pd->width_per_page;

	if (meta_y == 0.0)
		h = pd->height_first_page;
	else
		h = pd->height_per_page;

	gnome_print_gsave (ctx);
	/* FIXME: we need a clip region & a different translation ! */
	gnome_print_moveto (ctx, x, y);
	gnome_print_lineto (ctx, x + w, y);
	gnome_print_lineto (ctx, x + w, y + h);
	gnome_print_lineto (ctx, x, y + h);
	gnome_print_lineto (ctx, x, y);
	gnome_print_clip   (ctx);

	art_affine_translate (matrix, x - meta_x, y - meta_y);
	gnome_print_concat (ctx, matrix);

	if (!gnome_print_meta_render_from_object (ctx, pd->meta_data))
		g_warning ("Failed to meta render");

	gnome_print_grestore (ctx);

#if 0
	{
		gnome_print_gsave (ctx);
		gnome_print_setlinewidth (ctx, 0);
		gnome_print_setrgbcolor (ctx, 1.0, 0.0, 0.0);

		gnome_print_moveto (ctx, x, y);
		gnome_print_lineto (ctx, x + w, y);
		gnome_print_lineto (ctx, x + w, y + h);
		gnome_print_lineto (ctx, x, y + h);
		gnome_print_lineto (ctx, x, y);
		gnome_print_stroke (ctx);

		gnome_print_grestore (ctx);
	}
#endif
}

/**
 * bonobo_print_data_free:
 * @pd: the print data
 * 
 * Releases all resources associated with the print data.
 **/
void
bonobo_print_data_free (BonoboPrintData *pd)
{
	if (pd) {
		if (pd->meta_data)
			gtk_object_unref (GTK_OBJECT (pd->meta_data));
		pd->meta_data = NULL;
		g_free (pd);
	}
}

/**
 * bonobo_print_client_new:
 * @corba_print: a remote Bonobo_Print interface handle
 * 
 * Return value: a newly constructed BonoboPrintClient
 **/
BonoboPrintClient *
bonobo_print_client_new (Bonobo_Print corba_print)
{
	BonoboPrintClient *pbc;
	static gboolean warned = FALSE;

	g_return_val_if_fail (corba_print != CORBA_OBJECT_NIL, NULL);

	if (!warned) {
/* Still true, but to help Jon we'll keep quiet */
/*		g_warning ("The print client interface is horribly immature, use at your own risk");*/
		warned = TRUE;
	}

	pbc = gtk_type_new (bonobo_print_client_get_type ());

	pbc->corba_print = corba_print;

	return pbc;
}

/**
 * bonobo_print_client_get:
 * @object: the print client
 * 
 * This does a QI on a remote BonoboObjectClient object, and
 * if it supports the interface returns a newly constructed
 * BonoboPrintClient handle, otherwise NULL.
 * 
 * Return value: a new handle or NULL.
 **/
BonoboPrintClient *
bonobo_print_client_get (BonoboObjectClient *object)
{
	CORBA_Environment  ev;
	Bonobo_Print       interf;
	BonoboPrintClient *client = NULL;

	g_return_val_if_fail (object != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_OBJECT_CLIENT (object), NULL);

	CORBA_exception_init (&ev);

	interf = bonobo_object_client_query_interface (object, "IDL:Bonobo/Print:1.0", &ev);

	if (BONOBO_EX (&ev))
		g_warning ("Exception getting print interface");
	else if (interf == CORBA_OBJECT_NIL)
		g_warning ("No printing interface");
	else
		client = bonobo_print_client_new (interf);

	CORBA_exception_free (&ev);

	return client;
}

static void
bonobo_print_client_gtk_init (BonoboPrintClient *pbc)
{
	pbc->corba_print = CORBA_OBJECT_NIL;
}

/**
 * bonobo_print_client_get_type:
 *
 * Returns: The GtkType corresponding to the BonoboPrintClient
 * class.
 */
GtkType
bonobo_print_client_get_type (void)
{
	static GtkType type = 0;

	if (!type) {
		GtkTypeInfo info = {
			"BonoboPrintClient",
			sizeof (BonoboPrintClient),
			sizeof (BonoboPrintClientClass),
			(GtkClassInitFunc) NULL,
			(GtkObjectInitFunc) bonobo_print_client_gtk_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (gtk_object_get_type (), &info);
	}

	return type;
}
