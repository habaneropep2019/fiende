#ifndef HELLO_OBJECT_PRINT_H
#define HELLO_OBJECT_PRINT_H

#include "hello-embeddable.h"
#include <libgnomeprint/gnome-print.h>

void hello_object_print (GnomePrintContext         *ctx,
			 double                     width,
			 double                     height,
			 const Bonobo_PrintScissor *scissor,
			 gpointer                   user_data);

#endif
