#ifndef GNOME_H
#define GNOME_H

#if defined(GNOME_INTERFACE_VERSION)
#if GNOME_INTERFACE_VERSION > 1
#error "You are picking up an old header file because of a bad include path."
#error "Please make sure `gnome-config --cflags ...` output is before"
#error "any other -I options in your compilation flags."
#endif
#endif

#include "gnomesupport.h"

#include <gtk/gtk.h>
#include <gdk_imlib.h>
#include "libgnome/libgnome.h"
#include "libgnomeui/libgnomeui.h"
#endif
