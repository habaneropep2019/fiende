/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef __GPA_PRINTER_PRIVATE_H__
#define __GPA_PRINTER_PRIVATE_H__

BEGIN_GNOME_DECLS

#include "gpa-structs.h"

struct _GpaPrinter
{
	GtkObject object;

	gchar *name;
	gchar *id;

	GList *settings_list; /* List of GpaSettings */
	GpaModel *model;

	gboolean def;
};

GpaPrinter * gpa_printer_new (void);

END_GNOME_DECLS

#endif /* __GPA_PRINTER_PRIVATE_H__ */
