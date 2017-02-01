#include <gtk/gtk.h>
#include "gladeconfig.h"

#ifdef USE_GNOME
#include <gnome.h>
#endif

#include "gb.h"
#include "gbwidget.h"
#include "source.h"
#include "glade_gnome.h"

/* These are used for outputting signal handler prototypes. */
#define GB_PARAM_INDENT		40
#define GB_PARAM_TYPE_WIDTH	16


static void gb_widget_write_signals_source (GtkWidget * widget,
					    GbWidgetWriteSourceData * data);
static void gb_widget_write_signal_connection_source (GbWidgetWriteSourceData * data,
						      const gchar *signal_name,
						      const gchar *connect_object,
						      gboolean connect_after,
						      const gchar *handler_data,
						      const gchar *handler);
static gchar *get_type_name (GtkType type, gboolean * is_pointer);
static gchar *get_gdk_event (gchar * signal_name);
static gchar **lookup_signal_arg_names (gchar * type, gchar * signal_name);
static void gb_widget_write_accelerators_source (GtkWidget * widget,
					    GbWidgetWriteSourceData * data);

/*************************************************************************
 * Functions for writing C source code
 *************************************************************************/

void
gb_widget_write_source (GtkWidget * widget,
			GbWidgetWriteSourceData * data)
{
  GtkWidget *parent;
  GbWidget *gbwidget;
  GladeWidgetData *widget_data;
  gchar *class_id, *child_name;
  gint source_len;

  /* This is a temporary(?) kludge so that the code for GtkDialogs is OK.
     We stop the source code for the action_area from being written.
     GtkDialog & GnomeDialog need to output the code for their vbox &
     action_area children themselves, since they need to output special code
     to access them, e.g. "GTK_DIALOG (dialog1)->vbox". However, the vbox
     is the parent of the action_area, and so we have to stop the action_area
     being output using the standard code since that won't work.
     The problem is due to the dialogs having 2 special children, where one
     is a descendant of the other. I don't think this occurs anywhere else. */
  child_name = gb_widget_get_child_name (widget);
  if (child_name && data->create_widget)
    {
      if (!strcmp (child_name, "Dialog:action_area")
	  || !strcmp (child_name, "GnomeDialog:action_area"))
	return;
    }

  parent = data->parent;

  class_id = gb_widget_get_class_id (widget);
  data->write_children = TRUE;

  widget_data = gtk_object_get_data (GTK_OBJECT (widget), GB_WIDGET_DATA_KEY);
  /* If this isn't a gbwidget, skip it. */
  if (widget_data)
    {
      gbwidget = gb_widget_lookup_class (class_id);
      g_return_if_fail (gbwidget != NULL);

      /* For custom widgets, we don't have a default widget to compare with,
	 so all properties should be set. */
      if (GLADE_IS_CUSTOM_WIDGET (widget))
	{
	  data->standard_widget = NULL;
	}
      else
	{
	  data->standard_widget = (GtkWidget *) g_hash_table_lookup (data->standard_widgets, class_id);
	  if (data->standard_widget == NULL)
	    {
#ifdef USE_GNOME
	      /* FIXME: GnomeLibs 1.0.1 workaround - gtk_object_newv doesn't
		 return a valid GnomeAppBar or GnomeDateEdit, so we create it
		 ourself. */
	      if (!strcmp (class_id, "GnomeAppBar"))
		data->standard_widget = gnome_appbar_new (TRUE, TRUE, GNOME_PREFERENCES_NEVER);
	      else if (!strcmp (class_id, "GnomeDateEdit"))
		data->standard_widget = gnome_date_edit_new ((time_t) 0, TRUE,
							     TRUE);
	      else
		data->standard_widget = GTK_WIDGET (gtk_object_newv (gtk_type_from_name (class_id), 0, NULL));
#else
	      data->standard_widget = GTK_WIDGET (gtk_object_newv (gtk_type_from_name (class_id), 0, NULL));
#endif
	      g_hash_table_insert (data->standard_widgets, class_id,
				   data->standard_widget);
	    }
	}

      data->real_wname = source_create_valid_identifier (gtk_widget_get_name (widget));
      if (data->use_component_struct && widget_data->public_field)
	data->wname = g_strdup_printf ("%s->%s", data->component_name,
				       data->real_wname);
      else
	data->wname = data->real_wname;

      data->widget_data = widget_data;
      if (gbwidget->gb_widget_write_source)
	(gbwidget->gb_widget_write_source) (widget, data);
      else
	source_add (data, "  /* Skipping %s: unimplemented. */\n", class_id);

      /* Make sure there is a blank line after each widget, for readability. */
      source_len = data->source_buffers[GLADE_SOURCE]->len;
      if (source_len > 2
	  && (data->source_buffers[GLADE_SOURCE]->str[source_len - 1] != '\n'
	      || data->source_buffers[GLADE_SOURCE]->str[source_len - 2] != '\n'))
	source_add (data, "\n");

      if (data->wname != data->real_wname)
	g_free (data->real_wname);
      g_free (data->wname);
      data->wname = NULL;
      data->real_wname = NULL;
      data->parent = widget;
    }
  else if (GB_IS_PLACEHOLDER (widget) && parent)
    {
      if (GTK_IS_NOTEBOOK (parent))
	{
	  /* SPECIAL CODE: If notebook pages are empty (i.e. placeholders),
	     we create dummy widgets instead, so it still runs OK. */
	  if (child_name == NULL)
	    {
	      gchar *wname, *parent_name;
	  
	      wname = "empty_notebook_page";
	      /* Make sure the dummy widget is declared. */
	      source_ensure_decl (data, "  GtkWidget *empty_notebook_page;\n");

	      parent_name = gtk_widget_get_name (parent);
	      parent_name = source_create_valid_identifier (parent_name);
	      source_add (data,
			  "  %s = gtk_vbox_new (FALSE, 0);\n"
			  "  gtk_widget_show (%s);\n"
			  "  gtk_container_add (GTK_CONTAINER (%s), %s);\n"
			  "\n",
			  wname, wname, parent_name, wname);
	      g_free (parent_name);
	    }
	  else
	    {
	      /* For empty notebook tabs, we increment the 'last_child' written
		 value. */
	      gint col = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (parent),
							       "last_child"));
	      gtk_object_set_data (GTK_OBJECT (parent), "last_child",
				   GINT_TO_POINTER (col + 1));
	    }
	}
      else if (GTK_IS_CLIST (parent))
	{
	  /* For empty clist/ctree titles, we increment the 'last_child'
	     written value. */
	  if (child_name && (!strcmp (child_name, "CList:title")
			     || !strcmp (child_name, "CTree:title")))
	    {
	      gint col = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (parent),
							       "last_child"));
	      gtk_object_set_data (GTK_OBJECT (parent), "last_child",
				   GINT_TO_POINTER (col + 1));
	    }
	}
    }

  /* Recursively write source for children.
     We need to reset the parent after the children have been written. */
  data->create_widget = TRUE;
  if (data->write_children)
    gb_widget_children_foreach (widget, (GtkCallback) gb_widget_write_source, data);

  /* We need to reset some of the members of the GbWidgetWriteSourceData struct
     so that they work OK for the next sibling. */
  data->parent = parent;

  /* SPECIAL CODE: Finish off the GnomeUIInfo struct if we are building a
     Gnome menu. */
