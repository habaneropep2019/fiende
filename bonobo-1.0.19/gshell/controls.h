/*
 * Author:
 *   Martin Baulig <baulig@suse.de>
 *
 * Copyright 2000 SuSE GmbH.
 */

#ifndef _GSHELL_CONTROLS_H_
#define _GSHELL_CONTROLS_H_

#include <config.h>
#include <gnome.h>
#include <bonobo.h>

BonoboObjectClient *gshell_control_wrapper (const gchar *goad_id);

#endif
