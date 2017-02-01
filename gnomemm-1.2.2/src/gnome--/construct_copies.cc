/* 
 * Copyright 2001 The Gnome-- development team.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <gnome--/construct_copies.h>

//These functions are not declared publicly in GNOME.
//Therefore they are copy-and-pasted here.
//There are also copies of a few private functions that are used by the _construct functions.
//This should not be necessary with GNOME 2, but it doesn't look like GNOME 1.2 is going to be fixed.

#include <libgnomeui/gnome-canvas-rect-ellipse.h> 
#include <libgnomeui/gnome-canvas-image.h> 
#include <libgnomeui/gnome-canvas-text.h>

namespace Gnome {

#define LOGO_WIDTH 50.0
#define DRUID_PAGE_HEIGHT 318
#define DRUID_PAGE_WIDTH 516
#define DRUID_PAGE_LEFT_WIDTH 100.0

void
gnome_druid_page_finish_configure_size (GnomeDruidPageFinish *druid_page_finish, gint width, gint height)
{
	gfloat watermark_width = DRUID_PAGE_LEFT_WIDTH;
	gfloat watermark_height = (gfloat) height - LOGO_WIDTH + GNOME_PAD * 2.0;
	gfloat watermark_ypos = LOGO_WIDTH + GNOME_PAD * 2.0;

	if (druid_page_finish->watermark_image) {
		watermark_width = druid_page_finish->watermark_image->rgb_width;
		watermark_height = druid_page_finish->watermark_image->rgb_height;
		watermark_ypos = (gfloat) height - watermark_height;
		if (watermark_width < 1)
			watermark_width = 1.0;
		if (watermark_height < 1)
			watermark_height = 1.0;
	}

	gnome_canvas_item_set (druid_page_finish->background_item,
			       "x1", 0.0,
			       "y1", 0.0,
			       "x2", (gfloat) width,
			       "y2", (gfloat) height,
			       "width_units", 1.0, NULL);
	gnome_canvas_item_set (druid_page_finish->textbox_item,
			       "x1", (gfloat) watermark_width,
			       "y1", (gfloat) LOGO_WIDTH + GNOME_PAD * 2.0,
			       "x2", (gfloat) width,
			       "y2", (gfloat) height,
			       "width_units", 1.0, NULL);
	gnome_canvas_item_set (druid_page_finish->logoframe_item,
			       "x1", (gfloat) width - LOGO_WIDTH -GNOME_PAD,
			       "y1", (gfloat) GNOME_PAD,
			       "x2", (gfloat) width - GNOME_PAD,
			       "y2", (gfloat) GNOME_PAD + LOGO_WIDTH,
			       "width_units", 1.0, NULL);
	gnome_canvas_item_set (druid_page_finish->logo_item,
			       "x", (gfloat) width - GNOME_PAD - LOGO_WIDTH,
			       "y", (gfloat) GNOME_PAD,
			       "anchor", GTK_ANCHOR_NORTH_WEST,
			       "width", (gfloat) LOGO_WIDTH,
			       "height", (gfloat) LOGO_WIDTH, NULL);
	gnome_canvas_item_set (druid_page_finish->watermark_item,
			       "x", 0.0,
			       "y", watermark_ypos,
			       "anchor", GTK_ANCHOR_NORTH_WEST,
			       "width", watermark_width,
			       "height", watermark_height,
			       NULL);
	gnome_canvas_item_set (druid_page_finish->title_item,
			       "x", 15.0, 
			       "y", (gfloat) GNOME_PAD + LOGO_WIDTH / 2.0,
			       "anchor", GTK_ANCHOR_WEST,
			       NULL);
	gnome_canvas_item_set (druid_page_finish->text_item,
			       "x", ((width - watermark_width) * 0.5) + watermark_width,
			       "y", LOGO_WIDTH + GNOME_PAD * 2.0 + (height - (LOGO_WIDTH + GNOME_PAD * 2.0))/ 2.0,
			       "anchor", GTK_ANCHOR_CENTER,
			       NULL);
}

void
gnome_druid_page_finish_prepare (GnomeDruidPage *page,
				GtkWidget *druid,
				gpointer *data)
{
	gnome_druid_set_buttons_sensitive (GNOME_DRUID (druid), TRUE, FALSE, TRUE);
	gnome_druid_set_show_finish (GNOME_DRUID (druid), TRUE);
	gtk_widget_grab_default (GNOME_DRUID (druid)->finish);
}

void
gnome_druid_page_finish_construct (GnomeDruidPageFinish *druid_page_finish)
{
	/* set up the rest of the page */
	druid_page_finish->background_item =
		gnome_canvas_item_new (gnome_canvas_root (GNOME_CANVAS (druid_page_finish->canvas)),
				       gnome_canvas_rect_get_type (), NULL);
	druid_page_finish->textbox_item =
		gnome_canvas_item_new (gnome_canvas_root (GNOME_CANVAS (druid_page_finish->canvas)),
				       gnome_canvas_rect_get_type (), NULL);
	druid_page_finish->logoframe_item =
		gnome_canvas_item_new (gnome_canvas_root (GNOME_CANVAS (druid_page_finish->canvas)),
				       gnome_canvas_rect_get_type (), NULL);
	druid_page_finish->logo_item =
		gnome_canvas_item_new (gnome_canvas_root (GNOME_CANVAS (druid_page_finish->canvas)),
				       gnome_canvas_image_get_type (), NULL);
	if (druid_page_finish->logo_image != NULL)
		gnome_canvas_item_set (druid_page_finish->logo_item,
				       "image", druid_page_finish->logo_image, NULL);
	druid_page_finish->watermark_item =
		gnome_canvas_item_new (gnome_canvas_root (GNOME_CANVAS (druid_page_finish->canvas)),
				       gnome_canvas_image_get_type (),
				       "image", druid_page_finish->watermark_image, NULL);
	druid_page_finish->title_item =
		gnome_canvas_item_new (gnome_canvas_root (GNOME_CANVAS (druid_page_finish->canvas)),
				       gnome_canvas_text_get_type (), 
				       "text", druid_page_finish->title,
				       "font", "-adobe-helvetica-bold-r-normal-*-*-180-*-*-p-*-iso8859-1",
				       "fontset", "-adobe-helvetica-bold-r-normal-*-*-180-*-*-p-*-iso8859-1,*-r-*",
				       NULL);
	druid_page_finish->text_item =
		gnome_canvas_item_new (gnome_canvas_root (GNOME_CANVAS (druid_page_finish->canvas)),
				       gnome_canvas_text_get_type (),
				       "text", druid_page_finish->text,
				       "justification", GTK_JUSTIFY_LEFT,
				       "font", "-adobe-helvetica-medium-r-normal-*-*-120-*-*-p-*-iso8859-1",
				       "fontset", "-adobe-helvetica-medium-r-normal-*-*-120-*-*-p-*-iso8859-1,*-r-*",
				       NULL);

	gnome_druid_page_finish_configure_size (druid_page_finish, DRUID_PAGE_WIDTH, DRUID_PAGE_HEIGHT);
	gtk_signal_connect (GTK_OBJECT (druid_page_finish),
			    "prepare",
			    (GtkSignalFunc)gnome_druid_page_finish_prepare,
			    NULL);
}