#ifdef USE_GNOME
  if (data->project->gnome_support && GTK_IS_MENU_SHELL (widget))
    {
      glade_gnome_finish_menu_source (GTK_MENU_SHELL (widget), data);
    }
#endif
}


/* This is called by each GbWidget's write_source function to write all the
   common source code, including code to add the widget to its parent. */
void
gb_widget_write_standard_source (GtkWidget * widget,
				 GbWidgetWriteSourceData * data)
{
  GladeWidgetData *wdata = data->widget_data;
  gint i, width, height, can_focus, can_default;

  /* FIXME: unfinished. public fields were to be added to the component struct.
     Private fields would just be declared within the creation function. */
  if (wdata->public_field)
    source_add_decl (data, "  GtkWidget *%s;\n", data->real_wname);
  else
    source_add_decl (data, "  GtkWidget *%s;\n", data->real_wname);

  if (data->set_widget_names)
    source_add (data, "  gtk_widget_set_name (%s, \"%s\");\n", data->wname,
		data->real_wname);

  if (!data->use_component_struct)
    {
      /* For toplevel widgets we don't ref the widget, since if we do it never
	 gets destroyed. This may be a bug in GTK+ 1.2.3. */
      if (data->parent == NULL)
	{
	  source_add (data,
		      "  gtk_object_set_data (GTK_OBJECT (%s), %s, %s);\n",
		      data->component_name,
		      source_make_string (data->real_wname, FALSE),
		      data->wname);
	}
      else
	{
	  source_add (data,
		      "  gtk_widget_ref (%s);\n"
		      "  gtk_object_set_data_full (GTK_OBJECT (%s), %s, %s,\n"
		      "                            (GtkDestroyNotify) gtk_widget_unref);\n",
		      data->wname,
		      data->component_name,
		      source_make_string (data->real_wname, FALSE),
		      data->wname);
	}
    }

  /* SPECIAL CODE: menus are not shown here. */
  if (widget->parent && wdata->flags & GLADE_VISIBLE && !GTK_IS_MENU (widget))
    {
#ifdef USE_GNOME
      /* FIXME: GnomeDruidPageStandard bug workaround. It needs show_all(). */
      if (GNOME_IS_DRUID_PAGE_STANDARD (widget))
	source_add (data, "  gtk_widget_show_all (%s);\n", data->wname);
      else
	source_add (data, "  gtk_widget_show (%s);\n", data->wname);
#else
      source_add (data, "  gtk_widget_show (%s);\n", data->wname);
#endif
    }

  /* Output code to add widget to parent. */
  gb_widget_write_add_child_source (widget, data);

  if (wdata->flags & (GLADE_X_SET | GLADE_Y_SET))
    {
      /* SPECIAL CODE: Widgets in a GtkLayout are positioned with
	 gtk_layout_put(). */
      if (!widget->parent || !GTK_IS_LAYOUT (widget->parent))
	{
	  source_add (data, "  gtk_widget_set_uposition (%s, %i, %i);\n",
		      data->wname,
		      wdata->flags & GLADE_X_SET ? wdata->x : -2,
		      wdata->flags & GLADE_Y_SET ? wdata->y : -2);
	}
    }

  if (wdata->flags & (GLADE_WIDTH_SET | GLADE_HEIGHT_SET))
    {
      width = wdata->flags & GLADE_WIDTH_SET ? wdata->width : -2;
      height = wdata->flags & GLADE_HEIGHT_SET ? wdata->height : -2;

      /* GTK BUG WORKAROUND - a combo should manage the size of its entry.
	 I think this isn't needed any more (GTK+ 1.2.3). */
#if 0
      if (GTK_IS_COMBO (widget))
	source_add(data,
		   "  gtk_widget_set_usize (GTK_COMBO (%s)->entry, %i, %i);\n",
		   data->wname, width - 16 < 0 ? -2 : width - 16, height);
#endif

      source_add (data, "  gtk_widget_set_usize (%s, %i, %i);\n",
		  data->wname, width, height);
    }


  if (GTK_IS_CONTAINER (widget))
    {
      if (GTK_CONTAINER (widget)->border_width != 0)
	{
	  source_add (data,
		      "  gtk_container_set_border_width (GTK_CONTAINER (%s), %i);\n",
		      data->wname, GTK_CONTAINER (widget)->border_width);
	}
    }


  /* FIXME: Kludge to set separator menu items insensitive, so that they are
     skipped when using the cursor keys to move up/down the menu. */
  if (!(wdata->flags & GLADE_SENSITIVE)
      || (GTK_IS_MENU_ITEM (widget) && GTK_BIN (widget)->child == NULL))
    source_add (data, "  gtk_widget_set_sensitive (%s, FALSE);\n", data->wname);

  can_focus = GTK_WIDGET_CAN_FOCUS (widget);
  if (!data->standard_widget
      || can_focus != GTK_WIDGET_CAN_FOCUS (data->standard_widget))
    {
      if (can_focus)
	source_add (data, "  GTK_WIDGET_SET_FLAGS (%s, GTK_CAN_FOCUS);\n",
		    data->wname);
      /* SPECIAL CODE: toolbar widgets can't get focus by default. */
      else if (!gb_toolbar_is_toolbar_button (widget))
	source_add (data, "  GTK_WIDGET_UNSET_FLAGS (%s, GTK_CAN_FOCUS);\n",
		    data->wname);
    }
  can_default = GTK_WIDGET_CAN_DEFAULT (widget);
  if (!data->standard_widget
      || can_default != GTK_WIDGET_CAN_DEFAULT (data->standard_widget))
    {
      if (can_default)
	source_add (data, "  GTK_WIDGET_SET_FLAGS (%s, GTK_CAN_DEFAULT);\n",
		    data->wname);
      else
	source_add (data, "  GTK_WIDGET_UNSET_FLAGS (%s, GTK_CAN_DEFAULT);\n",
		    data->wname);
    }

  if (wdata->flags & GLADE_GRAB_FOCUS)
    {
      if (data->focus_widget)
	g_warning ("Multiple widgets with 'Has Focus' set: %s, %s",
		   data->focus_widget, data->real_wname);
      else
	data->focus_widget = g_strdup (data->wname);
    }

  if (wdata->flags & GLADE_GRAB_DEFAULT)
    {
      if (data->default_widget)
	g_warning ("Multiple widgets with 'Has Default' set: %s, %s",
		   data->default_widget, data->real_wname);
      else
	data->default_widget = g_strdup (data->wname);
    }

  /* SPECIAL CODE: toolbar children use the toolbar's tooltips. */
  if (wdata->tooltip && (!widget->parent || !GTK_IS_TOOLBAR (widget->parent)))
    {
      data->need_tooltips = TRUE;
      source_add (data, "  gtk_tooltips_set_tip (tooltips, %s, %s, NULL);\n",
		  data->wname,
		  source_make_string (wdata->tooltip, data->use_gettext));
    }

  if (!GTK_WIDGET_NO_WINDOW (widget))
    {
      GdkExtensionMode ext_mode = gtk_widget_get_extension_events (widget);
      gboolean written_first = FALSE;

      if (wdata->events)
	{
	  source_add (data, "  gtk_widget_set_events (%s, ", data->wname);
	  for (i = 0; i < GB_EVENT_MASKS_COUNT; i++)
	    {
	      if (wdata->events & GbEventMaskValues[i])
		{
		  if (!written_first)
		    source_add (data, "%s", GbEventMaskSymbols[i]);
		  else
		    source_add (data, " | %s", GbEventMaskSymbols[i]);
		  written_first = TRUE;
		}
	    }
	  source_add (data, ");\n");
	}

      if (ext_mode != GDK_EXTENSION_EVENTS_NONE)
	{
	  for (i = 0; GbExtensionModeChoices[i]; i++)
	    {
	      if (GbExtensionModeValues[i] == ext_mode)
		source_add (data, "  gtk_widget_set_extension_events (%s, %s);\n",
			    data->wname, GbExtensionModeSymbols[i]);
	    }
	}
    }

  gb_widget_write_signals_source (widget, data);
  gb_widget_write_accelerators_source (widget, data);
}


