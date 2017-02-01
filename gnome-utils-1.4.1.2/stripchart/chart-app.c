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
#include "pen.h"
#include "strip.h"

#include <sys/stat.h>

/*
 * digit_shift -- a hi_lo_fmt helper function.
 */
static void
digit_shift(char *str, char *digits, int width, int dpos)
{
  int w;
  if (++dpos <= 0)
    {
      *str++ = '.';
      for (w = 1; w < width; w++)
        *str++ = (++dpos <= 0) ? '0' : *digits++;
    }
  else
    for (w = 0; w < width; w++)
      {
        *str++ = *digits++;
        if (--dpos == 0)
          {
            *str++ = '.';
            w++;
          }
      }
}

/*
 * hi_lo_fmt -- formats a hi and lo value to the same magnitude.
 */
static void
hi_lo_fmt(double hv, char *hs, double lv, char *ls)
{
  int e, p, t, f;
  char s[100];

  sprintf(s, "% 22.16e", hv);
  e = atoi(s+20);
  s[2] = s[1]; s[1] = '0';
  t = 3 * ((e - (e < 0)) / 3);
  p = e - t;
  hs[0] = s[0];
  digit_shift(hs+1, s+2, 4, p);
  hs[6] = '\0';
  if (0 <= t && t <= 5)
    hs[5] = "\0KMGTP"[t/3];
  else
    sprintf(hs+5, "e%+d", t);

  if (ls)
    {
      sprintf(s, "% 22.16e", lv);
      f = atoi(s+20);
      s[2] = s[1]; s[1] = '0';
      p = f - t;
      ls[0] = s[0];
      digit_shift(ls+1, s+2, 4, p);
      ls[6] = '\0';
      if (0 <= t && t <= 5)
	ls[5] = "\0KMGTP"[t/3];
      else
	sprintf(ls+5, "e%+d", t);
    }
}

static char *
param_type_str(Param_page *page)
{
  int led = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(page->indicator));
  int log = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(page->log));

  return led ? "*" : (log ? "log" : "-");
}

/*
 * text_load_clist -- fills a clist with chart ident, current, and top values.
 */
static void
text_load_clist(Chart_app *app)
{
  int p = 0, row = 0;
  GtkWidget *nb_page;

  gtk_clist_freeze(GTK_CLIST(app->text_clist));
  gtk_clist_clear(GTK_CLIST(app->text_clist));

  while ((nb_page = gtk_notebook_get_nth_page(app->notebook, p)) != NULL)
    {
      double top, val;
      char *row_strs[4];
      char val_str[100], top_str[100];
      Param_page *page = gtk_object_get_user_data(GTK_OBJECT(nb_page));
      ChartDatum *datum = page->strip_data;

      if (datum && datum->active)
	{
	  row_strs[0] = gtk_editable_get_chars(GTK_EDITABLE(page->name), 0,-1);
	  row_strs[1] = val_str;
	  row_strs[2] = top_str;
	  row_strs[3] = param_type_str(page);

	  top = datum->adj->upper;
	  val = datum->history[datum->newest];
	  hi_lo_fmt(top, top_str, val, val_str);
	  gtk_clist_append(GTK_CLIST(app->text_clist), row_strs);
	  gtk_clist_set_foreground(GTK_CLIST(app->text_clist),
	    row, &page->strip_data->gdk_color[0]);
	  gtk_clist_set_background(GTK_CLIST(app->text_clist),
	    row, &app->text_window->style->bg[0]);
	  row++;
	}
      p++;
    }
  gtk_clist_columns_autosize(GTK_CLIST(app->text_clist));
  gtk_clist_thaw(GTK_CLIST(app->text_clist));
}

/*
 * text_refresh -- if the text window is up, refresh the values.
 */
void
text_refresh(Chart *chart, Chart_app *app)
{
  if (app->text_window && GTK_WIDGET_VISIBLE(app->text_window))
    text_load_clist(app);
}

/*
 * on_show_values -- pops up a top level textual display of current
 * parameter values in response to a mouse click.  Closes this window
 * if it's already displayed.
 */
