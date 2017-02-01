/*  Gtk+ User Interface Builder
 *  Copyright (C) 1999  Damon Chaplin
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <config.h>

#include <gnome.h>
#include "../gb.h"

/* Include the 21x21 icon pixmap for this widget, to be used in the palette */
#include "../graphics/gnome-about.xpm"

/*
 * This is the GbWidget struct for this widget (see ../gbwidget.h).
 * It is initialized in the init() function at the end of this file
 */
static GbWidget gbwidget;

static gchar *Copyright = "GnomeAbout::copyright";
static gchar *Authors = "GnomeAbout::authors";
static gchar *Comments = "GnomeAbout::comments";
static gchar *Logo = "GnomeAbout::logo";

static gchar *Modal = "GnomeAbout|GtkWindow::modal";
static gchar *WMName = "GnomeAbout|GtkWindow::wmclass_name";
static gchar *WMClass = "GnomeAbout|GtkWindow::wmclass_class";

/******
 * NOTE: To use these functions you need to uncomment them AND add a pointer
 * to the funtion in the GbWidget struct at the end of this file.
 ******/

/*
 * Creates a new GtkWidget of class GnomeAbout, performing any specialized
 * initialization needed for the widget to work correctly in this environment.
 * If a dialog box is used to initialize the widget, return NULL from this
 * function, and call data->callback with your new widget when it is done.
 * If the widget needs a special destroy handler, add a signal here.
 */
static GtkWidget*
gb_gnome_about_new (GbWidgetNewData *data)
{
  GtkWidget *new_widget;
  gchar *project_name;
  const gchar *authors[] = { NULL };

  project_name = glade_project_get_name (data->project);
  new_widget = gnome_about_new (project_name ? project_name : "",
				"x.x", NULL, authors, NULL, NULL);

  /* Make the About dialog modal by default. */
  gtk_object_set_data (GTK_OBJECT (new_widget), Modal, "TRUE");

  /* We connect a close signal handler which always returns TRUE so that
     the built-in close functionality is skipped. */
  gtk_signal_connect (GTK_OBJECT (new_widget), "close",
		      GTK_SIGNAL_FUNC (gtk_true), NULL);

  /* Now we connect our normal delete_event handler. */
  gtk_signal_connect (GTK_OBJECT (new_widget), "delete_event",
		      GTK_SIGNAL_FUNC (editor_close_window), NULL);

  return new_widget;
}


/*
 * Creates the components needed to edit the extra properties of this widget.
 */
static void
gb_gnome_about_create_properties (GtkWidget * widget, GbWidgetCreateArgData * data)
{
  /* We only support the Modal, WMName & WMClass properties for GnomeAbout
     dialogs. */
  gb_window_create_standard_properties (widget, data,
					NULL, NULL, NULL, Modal,
					NULL, NULL, NULL, NULL, NULL,
					WMName, WMClass);

  property_add_string (Copyright, _("Copyright:"), _("The copyright notice"));
  property_add_text (Authors, _("Authors:"), _("The authors of the package, one on each line"), 2);
  property_add_text (Comments, _("Comments:"), _("Additional information, such as a description of the package and its home page on the web"), 2);
  property_add_filename (Logo, _("Logo:"), _("The pixmap to use as the logo"));
}



/****************************************************************************
 * This is a horrible kludge, so we can update the properties of GnomeAbout
 * dynamically. I've copied code from gnome-about.c, and used some internal
 * GTK+ knowledge so we can get hold of the GnomeAboutInfo data structure
 * to change its contents. 
 * When gnome-about support dynamic configuration, we won't need this.
 ****************************************************************************/

#define GNOME_ABOUT_DEFAULT_WIDTH               100
#define GNOME_ABOUT_MAX_WIDTH                   600
#define BASE_LINE_SKIP                          4

typedef struct
{
	gchar	*title;
	gchar *copyright;
	GList *names;
	gchar *comments;
	GdkPixmap *logo;
	GdkBitmap *mask;
	gint logo_w, logo_h;
	gint w, h;
	GdkColor light_green;
	GdkFont *font_title,
		*font_copyright,
		*font_author,
		*font_names,
		*font_comments;
} GnomeAboutInfo;