void
gb_widget_write_add_child_source (GtkWidget * widget,
				  GbWidgetWriteSourceData * data)
{
  GbWidget *gbparent;
  GtkWidget *parent;
  gchar *parent_name;

  /* If the widget is created automatically by its parent, we don't need
     to add source to add it. */
  if (!data->create_widget)
    return;

  /* SPECIAL CODE: to handle menus. */
  if (GTK_IS_MENU (widget))
    parent = gtk_menu_get_attach_widget (GTK_MENU (widget));
  else
    parent = data->parent;

  /* Return if no parent exists */
  if (!parent)
    return;

  parent_name = gtk_widget_get_name (parent);
  parent_name = source_create_valid_identifier (parent_name);

  MSG2 ("Adding %s to %s", data->wname, parent_name);

  /* If the GbWidget has its own function to output source code to add a child,
     we use that, else we just output the default "gtk_container_add ()". */
  gbparent = gb_widget_lookup (parent);
  if (gbparent && gbparent->gb_widget_write_add_child_source)
    {
      (gbparent->gb_widget_write_add_child_source) (parent, parent_name,
						    widget, data);
    }
  else
    {
      source_add (data, "  gtk_container_add (GTK_CONTAINER (%s), %s);\n",
		  parent_name, data->wname);
    }

  g_free (parent_name);
}