void
on_show_values(GtkWidget *unused, Chart_app *app)
{
  if (app->text_window == NULL)
    {
      char *titles[4];
      titles[0] = _("Param");
      titles[1] = _("Current");
      titles[2] = _("Top");
      titles[3] = _("Scale");

      app->text_clist = gtk_clist_new_with_titles(
	sizeof(titles) / sizeof(*titles), titles);
      gtk_widget_show(app->text_clist);

      app->text_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
      gtk_widget_set_style(GTK_WIDGET(app->text_window), app->frame->style);
      gtk_container_add(GTK_CONTAINER(app->text_window), app->text_clist);
      gtk_window_set_policy(GTK_WINDOW(app->text_window),
	/*allow-shrink*/1, /*allow-grow*/1, /*auto-shrink*/1);
      gtk_window_set_transient_for(GTK_WINDOW(app->text_window),
	GTK_WINDOW(app->frame));
      gtk_widget_show(app->text_window);

      /* Load the initial batch of parameter values after showing the
         toplevel window.  This odd sequence is required to get the
         auto-sizing right. */
      text_load_clist(app);

      gtk_signal_connect(GTK_OBJECT(app->text_window),
	"delete-event", GTK_SIGNAL_FUNC(gtk_widget_hide), app->text_window);
    }
  else if (GTK_WIDGET_VISIBLE(app->text_window))
    gtk_widget_hide(app->text_window);
  else
    gtk_widget_show(app->text_window);
}

void
on_about_menu(void)
{
  const char *authors[] = { "John Kodis, kodis@jagunet.com", NULL };
  GtkWidget *about = gnome_about_new(
    _("Stripchart"), VERSION,
    _("Copyright 2000 John Kodis"),
    authors,
    _("The GNOME stripchart program plots various user-specified parameters "
      "as a function of time.  Its main use is to chart system performance "
      "parameters such as CPU load, CPU utilization, network traffic levels, "
      "and the like.  Other more ingenious uses are left as an exercise for "
      "the interested user."),
    GNOME_ICONDIR "/gnome-stripchart.png");
  gtk_widget_show(about);
}

void
on_help_menu(void)
{
  static GnomeHelpMenuEntry help_entry = { "stripchart", "index.html" };
  gnome_help_display(NULL, &help_entry);
}

void
menu_popup(GtkWidget *widget, GdkEvent *event, Chart_app *app)
{
  static GtkWidget *menu;

  if (menu == NULL)
    {
      GtkWidget *menu_item, *ed = app->editor;

      menu = gtk_menu_new();

      menu_item = gtk_menu_item_new_with_label(_("Help"));
      gtk_menu_append(GTK_MENU(menu), menu_item);
      gtk_signal_connect_object(GTK_OBJECT(menu_item),
	"activate", GTK_SIGNAL_FUNC(on_help_menu), GTK_OBJECT(widget));

      menu_item = gtk_menu_item_new_with_label(_("About"));
      gtk_menu_append(GTK_MENU(menu), menu_item);
      gtk_signal_connect_object(GTK_OBJECT(menu_item),
	"activate", GTK_SIGNAL_FUNC(on_about_menu), NULL);

      gtk_menu_append(GTK_MENU(menu), gtk_menu_item_new());

      menu_item = gtk_menu_item_new_with_label(_("Show values"));
      gtk_menu_append(GTK_MENU(menu), menu_item);
      gtk_signal_connect(GTK_OBJECT(menu_item),
	"activate", GTK_SIGNAL_FUNC(on_show_values), app);

      gtk_menu_append(GTK_MENU(menu), gtk_menu_item_new());

      menu_item = gtk_menu_item_new_with_label(_("Edit Prefs..."));
      gtk_menu_append(GTK_MENU(menu), menu_item);
      gtk_signal_connect(GTK_OBJECT(menu_item),
	"activate", GTK_SIGNAL_FUNC(on_prefs_edit), app);

      menu_item = gtk_menu_item_new_with_label(_("Edit Params..."));
      gtk_menu_append(GTK_MENU(menu), menu_item);
      gtk_signal_connect(GTK_OBJECT(menu_item),
	"activate", GTK_SIGNAL_FUNC(on_param_edit), ed);

      gtk_menu_append(GTK_MENU(menu), gtk_menu_item_new());

      menu_item = gtk_menu_item_new_with_label(_("Exit"));
      gtk_menu_append(GTK_MENU(menu), menu_item);
      gtk_signal_connect_object(GTK_OBJECT(menu_item),
	"activate", GTK_SIGNAL_FUNC(gtk_main_quit), NULL);

      gtk_widget_show_all(menu);
    }

  gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, 
    ((GdkEventButton*)event)->button, ((GdkEventButton*)event)->time);
}

void
on_button_press(GtkWidget *win, GdkEvent *event, Chart_app *app)
{
  int button = ((GdkEventButton*)event)->button;
  #ifdef DEBUG
  printf("on button press: %p, %p, %p\n", win, event, app);
  #endif
  switch (button)
    {
    case 1: on_show_values(NULL, app); break;
    case 3: menu_popup(win, event, app); break;
    }
}

