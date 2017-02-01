/*
 * Geometry string parsing code
 * Copyright (C) 1998 the Free Software Foundation
 *
 * Author: Miguel de Icaza
 */
#include <config.h>
#include <string.h>
#include <gtk/gtk.h>
#include <ctype.h>
#include "gnome-geometry.h"

static int
get_number (const char **geometry)
{
	int value = 0;
	int mult  = 1;
	
	if (**geometry == '-'){
		mult = -1;
		(*geometry)++;
	}
	while (**geometry && isdigit (**geometry)){
		value = value * 10 + (**geometry - '0');
		(*geometry)++;
	}
	return value * mult;
}

/*
 */

/**
 * gnome_parse_geometry
 * @geometry: geometry string to be parsed
 * @xpos: X position geometry component
 * @ypos: Y position geometry component
 * @width: pixel width geometry component
 * @height: pixel height geometry component
 *
 * Description:
 * Parses the geometry string passed in @geometry, and fills
 * @xpos, @ypos, @width, and @height with
 * the corresponding values upon completion of the parse.
 * If the parse fails, it should be assumed that @xpos, @ypos, @width,
 * and @height contain undefined values.
 *
 * Returns:
 * %TRUE if the geometry was successfully parsed, %FALSE otherwise.
 **/

gboolean
gnome_parse_geometry (const gchar *geometry, gint *xpos, 
		      gint *ypos, gint *width, gint *height)
{
	int subtract;

	g_return_val_if_fail (xpos != NULL, FALSE);
	g_return_val_if_fail (ypos != NULL, FALSE);
	g_return_val_if_fail (width != NULL, FALSE);
	g_return_val_if_fail (height != NULL, FALSE);
	
	*xpos = *ypos = *width = *height = -1;

	if (!geometry)
		return FALSE;

	if (*geometry == '=')
		geometry++;
	if (!*geometry)
		return FALSE;
	if (isdigit (*geometry))
		*width = get_number (&geometry);
	if (!*geometry)
		return TRUE;
	if (*geometry == 'x' || *geometry == 'X'){
		geometry++;
		*height = get_number (&geometry);
	}
	if (!*geometry)
		return 1;
	if (*geometry == '+'){
		subtract = 0;
		geometry++;
	} else if (*geometry == '-'){
		subtract = gdk_screen_width ();
		geometry++;
	} else
		return FALSE;
	*xpos = get_number (&geometry);
	if (subtract)
		*xpos = subtract - *xpos;
	if (!*geometry)
		return TRUE;
	if (*geometry == '+'){
		subtract = 0;
		geometry++;
	} else if (*geometry == '-'){
		subtract = gdk_screen_height ();
		geometry++;
	} else
		return FALSE;
	*ypos = get_number (&geometry);
	if (subtract)
		*ypos = subtract - *ypos;
	return TRUE;
}

/* lifted from gnomecal */

#define BUFSIZE 32

/**
 * gnome_geometry_string
 * @window: Pointer to window or dialog object
 *
 * Description:
 * Determines the size and position of @window (must be a window or
 * dialog), and returns that information as an X geometry string.
 * Geometry strings are in the form of WIDTHxHEIGHT+X+Y.
 *
 * Returns: Newly-allocated string containing geometry string for given
 * window.  Contents must be g_free'd.
 **/

gchar * gnome_geometry_string (GdkWindow * window)
{
  gint x, y, w, h;
  gchar *buffer = g_malloc (BUFSIZE + 1);
  
  gdk_window_get_root_origin (window, &x, &y);
  gdk_window_get_size (window, &w, &h);

  g_snprintf (buffer, BUFSIZE + 1, "%dx%d+%d+%d", w, h, x, y);
  
#ifdef GNOME_ENABLE_DEBUG
  g_print("Geometry string: %s\n", buffer);
#endif

  return buffer;
}

