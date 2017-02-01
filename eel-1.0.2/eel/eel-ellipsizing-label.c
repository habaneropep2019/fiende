/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-ellipsizing-label.c: Subclass of GtkLabel that ellipsizes the text.

   Copyright (C) 2001 Eazel, Inc.

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: John Sullivan <sullivan@eazel.com>,
 */

#include <config.h>
#include "eel-ellipsizing-label.h"

#include "eel-gdk-font-extensions.h"
#include "eel-gtk-macros.h"
#include "eel-string.h"

struct EelEllipsizingLabelDetails
{
	char *full_text;
};

static void eel_ellipsizing_label_initialize_class (EelEllipsizingLabelClass *class);
static void eel_ellipsizing_label_initialize       (EelEllipsizingLabel      *label);
static void real_destroy                           (GtkObject                *object);
static void real_size_request                      (GtkWidget                *widget,
						    GtkRequisition           *requisition);
static void real_size_allocate                     (GtkWidget                *widget,
						    GtkAllocation            *allocation);
static void real_style_set                         (GtkWidget                *widget,
						    GtkStyle                 *previous_style);

EEL_DEFINE_CLASS_BOILERPLATE (EelEllipsizingLabel, eel_ellipsizing_label, GTK_TYPE_LABEL)

static void
eel_ellipsizing_label_initialize_class (EelEllipsizingLabelClass *klass)
{
	GtkWidgetClass *widget_class;
	GtkObjectClass *object_class;

	object_class = GTK_OBJECT_CLASS (klass);
	widget_class = GTK_WIDGET_CLASS (klass);

	object_class->destroy = real_destroy;

	widget_class->size_request = real_size_request;
	widget_class->size_allocate = real_size_allocate;
  	widget_class->style_set = real_style_set;
}

static void
eel_ellipsizing_label_initialize (EelEllipsizingLabel *label)
{
	label->details = g_new0 (EelEllipsizingLabelDetails, 1);
}

static void
real_destroy (GtkObject *object)
{
	EelEllipsizingLabel *label;

	g_return_if_fail (EEL_IS_ELLIPSIZING_LABEL (object));

	label = EEL_ELLIPSIZING_LABEL (object);

	g_free (label->details->full_text);
	g_free (label->details);
}

GtkWidget*
eel_ellipsizing_label_new (const char *string)
{
	EelEllipsizingLabel *label;
  
	label = gtk_type_new (EEL_TYPE_ELLIPSIZING_LABEL);
	eel_ellipsizing_label_set_text (label, string);
  
	return GTK_WIDGET (label);
}

static void
recompute_ellipsized_text (EelEllipsizingLabel *label, int width)
{
	char *ellipsized_text;

	if (label->details->full_text == NULL) {
		ellipsized_text = NULL;
	} else {
		ellipsized_text = eel_string_ellipsize (label->details->full_text, 
							GTK_WIDGET (label)->style->font, 
							width,
							EEL_ELLIPSIZE_MIDDLE);
	}

	gtk_label_set_text (GTK_LABEL (label), ellipsized_text);
	g_free (ellipsized_text);
}

void
eel_ellipsizing_label_set_text (EelEllipsizingLabel *label, 
				     const char *string)
{
	g_return_if_fail (EEL_IS_ELLIPSIZING_LABEL (label));

	if (eel_str_is_equal (string, label->details->full_text)) {
		return;
	}

	g_free (label->details->full_text);
	label->details->full_text = g_strdup (string);

	recompute_ellipsized_text (label, GTK_WIDGET (label)->allocation.width);
}

static void
real_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
	EEL_CALL_PARENT (GTK_WIDGET_CLASS, size_request, (widget, requisition));

	/* Don't demand any particular width; will draw ellipsized into whatever size we're given */
	requisition->width = 0;
}

static void	
real_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
	recompute_ellipsized_text (EEL_ELLIPSIZING_LABEL (widget), allocation->width);
	
	EEL_CALL_PARENT (GTK_WIDGET_CLASS, size_allocate, (widget, allocation));
}

static void 
real_style_set (GtkWidget *widget, GtkStyle  *previous_style)
{
	recompute_ellipsized_text (EEL_ELLIPSIZING_LABEL (widget), widget->allocation.width);
	
	EEL_CALL_PARENT (GTK_WIDGET_CLASS, style_set, (widget, previous_style));
}
       			      