typedef struct _GtkHandler		GtkHandler;
struct _GtkHandler
{
  guint		   id;
  GtkHandler	  *next;
  GtkHandler	  *prev;
  guint		   blocked : 20;
  guint		   object_signal : 1;
  guint		   after : 1;
  guint		   no_marshal : 1;
  guint16	   ref_count;
  guint16	   signal_id;
  GtkSignalFunc	   func;
  gpointer	   func_data;
  GtkSignalDestroy destroy_func;
};

GnomeAboutInfo*
find_gnome_about_info (GtkWidget *widget, GtkWidget **drawing_area)
{
  static GQuark gtk_handler_quark = 0;

  GtkWidget *vbox, *frame = NULL, *child_widget;
  GList *child;
  GtkHandler *handler;

  vbox = GNOME_DIALOG (widget)->vbox;
  g_return_val_if_fail (GTK_IS_VBOX (vbox), NULL);
  child = GTK_BOX (vbox)->children;
  while (child)
    {
      child_widget = ((GtkBoxChild*) (child->data))->widget;
      if (GTK_IS_FRAME (child_widget))
	{
	  frame = child_widget;
	  break;
	}
      child = child->next;
    }
  g_return_val_if_fail (frame != NULL, NULL);
  g_return_val_if_fail (GTK_IS_FRAME (frame), NULL);
  *drawing_area = GTK_BIN (frame)->child;
  g_return_val_if_fail (GTK_IS_DRAWING_AREA (*drawing_area), NULL);

  if (gtk_handler_quark == 0)
    gtk_handler_quark = g_quark_try_string ("gtk-signal-handlers");
  g_return_val_if_fail (gtk_handler_quark != 0, NULL);

  handler = gtk_object_get_data_by_id (GTK_OBJECT (*drawing_area),
				       gtk_handler_quark);
  g_return_val_if_fail (handler != NULL, NULL);
  while (handler)
    {
      if (handler->func_data
	  && !strcmp (gtk_signal_name (handler->signal_id), "expose-event"))
	return (GnomeAboutInfo*) (handler->func_data);
      handler = handler->next;
    }
  return NULL;
}

static void
gnome_about_calc_size (GnomeAboutInfo *gai)
{
	GList *name;
	gint num_pars, i, h, w, len[4], tmpl;
	const gchar *p;
	gfloat maxlen;

	w = GNOME_ABOUT_DEFAULT_WIDTH;
	h = 1;

	if (gai->title)
	{
		len[0] = gdk_string_measure (gai->font_title, gai->title);
		h += 4 + gai->font_title->descent + gai->font_title->ascent + 14;
	}
	else
		len[0] = 0;

	if (gai->copyright)
	{
		h += gai->font_copyright->descent + gai->font_copyright->ascent + 8;
		len[1] = gdk_string_measure (gai->font_copyright, gai->copyright);
	}
	else
		len[1] = 0;

	len[2] = 0;
	if (gai->names)
	{
		name = g_list_first (gai->names);
		while (name)
		{
			tmpl = gdk_string_measure (gai->font_names, name->data);
			if (tmpl > len[2])
				len[2] = tmpl;
			name = name->next;
		}
		tmpl = gdk_string_measure (gai->font_names, _("Authors: "));
		len[2] += tmpl + 15;
		h += g_list_length (gai->names) * 
			(gai->font_names->descent + 
			 gai->font_names->ascent + 
			 BASE_LINE_SKIP ) + 4;
	}

	maxlen = (gfloat) w;
	for (i=0; i<3; i++)
		if (len[i] > maxlen)
			maxlen = (gfloat) len[i];
    
	w = (int) (maxlen * 1.2);
	if ( w > GNOME_ABOUT_MAX_WIDTH )
		w = GNOME_ABOUT_MAX_WIDTH;

	if (gai->comments)
	{
		num_pars = 1;
		p = gai->comments;
		while (*p != '\0')
		{
			if (*p == '\n')
				num_pars++;
			p++;
		}
      
		i = gdk_string_measure (gai->font_comments, gai->comments);
		i /= w - 16;
		i += 1 + num_pars;;
		h += i * (gai->font_comments->descent + gai->font_comments->ascent);
	}

	gai->w = w+4;
	gai->h = h+3;

	return;
}  


/*
 * Gets the properties of the widget. This is used for both displaying the
 * properties in the property editor, and also for saving the properties.
 */
