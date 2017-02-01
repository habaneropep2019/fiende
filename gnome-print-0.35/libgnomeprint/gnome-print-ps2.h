/* -*- Mode: C; tab-width: 2; indent-tabs-mode: t; c-basic-offset: 2 -*- */
#ifndef __GNOME_PRINT_PS2_H__
#define __GNOME_PRINT_PS2_H__

#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gp-path.h>

BEGIN_GNOME_DECLS

#define GNOME_TYPE_PRINT_PS2            (gnome_print_ps2_get_type ())
#define GNOME_PRINT_PS2(obj)            (GTK_CHECK_CAST ((obj), GNOME_TYPE_PRINT_PS2, GnomePrintPs2))
#define GNOME_PRINT_PS2_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GNOME_TYPE_PRINT_PS2, GnomePrintPs2Class))
#define GNOME_IS_PRINT_PS2(obj)         (GTK_CHECK_TYPE ((obj), GNOME_TYPE_PRINT_PS2))
#define GNOME_IS_PRINT_PS2_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_PRINT_PS2))

typedef struct _GnomePrintPs2       GnomePrintPs2;
typedef struct _GnomePrintPs2Class  GnomePrintPs2Class;


GtkType 	      gnome_print_ps2_get_type (void);
GnomePrintPs2 * gnome_print_ps2_new (GnomePrinter *printer, const char *paper_name);


END_GNOME_DECLS

#endif /* __GNOME_PRINT_PS2_H__ */