void
gnome_druid_page_standard_prepare (GnomeDruidPage *page,
				   GtkWidget *druid,
				   gpointer *data)
{
	gnome_druid_set_buttons_sensitive (GNOME_DRUID (druid), TRUE, TRUE, TRUE);
	gnome_druid_set_show_finish (GNOME_DRUID (druid), FALSE);
	gtk_widget_grab_default (GNOME_DRUID (druid)->next);
}

void
gnome_druid_page_standard_configure_size (GnomeDruidPageStandard *druid_page_standard, gint width, gint height)
{
	gnome_canvas_item_set (druid_page_standard->background_item,
			       "x1", 0.0,
			       "y1", 0.0,
			       "x2", (gfloat) width,
			       "y2", (gfloat) LOGO_WIDTH + GNOME_PAD * 2,
			       "width_units", 1.0, NULL);
	gnome_canvas_item_set (druid_page_standard->logoframe_item,
			       "x1", (gfloat) width - LOGO_WIDTH - GNOME_PAD,
			       "y1", (gfloat) GNOME_PAD,
			       "x2", (gfloat) width - GNOME_PAD,
			       "y2", (gfloat) GNOME_PAD + LOGO_WIDTH,
			       "width_units", 1.0, NULL);
	gnome_canvas_item_set (druid_page_standard->logo_item,
			       "x", (gfloat) width - GNOME_PAD - LOGO_WIDTH,
			       "y", (gfloat) GNOME_PAD,
			       "anchor", GTK_ANCHOR_NORTH_WEST,
			       "width", (gfloat) LOGO_WIDTH,
			       "height", (gfloat) LOGO_WIDTH, NULL);
}