static void
gb_widget_write_signals_source (GtkWidget * widget,
				GbWidgetWriteSourceData * data)
{
  GList *item;
  GladeSignal *signal;
  gboolean skip_handler_code;
  gchar *handler;

  item = data->widget_data->signals;
  while (item)
    {
      signal = (GladeSignal *) item->data;
      item = item->next;
      skip_handler_code = FALSE;

      /* If we're appending new/changed signals and this is an old one, then
	 skip it. Note that we also skip it if the last mod time is 0, which
	 will happen for XML files created before the last mod tag was added.*/
      MSG2 ("Creating Signals:%s Handler: %s",
	    data->creating_callback_files ? "TRUE" : "FALSE", signal->handler);
      MSG1 ("LastMod:%s", ctime (&signal->last_modification_time));
      MSG1 ("LastWrite:%s", ctime (&data->last_write_time));

      if (!data->creating_callback_files
	  && (signal->last_modification_time <= data->last_write_time))
	{
	  MSG1 ("Skipping signal: %s", signal->handler);
	  skip_handler_code = TRUE;
	}

      if (!signal->name)
	{
	  /* FIXME: store warnings */
	  g_warning ("Signal name not set");
	  continue;
	}

      if (!signal->handler)
	{
	  /* FIXME: store warnings */
	  g_warning ("Signal handler not set");
	  continue;
	}

      /* Make sure handler function name is valid. */
      handler = source_create_valid_identifier (signal->handler);

      /* Output code to connect signal. */
      gb_widget_write_signal_connection_source (data, signal->name,
						signal->object, signal->after,
						signal->data, handler);

      /* We don't need to output code for standard GTK functions. */
      if (!strcmp (handler, "gtk_widget_show")
	  || !strcmp (handler, "gtk_widget_hide")
	  || !strcmp (handler, "gtk_widget_grab_focus")
	  || !strcmp (handler, "gtk_widget_destroy")
	  || !strcmp (handler, "gtk_window_activate_default")
	  || !strcmp (handler, "gtk_true")
	  || !strcmp (handler, "gtk_false")
	  || !strcmp (handler, "gtk_main_quit"))
	skip_handler_code = TRUE;


      /* If we're appending new/changed signals and this is an old one, we
	 don't want to output the source code for it. */
      if (!skip_handler_code)
	{
	  gb_widget_write_signal_handler_source (widget, data, signal->name,
						 handler);
	}
      g_free (handler);
    }
}


/* This outputs a signal handler declaration and empty function. handler
   should be a valid identifier. */
