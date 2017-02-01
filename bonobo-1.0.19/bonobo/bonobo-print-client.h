/**
 * bonobo-print-client.h: a print client interface for compound documents.
 *
 * Author:
 *    Michael Meeks (mmeeks@gnu.org)
 *
 * Copyright 2000, Helix Code, Inc.
 */
#ifndef __BONOBO_PRINT_CLIENT_H__
#define __BONOBO_PRINT_CLIENT_H__

#include <stdarg.h>
#include <libgnome/gnome-defs.h>
#include <bonobo/bonobo-object-client.h>
#include <bonobo/bonobo-print.h>
#include <libgnomeprint/gnome-print.h>

BEGIN_GNOME_DECLS

#define BONOBO_PRINT_CLIENT_TYPE        (bonobo_print_client_get_type ())
#define BONOBO_PRINT_CLIENT(o)          (GTK_CHECK_CAST ((o), BONOBO_PRINT_CLIENT_TYPE, BonoboPrintClient))
#define BONOBO_PRINT_CLIENT_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_PRINT_CLIENT_TYPE, BonoboPrintClientClass))
#define BONOBO_IS_PRINT_CLIENT(o)       (GTK_CHECK_TYPE ((o), BONOBO_PRINT_CLIENT_TYPE))
#define BONOBO_IS_PRINT_CLIENT_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_PRINT_CLIENT_TYPE))

typedef struct {
	GtkObject    parent;
	Bonobo_Print corba_print;
} BonoboPrintClient;

typedef struct {
	GtkObjectClass			parent;
} BonoboPrintClientClass;

/* FIXME: at the least, this should be opaque */
typedef struct {
	double width;
	double height;
	
	double width_first_page;
	double width_per_page;
	double height_first_page;
	double height_per_page;

	GnomePrintMeta *meta_data;
} BonoboPrintData;

GtkType             bonobo_print_client_get_type           (void);
BonoboPrintClient  *bonobo_print_client_new                (Bonobo_Print         corba_print);
BonoboPrintClient  *bonobo_print_client_get                (BonoboObjectClient  *object);

void                bonobo_print_client_render             (BonoboPrintClient   *client,
							    BonoboPrintData     *pd);

BonoboPrintData    *bonobo_print_data_new                  (double               width,
							    double               height);

BonoboPrintData    *bonobo_print_data_new_full             (double               width,
							    double               height,
							    double               width_first_page,
							    double               width_per_page,
							    double               height_first_page,
							    double               height_per_page);

void                bonobo_print_data_free                 (BonoboPrintData     *pd);

GnomePrintMeta     *bonobo_print_data_get_meta             (BonoboPrintData     *pd);
void                bonobo_print_data_render               (GnomePrintContext   *pc,
							    double               x,
							    double               y,
							    BonoboPrintData     *pd,
							    double               meta_x,
							    double               meta_y);

END_GNOME_DECLS

#endif /* ! ___BONOBO_PRINT_CLIENT_H__ */
