/* Stripchart -- the gnome-utils stripchart plotting utility
 * Copyright (C) 2000 John Kodis <kodis@jagunet.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "chart-app.h"

#include <gnome-xml/parser.h>	/* XML input routines */
#include <gnome-xml/tree.h>	/* XML output routines */


static char *whitespace = " \t\r\n";

typedef struct
{
  Param_desc *desc;
  Param_page *page;
  Chart_app *app;
}
Param_name_page;

static void
set_entry(GtkWidget *entry, const char *text)
{
  gtk_entry_set_text(GTK_ENTRY(entry), text? text: "");
}

static void
set_color(ChartDatum *datum, char *color_name, int cnum)
{
  Chart *chart = datum->chart;

  gdk_color_parse(color_name, &datum->gdk_color[cnum]);
  gdk_color_alloc(chart->colormap, &datum->gdk_color[cnum]);
  datum->gdk_gc[cnum] = gdk_gc_new(GTK_WIDGET(chart)->window);
  gdk_gc_set_foreground(datum->gdk_gc[cnum], &datum->gdk_color[cnum]);
}

static void
on_color_set(GnomeColorPicker *picker,
  guint red, guint grn, guint blu, guint alpha, Param_page *page)
{
  int cnum;
  char color_name[20];
  sprintf(color_name, "#%04x%04x%04x", red, grn, blu);

  for (cnum = 0; cnum <= page->colors; cnum++)
    if (GNOME_COLOR_PICKER(page->color[cnum]) == picker)
      break;

  if (cnum < page->colors && page->strip_data && page->pen_data)
    {
      set_color(page->strip_data, color_name, cnum);
      set_color(page->pen_data, color_name, cnum);
    }
}

static void
add_color(Param_page *page)
{
  if (page->colors <= page->shown)
    {
      page->color = realloc(page->color,
	(page->colors + 1) * sizeof(*page->color));
      page->color[page->colors] = gnome_color_picker_new();
      gtk_box_pack_start(GTK_BOX(page->color_hbox),
	page->color[page->colors], FALSE, FALSE, 0);
      gtk_signal_connect(GTK_OBJECT(page->color[page->colors]),
	"color_set", on_color_set, page);
      page->colors++;
    }
  gtk_widget_show(page->color[page->shown++]);
}

static void
param_page_set_from_desc(Param_page *page, Param_desc *desc)
{
  int c;
  char *names, *color;

  set_entry(page->name, desc? desc->name: NULL);

  gtk_text_set_point(GTK_TEXT(page->desc), 0);
  gtk_text_forward_delete(GTK_TEXT(page->desc),
    gtk_text_get_length(GTK_TEXT(page->desc)));
  gtk_text_insert(GTK_TEXT(page->desc),
    NULL, NULL, NULL, desc? desc->desc: "", -1);

  set_entry(page->eqn, desc? desc->eqn: NULL);
  set_entry(page->fn, desc? desc->fn: NULL);
  set_entry(page->pattern, desc? desc->pattern: NULL);
  set_entry(page->top_min, desc? desc->top_min: NULL);
  set_entry(page->top_max, desc? desc->top_max: NULL);
  set_entry(page->bot_min, desc? desc->bot_min: NULL);
  set_entry(page->bot_max, desc? desc->bot_max: NULL);

  if (desc == NULL)
    goto RETURN;

  switch (str_to_scale_style(desc->scale))
    {
    case chart_scale_log:
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(page->log), TRUE);
      break;
    default:
    case chart_scale_linear:
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(page->linear), TRUE);
      break;
    }

  switch (str_to_plot_style(desc->plot))
    {
    case chart_plot_point:
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(page->point), TRUE);
      break;
    default:
    case chart_plot_line:
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(page->line), TRUE);
      break;
    case chart_plot_solid:
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(page->solid), TRUE);
      break;
    case chart_plot_indicator:
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(page->indicator), TRUE);
      break;
    }

  do {
    add_color(page);
  } while (desc && page->colors < desc->colors);

  names = g_strdup(desc->color_names);
  color = strtok(names, whitespace);
  for (c = 0; color != NULL; c++)
    {
      GdkColor rgb;
      if (gdk_color_parse(color, &rgb))
	gnome_color_picker_set_i16(GNOME_COLOR_PICKER(page->color[c]),
	  rgb.red, rgb.green, rgb.blue, 0xffff);
      color = strtok(NULL, whitespace);
    }
  g_free(names);

 RETURN:
  page->changed = FALSE; /* Only manual changes count. */
}

static Param_page *
get_current_page_param(GtkNotebook *notebook)
{
  GtkWidget *page;
  int pageno = gtk_notebook_get_current_page(notebook);

  if (pageno < 0)
    return NULL;

  page = gtk_notebook_get_nth_page(notebook, pageno);
  return (Param_page *)gtk_object_get_user_data(GTK_OBJECT(page));
}

/*
 * param_opt_ingest -- reads an XML parameter file into a
 * null-terminated array of Param_desc objects. 
 */