void
gb_widget_write_signal_handler_source (GtkWidget *widget,
				       GbWidgetWriteSourceData * data,
				       const gchar *signal_name,
				       const gchar *handler)
{
  guint signal_id;
  GtkSignalQuery *query_info;
  gint param, i;
  gchar buffer[1024];
  gchar *ret_type, *pos, *type_name, *arg_name, *object_name, *object_arg;
  gchar *object_arg_start;
  gboolean is_pointer;
  gint param_num, widget_num, event_num, callback_num;
  gint handler_len, object_name_len, type_name_len, num_spaces;
  gint *arg_num;
  gchar **arg_names;
  gchar *signal_name_copy;

  /* Check if we have already output the handler. */
  if (g_hash_table_lookup (data->handlers_output, handler))
    return;

  /* Remember that we have output the handler. */
  g_hash_table_insert (data->handlers_output, g_strdup (handler), "Output");


  signal_id = gtk_signal_lookup (signal_name, GTK_OBJECT_TYPE (widget));
  g_return_if_fail (signal_id != 0);

  query_info = gtk_signal_query (signal_id);
  g_return_if_fail (query_info != NULL);

  /* Output the return type and function name. */
  ret_type = get_type_name (query_info->return_val, &is_pointer);
  pos = buffer;
  sprintf (pos, "%s%s\n%s", ret_type, is_pointer ? "*" : "", handler);
  pos += strlen (pos);

  handler_len = strlen (handler);
  if (handler_len >= GB_PARAM_INDENT - 1)
    {
      *pos++ = '\n';
      for (i = 0; i < GB_PARAM_INDENT; i++)
	*pos++ = ' ';
    }
  else
    {
      num_spaces = GB_PARAM_INDENT - handler_len - 1;
      for (i = 0; i < num_spaces; i++)
	*pos++ = ' ';
    }

  widget_num = 0;

  /* Output the signal object type and the argument name. We assume the
     type is a pointer - I think that is OK. We remove "Gtk" and convert
     to lower case for the argument name. */
  object_name = gtk_type_name (query_info->object_type);
  sprintf (pos, "(%s ", object_name);
  pos += strlen (pos);
  object_name_len = strlen (object_name);
  if (object_name_len + 1 < GB_PARAM_TYPE_WIDTH)
    {
      num_spaces = GB_PARAM_TYPE_WIDTH - object_name_len - 1;
      for (i = 0; i < num_spaces; i++)
	*pos++ = ' ';
    }
  object_arg = (!strncmp (object_name, "Gtk", 3)) ? object_name + 3 : object_name;
  object_arg_start = pos;
  sprintf (pos, "*%s", object_arg);
  pos += strlen (pos);
  g_strdown (object_arg_start);
  if (!strcmp (object_arg_start, "widget"))
    widget_num++;
  sprintf (pos, ",\n");
  pos += strlen (pos);

  /* Convert signal name to use underscores rather than dashes '-'. */
  signal_name_copy = g_strdup (query_info->signal_name);
  for (i = 0; signal_name_copy[i]; i++)
    {
      if (signal_name_copy[i] == '-')
	signal_name_copy[i] = '_';
    }

  /* Output the signal parameters. */
  arg_names = lookup_signal_arg_names (object_name, signal_name_copy);

  param_num = 1;
  event_num = callback_num = 0;
  for (param = 0; param < query_info->nparams; param++)
    {
      for (i = 0; i < GB_PARAM_INDENT; i++)
	*pos++ = ' ';

      if (arg_names)
	{
	  sprintf (pos, "%s,\n", arg_names[param]);
	  pos += strlen (pos);
	}
      else
	{
	  type_name = get_type_name (query_info->params[param], &is_pointer);
	  /* Most arguments to the callback are called "arg1", "arg2", etc.
	     GtkWidgets are called "widget", "widget2", ...
	     GdkEvents are called "event", "event2", ...
	     GtkCallbacks are called "callback", "callback2", ... */
	  if (!strcmp (type_name, "GtkWidget"))
	    {
	      arg_name = "widget";
	      arg_num = &widget_num;
	    }
	  else if (!strcmp (type_name, "GdkEvent"))
	    {
	      type_name = get_gdk_event (signal_name_copy);
	      arg_name = "event";
	      arg_num = &event_num;
	      is_pointer = TRUE;
	    }
	  else if (!strcmp (type_name, "GtkCallback"))
	    {
	      arg_name = "callback";
	      arg_num = &callback_num;
	    }
	  else
	    {
	      arg_name = "arg";
	      arg_num = &param_num;
	    }
	  sprintf (pos, "%s ", type_name);
	  pos += strlen (pos);
	  type_name_len = strlen (type_name);
	  if (type_name_len + 1 < GB_PARAM_TYPE_WIDTH)
	    {
	      num_spaces = GB_PARAM_TYPE_WIDTH - type_name_len - 1;
	      for (i = 0; i < num_spaces; i++)
		*pos++ = ' ';
	    }
	  if (!arg_num || *arg_num == 0)
	    sprintf (pos, "%s%s,\n", is_pointer ? "*" : " ", arg_name);
	  else
	    sprintf (pos, "%s%s%i,\n", is_pointer ? "*" : " ", arg_name,
		     *arg_num);
	  pos += strlen (pos);
	  
	  if (arg_num)
	    *arg_num += 1;
	}
    }

  /* Add the final user_data parameter, common to all handlers. */
  for (i = 0; i < GB_PARAM_INDENT; i++)
    *pos++ = ' ';
  sprintf (pos, "gpointer         user_data)");
  pos += strlen (pos);

  /* Output the declaration of the handler, which uses the same buffer. */
  source_add_to_buffer (data, GLADE_CALLBACK_DECLARATIONS,
			"\n%s;\n", buffer);

  /* Output the empty handler function, returning FALSE if the return type is
     bool (i.e. for the GdkEvent handlers). */
  source_add_to_buffer (data, GLADE_CALLBACK_SOURCE,
			"\n%s\n{\n\n%s}\n\n",
			buffer,
			query_info->return_val == GTK_TYPE_BOOL
			? "  return FALSE;\n" : "");

  g_free (signal_name_copy);
  g_free (query_info);
}


