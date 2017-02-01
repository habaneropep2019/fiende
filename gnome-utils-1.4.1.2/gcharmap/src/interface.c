/*
 *  Gnome Character Map
 *  interface.c - The main window
 *
 *  Copyright (C) Hongli Lai
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
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

#ifndef _INTERFACE_C_
#define _INTERFACE_C_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <interface.h>
#include <menus.h>
#include <callbacks.h>
MainApp *mainapp;


static GtkWidget *
create_button (const gchar *label, GtkSignalFunc func)
{
    GtkWidget *button;

    button = gtk_button_new_with_label (label);
    GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
    if (func != NULL)
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
          func, NULL);
    gtk_widget_show (button);
    return button;
}


static GtkWidget *
create_chartable (void)
{
    GtkWidget *chartable, *button;
    gint v, h;

    chartable = gtk_table_new (8, 24, TRUE);

    for (v = 0; v <= 3; v++)
    {
        for (h = 0; h <= 23; h++)
        {
	    char *s;
	    int ch = v * 24 + h + 32;
	    if (ch == 127)
		    s = g_strdup (_("del"));
	    else
		    s = g_strdup_printf ("%c", (char)ch);

            button = gtk_button_new_with_label (s);
            mainapp->buttons = g_list_append (mainapp->buttons, button);
            gtk_widget_set_style (GTK_BIN (button)->child, mainapp->btnstyle);
            gtk_table_attach (GTK_TABLE (chartable), button, h, h + 1, v, v + 1,
              (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
              (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
              0, 0);

            gtk_signal_connect (GTK_OBJECT (button), "clicked",
              GTK_SIGNAL_FUNC (cb_charbtn_click), NULL);
            gtk_signal_connect (GTK_OBJECT (button), "enter",
              GTK_SIGNAL_FUNC (cb_charbtn_enter), NULL);
            gtk_signal_connect (GTK_OBJECT (button), "leave",
              GTK_SIGNAL_FUNC (cb_charbtn_leave), NULL);

            g_free (s);
        }
    }

    for (v = 0; v <= 3; v++)
    {
        for (h = 0; h <= 23; h++)
        {
	    char *s;
	    int ch = v * 24 + h + 161;

	    if (ch > 0xff)
		    continue;

            s = g_strdup_printf ("%c", (char)ch);

            button = gtk_button_new_with_label (s);
            mainapp->buttons = g_list_append (mainapp->buttons, button);
            gtk_widget_set_style (GTK_BIN (button)->child, mainapp->btnstyle);
            gtk_table_attach (GTK_TABLE (chartable), button,
              h, h + 1, v + 4, v + 5,
              (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
              (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
              0, 0);

            gtk_signal_connect (GTK_OBJECT (button), "clicked",
              GTK_SIGNAL_FUNC (cb_charbtn_click), NULL);
            gtk_signal_connect (GTK_OBJECT (button), "enter",
              GTK_SIGNAL_FUNC (cb_charbtn_enter), NULL);
            gtk_signal_connect (GTK_OBJECT (button), "leave",
              GTK_SIGNAL_FUNC (cb_charbtn_leave), NULL);

            g_free (s);
        }
    }

    gtk_widget_show_all (chartable);
    gtk_widget_push_style (mainapp->btnstyle);
    gtk_widget_pop_style ();
    return chartable;
}


static void
main_app_create_ui (MainApp *app)
{
    GtkWidget *appbar;
    GtkWidget *vbox, *hbox, *hbox2, *vbox2;
    GtkWidget *vsep, *alabel;
    GtkWidget *chartable;
    GtkWidget *buttonbox, *button;
    GtkWidget *viewport;

    /* Main window */
    {
        GnomeDockLayoutItem *item;

        app->window = gnome_app_new (_(PACKAGE), _("Gnome Character Map"));
        gtk_widget_set_name (app->window, "mainapp");
        gtk_signal_connect_object (GTK_OBJECT (app->window), "destroy",
          GTK_SIGNAL_FUNC (main_app_destroy), GTK_OBJECT (app));
        gtk_widget_realize (app->window);

        appbar = gnome_appbar_new (FALSE, TRUE, GNOME_PREFERENCES_USER);
        gnome_app_set_statusbar (GNOME_APP (app->window), appbar);
        gtk_widget_show (appbar);

        gnome_app_create_menus (GNOME_APP (app->window), menubar);
        gnome_app_install_menu_hints (GNOME_APP (app->window), menubar);

        gnome_app_create_toolbar (GNOME_APP (app->window), toolbar);
        item = g_list_nth_data (GNOME_APP (app->window)->layout->items, 0);
        app->actionbar = GTK_WIDGET (item->item);
    }

    /* The toplevel vbox */
    vbox = gtk_vbox_new (FALSE, 8);
    gnome_app_set_contents (GNOME_APP (app->window), vbox);
    gtk_container_set_border_width (GTK_CONTAINER (GNOME_APP (
      app->window)->contents), 8);

    {
        GnomeDockLayoutItem *item;

        hbox = gtk_hbox_new (FALSE, 6);
        gtk_container_set_border_width (GTK_CONTAINER (hbox), 1);
        gnome_app_add_docked (GNOME_APP (app->window), hbox, _("Action Toolbar"),
          GNOME_DOCK_ITEM_BEH_EXCLUSIVE | GNOME_DOCK_ITEM_BEH_NEVER_VERTICAL,
          GNOME_DOCK_TOP, 2, 0, 1);

        app->fontpicker = gnome_font_picker_new ();
        gnome_font_picker_set_mode (GNOME_FONT_PICKER (app->fontpicker),
          GNOME_FONT_PICKER_MODE_FONT_INFO);
        gnome_font_picker_fi_set_use_font_in_label (GNOME_FONT_PICKER (app->fontpicker),
          FALSE, 12);
        gnome_font_picker_set_font_name (GNOME_FONT_PICKER (app->fontpicker),
          "-*-helvetica-medium-r-normal-*-12-*-*-*-p-*-*-*");
        gtk_button_set_relief (GTK_BUTTON (app->fontpicker), GTK_RELIEF_NONE);
        gtk_box_pack_start (GTK_BOX (hbox), app->fontpicker, FALSE, TRUE, 0);
        gtk_signal_connect (GTK_OBJECT (app->fontpicker), "font_set",
          GTK_SIGNAL_FUNC (cb_fontpicker_font_set), NULL);

        vsep = gtk_vseparator_new ();
        gtk_box_pack_start (GTK_BOX (hbox), vsep, FALSE, FALSE, 0);

        alabel = gtk_label_new (_("Text to copy:"));
        gtk_box_pack_start (GTK_BOX (hbox), alabel, FALSE, TRUE, 0);

        app->entry = gtk_entry_new ();
        gtk_box_pack_start (GTK_BOX (hbox), app->entry, TRUE, TRUE, 0);
        gnome_popup_menu_attach (gnome_popup_menu_new (edit_menu),
          app->entry, NULL);

        gtk_widget_show_all (hbox);
        item = g_list_nth_data (GNOME_APP (app->window)->layout->items, 0);
        app->textbar = GTK_WIDGET (item->item);
        gtk_container_set_border_width (GTK_CONTAINER (app->textbar), 1);
    }

    hbox2 = gtk_hbox_new (FALSE, 8);
    gtk_box_pack_start (GTK_BOX (vbox), hbox2, TRUE, TRUE, 0);

    /* The character table */
    {
        GtkWidget *tmp;
	GdkFont *font;

        tmp = gtk_button_new ();
        gtk_widget_ensure_style(tmp);
        app->btnstyle = gtk_style_copy (gtk_widget_get_style (tmp));
        font = gdk_fontset_load (
          _("-*-helvetica-medium-r-normal-*-12-*-*-*-p-*-*-*,*-r-*")
        );
	if (font != NULL)
		app->btnstyle->font = font;
        gtk_widget_destroy (tmp);

        chartable = create_chartable ();
        gtk_box_pack_start (GTK_BOX (hbox2), chartable, TRUE, TRUE, 0);
        app->chartable = chartable;
    }

    vbox2 = gtk_vbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox2), vbox2, FALSE, TRUE, 0);


    /* The buttonbox */
    {
        buttonbox = gtk_vbutton_box_new ();
        gtk_button_box_set_layout (GTK_BUTTON_BOX (buttonbox), GTK_BUTTONBOX_START);
        gtk_button_box_set_spacing (GTK_BUTTON_BOX (buttonbox), 0);
        gtk_box_pack_start (GTK_BOX (vbox2), buttonbox, TRUE, TRUE, 0);

        gtk_container_add (GTK_CONTAINER (buttonbox),
          create_button (_("Exit"), GTK_SIGNAL_FUNC (cb_exit_click)));
        button = create_button (_("Copy"), GTK_SIGNAL_FUNC (cb_copy_click));
        gtk_container_add (GTK_CONTAINER (buttonbox), button);
        gtk_widget_grab_default (button);
        gtk_container_add (GTK_CONTAINER (buttonbox),
          create_button (_("About"), GTK_SIGNAL_FUNC (cb_about_click)));
        gtk_container_add (GTK_CONTAINER (buttonbox),
          create_button (_("Help"), GTK_SIGNAL_FUNC (cb_help_click)));
    }

    /* The zoom viewport */
    {
        GtkStyle *style;
        GdkColor black = {0, 0, 0, 0};
        GdkColor white = {0, 0xFFFF, 0xFFFF, 0xFFFF};
	GdkFont *font;
        guint8 i;

        viewport = gtk_viewport_new (NULL, NULL);
        gdk_color_alloc (gtk_widget_get_colormap (viewport), &black);
        gdk_color_alloc (gtk_widget_get_colormap (viewport), &white);

        style = gtk_style_copy (gtk_widget_get_style (viewport));
        for (i = 0; i < 5; i++) style->fg[i] = white;
        for (i = 0; i < 5; i++) style->bg[i] = black;
        font = gdk_fontset_load (
          _("-*-helvetica-bold-r-normal-*-*-180-*-*-p-*-*-*,*-r-*")
        );
	if (font != NULL)
		style->font = font;

        gtk_widget_set_style (viewport, style);
        gtk_box_pack_start (GTK_BOX (vbox2), viewport, FALSE, TRUE, 0);

        app->preview_label = gtk_label_new (NULL);
        gtk_container_add (GTK_CONTAINER (viewport), app->preview_label);
        gtk_widget_set_style (app->preview_label, style);

        gtk_widget_push_style (style);
        gtk_widget_pop_style ();
    }

    gtk_widget_show_all (vbox);
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (
      view_menu[0].widget), TRUE);
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (
      view_menu[1].widget), TRUE);
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (
      view_menu[2].widget), TRUE);
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (
      settings_menu[3].widget), TRUE);
    gtk_widget_grab_focus (app->entry);
}