Param_desc **
param_desc_ingest(const char *fn)
{
  int item_count = 0;
  Param_desc **desc_ptr = g_malloc(sizeof(*desc_ptr));
  xmlDocPtr doc = xmlParseFile(fn);
  xmlNodePtr list, param, elem;

  if (!doc || doc->type != XML_DOCUMENT_NODE
  ||  !doc->root || doc->root->type != XML_ELEMENT_NODE
  ||  !streq(doc->root->name, "stripchart") )
    {
      static int complaints;
      if (complaints++ == 0)
	error("Can't parse parameter file \"%s\".\n", fn);
      return NULL;
    }

  for (list = doc->root->childs; list != NULL; list = list->next)
    if (list->type == XML_ELEMENT_NODE && streq(list->name, "parameter-list"))
      for (param = list->childs; param; param = param->next)
	if (param->type == XML_ELEMENT_NODE && streq(param->name, "parameter"))
	  {
	    Param_desc *desc = g_malloc0(sizeof(*desc));
	    for (elem = param->childs; elem; elem = elem->next)
	      {
		const char *key = elem->name;
		const char *val = elem->childs->content;

		if (streq(key, "name"))
		  desc->name = (char *)val;
		else if (streq(key, "description"))
		  desc->desc = (char *)val;
		else if (streq(key, "equation"))
		  desc->eqn = (char *)val;
		else if (streq(key, "filename"))
		  desc->fn = (char *)val;
		else if (streq(key, "pattern"))
		  desc->pattern = (char *)val;
		else if (streq(key, "top-min"))
		  desc->top_min = (char *)val;
		else if (streq(key, "top-max"))
		  desc->top_max = (char *)val;
		else if (streq(key, "bot-min"))
		  desc->bot_min = (char *)val;
		else if (streq(key, "bot-max"))
		  desc->bot_max = (char *)val;
		else if (streq(key, "scale"))
		  desc->scale = (char *)val;
		else if (streq(key, "plot"))
		  desc->plot = (char *)val;
		else if (streq(key, "color"))
		  {
		    char *color, *names = g_strdup((char *)val);
		    desc->color_names = g_strdup((char *)val);
		    desc->colors = 0;
		    color = strtok(names, whitespace);
		    while (color != NULL)
		      {
			desc->colors++;
			color = strtok(NULL, whitespace);
		      }
		    g_free(names);
		  }
		else
		  fprintf(stderr,
		    "%s: file %s: unrecognized tag \"%s\" containing \"%s\"\n",
		    prog_name, fn, key, val);
	      }
/*
  	    if (streq(desc->scale, "log")) desc->s_log = 1;
  	    if (streq(desc->type, "indicator")) desc->t_led = 1;
*/
	    desc_ptr = g_realloc(desc_ptr,
	      (item_count + 2) * sizeof(*desc_ptr));
	    desc_ptr[item_count++] = desc;
	  }

  desc_ptr[item_count] = NULL;
  return desc_ptr;
}

static char *
edit_str(GtkWidget *entry)
{
  char *s = gtk_editable_get_chars(GTK_EDITABLE(entry), 0, -1);
  return (s && *s) ? strdup(s) : NULL;
}

static void
page_to_desc(Param_page *page, Param_desc *desc)
{
  int c, len;

  if (!*gtk_editable_get_chars(GTK_EDITABLE(page->eqn), 0, -1)
    &&  !*gtk_editable_get_chars(GTK_EDITABLE(page->fn), 0, -1))
    return;

  desc->name = edit_str(page->name);
  desc->desc = edit_str(page->desc);
  desc->eqn = edit_str(page->eqn);
  desc->fn = edit_str(page->fn);
  desc->pattern = edit_str(page->pattern);
  desc->top_min = edit_str(page->top_min);
  desc->top_max = edit_str(page->top_max);
  desc->bot_min = edit_str(page->bot_min);
  desc->bot_max = edit_str(page->bot_max);

  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(page->log)))
    desc->scale = strdup("log");
  else
    desc->scale = strdup("linear");

  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(page->indicator)))
    desc->plot = strdup("indicator");
  else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(page->point)))
    desc->plot = strdup("point");
  else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(page->solid)))
    desc->plot = strdup("solid");
  else
    desc->plot = strdup("line");

  desc->colors = page->shown;
  desc->color_names = g_malloc(page->shown * 10);
  for (c = len = 0; c < page->shown; c++)
    {
      guint8 r, g, b;
      gnome_color_picker_get_i8(GNOME_COLOR_PICKER(page->color[c]),
	&r, &g, &b, NULL);
      len += sprintf(desc->color_names + len, "#%02x%02x%02x ", r, g, b);
    }
  if (len)
    desc->color_names[len - 1] = '\0';
}

static void
add_node(xmlNodePtr node, const char *key, const char *val)
{
  if (val && *val)
    xmlNewChild(node, NULL, key, val);
}