static void
gb_widget_write_signal_connection_source (GbWidgetWriteSourceData * data,
					  const gchar *signal_name,
					  const gchar *connect_object,
					  gboolean connect_after,
					  const gchar *handler_data,
					  const gchar *handler)
{
  if (connect_object && connect_object[0])
    {
      if (connect_after)
	{
	  source_add_to_buffer (data, GLADE_SIGNAL_CONNECTIONS,
				"  gtk_signal_connect_object_after (GTK_OBJECT (%s), \"%s\",\n"
				"                                   GTK_SIGNAL_FUNC (%s),\n"
				"                                   GTK_OBJECT (%s));\n",
				data->wname, signal_name, handler,
				connect_object);
	}
      else
	{
	  source_add_to_buffer (data, GLADE_SIGNAL_CONNECTIONS,
				"  gtk_signal_connect_object (GTK_OBJECT (%s), \"%s\",\n"
				"                             GTK_SIGNAL_FUNC (%s),\n"
				"                             GTK_OBJECT (%s));\n",
				data->wname, signal_name, handler,
				connect_object);
	}
    }
  else
    {
      if (connect_after)
	{
	  source_add_to_buffer (data, GLADE_SIGNAL_CONNECTIONS,
				"  gtk_signal_connect_after (GTK_OBJECT (%s), \"%s\",\n"
				"                            GTK_SIGNAL_FUNC (%s),\n"
				"                            %s);\n",
				data->wname, signal_name, handler,
				handler_data ? handler_data : "NULL");
	}
      else
	{
	  source_add_to_buffer (data, GLADE_SIGNAL_CONNECTIONS,
				"  gtk_signal_connect (GTK_OBJECT (%s), \"%s\",\n"
				"                      GTK_SIGNAL_FUNC (%s),\n"
				"                      %s);\n",
				data->wname, signal_name, handler,
				handler_data ? handler_data : "NULL");
	}
    }
}


/* Returns the type name to use for a signal argument or return value, given
   the GtkType from the signal info. It also sets is_pointer to TRUE if the
   argument needs a '*' since it is a pointer. */
static gchar *
get_type_name (GtkType type, gboolean * is_pointer)
{
  gchar *type_name;

  *is_pointer = FALSE;
  type_name = gtk_type_name (type);

  switch (type) {
  case GTK_TYPE_NONE:
  case GTK_TYPE_CHAR:
  case GTK_TYPE_UCHAR:
  case GTK_TYPE_BOOL:
  case GTK_TYPE_INT:
  case GTK_TYPE_UINT:
  case GTK_TYPE_LONG:
  case GTK_TYPE_ULONG:
  case GTK_TYPE_FLOAT:
  case GTK_TYPE_DOUBLE:
  case GTK_TYPE_POINTER:
    /* These all have normal C type names so they are OK. */
    return type_name;

  case GTK_TYPE_STRING:
    /* A GtkString is really a gchar*. */
    *is_pointer = TRUE;
    return "gchar";

  case GTK_TYPE_ENUM:
  case GTK_TYPE_FLAGS:
    /* We use a gint for both of these. Hopefully a subclass with a decent
       name will be registered and used instead, as GTK+ does itself. */
    return "gint";

  case GTK_TYPE_BOXED:
    /* A boxed value is just an opaque pointer, I think. */
    return "gpointer";

  case GTK_TYPE_SIGNAL:
  case GTK_TYPE_ARGS:
  case GTK_TYPE_FOREIGN:
  case GTK_TYPE_CALLBACK:
  case GTK_TYPE_C_CALLBACK:
    /* FIXME: These are wrong. I think they expand into more than 1 argument.
       See the GtkArg struct in gtktypeutils.h and gtkargcollector.c.
       Fortunately I doubt anything uses these as signal args. */
    return "gpointer";

  default:
    break;
  }

  /* For all GtkObject subclasses we can use the class name with a "*",
     e.g. 'GtkWidget *'. */
  if (gtk_type_is_a (type, GTK_TYPE_OBJECT))
    *is_pointer = TRUE;

  return type_name;
}


/* Returns the type name to use for the GdkEvent arg of a signal handler,
   based on the signal name. This assumes that there will only be one GdkEvent
   arg to the signal handler, but that is OK since for the signals we support
   here we know that is true. If any new signals come along with more than one
   GdkEvent arg, it will just use "GdkEvent" for all of them, which is still
   OK. */