void
gnome_druid_page_standard_construct (GnomeDruidPageStandard *druid_page_standard)
{
	/* set up the rest of the page */
	druid_page_standard->background_item =
		gnome_canvas_item_new (gnome_canvas_root (GNOME_CANVAS (druid_page_standard->canvas)),
				       gnome_canvas_rect_get_type (), NULL);

	druid_page_standard->logoframe_item =
		gnome_canvas_item_new (gnome_canvas_root (GNOME_CANVAS (druid_page_standard->canvas)),
				       gnome_canvas_rect_get_type (), NULL);

	druid_page_standard->logo_item =
		gnome_canvas_item_new (gnome_canvas_root (GNOME_CANVAS (druid_page_standard->canvas)),
				       gnome_canvas_image_get_type (), NULL);
	if (druid_page_standard->logo_image != NULL) {
		gnome_canvas_item_set (druid_page_standard->logo_item,
				       "image", druid_page_standard->logo_image, NULL);
	}
	druid_page_standard->title_item =
		gnome_canvas_item_new (gnome_canvas_root (GNOME_CANVAS (druid_page_standard->canvas)),
				       gnome_canvas_text_get_type (), 
				       "text", druid_page_standard->title,
				       "font", "-adobe-helvetica-bold-r-normal-*-*-180-*-*-p-*-iso8859-1",
				       "fontset", "-adobe-helvetica-bold-r-normal-*-*-180-*-*-p-*-iso8859-1,*-r-*",
				       NULL);
	gnome_canvas_item_set (druid_page_standard->title_item,
			       "x", 15.0,
			       "y", (gfloat) GNOME_PAD + LOGO_WIDTH / 2.0,
			       "anchor", GTK_ANCHOR_WEST,
			       NULL);

	gnome_druid_page_standard_configure_size (druid_page_standard, DRUID_PAGE_WIDTH, GNOME_PAD * 2 + LOGO_WIDTH);
	gtk_signal_connect (GTK_OBJECT (druid_page_standard),
			    "prepare",
			    (GtkSignalFunc)gnome_druid_page_standard_prepare,
			    NULL);

}



void
gnome_druid_page_start_prepare (GnomeDruidPage *page,
				GtkWidget *druid,
				gpointer *data)
{
	gnome_druid_set_buttons_sensitive (GNOME_DRUID (druid), FALSE, TRUE, TRUE);
	gnome_druid_set_show_finish (GNOME_DRUID (druid), FALSE);
	gtk_widget_grab_default (GNOME_DRUID (druid)->next);
}