int
opts_to_file(Chart_app *app, char *fn)
{
  int p = 0, stat;
  xmlDocPtr doc;
  xmlNodePtr list, node;
  GtkWidget *nb_page;

  doc = xmlNewDoc("1.0");
  doc->root = xmlNewDocNode(doc, NULL, "stripchart", NULL);

  prefs_to_doc(app, doc);

  list = xmlNewChild(doc->root, NULL, "parameter-list", NULL);

  while ((nb_page = gtk_notebook_get_nth_page(app->notebook, p)) != NULL)
    {
      Param_page *page = gtk_object_get_user_data(GTK_OBJECT(nb_page));
      if (page->strip_data->active)
	{
	  Param_desc *desc = g_malloc0(sizeof(*desc));
	  page_to_desc(page, desc);
	  node = xmlNewChild(list, NULL, "parameter", NULL);

	  add_node(node, "name", desc->name);
	  add_node(node, "description", desc->desc);
	  add_node(node, "equation", desc->eqn);
	  add_node(node, "filename", desc->fn);
	  add_node(node, "pattern", desc->pattern);
	  add_node(node, "top-min", desc->top_min);
	  add_node(node, "top-max", desc->top_max);
	  add_node(node, "bot-min", desc->bot_min);
	  add_node(node, "bot-max", desc->bot_max);
	  add_node(node, "scale", desc->scale);
	  add_node(node, "plot", desc->plot);
	  add_node(node, "color", desc->color_names);
	  g_free(desc);
	}
      p++;
    }
  stat = xmlSaveFile(fn, doc);
#ifdef DEBUG
  printf("opts to file: wrote %d bytes to %s\n", stat, fn);
#endif
  xmlFreeDoc(doc);
  return stat;
}

static void
on_save(GtkWidget *w, Chart_app *app)
{
  if (opts_to_file(app, app->config_fn) <= 0)
    error("can't save parameters to file \"%s\".", app->config_fn);
  if (app->file_sel)
    gtk_widget_hide(app->file_sel);
}

static void
on_save_as_cancel(GtkWidget *w, Chart_app *app)
{
  gtk_widget_hide(app->file_sel);
}

static void
on_save_as_okay(GtkWidget *w, Chart_app *app)
{
  char *fn;

  free(app->config_fn);
  fn = gtk_file_selection_get_filename(GTK_FILE_SELECTION(app->file_sel));
  app->config_fn = strdup(fn);

  on_save(w, app);
}

static void
on_save_as(GtkMenuItem *menuitem, Chart_app *app)
{
  if (app->file_sel == NULL)
    {
      GtkWidget *fs = gtk_file_selection_new("Save As...");
      gtk_file_selection_hide_fileop_buttons(GTK_FILE_SELECTION(fs));
      gtk_file_selection_set_filename(GTK_FILE_SELECTION(fs), app->config_fn);
      gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(fs)->cancel_button),
	"clicked", GTK_SIGNAL_FUNC(on_save_as_cancel), app);
      gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(fs)->ok_button),
	"clicked", GTK_SIGNAL_FUNC(on_save_as_okay), app);
      app->file_sel = fs;
    }
  gtk_widget_show(app->file_sel);
}

static void
on_close(GtkMenuItem *menuitem, Chart_app *app)
{
  gtk_widget_hide(GTK_WIDGET(app->editor));
}

/*
 * on_option -- called when the user selects a canned parameter from
 * the Params window's Parameter menu.  Deactivate the current
 * parameter, if any, set the parameter options, and activate the new
 * parameter.
 */
static void
on_option(GtkMenuItem *menuitem, Param_name_page *po)
{
  GtkNotebook *notebook = po->app->notebook;
  int pageno = gtk_notebook_get_current_page(notebook);
  GtkWidget *nb_page = gtk_notebook_get_nth_page(notebook, pageno);
  Param_page *page = gtk_object_get_user_data(GTK_OBJECT(nb_page));
  ChartDatum *strip_datum = NULL, *pen_datum = NULL;

  param_page_set_from_desc(po->page, po->desc);
  if (po->desc->name && *po->desc->name)
    gtk_notebook_set_tab_label_text(notebook, nb_page, po->desc->name);

  strip_datum = chart_equation_add(CHART(po->app->strip),
    po->app->strip_param_group, po->desc, NULL, pageno,
    str_to_plot_style(po->desc->plot) != chart_plot_indicator);

  if (strip_datum)
    {
      pen_datum = chart_equation_add(CHART(po->app->pen),
	po->app->pen_param_group, po->desc, strip_datum->adj, pageno, FALSE);
    }

  if (strip_datum && pen_datum)
    {
      if (page->strip_data != NULL)
	chart_parameter_deactivate(CHART(po->app->strip), page->strip_data);
      page->strip_data = strip_datum;

      if (page->pen_data != NULL)
	chart_parameter_deactivate(CHART(po->app->strip), page->pen_data);
      page->pen_data = pen_datum;
    }
}