static gchar *
get_gdk_event (gchar * signal_name)
{
  static gchar *GbGDKEvents[] =
  {
    "button_press_event",		"GdkEventButton",
    "button_release_event",		"GdkEventButton",
    "motion_notify_event",		"GdkEventMotion",
    "delete_event",			"GdkEvent",
    "destroy_event",			"GdkEvent",
    "expose_event",			"GdkEventExpose",
    "key_press_event",			"GdkEventKey",
    "key_release_event",		"GdkEventKey",
    "enter_notify_event",		"GdkEventCrossing",
    "leave_notify_event",		"GdkEventCrossing",
    "configure_event",			"GdkEventConfigure",
    "focus_in_event",			"GdkEventFocus",
    "focus_out_event",			"GdkEventFocus",
    "map_event",			"GdkEvent",
    "unmap_event",			"GdkEvent",
    "property_notify_event",		"GdkEventProperty",
    "selection_clear_event",		"GdkEventSelection",
    "selection_request_event",		"GdkEventSelection",
    "selection_notify_event",		"GdkEventSelection",
    "proximity_in_event",		"GdkEventProximity",
    "proximity_out_event",		"GdkEventProximity",
    "drag_begin_event",			"GdkEventDragBegin",
    "drag_request_event",		"GdkEventDragRequest",
    "drag_end_event",			"GdkEventDragRequest",
    "drop_enter_event",			"GdkEventDropEnter",
    "drop_leave_event",			"GdkEventDropLeave",
    "drop_data_available_event",	"GdkEventDropDataAvailable",
    "other_event",			"GdkEventOther",
    "client_event",			"GdkEventClient",
    "no_expose_event",			"GdkEventNoExpose",
    NULL
  };

  gint i;

  for (i = 0; GbGDKEvents[i]; i += 2)
    {
      if (!strcmp (signal_name, GbGDKEvents[i]))
	return GbGDKEvents[i + 1];
    }
  return "GdkEvent";
}


/* This returns argument names to use for some known GTK signals. */
/* Note: Could possibly return the types to use as well, since several of
   them could be better, e.g. "GtkOrientation" instead of "gint". */
