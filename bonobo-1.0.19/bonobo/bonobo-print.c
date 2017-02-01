/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-print.c: Remote printing support, client side.
 *
 * Author:
 *     Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2000 Helix Code, Inc.
 */
#include <config.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-print.h>
#include <bonobo/bonobo-stream.h>
#include <bonobo/bonobo-stream-memory.h>

#include <libgnomeprint/gnome-print.h>

#undef PRINT_DEBUG

#define PARENT_TYPE BONOBO_X_OBJECT_TYPE

#define CLASS(o) BONOBO_PRINT_CLASS(GTK_OBJECT(o)->klass)

static inline BonoboPrint *
bonobo_print_from_servant (PortableServer_Servant _servant)
{
	if (!BONOBO_IS_PRINT (bonobo_object_from_servant (_servant)))
		return NULL;
	else
		return BONOBO_PRINT (bonobo_object_from_servant (_servant));
}

static Bonobo_Stream
impl_render (PortableServer_Servant        _servant,
	     const Bonobo_PrintDimensions *pd,
	     const Bonobo_PrintScissor    *scissor,
	     CORBA_Environment            *ev)
{
	GnomePrintMeta    *meta_context;
	BonoboPrint       *print;
	BonoboStream      *stream;
	void              *buffer;
	int                buf_len;
	GnomePrintContext *ctx;

#ifdef PRINT_DEBUG
	g_warning ("Rendering");
#endif
	print = bonobo_print_from_servant (_servant);
	g_return_val_if_fail (print != NULL, CORBA_OBJECT_NIL);

	g_return_val_if_fail (pd != CORBA_OBJECT_NIL, CORBA_OBJECT_NIL);

	meta_context = gnome_print_meta_new ();
		
#ifdef PRINT_DEBUG
	g_warning ("Render %g %g", pd->width, pd->height);
#endif

	ctx = GNOME_PRINT_CONTEXT (meta_context);

	gnome_print_gsave (ctx);

	if (print->render)
		print->render (ctx, pd->width, pd->height,
			       scissor, print->user_data);

	else if (CLASS (print)->render)
		CLASS (print)->render (ctx, pd->width, pd->height,
				       scissor, print->user_data);

	else
		g_warning ("No render method on print object");

	gnome_print_grestore (ctx);
	gnome_print_context_close (ctx);

	gnome_print_meta_access_buffer (meta_context,
					&buffer, &buf_len);

	/*
	 * FIXME: this does an expensive mem-copy that we could
	 * avoid easily with a custom stream.
	 */
	stream = bonobo_stream_mem_create (
		buffer, buf_len, TRUE, FALSE);

	gtk_object_unref (GTK_OBJECT (meta_context));

	return CORBA_Object_duplicate (BONOBO_OBJREF (stream), ev);
}

static void
bonobo_print_class_init (BonoboPrintClass *klass)
{
	POA_Bonobo_Print__epv *epv = &klass->epv;

	klass->render = NULL;

	epv->render = impl_render;
}

/**
 * bonobo_print_get_type:
 *
 * Returns: the GtkType for a BonoboPrint object.
 */
GtkType
bonobo_print_get_type (void)
{
	static GtkType type = 0;

	if (!type) {
		GtkTypeInfo info = {
			"BonoboPrint",
			sizeof (BonoboPrint),
			sizeof (BonoboPrintClass),
			(GtkClassInitFunc) bonobo_print_class_init,
			(GtkObjectInitFunc) NULL,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = bonobo_x_type_unique (
			PARENT_TYPE,
			POA_Bonobo_Print__init, NULL,
			GTK_STRUCT_OFFSET (BonoboPrintClass, epv),
			&info);
	}

	return type;
}

/**
 * bonobo_print_construct:
 * @p: the print object
 * @render: the render method
 * @user_data: the render method's user data
 * 
 * Construct @p setting its @render and @user_data pointers
 * 
 * Return value: a constructed BonoboPrint object
 **/
BonoboPrint *
bonobo_print_construct (BonoboPrint         *p,
			BonoboPrintRenderFn *render,
			gpointer             user_data)
{
	p->render       = render;
	p->user_data    = user_data;

	return p;
}

/**
 * bonobo_print_new:
 * @render: the render function
 * @user_data: the render's user data function
 * 
 * Create a new bonobo-print implementing BonoboObject
 * interface.
 *
 * This interface is called to ask a component to
 * @render itself to a print context with the specified
 * width and height, and scissoring data.
 * 
 * Return value: a new BonoboPrint interface
 **/
BonoboPrint *
bonobo_print_new (BonoboPrintRenderFn *render,
		  gpointer             user_data)
{
	BonoboPrint *p;

	p = gtk_type_new (bonobo_print_get_type ());
	g_return_val_if_fail (p != NULL, NULL);

	return bonobo_print_construct (p, render, user_data);
}