static GtkWidget *
param_options_menu(Chart_app *app, Param_page *page)
{
  static Param_desc **descs;
  Param_desc **desc;
  GtkWidget *menu;

  if (descs == NULL)
    descs = param_desc_ingest(app->params_fn);

  menu = gtk_menu_new();
  for (desc = descs; desc && *desc; desc++)
    {
      Param_name_page *pg_opt = g_malloc(sizeof(*pg_opt));
      GtkWidget *menuitem = gtk_menu_item_new_with_label((*desc)->name);
      gtk_widget_show(menuitem);
      gtk_menu_append(GTK_MENU(menu), menuitem);
      pg_opt->page = page;
      pg_opt->desc = *desc;
      pg_opt->app = app;
      gtk_signal_connect(GTK_OBJECT(menuitem),
	"activate", GTK_SIGNAL_FUNC(on_option), pg_opt);
    }

  return menu;
}

static void
on_add_color(GtkMenuItem *menuitem, Chart_app *app)
{
  Param_page *page = get_current_page_param(app->notebook);
  add_color(page);
}

static void
on_delete_color(GtkMenuItem *menuitem, Chart_app *app)
{
  Param_page *page = get_current_page_param(app->notebook);

  if (page->shown > 1)
    gtk_widget_hide(page->color[--page->shown]);
}

static void
on_notebook_switch_page(GtkNotebook *notebook,
  GtkNotebookPage *page, gint page_num, Chart_app *app)
{
  #ifdef DEBUG
  int n = gtk_notebook_get_current_page(app->notebook);
  printf("switch page: %p, %d, %d\n", app, n, page_num);
  #endif
}

static void
on_relabel(GtkWidget *widget, Param_page *page)
{
  gtk_notebook_set_tab_label_text(GTK_NOTEBOOK(page->notebook),
    page->table, gtk_editable_get_chars(GTK_EDITABLE(widget), 0, -1));
}

static void
on_altered(GtkWidget *unused, Param_page *page)
{
  Param_desc desc;
  ChartPlotStyle plot_style;
  ChartScaleStyle scale_style;

  if (page->strip_data == NULL)
    return;

  #ifdef DEBUG
  printf("on altered: page=%p\n", page);
  #endif
  page_to_desc(page, &desc);

  plot_style = str_to_plot_style(desc.plot);
  chart_set_plot_style(page->pen_data, plot_style);
  chart_set_plot_style(page->strip_data, plot_style);

  scale_style = str_to_scale_style(desc.scale);
  chart_set_scale_style(page->pen_data, scale_style);
  chart_set_scale_style(page->strip_data, scale_style);
}

static void
on_change(GtkWidget *unused, Param_page *page)
{
  page->changed = TRUE;
}

static void
on_top_max(GtkWidget *widget, Param_page *page)
{
  char *s = gtk_editable_get_chars(GTK_EDITABLE(widget), 0, -1);

  if (page->strip_data == NULL)
    return;

  #ifdef DEBUG
  printf("top max: %p, %p, %p, \"%s\", %f\n",
    widget, page, page->strip_data, s, atof(s));
  #endif
  chart_set_top_max(page->strip_data, atof(s));
}

static void
on_top_min(GtkWidget *widget, Param_page *page)
{
  char *s = gtk_editable_get_chars(GTK_EDITABLE(widget), 0, -1);

  if (page->strip_data == NULL)
    return;

  #ifdef DEBUG
  printf("top min: %p, %p, %p, \"%s\", %f\n", 
    widget, page, page->strip_data, s, atof(s));
  #endif
  chart_set_top_min(page->strip_data, atof(s));
}

static void
on_bot_max(GtkWidget *widget, Param_page *page)
{
  char *s = gtk_editable_get_chars(GTK_EDITABLE(widget), 0, -1);

  if (page->strip_data == NULL)
    return;

  #ifdef DEBUG
  printf("bot max: %p, %p, %p, \"%s\", %f\n", 
    widget, page, page->strip_data, s, atof(s));
  #endif
  chart_set_bot_max(page->strip_data, atof(s));
}

static void
on_bot_min(GtkWidget *widget, Param_page *page)
{
  char *s = gtk_editable_get_chars(GTK_EDITABLE(widget), 0, -1);

  if (page->strip_data == NULL)
    return;

  #ifdef DEBUG
  printf("bot min: %p, %p, %p, \"%s\", %f\n", 
    widget, page, page->strip_data, s, atof(s));
  #endif
  chart_set_bot_min(page->strip_data, atof(s));
}

static void
on_type_toggle(GtkToggleButton *type, Param_page *page)
{
  int c;
  if (gtk_toggle_button_get_active(type))
    for (c = 1; c < page->colors; c++)
      gtk_widget_hide(page->color[c]);
  else
    for (c = 1; c < page->shown; c++)
      gtk_widget_show(page->color[c]);

  on_change(GTK_WIDGET(type), page);
}

