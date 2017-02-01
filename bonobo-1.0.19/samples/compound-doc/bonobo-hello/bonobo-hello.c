/* $Id: bonobo-hello.c,v 1.16 2001/01/19 14:14:19 dietmar Exp $ */
/*
  Bonobo-Hello Copyright (C) 2000 ÉRDI Gergõ <cactus@cactus.rulez.org>
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 2
  (included in the RadioActive distribution in doc/GPL) as published by
  the Free Software Foundation.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "config.h"
#include <bonobo.h>

#include "hello-embeddable.h"

static BonoboObject*
hello_embeddable_factory (BonoboGenericFactory *f, gpointer data)
{
	HelloBonoboEmbeddable *embeddable;

	embeddable = gtk_type_new (HELLO_BONOBO_EMBEDDABLE_TYPE);

	g_return_val_if_fail(embeddable != NULL, NULL);

	embeddable = hello_bonobo_embeddable_construct (embeddable);

	return BONOBO_OBJECT (embeddable);
}

BONOBO_OAF_FACTORY ("OAFIID:Bonobo_Sample_Hello_EmbeddableFactory",
		    "bonobo hello", VERSION,
		    hello_embeddable_factory,
		    NULL)