static int
proc_arg(int opt, const char *arg)
{
  int geometry_flags;
  switch (opt)
    {
    case 'f':
      config_fn = strdup(arg);
      break;
    case 'g':
      geometry_flags = gnome_parse_geometry(arg,
	&geometry_x, &geometry_y, &geometry_w, &geometry_h);
      #ifdef DEBUG
      printf("geo: (%s): 0x%x, %d,%d; %d,%d\n", arg,
	geometry_flags, geometry_x, geometry_y, geometry_w, geometry_h);
      #endif
      break;
    }
  return 0;
}

void
popt_arg_extractor(
  poptContext state, enum poptCallbackReason reason,
  const struct poptOption *opt, const char *arg, void *data )
{
  if (proc_arg(opt->val, arg))
    {
      poptPrintUsage(state, stderr, 0);
      exit(EXIT_FAILURE);
    }
}

Chart_app *
chart_app_new(void)
{
  int p;
  char *config_path[20];
  struct stat stat_buf;
  Param_desc **param_desc = NULL;
  Chart_app *app = g_malloc(sizeof(*app));

  app->pen_param_group = g_malloc0(sizeof(*app->pen_param_group));
  app->strip_param_group = g_malloc0(sizeof(*app->strip_param_group));
  app->text_window = NULL; app->file_sel = NULL; app->prefs = NULL;

  app->hbox = gtk_hbox_new(/*homo*/0, /*pad*/0);
  gtk_widget_show(app->hbox);

  app->strip = strip_new();
  gtk_widget_show(app->strip);
  gtk_box_pack_start(GTK_BOX(app->hbox),
    app->strip, /*expand*/1, /*fill*/1, /*pad*/0);
  gtk_signal_connect(GTK_OBJECT(app->strip),
    "chart_pre_update", chart_start, app->strip_param_group);

  app->pen_sep = gtk_vseparator_new();
  gtk_box_pack_start(GTK_BOX(app->hbox), app->pen_sep,
    /*exp*/0, /*fill*/1, /*pad*/0);

  app->pen = pen_new();
  gtk_widget_set_usize(GTK_WIDGET(app->pen), 10, -1);
  gtk_box_pack_start(GTK_BOX(app->hbox), app->pen,
    /*exp*/0, /*fill*/1, /*pad*/0);
  gtk_signal_connect(GTK_OBJECT(app->pen),
    "chart_pre_update", chart_start, app->pen_param_group);

  gtk_signal_connect(GTK_OBJECT(app->strip),
    "chart_post_update", text_refresh, app);
  strip_set_ticks(STRIP(app->strip), 1, 0, 0);

  app->strip_param_group->filter = 0.5;
  app->strip_param_group->interval = 5000;
  gettimeofday(&app->strip_param_group->t_now, NULL);

  app->pen_param_group->filter = 0.5;
  app->pen_param_group->interval = 1000;
  gettimeofday(&app->pen_param_group->t_now, NULL);

  p = 0;
  app->config_fn = g_strdup_printf("%s/.stripchart.conf", getenv("HOME"));
  if (config_fn != NULL)
    config_path[p++] = config_fn;
  else
    {
      config_path[p++] = app->config_fn;
      config_path[p++] = g_strdup("stripchart.conf");
      config_path[p++] = g_strdup(CONFDIR "/stripchart.conf");
    }
  config_path[p] = NULL;

  for (p = 0; config_path[p] != NULL; p++)
    if (stat(config_path[p], &stat_buf) == 0)
      {
	param_desc = param_desc_ingest(config_path[p]);
	prefs_ingest(app, config_path[p]);
	if (app->pen_param_group->visible)
	  {
	    gtk_widget_show(app->pen_sep);
	    gtk_widget_show(app->pen);
	  }
	break;
      }
  if (config_path[p] == NULL)
    error("no config file found, proceeding anyway\n");

  strip_set_default_history_size(STRIP(app->strip), gdk_screen_width());
  chart_set_interval(CHART(app->strip), app->strip_param_group->interval);
  chart_set_interval(CHART(app->pen), app->pen_param_group->interval);

  app->params_fn = CONFDIR "/stripchart.params";
  if (stat(app->params_fn, &stat_buf) != 0)
    app->params_fn = "stripchart.params";
  create_editor(app);

  if (param_desc != NULL)
    for (p = 0; param_desc[p]; p++)
      {
	Param_page *page = add_page_before(app, p, param_desc[p]);

	page->strip_data = chart_equation_add(CHART(app->strip),
	  app->strip_param_group, param_desc[p], NULL, 0,
	  str_to_plot_style(param_desc[p]->plot) != chart_plot_indicator);

	page->pen_data = chart_equation_add(CHART(app->pen),
	  app->pen_param_group, param_desc[p], page->strip_data->adj, 0,FALSE);
      }

  return app;
}