static void
create_param_page(Chart_app *app, Param_page *page, Param_desc *desc)
{
  GtkWidget *label, *desc_scroller, *param_optionmenu;
  GtkWidget *param_hbox, *scale_hbox, *type_hbox, *top_hbox, *bot_hbox;
  GSList *scale_hbox_group = NULL, *type_hbox_group = NULL;

  page->notebook = GTK_WIDGET(app->notebook);
  page->table = gtk_table_new(10, 2, FALSE);
  gtk_widget_show(page->table);

  label = gtk_label_new(_("Parameter"));
  gtk_widget_show(label);
  gtk_table_attach(GTK_TABLE(page->table),
    label, 0, 1, 0, 1, 0, 0, 0, 0);

  param_hbox = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(param_hbox);
  gtk_table_attach(GTK_TABLE(page->table),
    param_hbox, 1, 2, 0, 1, GTK_FILL, GTK_FILL, 0, 0);

  page->name = gtk_entry_new();
  gtk_widget_show(page->name);
  gtk_signal_connect(GTK_OBJECT(page->name), "changed", on_relabel, page);
  gtk_box_pack_start(GTK_BOX(param_hbox), page->name, TRUE, TRUE, 0);

  param_optionmenu = gtk_option_menu_new();
  gtk_widget_show(param_optionmenu);
  gtk_box_pack_start(GTK_BOX(param_hbox), param_optionmenu, FALSE, FALSE, 0);
  gtk_option_menu_set_menu(GTK_OPTION_MENU(param_optionmenu),
    param_options_menu(app, page));

  label = gtk_label_new(_("Description"));
  gtk_widget_show(label);
  gtk_table_attach(GTK_TABLE(page->table),
    label, 0, 1, 1, 2, 0, 0, 0, 0);

  desc_scroller = gtk_scrolled_window_new(NULL, NULL);
  gtk_widget_show(desc_scroller);
  gtk_table_attach(GTK_TABLE(page->table),
    desc_scroller, 1, 2, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(desc_scroller),
    GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
  page->desc = gtk_text_new(NULL, NULL);
  gtk_widget_show(page->desc);
  gtk_text_set_editable(GTK_TEXT(page->desc), TRUE);
  gtk_text_set_word_wrap(GTK_TEXT(page->desc), TRUE);
  gtk_container_add(GTK_CONTAINER(desc_scroller), page->desc);

  label = gtk_label_new(_("Equation"));
  gtk_widget_show(label);
  gtk_table_attach(GTK_TABLE(page->table),
    label, 0, 1, 2, 3, 0, 0, 0, 0);

  page->eqn = gtk_entry_new();
  gtk_widget_show(page->eqn);
  gtk_signal_connect(GTK_OBJECT(page->eqn), "changed", on_change, page);
  gtk_table_attach(GTK_TABLE(page->table),
    page->eqn, 1, 2, 2, 3, (GTK_EXPAND | GTK_FILL), 0, 0, 0);

  label = gtk_label_new(_("Filename"));
  gtk_widget_show(label);
  gtk_table_attach(GTK_TABLE(page->table),
    label, 0, 1, 3, 4, 0, 0, 0, 0);

  page->fn = gtk_entry_new();
  gtk_widget_show(page->fn);
  gtk_signal_connect(GTK_OBJECT(page->fn), "changed", on_change, page);
  gtk_table_attach(GTK_TABLE(page->table),
    page->fn, 1, 2, 3, 4, (GTK_EXPAND | GTK_FILL), 0, 0, 0);

  label = gtk_label_new(_("Pattern"));
  gtk_widget_show(label);
  gtk_table_attach(GTK_TABLE(page->table),
    label, 0, 1, 4, 5, 0, 0, 0, 0);

  page->pattern = gtk_entry_new();
  gtk_widget_show(page->pattern);
  gtk_signal_connect(GTK_OBJECT(page->pattern), "changed", on_change, page);
  gtk_table_attach(GTK_TABLE(page->table),
    page->pattern, 1, 2, 4, 5, (GTK_EXPAND | GTK_FILL), 0, 0, 0);

  label = gtk_label_new(_("Top"));
  gtk_widget_show(label);
  gtk_table_attach(GTK_TABLE(page->table),
    label, 0, 1, 5, 6, 0, 0, 0, 0);

  top_hbox = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(top_hbox);
  gtk_table_attach(GTK_TABLE(page->table),
    top_hbox, 1, 2, 5, 6, 0, 0, 0, 0);

  label = gtk_label_new(_("Min"));
  gtk_widget_show(label);
  gtk_box_pack_start(GTK_BOX(top_hbox), label, FALSE, FALSE, 0);

  page->top_min = gtk_entry_new();
  gtk_widget_show(page->top_min);
  gtk_signal_connect(GTK_OBJECT(page->top_min), "changed", on_top_min, page);
  gtk_box_pack_start(GTK_BOX(top_hbox), page->top_min, FALSE, FALSE, 0);

  label = gtk_label_new(_("Max"));
  gtk_widget_show(label);
  gtk_box_pack_start(GTK_BOX(top_hbox), label, FALSE, FALSE, 0);

  page->top_max = gtk_entry_new();
  gtk_widget_show(page->top_max);
  gtk_signal_connect(GTK_OBJECT(page->top_min), "changed", on_top_max, page);
  gtk_box_pack_start(GTK_BOX(top_hbox), page->top_max, FALSE, FALSE, 0);

  label = gtk_label_new(_("Bottom"));
  gtk_widget_show(label);
  gtk_table_attach(GTK_TABLE(page->table),
    label, 0, 1, 6, 7, 0, 0, 0, 0);

  bot_hbox = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(bot_hbox);
  gtk_table_attach(GTK_TABLE(page->table),
    bot_hbox, 1, 2, 6, 7, 0, 0, 0, 0);

  label = gtk_label_new(_("Min"));
  gtk_widget_show(label);
  gtk_box_pack_start(GTK_BOX(bot_hbox), label, FALSE, FALSE, 0);

  page->bot_min = gtk_entry_new();
  gtk_widget_show(page->bot_min);
  gtk_signal_connect(GTK_OBJECT(page->bot_min), "changed", on_bot_min, page);
  gtk_box_pack_start(GTK_BOX(bot_hbox), page->bot_min, FALSE, FALSE, 0);

  label = gtk_label_new(_("Max"));
  gtk_widget_show(label);
  gtk_box_pack_start(GTK_BOX(bot_hbox), label, FALSE, FALSE, 0);

  page->bot_max = gtk_entry_new();
  gtk_widget_show(page->bot_max);
  gtk_signal_connect(GTK_OBJECT(page->bot_min), "changed", on_bot_max, page);
  gtk_box_pack_start(GTK_BOX(bot_hbox), page->bot_max, FALSE, FALSE, 0);

  label = gtk_label_new(_("Scale"));
  gtk_widget_show(label);
  gtk_table_attach(GTK_TABLE(page->table),
    label, 0, 1, 7, 8, 0, 0, 0, 0);

  scale_hbox = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(scale_hbox);
  gtk_table_attach(GTK_TABLE(page->table),
    scale_hbox, 1, 2, 7, 8, GTK_FILL, GTK_FILL, 0, 0);

  page->linear =
    gtk_radio_button_new_with_label(scale_hbox_group, _("Linear"));
  scale_hbox_group =
    gtk_radio_button_group(GTK_RADIO_BUTTON(page->linear));
  gtk_signal_connect(GTK_OBJECT(page->linear), "toggled", on_altered, page);
  gtk_widget_show(page->linear);
  gtk_box_pack_start(GTK_BOX(scale_hbox), page->linear, FALSE, FALSE, 0);

  page->log =
    gtk_radio_button_new_with_label(scale_hbox_group, _("Logarithmic"));
  scale_hbox_group = gtk_radio_button_group(GTK_RADIO_BUTTON(page->log));
  gtk_widget_show(page->log);
  gtk_box_pack_start(GTK_BOX(scale_hbox), page->log, FALSE, FALSE, 0);

  label = gtk_label_new(_("Type"));
  gtk_widget_show(label);
  gtk_table_attach(GTK_TABLE(page->table),
    label, 0, 1, 8, 9, 0, 0, 0, 0);

  type_hbox = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(type_hbox);
  gtk_table_attach(GTK_TABLE(page->table),
    type_hbox, 1, 2, 8, 9, GTK_FILL, GTK_FILL, 0, 0);

  page->indicator =
    gtk_radio_button_new_with_label(type_hbox_group, _("Indicator"));
  type_hbox_group =
    gtk_radio_button_group(GTK_RADIO_BUTTON(page->indicator));
  gtk_widget_show(page->indicator);
  gtk_box_pack_start(GTK_BOX(type_hbox), page->indicator, FALSE, FALSE, 0);

  page->line =
    gtk_radio_button_new_with_label(type_hbox_group, _("Line"));
  type_hbox_group =
    gtk_radio_button_group(GTK_RADIO_BUTTON(page->line));
  gtk_widget_show(page->line);
  gtk_box_pack_start(GTK_BOX(type_hbox), page->line, FALSE, FALSE, 0);

  page->point =
    gtk_radio_button_new_with_label(type_hbox_group, _("Point"));
  type_hbox_group =
    gtk_radio_button_group(GTK_RADIO_BUTTON(page->point));
  gtk_widget_show(page->point);
  gtk_box_pack_start(GTK_BOX(type_hbox), page->point, FALSE, FALSE, 0);

  page->solid =
    gtk_radio_button_new_with_label(type_hbox_group, _("Solid"));
  type_hbox_group =
    gtk_radio_button_group(GTK_RADIO_BUTTON(page->solid));
  gtk_widget_show(page->solid);
  gtk_box_pack_start(GTK_BOX(type_hbox), page->solid, FALSE, FALSE, 0);

  gtk_signal_connect(GTK_OBJECT(page->indicator), "toggled", on_type_toggle, page);
  gtk_signal_connect(GTK_OBJECT(page->line), "toggled", on_type_toggle, page);
  gtk_signal_connect(GTK_OBJECT(page->point), "toggled", on_type_toggle, page);
  gtk_signal_connect(GTK_OBJECT(page->solid), "toggled", on_type_toggle, page);

  label = gtk_label_new(_("Color"));
  gtk_widget_show(label);
  gtk_table_attach(GTK_TABLE(page->table),
    label, 0, 1, 9, 10, 0, 0, 0, 0);

  page->color_hbox = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(page->color_hbox);
  gtk_table_attach(GTK_TABLE(page->table),
    page->color_hbox, 1, 2, 9, 10, GTK_FILL, GTK_FILL, 0, 0);

  param_page_set_from_desc(page, desc);
}

Param_page *
add_page_before(Chart_app *app, int n, Param_desc *desc)
{
  static int pno;
  char pno_str[100];
  Param_page *page = g_malloc0(sizeof(*page));

  if (desc && desc->name)
    strcpy(pno_str, desc->name);
  else
    sprintf(pno_str, "Param %d", pno++);

  create_param_page(app, page, desc);

  gtk_notebook_insert_page(app->notebook,
    page->table, gtk_label_new(pno_str), n);
  gtk_notebook_set_page(app->notebook, n);
  gtk_object_set_user_data(GTK_OBJECT(page->table), page);

#ifdef DEBUG
  printf("add_param: n %d, page %p\n", n, page);
#endif /* DEBUG */
  return page;
}

static void
on_add_before_param(GtkMenuItem *menuitem, Chart_app *app)
{
  int n = gtk_notebook_get_current_page(app->notebook);
  add_page_before(app, n+0, NULL);
}

static void
on_add_after_param(GtkMenuItem *menuitem, Chart_app *app)
{
  int n = gtk_notebook_get_current_page(app->notebook);
  add_page_before(app, n+1, NULL);
}

static void
on_apply(GtkMenuItem *menuitem, Chart_app *app)
{
  int p = 0;
  GtkWidget *nb_page;

  while ((nb_page = gtk_notebook_get_nth_page(app->notebook, p)) != NULL)
    {
      Param_page *page = gtk_object_get_user_data(GTK_OBJECT(nb_page));
      if (page->changed)
	{
	  ChartDatum *strip_datum = NULL, *pen_datum = NULL;
	  Param_desc *desc = g_malloc0(sizeof(*desc));
	  page_to_desc(page, desc);

	  strip_datum = chart_equation_add(CHART(app->strip),
	    app->strip_param_group, desc, NULL, p,
	    str_to_plot_style(desc->plot) != chart_plot_indicator);

	  if (strip_datum)
	    {
	      pen_datum = chart_equation_add(CHART(app->pen),
		app->pen_param_group, desc, strip_datum->adj, p, FALSE);
	    }

	  if (strip_datum && pen_datum)
	    {
	      if (page->strip_data && page->strip_data->active)
		chart_parameter_deactivate(CHART(app->strip),page->strip_data);
	      page->strip_data = strip_datum;

	      if (page->pen_data && page->pen_data->active)
		chart_parameter_deactivate(CHART(app->pen), page->pen_data);
	      page->pen_data = pen_datum;
	    }

	  g_free(desc);
	}
      p++;
    }
}

static void
on_delete_param(GtkMenuItem *menuitem, Chart_app *app)
{
  int n = gtk_notebook_get_current_page(app->notebook);
  GtkWidget *nb_page = gtk_notebook_get_nth_page(app->notebook, n);
  Param_page *page = gtk_object_get_user_data(GTK_OBJECT(nb_page));

  chart_parameter_deactivate(CHART(app->strip), page->strip_data);
  chart_parameter_deactivate(CHART(app->pen  ), page->pen_data);

  gtk_notebook_remove_page(app->notebook, n);
  if (app->notebook->children == NULL)
    add_page_before(app, 0, NULL);
}

static GnomeUIInfo file_menu_uiinfo[] =
{
  {
    GNOME_APP_UI_ITEM, N_("Save"),
    NULL,
    on_save, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_SAVE,
    0, 0, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("Save As..."),
    NULL,
    on_save_as, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_SAVE_AS,
    0, 0, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("Close"),
    NULL,
    on_close, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_EXIT,
    0, 0, NULL
  },
  GNOMEUIINFO_END
};

static GnomeUIInfo edit_menu_uiinfo[] =
{
  {
    GNOME_APP_UI_ITEM, N_("Add Parameter Before"),
    NULL,
    on_add_before_param, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_BACK,
    0, 0, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("Add Parameter After"),
    NULL,
    on_add_after_param, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_FORWARD,
    0, 0, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("Apply Parameter Changes"),
    NULL,
    on_apply, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_BUTTON_APPLY,
    0, 0, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("Delete Parameter"),
    NULL,
    on_delete_param, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_CUT,
    0, 0, NULL
  },
  GNOMEUIINFO_SEPARATOR,
  {
    GNOME_APP_UI_ITEM, N_("Add Color"),
    NULL,
    on_add_color, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_FORWARD,
    0, 0, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("Delete Color"),
    NULL,
    on_delete_color, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_CUT,
    0, 0, NULL
  },
  GNOMEUIINFO_END
};

static GnomeUIInfo help_menu_uiinfo[] =
{
  {
    GNOME_APP_UI_ITEM, N_("About"),
    NULL,
    on_about_menu, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_ABOUT,
    0, 0, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("Info"),
    NULL,
    on_help_menu, NULL, NULL,
    GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_BOOK_OPEN,
    0, 0, NULL
  },
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_HELP("stripchart"),
  GNOMEUIINFO_END
};

static GnomeUIInfo edit_menubar_uiinfo[] =
{
  {
    GNOME_APP_UI_SUBTREE, N_("File"),
    NULL,
    file_menu_uiinfo, NULL, NULL,
    GNOME_APP_PIXMAP_NONE, N_("File"),
    0, 0, NULL
  },
  {
    GNOME_APP_UI_SUBTREE, N_("Edit"),
    NULL,
    edit_menu_uiinfo, NULL, NULL,
    GNOME_APP_PIXMAP_NONE, N_("Edit"),
    0, 0, NULL
  },
  {
    GNOME_APP_UI_SUBTREE, N_("Help"),
    NULL,
    help_menu_uiinfo, NULL, NULL,
    GNOME_APP_PIXMAP_NONE, N_("Help"),
    0, 0, NULL
  },
  GNOMEUIINFO_END
};

static void
on_edit_menu(GtkMenuItem *item, Chart_app *app)
{
  GnomeUIInfo *menu = edit_menu_uiinfo;
  Param_page *page = get_current_page_param(app->notebook);
  GtkToggleButton *indy = GTK_TOGGLE_BUTTON(page->indicator);
  int is_indicator = gtk_toggle_button_get_active(indy);

  gtk_widget_set_sensitive(GTK_WIDGET(menu[2].widget), page->changed);
  gtk_widget_set_sensitive(GTK_WIDGET(menu[5].widget), is_indicator);
  gtk_widget_set_sensitive(GTK_WIDGET(menu[6].widget),
    is_indicator && page->shown > 1);
}

static void
on_editor_delete(GtkWidget *win, GdkEvent *event, void *nil)
{
  gtk_widget_hide(win);
}

void
on_param_edit(GtkWidget *unused, GtkWidget *editor)
{
  if (! GTK_WIDGET_VISIBLE(editor))
    gtk_widget_show(editor);
}

void
create_editor(Chart_app *app)
{
  GtkWidget *edit_vbox, *edit_handlebox, *edit_menubar;

#if 1
  app->editor = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(app->editor), _("Parameter Editor"));
  gtk_window_set_position(GTK_WINDOW(app->editor), GTK_WIN_POS_NONE);
  gtk_signal_connect(GTK_OBJECT(app->editor),
    "delete-event", on_editor_delete, NULL);

  edit_vbox = gtk_vbox_new(FALSE, 0);
  gtk_widget_show(edit_vbox);
  gtk_container_add(GTK_CONTAINER(app->editor), edit_vbox);
#else
  app->editor = gnome_dialog_new(_("Parameters"), NULL);
  gnome_dialog_close_hides(GNOME_DIALOG(app->editor), TRUE);
  gtk_signal_connect(GTK_OBJECT(app->editor),
    "delete-event", (GtkSignalFunc)on_editor_delete, NULL);
  edit_vbox = GNOME_DIALOG(app->editor)->vbox;
#endif

  edit_handlebox = gtk_handle_box_new();
  gtk_widget_show(edit_handlebox);
  gtk_box_pack_start(GTK_BOX(edit_vbox), edit_handlebox, TRUE, TRUE, 0);

  edit_menubar = gtk_menu_bar_new();
  gtk_widget_show(edit_menubar);
  gtk_container_add(GTK_CONTAINER(edit_handlebox), edit_menubar);
  gnome_app_fill_menu_with_data(GTK_MENU_SHELL(edit_menubar),
    edit_menubar_uiinfo, NULL, FALSE, 0, app);

  app->notebook = GTK_NOTEBOOK(gtk_notebook_new());
  gtk_widget_show(GTK_WIDGET(app->notebook));
  gtk_box_pack_start(GTK_BOX(edit_vbox),
    GTK_WIDGET(app->notebook), TRUE, TRUE, 0);
  gtk_notebook_set_tab_pos(app->notebook, GTK_POS_BOTTOM);

  gtk_signal_connect(GTK_OBJECT(app->notebook),
    "switch_page", GTK_SIGNAL_FUNC(on_notebook_switch_page), app);
  gtk_signal_connect(GTK_OBJECT(edit_menubar_uiinfo[1].widget),
    "activate", GTK_SIGNAL_FUNC(on_edit_menu), app);
}