static void
main_app_init (MainApp *obj)
{
    mainapp = obj;
    main_app_create_ui (obj);
}


guint
main_app_get_type (void)
{
    static guint ga_type = 0;

    if (!ga_type) {
        GtkTypeInfo ga_info = {
          "MainApp",
          sizeof (MainApp),
          sizeof (MainAppClass),
          (GtkClassInitFunc) NULL,
          (GtkObjectInitFunc) main_app_init,
          (GtkArgSetFunc) NULL,
          (GtkArgGetFunc) NULL,
          (GtkClassInitFunc) NULL
        };
        ga_type = gtk_type_unique (gtk_object_get_type (), &ga_info);
    }
    return ga_type;
}


MainApp *
main_app_new (void)
{
    return MAIN_APP (gtk_type_new ((GtkType) MAIN_APP_TYPE));
}


void
main_app_destroy (MainApp *obj)
{
    g_return_if_fail (obj != NULL);
    g_return_if_fail (MAIN_IS_APP (obj) == TRUE);

    if (obj->window != NULL) gtk_widget_destroy (obj->window);
    if (obj->btnstyle != NULL) g_free (obj->btnstyle);
    gtk_object_destroy (GTK_OBJECT (obj));
    gtk_main_quit ();
}


#endif /* _INTERFACE_C_ */