void
gnome_druid_page_start_configure_size (GnomeDruidPageStart *druid_page_start, gint width, gint height)
{
	gfloat watermark_width = DRUID_PAGE_LEFT_WIDTH;
	gfloat watermark_height = (gfloat) height - LOGO_WIDTH + GNOME_PAD * 2.0;
	gfloat watermark_ypos = LOGO_WIDTH + GNOME_PAD * 2.0;

	if (druid_page_start->watermark_image) {
		watermark_width = druid_page_start->watermark_image->rgb_width;
		watermark_height = druid_page_start->watermark_image->rgb_height;
		watermark_ypos = (gfloat) height - watermark_height;
		if (watermark_width < 1)
			watermark_width = 1.0;
		if (watermark_height < 1)
			watermark_height = 1.0;
	}

	gnome_canvas_item_set (druid_page_start->background_item,
			       "x1", 0.0,
			       "y1", 0.0,
			       "x2", (gfloat) width,
			       "y2", (gfloat) height,
			       "width_units", 1.0, NULL);
	gnome_canvas_item_set (druid_page_start->textbox_item,
			       "x1", watermark_width,
			       "y1", LOGO_WIDTH + GNOME_PAD * 2.0,
			       "x2", (gfloat) width,
			       "y2", (gfloat) height,
			       "width_units", 1.0, NULL);
	gnome_canvas_item_set (druid_page_start->logoframe_item,
			       "x1", (gfloat) width - LOGO_WIDTH -GNOME_PAD,
			       "y1", (gfloat) GNOME_PAD,
			       "x2", (gfloat) width - GNOME_PAD,
			       "y2", (gfloat) GNOME_PAD + LOGO_WIDTH,
			       "width_units", 1.0, NULL);
	gnome_canvas_item_set (druid_page_start->logo_item,
			       "x", (gfloat) width - GNOME_PAD - LOGO_WIDTH,
			       "y", (gfloat) GNOME_PAD,
			       "anchor", GTK_ANCHOR_NORTH_WEST,
			       "width", (gfloat) LOGO_WIDTH,
			       "height", (gfloat) LOGO_WIDTH, NULL);
	gnome_canvas_item_set (druid_page_start->watermark_item,
			       "x", 0.0,
			       "y", watermark_ypos,
			       "anchor", GTK_ANCHOR_NORTH_WEST,
			       "width", watermark_width,
			       "height", watermark_height,
			       NULL);
	gnome_canvas_item_set (druid_page_start->title_item,
			       "x", 15.0,
			       "y", (gfloat) GNOME_PAD + LOGO_WIDTH / 2.0,
			       "anchor", GTK_ANCHOR_WEST,
			       NULL);
	gnome_canvas_item_set (druid_page_start->text_item,
			       "x", ((width - watermark_width) * 0.5) + watermark_width,
			       "y", LOGO_WIDTH + GNOME_PAD * 2.0 + (height - (LOGO_WIDTH + GNOME_PAD * 2.0))/ 2.0,
			       "anchor", GTK_ANCHOR_CENTER,
			       NULL);
}
void
gnome_druid_page_start_construct (GnomeDruidPageStart *druid_page_start)
{
	/* set up the rest of the page */
	druid_page_start->background_item =
		gnome_canvas_item_new (gnome_canvas_root (GNOME_CANVAS (druid_page_start->canvas)),
				       gnome_canvas_rect_get_type (), NULL);

	druid_page_start->textbox_item =
		gnome_canvas_item_new (gnome_canvas_root (GNOME_CANVAS (druid_page_start->canvas)),
				       gnome_canvas_rect_get_type (), NULL);

	druid_page_start->logoframe_item =
		gnome_canvas_item_new (gnome_canvas_root (GNOME_CANVAS (druid_page_start->canvas)),
				       gnome_canvas_rect_get_type (), NULL);

	druid_page_start->logo_item =
		gnome_canvas_item_new (gnome_canvas_root (GNOME_CANVAS (druid_page_start->canvas)),
				       gnome_canvas_image_get_type (),
				       "image", druid_page_start->logo_image, NULL);

	if (druid_page_start->logo_image != NULL)
		gnome_canvas_item_set (druid_page_start->logo_item,
				       "image", druid_page_start->logo_image, NULL);


	druid_page_start->watermark_item =
		gnome_canvas_item_new (gnome_canvas_root (GNOME_CANVAS (druid_page_start->canvas)),
				       gnome_canvas_image_get_type (),
				       "image", druid_page_start->watermark_image, NULL);

	druid_page_start->title_item =
		gnome_canvas_item_new (gnome_canvas_root (GNOME_CANVAS (druid_page_start->canvas)),
				       gnome_canvas_text_get_type (),
				       "text", druid_page_start->title,
				       "font", "-adobe-helvetica-bold-r-normal-*-*-180-*-*-p-*-iso8859-1",
				       "fontset", "-adobe-helvetica-bold-r-normal-*-*-180-*-*-p-*-iso8859-1,*-r-*",
				       NULL);

	druid_page_start->text_item =
		gnome_canvas_item_new (gnome_canvas_root (GNOME_CANVAS (druid_page_start->canvas)),
				       gnome_canvas_text_get_type (),
				       "text", druid_page_start->text,
				       "justification", GTK_JUSTIFY_LEFT,
				       "font", "-adobe-helvetica-medium-r-normal-*-*-120-*-*-p-*-iso8859-1",
				       "fontset", "-adobe-helvetica-medium-r-normal-*-*-120-*-*-p-*-iso8859-1,*-r-*",
				       NULL);

	gnome_druid_page_start_configure_size (druid_page_start, DRUID_PAGE_WIDTH, DRUID_PAGE_HEIGHT);
	gtk_signal_connect (GTK_OBJECT (druid_page_start),
			    "prepare",
			    (GtkSignalFunc)gnome_druid_page_start_prepare,
			    NULL);
}

