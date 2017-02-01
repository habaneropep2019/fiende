#ifndef SAMPLE_COMPONENT_IO_H
#define SAMPLE_COMPONENT_IO_H

#include <bonobo.h>

#include "container.h"
#include "component.h"

void object_load    (BonoboObjectClient *embeddable,
		     Bonobo_Stream       stream,
		     CORBA_Environment  *ev);

void object_save    (BonoboObjectClient *embeddable,
		     Bonobo_Stream       stream,
		     CORBA_Environment  *ev);

#endif