static void
gb_gnome_about_get_properties (GtkWidget *widget, GbWidgetGetArgData * data)
{
  gchar *logo_filename;
  GnomeAboutInfo *ai;
  GtkWidget *drawing_area;
  GList *elem;
  gchar *authors;
  gint len;

  gb_window_get_standard_properties (widget, data,
				     NULL, NULL, NULL, Modal,
				     NULL, NULL, NULL, NULL, NULL,
				     WMName, WMClass);

  logo_filename = gtk_object_get_data (GTK_OBJECT (widget), Logo);
  gb_widget_output_pixmap_filename (data, Logo, logo_filename);

  ai = find_gnome_about_info (widget, &drawing_area);
  if (ai == NULL)
    {
      g_warning ("Can't find GnomeAboutInfo to change properties");
      return;
    }

  gb_widget_output_translatable_string (data, Copyright, ai->copyright);

  len = 0;
  elem = ai->names;
  while (elem)
    {
      len += strlen ((gchar*)elem->data) + 1;
      elem = elem->next;
    }
  authors = g_malloc (len + 1);
  authors[0] = '\0';
  elem = ai->names;
  while (elem)
    {
      strcat (authors, (gchar*)elem->data);
      strcat (authors, "\n");
      elem = elem->next;
    }
  gb_widget_output_text (data, Authors, authors);
  g_free (authors);

  gb_widget_output_translatable_text (data, Comments, ai->comments);
}



/*
 * Sets the properties of the widget. This is used for both applying the
 * properties changed in the property editor, and also for loading.
 */
static void
gb_gnome_about_set_properties (GtkWidget * widget, GbWidgetSetArgData * data)
{
  GnomeAboutInfo *ai;
  GtkWidget *drawing_area;
  gchar *copyright, *authors, *comments;
  gint w,h;
  gchar *filename, *old_filename;

  gb_window_set_standard_properties (widget, data,
				     NULL, NULL, NULL, Modal,
				     NULL, NULL, NULL, NULL, NULL,
				     WMName, WMClass);

  ai = find_gnome_about_info (widget, &drawing_area);
  if (ai == NULL)
    {
      g_warning ("Can't find GnomeAboutInfo to change properties");
      return;
    }

  copyright = gb_widget_input_string (data, Copyright);
  if (data->apply)
    {
      g_free (ai->copyright);
      ai->copyright = g_strdup (copyright);
    }

  authors = gb_widget_input_text (data, Authors);
  if (data->apply)
    {
      GList *elem;
      gchar *pos, *authors_end;

      elem = ai->names;
      while (elem)
	{
	  g_free (elem->data);
	  elem = elem->next;
	}
      g_list_free (ai->names);
      ai->names = NULL;

      pos = authors;
      authors_end = &authors[strlen (authors)];
      while (pos < authors_end)
	{
	  gchar *author_end = strchr (pos, '\n');
	  if (author_end == NULL)
	    author_end = authors_end;
	  *author_end = '\0';
	  ai->names = g_list_append (ai->names, g_strdup (pos));
	  pos = author_end + 1;
	}
    }
  if (data->action == GB_APPLYING)
    g_free (authors);

  comments = gb_widget_input_text (data, Comments);
  if (data->apply)
    {
      g_free (ai->comments);
      ai->comments = g_strdup (comments);
    }
  if (data->action == GB_APPLYING)
    g_free (comments);

  filename = gb_widget_input_pixmap_filename (data, Logo);
  if (data->apply)
    {
      if (filename && filename[0] == '\0')
	filename = NULL;

      old_filename = gtk_object_get_data (GTK_OBJECT (widget), Logo);
      if (old_filename)
	{
	  glade_project_remove_pixmap (data->project, old_filename);
	  g_free (old_filename);
	}

      gtk_object_set_data (GTK_OBJECT (widget), Logo, g_strdup (filename));

      if (ai->logo)
	{
	  gdk_pixmap_unref (ai->logo);
	  ai->logo = NULL;

	  if (ai->mask)
	    {
	      gdk_bitmap_unref (ai->mask);
	      ai->mask = NULL;
	    }
	}

      if (filename)
	{
	  glade_project_add_pixmap (data->project, filename);

	  /* More code copied from gnome-about.c */
	  if (gdk_imlib_load_file_to_pixmap (filename, &ai->logo, &ai->mask))
	    {
	      gdk_window_get_size ((GdkWindow *) ai->logo,
				   &ai->logo_w, &ai->logo_h);
	    }
	}
    }
  if (data->action == GB_LOADING)
    g_free (filename);


  gnome_about_calc_size (ai);
  w = ai->w; h = ai->h;
  if (ai->logo)
    {
      h += 4 + ai->logo_h;
      ai->h = h;
      ai->w = MAX (w, (ai->logo_w + 6)); 
      w = ai->w;
    }
  gtk_widget_set_usize ( GTK_WIDGET (drawing_area), w, h);
}