//This function doesn't exist at all in GNOME, but is in GNOME 2. I created it from the
//gnome_message_box_new() code.

void
gnome_message_box_constructv (GnomeMessageBox* message_box, 
            const gchar           *message,
		        const gchar           *message_box_type,
		      	const gchar 	     **buttons)
{
	GtkWidget *label, *hbox;
	GtkWidget *pixmap = NULL;
	char *s;
	GtkStyle *style;
	gint i = 0;

	style = gtk_widget_get_style (GTK_WIDGET (message_box));

	/* Make noises, basically */
	gnome_triggers_vdo(message, message_box_type, NULL);

	if (strcmp(GNOME_MESSAGE_BOX_INFO, message_box_type) == 0)
	{
		gtk_window_set_title (GTK_WINDOW (message_box), _("Information"));
		s = gnome_pixmap_file("gnome-info.png");
		if (s) {
                        pixmap = gnome_pixmap_new_from_file(s);
                        g_free(s);
                }
	}
	else if (strcmp(GNOME_MESSAGE_BOX_WARNING, message_box_type) == 0)
	{
		gtk_window_set_title (GTK_WINDOW (message_box), _("Warning"));
		s = gnome_pixmap_file("gnome-warning.png");
		if (s) {
                        pixmap = gnome_pixmap_new_from_file(s);
                        g_free(s);
                }
	}
	else if (strcmp(GNOME_MESSAGE_BOX_ERROR, message_box_type) == 0)
	{
		gtk_window_set_title (GTK_WINDOW (message_box), _("Error"));
		s = gnome_pixmap_file("gnome-error");
		if (s) {
                        pixmap = gnome_pixmap_new_from_file(s);
                        g_free(s);
                }
	}
	else if (strcmp(GNOME_MESSAGE_BOX_QUESTION, message_box_type) == 0)
	{
		gtk_window_set_title (GTK_WINDOW (message_box), _("Question"));
		s = gnome_pixmap_file("gnome-question.png");
		if (s) {
                        pixmap = gnome_pixmap_new_from_file(s);
                        g_free(s);
                }
	}
	else
	{
		gtk_window_set_title (GTK_WINDOW (message_box), _("Message"));
	}

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX(GNOME_DIALOG(message_box)->vbox),
			    hbox, TRUE, TRUE, 10);
	gtk_widget_show (hbox);

	if ( (pixmap == NULL) ||
	     (GNOME_PIXMAP(pixmap)->pixmap == NULL) ) {
        	if (pixmap) gtk_widget_destroy(pixmap);
		s = gnome_pixmap_file("gnome-default.png");
         	if (s) {
			pixmap = gnome_pixmap_new_from_file(s);
                        g_free(s);
                } else
			pixmap = NULL;
	}
	if (pixmap) {
		gtk_box_pack_start (GTK_BOX(hbox), 
				    pixmap, FALSE, TRUE, 0);
		gtk_widget_show (pixmap);
	}

	label = gtk_label_new (message);
	gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
	gtk_widget_show (label);

	while (buttons[i]) {
	  gnome_dialog_append_button ( GNOME_DIALOG(message_box), 
				       buttons[i]);
	  i++;
	};
	
	if (g_list_length (GNOME_DIALOG (message_box)->buttons) > 0) {
		gtk_widget_grab_focus ((GtkWidget*)g_list_last (GNOME_DIALOG (message_box)->buttons)->data);
	}
	
	gnome_dialog_set_close ( GNOME_DIALOG(message_box),
				 TRUE );
}

