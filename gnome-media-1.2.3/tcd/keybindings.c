#include <gnome.h>

#include "keybindings.h"
#include "gtcd_public.h"
#include "prefs.h"

GList *keys=NULL;

void add_key_binding(GtkWidget *widget, char *signal, char *desc, Shortcut *key)
{
	KeyBinding *kb;

	kb = g_new(KeyBinding, 1);

	kb->widget = widget;
	kb->signal = signal;
	kb->desc = desc;
	kb->key = key;

	/* Add the accelerator both with and without the shift key.
	   This is necessary to support keys that can only be typed by
	   using the shift key.  It has the nice side effect of making
	   case irrelevant for alphabetic accelerator keys. */
	gtk_widget_add_accelerator(widget, signal, accel, key->key,
				   key->mods, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator(widget, signal, accel, key->key,
				   key->mods | GDK_SHIFT_MASK,
				   GTK_ACCEL_VISIBLE);

	keys = g_list_append(keys, kb);
}