/*
 * Adds menu items to a context menu which is just about to appear!
 * Add commands to aid in editing a GnomeAbout, with signals pointing to
 * other functions in this file.
 */
/*
static void
gb_gnome_about_create_popup_menu (GtkWidget * widget, GbWidgetCreateMenuData * data)
{

}
*/



/*
 * Writes the source code needed to create this widget.
 * You have to output everything necessary to create the widget here, though
 * there are some convenience functions to help.
 */
static void
gb_gnome_about_write_source (GtkWidget * widget, GbWidgetWriteSourceData * data)
{
  GnomeAboutInfo *ai;
  GtkWidget *drawing_area;
  GList *elem;
  gchar *copyright, *comments;
  gchar *logo_filename;

  ai = find_gnome_about_info (widget, &drawing_area);
  if (ai == NULL)
    {
      g_warning ("Can't find GnomeAboutInfo");
      return;
    }

  if (data->create_widget)
    {
      gchar *project_name = glade_project_get_name (data->project);

      /* Output the array of authors. */
      source_add_decl (data, "  const gchar *authors[] = {\n");
      elem = ai->names;
      while (elem)
	{
	  source_add_decl (data, "    %s,\n",
			   source_make_string ((gchar*)elem->data, FALSE));
	  elem = elem->next;
	}
      source_add_decl (data, "    NULL\n  };\n");

      copyright = g_strdup (source_make_string (ai->copyright ? ai->copyright
						: "", data->use_gettext));
      comments = g_strdup (source_make_string (ai->comments ? ai->comments :
					       "", data->use_gettext));
      source_add (data,
		  "  %s = gnome_about_new (%s, VERSION,\n"
		  "                        %s,\n"
		  "                        authors,\n"
		  "                        %s,\n",
		  data->wname,
		  source_make_string (project_name ? project_name : "", FALSE),
		  copyright, comments);

      logo_filename = gtk_object_get_data (GTK_OBJECT (widget), Logo);
      if (logo_filename && logo_filename[0])
	{
	  source_add (data,
		      "                        \"%s/%s\");\n",
		      data->program_name, g_basename (logo_filename));
	}
      else
	{
	  source_add (data,
		      "                        NULL);\n");
	}

      g_free (comments);
      g_free (copyright);
    }

  gb_widget_write_standard_source (widget, data);

  gb_window_write_standard_source (widget, data,
				   NULL, NULL, NULL, Modal,
				   NULL, NULL, NULL, NULL, NULL,
				   WMName, WMClass);
}



/*
 * Initializes the GbWidget structure.
 * I've placed this at the end of the file so we don't have to include
 * declarations of all the functions.
 */
GbWidget*
gb_gnome_about_init ()
{
  /* Initialise the GTK type */
  gnome_about_get_type();

  /* Initialize the GbWidget structure */
  gb_widget_init_struct(&gbwidget);

  /* Fill in the pixmap struct & tooltip */
  gbwidget.pixmap_struct = gnome_about_xpm;
  gbwidget.tooltip = _("Gnome About Dialog");

  /* Fill in any functions that this GbWidget has */
  gbwidget.gb_widget_new		= gb_gnome_about_new;
  gbwidget.gb_widget_create_properties	= gb_gnome_about_create_properties;
  gbwidget.gb_widget_get_properties	= gb_gnome_about_get_properties;
  gbwidget.gb_widget_set_properties	= gb_gnome_about_set_properties;
  gbwidget.gb_widget_write_source	= gb_gnome_about_write_source;
/*
  gbwidget.gb_widget_create_popup_menu	= gb_gnome_about_create_popup_menu;
*/

  return &gbwidget;
}