//Clock stuff:

static void gtk_clock_gen_str(GtkClock *clock)
{
	gchar timestr[64];
	time_t secs;

	switch (clock->type) {
	case GTK_CLOCK_DECREASING:
                secs = clock->seconds-time(NULL);
		break;
	case GTK_CLOCK_INCREASING:
                secs = time(NULL)-clock->seconds;
		break;
	case GTK_CLOCK_REALTIME:
		secs = time(NULL);
		break;
	}


	if (clock->type == GTK_CLOCK_REALTIME) {
		clock->tm = localtime(&secs);
	} else {
		clock->tm->tm_hour = secs/3600;
		secs -= clock->tm->tm_hour*3600;
		clock->tm->tm_min = secs/60;
		clock->tm->tm_sec = secs - clock->tm->tm_min*60;
	}
	
	strftime(timestr, 64, clock->fmt, clock->tm);
	gtk_label_set_text(GTK_LABEL(clock), timestr);
}

static gint gtk_clock_timer_callback(gpointer data)
{
	GtkClock *clock = (GtkClock *)data;

	GDK_THREADS_ENTER ();
	gtk_clock_gen_str(clock);
	GDK_THREADS_LEAVE ();

	return TRUE;
}

static gint gtk_clock_timer_first_callback(gpointer data)
{
	GtkClock *clock = (GtkClock *)data;
        gint tmpid;

	GDK_THREADS_ENTER();
	
	gtk_clock_gen_str(clock);

	tmpid =  gtk_timeout_add(1000*clock->update_interval,
				 gtk_clock_timer_callback, clock);

	gtk_clock_stop(clock);

	clock->timer_id = tmpid;

	GDK_THREADS_LEAVE();

	return FALSE;
}

void
gtk_clock_construct(GtkClock* clock, GtkClockType type)
{
	clock->type = (GtkClockType)type;
	
	if ( (clock->type) == GTK_CLOCK_REALTIME) {
		clock->fmt = g_strdup("%H:%M");
		clock->update_interval = 60;
		clock->tm = localtime(&clock->seconds);
		clock->timer_id = gtk_timeout_add(1000*(60-clock->tm->tm_sec),
						  gtk_clock_timer_first_callback, clock);
	} else {
		clock->fmt = g_strdup("%H:%M:%S");
		clock->tm = g_new(struct tm, 1);
		memset(clock->tm, 0, sizeof(struct tm));
		clock->update_interval = 1;
	}

	gtk_clock_gen_str(clock);	
}

} //namespace GNOME