static gchar **
lookup_signal_arg_names (gchar * type, gchar * signal_name)
{
  /* Each arg array starts with the object type name and the signal name,
     and then signal arguments follow. Note that the spacing is hardcoded. */
  static gchar *GbArgTable[][8] =
  {
    {"GtkCList", "select_row",
     "gint             row",
     "gint             column",
     "GdkEvent        *event"},
    {"GtkCList", "unselect_row",
     "gint             row",
     "gint             column",
     "GdkEvent        *event"},
    {"GtkCList", "click_column",
     "gint             column"},

    {"GtkCList", "resize_column",
     "gint             column",
     "gint             width"},

    {"GtkCList", "extend_selection",
     "GtkScrollType    scroll_type",
     "gfloat           position",
     "gboolean         auto_start_selection"},
    {"GtkCList", "scroll_vertical",
     "GtkScrollType    scroll_type",
     "gfloat           position"},
    {"GtkCList", "scroll_horizontal",
     "GtkScrollType    scroll_type",
     "gfloat           position"},
    {"GtkContainer", "focus",
     "GtkDirectionType direction"},
    {"GtkCTree", "tree_select_row",
     "GList           *node",
     "gint             column"},
    {"GtkCTree", "tree_unselect_row",
     "GList           *node",
     "gint             column"},

    {"GtkCTree", "tree_expand",
     "GList           *node"},
    {"GtkCTree", "tree_collapse",
     "GList           *node"},
    {"GtkCTree", "tree_move",
     "GList           *node",
     "GList           *new_parent",
     "GList           *new_sibling"},
    {"GtkCTree", "change_focus_row_expansion",
     "GtkCTreeExpansionType expansion"},

    {"GtkEditable", "insert_text",
     "gchar           *new_text",
     "gint             new_text_length",
     "gint            *position"},
    {"GtkEditable", "delete_text",
     "gint             start_pos",
     "gint             end_pos"},
    {"GtkEditable", "set_editable",
     "gboolean         is_editable"},
    {"GtkEditable", "move_cursor",
     "gint             x",
     "gint             y"},
    {"GtkEditable", "move_word",
     "gint             num_words"},
    {"GtkEditable", "move_page",
     "gint             x",
     "gint             y"},
    {"GtkEditable", "move_to_row",
     "gint             row"},
    {"GtkEditable", "move_to_column",
     "gint             column"},

    {"GtkEditable", "kill_char",
     "gint             direction"},
    {"GtkEditable", "kill_word",
     "gint             direction"},
    {"GtkEditable", "kill_line",
     "gint             direction"},


    {"GtkInputDialog", "enable_device",
     "gint             deviceid"},
    {"GtkInputDialog", "disable_device",
     "gint             deviceid"},

    {"GtkListItem", "extend_selection",
     "GtkScrollType    scroll_type",
     "gfloat           position",
     "gboolean         auto_start_selection"},
    {"GtkListItem", "scroll_vertical",
     "GtkScrollType    scroll_type",
     "gfloat           position"},
    {"GtkListItem", "scroll_horizontal",
     "GtkScrollType    scroll_type",
     "gfloat           position"},

    {"GtkMenuShell", "move_current",
     "GtkMenuDirectionType direction"},
    {"GtkMenuShell", "activate_current",
     "gboolean         force_hide"},


    {"GtkNotebook", "switch_page",
     "GtkNotebookPage *page",
     "gint             page_num"},
    {"GtkStatusbar", "text_pushed",
     "guint            context_id",
     "gchar           *text"},
    {"GtkStatusbar", "text_popped",
     "guint            context_id",
     "gchar           *text"},
    {"GtkTipsQuery", "widget_entered",
     "GtkWidget       *widget",
     "gchar           *tip_text",
     "gchar           *tip_private"},
    {"GtkTipsQuery", "widget_selected",
     "GtkWidget       *widget",
     "gchar           *tip_text",
     "gchar           *tip_private",
     "GdkEventButton  *event"},
    {"GtkToolbar", "orientation_changed",
     "GtkOrientation   orientation"},
    {"GtkToolbar", "style_changed",
     "GtkToolbarStyle  style"},
    {"GtkWidget", "draw",
     "GdkRectangle    *area"},
    {"GtkWidget", "size_request",
     "GtkRequisition  *requisition"},
    {"GtkWidget", "size_allocate",
     "GtkAllocation   *allocation"},
    {"GtkWidget", "state_changed",
     "GtkStateType     state"},
    {"GtkWidget", "style_set",
     "GtkStyle        *previous_style"},

    {"GtkWidget", "install_accelerator",
     "gchar           *signal_name",
     "gchar            key",
     "gint             modifiers"},

    {"GtkWidget", "add_accelerator",
     "guint            accel_signal_id",
     "GtkAccelGroup   *accel_group",
     "guint            accel_key",
     "GdkModifierType  accel_mods",
     "GtkAccelFlags    accel_flags"},

    {"GtkWidget", "parent_set",
     "GtkObject       *old_parent"},

    {"GtkWidget", "remove_accelerator",
     "GtkAccelGroup   *accel_group",
     "guint            accel_key",
     "GdkModifierType  accel_mods"},

    {"GtkWidget", "debug_msg",
     "gchar           *message"},
    {"GtkWindow", "move_resize",
     "gint            *x",
     "gint            *y",
     "gint             width",
     "gint             height"},
    {"GtkWindow", "set_focus",
     "GtkWidget       *widget"},

    {"GtkWidget", "selection_get",
     "GtkSelectionData *data",
     "guint            info",
     "guint            time"},
    {"GtkWidget", "selection_received",
     "GtkSelectionData *data",
     "guint            time"},

    {"GtkWidget", "drag_begin",
     "GdkDragContext  *drag_context"},
    {"GtkWidget", "drag_end",
     "GdkDragContext  *drag_context"},
    {"GtkWidget", "drag_data_delete",
     "GdkDragContext  *drag_context"},
    {"GtkWidget", "drag_leave",
     "GdkDragContext  *drag_context",
     "guint            time"},
    {"GtkWidget", "drag_motion",
     "GdkDragContext  *drag_context",
     "gint             x",
     "gint             y",
     "guint            time"},
    {"GtkWidget", "drag_drop",
     "GdkDragContext  *drag_context",
     "gint             x",
     "gint             y",
     "guint            time"},
    {"GtkWidget", "drag_data_get",
     "GdkDragContext  *drag_context",
     "GtkSelectionData *data",
     "guint            info",
     "guint            time"},
    {"GtkWidget", "drag_data_received",
     "GdkDragContext  *drag_context",
     "gint             x",
     "gint             y",
     "GtkSelectionData *data",
     "guint            info",
     "guint            time"},

    {NULL}
  };

  gint i;

  for (i = 0; GbArgTable[i][0]; i++)
    {
      if (!strcmp (type, GbArgTable[i][0])
	  && !strcmp (signal_name, GbArgTable[i][1]))
	return &GbArgTable[i][2];
    }
  return NULL;
}


static void
gb_widget_write_accelerators_source (GtkWidget * widget,
				     GbWidgetWriteSourceData * data)
{
  GList *item = data->widget_data->accelerators;
  GladeAccelerator *accel;

  while (item)
    {
      accel = (GladeAccelerator *) item->data;
      item = item->next;

      /* The code to create the accel_group is output in source.c */
      data->need_accel_group = TRUE;
      source_add (data,
		  "  gtk_widget_add_accelerator (%s, \"%s\", accel_group,\n"
		  "                              GDK_%s, %s,\n"
		  "                              GTK_ACCEL_VISIBLE);\n",
		  data->wname, accel->signal, accel->key,
		  glade_util_create_modifiers_string (accel->modifiers));
    }
}

