/*
 * written by J. Marcin Gorycki <marcin.gorycki@intel.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * For more details see the file COPYING.
 */

#include "tetris.h"
#include "field.h"
#include "blockops.h"
#include "blocks.h"
#include "preview.h"
#include "scoreframe.h"

#include <gdk/gdkkeysyms.h>
#include <config.h>
#include <dirent.h>

GdkPixmap *pix;

GdkImlibImage **pic;

int LINES = 20;
int COLUMNS = 11;

int BLOCK_SIZE = 40;

int posx = COLUMNS / 2;
int posy = 0;

int blocknr = 0;
int rot = 0;
int color = 0;

int blocknr_next = -1;
int rot_next = -1;
int color_next = -1;

bool random_block_colors = false;
bool do_preview = true;
bool rotateCounterClockWise = true;

char *Tetris::blockPixmapTmp = 0;
char *Tetris::bgPixmapTmp = 0;

Tetris::Tetris(int cmdlLevel): 
	blockPixmap(0),
	bgPixmap(0),
	field(0),
	preview(0),
	scoreFrame(0),
	paused(false), 
	timeoutId(-1), 
	onePause(false), 
	image(0),
	bgimage(0),
	setupdialog(0), 
	cmdlineLevel(cmdlLevel), 
	doPreviewTmp(do_preview), 
	randomBlocksTmp(random_block_colors),
	fastFall(false)
{
	pic = new GdkImlibImage*[tableSize];
	for (int i = 0; i < tableSize; ++i)
		pic[i] = 0;
	
	w = gnome_app_new("gnometris", _("Gnometris"));
	gtk_window_set_policy(GTK_WINDOW(w), FALSE, FALSE, TRUE);
	gtk_signal_connect(GTK_OBJECT(w), "delete_event", (GtkSignalFunc)gameQuit, this);

	static GnomeUIInfo game_menu[] = 
	{
		GNOMEUIINFO_MENU_NEW_GAME_ITEM(gameNew, this),
		GNOMEUIINFO_MENU_PAUSE_GAME_ITEM(gamePause, this),
		GNOMEUIINFO_SEPARATOR,
		GNOMEUIINFO_MENU_SCORES_ITEM(gameTopTen, this),
		GNOMEUIINFO_MENU_END_GAME_ITEM(gameEnd, this),
		GNOMEUIINFO_SEPARATOR,
		GNOMEUIINFO_MENU_EXIT_ITEM(gameQuit, this),
		GNOMEUIINFO_END
	};

	gameMenuPtr = game_menu;
	
	static GnomeUIInfo settings_menu[] = 
	{
		GNOMEUIINFO_MENU_PREFERENCES_ITEM(gameProperties, this),
		GNOMEUIINFO_END
	};

	gameSettingsPtr = settings_menu;
	
	GnomeUIInfo help_menu[] = 
	{
		GNOMEUIINFO_HELP((gpointer)"gnometris"),
		GNOMEUIINFO_MENU_ABOUT_ITEM(gameAbout, this),
		GNOMEUIINFO_END
	};

	GnomeUIInfo mainmenu[] = 
	{
		GNOMEUIINFO_MENU_GAME_TREE(game_menu),
		GNOMEUIINFO_MENU_SETTINGS_TREE(settings_menu),
		GNOMEUIINFO_MENU_HELP_TREE(help_menu),
		GNOMEUIINFO_END
	};

	line_fill_height = 0;
	line_fill_prob = 5;

	blockPixmap = strdup(gnome_config_get_string_with_default(
		"/gnometris/Properties/BlockPixmap=7blocks-tig.png", 0));
	bgPixmap = strdup(gnome_config_get_string_with_default(
		"/gnometris/Properties/BackgroundPixmap=fishy-bg.png", 0));

	gnome_app_create_menus(GNOME_APP(w), mainmenu);

	GtkWidget * hb = gtk_hbox_new(FALSE, 0);
	gnome_app_set_contents(GNOME_APP(w), hb);

	field = new Field(/*ops*/);
	ops = new BlockOps(field);

	gtk_widget_set_events(w, gtk_widget_get_events(w) | 
						  GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK);
	
	GtkWidget *vb1 = gtk_vbox_new(FALSE, 0);
	gtk_container_border_width(GTK_CONTAINER(vb1), 10);
	gtk_box_pack_start_defaults(GTK_BOX(vb1), field->getWidget());
	gtk_box_pack_start(GTK_BOX(hb), vb1, 0, 0, 0);
	field->show();
	setupPixmap();

	gtk_signal_connect(GTK_OBJECT(w), "event", (GtkSignalFunc)eventHandler, this);
  
	GtkWidget *vb2 = gtk_vbox_new(FALSE, 0);
	gtk_container_border_width(GTK_CONTAINER(vb2), 10);
	gtk_box_pack_end(GTK_BOX(hb), vb2, 0, 0, 0);
	
	preview = new Preview();
	
	gtk_box_pack_start(GTK_BOX(vb2), preview->getWidget(), 0, 0, 0);
	
	preview->show();

	scoreFrame = new ScoreFrame(cmdlineLevel);
	
	gtk_box_pack_end(GTK_BOX(vb2), scoreFrame->getWidget(), 0, 0, 0);

	gtk_widget_show(hb);
	gtk_widget_show(vb1);
	gtk_widget_show(vb2);
	scoreFrame->show();
	gtk_widget_show(w);

	gtk_widget_set_sensitive(gameMenuPtr[1].widget, FALSE);
	gtk_widget_set_sensitive(gameMenuPtr[4].widget, FALSE);
	gtk_widget_set_sensitive(gameSettingsPtr[0].widget, TRUE);
}

Tetris::~Tetris()
{
	delete ops;
	delete field;
	delete preview;
	delete scoreFrame;

	if (image)
		gdk_imlib_destroy_image(image);
	if (bgimage)
		gdk_imlib_destroy_image(bgimage);

	if (blockPixmap)
		g_free(blockPixmap);
	if (bgPixmap)
		g_free(bgPixmap);

	delete[] pic;
}

void 
Tetris::setupdialogDestroy(GtkWidget *widget, void *d)
{
	Tetris *t = (Tetris*) d;
	if (t->setupdialog)
		gtk_widget_destroy(t->setupdialog);
	t->setupdialog = 0;
}

void
Tetris::setupPixmap()
{
	char *pixname, *fullpixname;
	
	pixname = g_copy_strings("gnometris/", blockPixmap, 0);
	fullpixname = gnome_unconditional_pixmap_file(pixname);
	g_free(pixname);

	if (!g_file_exists(fullpixname)) 
	{
                char *message = g_strdup_printf (
                        "Could not find the theme:\n%s\n\n"
                        "Please check you Gnometris instalation", fullpixname);
                GtkWidget *w = gnome_error_dialog (message);
                gnome_dialog_run_and_close (GNOME_DIALOG(w));
                g_free (message);
		exit(1);
	}

	if(image)
		gdk_imlib_destroy_image(image);

	image = gdk_imlib_load_image(fullpixname);

	if (image == NULL) {
		char *message = g_strdup_printf (
			"Same Gnome can't load the image file:\n%s\n\n"
			"Please check you Gnometris instalation", fullpixname);
		GtkWidget *w = gnome_error_dialog (message);
		gnome_dialog_run_and_close (GNOME_DIALOG(w));
		g_free (message);
		exit (1);
	}

	gdk_imlib_render(image, image->rgb_width, image->rgb_height);
	pix = gdk_imlib_move_image(image);
	
	BLOCK_SIZE = image->rgb_height;
	nr_of_colors = image->rgb_width / BLOCK_SIZE;

	for (int i = 0; i < tableSize; ++i)
	{
		if (pic[i])
			gdk_imlib_destroy_image(pic[i]);
		pic[i] = gdk_imlib_crop_and_clone_image(image, (i % nr_of_colors) * 
												BLOCK_SIZE, 0, BLOCK_SIZE, BLOCK_SIZE);
	}

	pixname = g_copy_strings("gnometris/bg/", bgPixmap, 0);
	fullpixname = gnome_unconditional_pixmap_file(pixname);
	g_free(pixname);

	if(bgimage)
		gdk_imlib_destroy_image(bgimage);

	if (g_file_exists(fullpixname)) 
		bgimage = gdk_imlib_load_image(fullpixname);
	else
		bgimage = 0;

	if (field)
	{
		field->updateSize(bgimage);
		gtk_widget_draw(field->getWidget(), 0);
	}
	
	if (preview)
	{
		preview->updateSize();
		gtk_widget_draw(preview->getWidget(), 0);
	}

	// FIXME: this really sucks, but I can't find a better way to resize 
	// all widgets after the block pixmap change
	if (scoreFrame)
	{
		int l = scoreFrame->getLevel();
		scoreFrame->setLevel(l + 1);
		scoreFrame->setLevel(l);
	}
	
}

void
Tetris::doSetup(GtkWidget *widget, void *d)
{
	Tetris *t = (Tetris*) d;
	t->cmdlineLevel = 0;
	t->startingLevel = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(t->sentry));
	t->line_fill_prob = gtk_spin_button_get_value_as_int(
		GTK_SPIN_BUTTON(t->fill_prob_spinner));
	t->line_fill_height = gtk_spin_button_get_value_as_int(
		GTK_SPIN_BUTTON(t->fill_height_spinner));
	do_preview = t->doPreviewTmp;
	random_block_colors = t->randomBlocksTmp;
 	rotateCounterClockWise = t->rotateCounterClockWiseTmp;
	
	gnome_config_set_int("/gnometris/Properties/StartingLevel", t->startingLevel);
	gnome_config_set_int("/gnometris/Properties/DoPreview", do_preview);
	gnome_config_set_int("/gnometris/Properties/RandomBlockColors", random_block_colors);
 	gnome_config_set_int("/gnometris/Properties/RotateCounterClockWise", 
						 rotateCounterClockWise);
	gnome_config_set_int("/gnometris/Properties/LineFillHeight", t->line_fill_height);
	gnome_config_set_int("/gnometris/Properties/LineFillProbability", t->line_fill_prob);
	
	if (blockPixmapTmp)
	{
		if (t->blockPixmap)
			free(t->blockPixmap);
		t->blockPixmap = strdup(blockPixmapTmp);
	}
	if (bgPixmapTmp)
	{
		if (t->bgPixmap)
			free(t->bgPixmap);
		t->bgPixmap = strdup(bgPixmapTmp);
	}
	
	gnome_config_set_string("/gnometris/Properties/BlockPixmap", t->blockPixmap);
	gnome_config_set_string("/gnometris/Properties/BackgroundPixmap", t->bgPixmap);
	
	gnome_config_sync();
	
	t->scoreFrame->setLevel(t->startingLevel);
	t->scoreFrame->setStartingLevel(t->startingLevel);
	setupdialogDestroy(widget, d);
	t->setupPixmap();
}

void 
Tetris::setSelectionPreview(GtkWidget *widget, void *d)
{
	Tetris *t = (Tetris*) d;
	t->doPreviewTmp = GTK_TOGGLE_BUTTON(widget)->active;
}

void 
Tetris::setSelectionBlocks(GtkWidget *widget, void *d)
{
	Tetris *t = (Tetris*) d;
	t->randomBlocksTmp = GTK_TOGGLE_BUTTON(widget)->active;
}

void 
Tetris::setRotateCounterClockWise(GtkWidget *widget, void *d)
{
	Tetris *t = (Tetris*) d;
	t->rotateCounterClockWiseTmp = GTK_TOGGLE_BUTTON(widget)->active;
}

void
Tetris::setSelection(GtkWidget *widget, void *data)
{
	blockPixmapTmp = (char*) data;
//	g_print("s:bl: %s\n", blockPixmapTmp);
}

void
Tetris::setBGSelection(GtkWidget *widget, void *data)
{
	bgPixmapTmp = (char*) data;
//	g_print("s:bg: %s\n", bgPixmapTmp);
}

void
Tetris::freeStr(GtkWidget *widget, void *data)
{
	free(data);
}

void
Tetris::fillMenu(GtkWidget *menu, char *pixname, char *dirname, 
				 GtkSignalFunc selectFunc, bool addnone /*= false*/)
{
	struct dirent *e;
	char *dname = gnome_unconditional_pixmap_file(dirname);
	DIR *dir;
	int itemno = 0;
	
	dir = opendir (dname);

	if (!dir)
		return;
	
	GtkWidget *item;
	char *s;
	
	while ((e = readdir (dir)) != 0)
	{
		s = strdup(e->d_name);

		if (!strstr (e->d_name, ".png")) 
		{
			free(s);
			continue;
		}
			
		item = gtk_menu_item_new_with_label(s);
		gtk_widget_show(item);
		gtk_menu_append(GTK_MENU(menu), item);
		gtk_signal_connect(GTK_OBJECT(item), "activate", selectFunc, s);
		gtk_signal_connect(GTK_OBJECT(item), "destroy", (GtkSignalFunc) freeStr, s);
	  
		if (!strcmp(pixname, s))
		{
		  gtk_menu_set_active(GTK_MENU(menu), itemno);
		}
		itemno++;
	}
	
	if (addnone)
	{
		s = strdup("<none>");
		item = gtk_menu_item_new_with_label(s);
		gtk_widget_show(item);
		gtk_menu_append(GTK_MENU(menu), item);
		gtk_signal_connect(GTK_OBJECT(item), "activate", selectFunc, s);
		gtk_signal_connect(GTK_OBJECT(item), "destroy", (GtkSignalFunc) freeStr, s);
	}
	
	closedir(dir);
}

int 
Tetris::gameProperties(GtkWidget *widget, void *d)
{
	GtkWidget *allBoxes;
	GtkWidget *box, *box2;
//	GtkWidget *button;
	GtkWidget *label;
	GtkWidget *frame;
	GtkObject *adj;
        
	Tetris *t = (Tetris*) d;
	
	if (t->setupdialog) 
		return FALSE;


	t->line_fill_height = gnome_config_get_int_with_default(
		"/gnometris/Properties/LineFillHeight=0",0);
	t->line_fill_prob = gnome_config_get_int_with_default(
		"/gnometris/Properties/LineFillProbability=5",0);

	t->setupdialog = gnome_dialog_new(_("Gnometris setup"), 
									  GNOME_STOCK_BUTTON_OK, 
									  GNOME_STOCK_BUTTON_CANCEL, 
									  0);
	gnome_dialog_set_parent(GNOME_DIALOG(t->setupdialog), GTK_WINDOW(t->w));

	gtk_container_border_width(GTK_CONTAINER(t->setupdialog), 10);
	GTK_WINDOW(t->setupdialog)->position = GTK_WIN_POS_MOUSE;
	gtk_signal_connect(GTK_OBJECT(t->setupdialog), "close",
					   GTK_SIGNAL_FUNC(setupdialogDestroy), d);

	allBoxes = gtk_vbox_new(FALSE, 5);
	
	frame = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(allBoxes), frame, TRUE, TRUE, 0);
	
	box = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), box);

	gtk_widget_show(box);
	gtk_widget_show(frame);

	box2 = gtk_hbox_new(FALSE, 0);
	gtk_container_border_width(GTK_CONTAINER(box2), 10);
	gtk_box_pack_start(GTK_BOX(box), box2, TRUE, TRUE, 0);
	label = gtk_label_new(_("Number of pre-filled rows:"));
	gtk_box_pack_start(GTK_BOX(box2), label, 0, 0, 0);
	gtk_widget_show(label);
	adj = gtk_adjustment_new(t->line_fill_height,0,LINES-1,1,5,0);
	t->fill_height_spinner = gtk_spin_button_new(GTK_ADJUSTMENT(adj),10,0);
	gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(t->fill_height_spinner),
									  GTK_UPDATE_ALWAYS
#ifndef HAVE_GTK_SPIN_BUTTON_SET_SNAP_TO_TICKS
									  | GTK_UPDATE_SNAP_TO_TICKS
#endif
									  );
#ifdef HAVE_GTK_SPIN_BUTTON_SET_SNAP_TO_TICKS
	gtk_spin_button_set_snap_to_ticks(GTK_SPIN_BUTTON(t->fill_height_spinner), 1);
#endif
	gtk_box_pack_end(GTK_BOX(box2), t->fill_height_spinner, 0, 0, 0);
	gtk_widget_show(t->fill_height_spinner);	
	gtk_widget_show(box2);

	box2 = gtk_hbox_new(FALSE, 0);
	gtk_container_border_width(GTK_CONTAINER(box2), 10);
	gtk_box_pack_start(GTK_BOX(box), box2, TRUE, TRUE, 0);
	label = gtk_label_new(_("Density of blocks in a pre-filled row:"));
	gtk_box_pack_start(GTK_BOX(box2), label, 0, 0, 0);
	gtk_widget_show(label);
	adj = gtk_adjustment_new(t->line_fill_prob,0,10,1,5,0);
	t->fill_prob_spinner = gtk_spin_button_new(GTK_ADJUSTMENT(adj),10,0);
	gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(t->fill_prob_spinner),
									  GTK_UPDATE_ALWAYS
#ifndef HAVE_GTK_SPIN_BUTTON_SET_SNAP_TO_TICKS
									  | GTK_UPDATE_SNAP_TO_TICKS
#endif
									  );
#ifdef HAVE_GTK_SPIN_BUTTON_SET_SNAP_TO_TICKS
	gtk_spin_button_set_snap_to_ticks(GTK_SPIN_BUTTON(t->fill_prob_spinner), 1);
#endif
	gtk_box_pack_end(GTK_BOX(box2), t->fill_prob_spinner, 0, 0, 0);
	gtk_widget_show(t->fill_prob_spinner);	
	gtk_widget_show(box2);

	box2 = gtk_hbox_new(FALSE, 0);
	gtk_container_border_width(GTK_CONTAINER(box2), 10);
	gtk_box_pack_start(GTK_BOX(box), box2, TRUE, TRUE, 0);
	label = gtk_label_new(_("Starting Level:"));
	gtk_box_pack_start(GTK_BOX(box2), label, 0, 0, 0);
	gtk_widget_show(label);

	t->startingLevel = gnome_config_get_int_with_default(
		"/gnometris/Properties/StartingLevel=1", 0);
	random_block_colors = gnome_config_get_int_with_default(
		"/gnometris/Properties/RandomBlockColors=0", 0) != 0;
	do_preview = gnome_config_get_int_with_default(
		"/gnometris/Properties/DoPreview=0", 0) != 0;
 	rotateCounterClockWise = gnome_config_get_int_with_default(
		"/gnometris/Properties/RotateCounterClockWise=0", NULL) != 0;

	adj = gtk_adjustment_new(t->startingLevel, 1, 10, 1, 5, 10);
	t->sentry = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 10, 0);
	gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(t->sentry),
									  GTK_UPDATE_ALWAYS
#ifndef HAVE_GTK_SPIN_BUTTON_SET_SNAP_TO_TICKS
									  | GTK_UPDATE_SNAP_TO_TICKS
#endif
									  );
#ifdef HAVE_GTK_SPIN_BUTTON_SET_SNAP_TO_TICKS
	gtk_spin_button_set_snap_to_ticks(GTK_SPIN_BUTTON(t->sentry), 1);
#endif
	gtk_box_pack_end(GTK_BOX(box2), t->sentry, FALSE, 0, 0);
	gtk_widget_show(t->sentry);
	gtk_widget_show(box2);

	GtkWidget *cb = gtk_check_button_new_with_label(_("Preview next block"));
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(cb), do_preview);
	gtk_signal_connect(GTK_OBJECT(cb), "clicked", (GtkSignalFunc)setSelectionPreview, d);
	gtk_box_pack_start(GTK_BOX(box), cb, 0, 0, 0);
	gtk_widget_show(cb);

	cb = gtk_check_button_new_with_label(_("Random block colors"));
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(cb), random_block_colors);
	gtk_signal_connect(GTK_OBJECT(cb), "clicked", (GtkSignalFunc)setSelectionBlocks, d);
	gtk_box_pack_start(GTK_BOX(box), cb, 0, 0, 0);
	gtk_widget_show(cb);

 	cb = gtk_check_button_new_with_label(_("Rotate counterclockwise"));
 	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(cb), rotateCounterClockWise);
 	gtk_signal_connect(GTK_OBJECT(cb), "clicked", 
					   (GtkSignalFunc)setRotateCounterClockWise, d);
 	gtk_box_pack_start(GTK_BOX(box), cb, 0, 0, 0);
 	gtk_widget_show(cb);

	gtk_widget_show(box);

	blockPixmapTmp=0;
	box2 = gtk_hbox_new(FALSE, 0);
	gtk_container_border_width(GTK_CONTAINER(box2), 10);
	gtk_box_pack_start(GTK_BOX(box), box2, TRUE, TRUE, 0);
	label = gtk_label_new(_("Block Pixmap: "));
	gtk_box_pack_start(GTK_BOX(box2), label, 0, 0, 0);
	gtk_widget_show(label);
	GtkWidget *omenu = gtk_option_menu_new();
	GtkWidget *menu = gtk_menu_new();
	t->fillMenu(menu, t->blockPixmap, "gnometris", (GtkSignalFunc)setSelection);
	gtk_widget_show(omenu);
	gtk_option_menu_set_menu(GTK_OPTION_MENU(omenu), menu);
	gtk_box_pack_end(GTK_BOX(box2), omenu, 0, 0, 0);
	gtk_widget_show(box2);

	bgPixmapTmp=0;
	box2 = gtk_hbox_new(FALSE, 0);
	gtk_container_border_width(GTK_CONTAINER(box2), 10);
	gtk_box_pack_start(GTK_BOX(box), box2, TRUE, TRUE, 0);
	label = gtk_label_new(_("Background Pixmap: "));
	gtk_box_pack_start(GTK_BOX(box2), label, 0, 0, 0);
	gtk_widget_show(label);
	omenu = gtk_option_menu_new();
	menu = gtk_menu_new();
	t->fillMenu(menu, t->bgPixmap, "gnometris/bg", (GtkSignalFunc)setBGSelection, true);
	gtk_widget_show(omenu);
	gtk_option_menu_set_menu(GTK_OPTION_MENU(omenu), menu);
	gtk_box_pack_end(GTK_BOX(box2), omenu, 0, 0, 0);
	gtk_widget_show(box2);

	gtk_box_pack_start_defaults(GTK_BOX(GNOME_DIALOG(t->setupdialog)->vbox), allBoxes);
	gtk_widget_show(allBoxes);

	gnome_dialog_button_connect(GNOME_DIALOG(t->setupdialog), 0, 
								GTK_SIGNAL_FUNC(doSetup), d);
	gnome_dialog_button_connect(GNOME_DIALOG(t->setupdialog), 1, 
								GTK_SIGNAL_FUNC(setupdialogDestroy), d);
	
	gtk_widget_show(t->setupdialog);
	return TRUE;
}

int
Tetris::gamePause(GtkWidget *widget, void *d)
{
	Tetris *t = (Tetris*) d;
	t->togglePause();
	return TRUE;
}

int
Tetris::gameEnd(GtkWidget *widget, void *d)
{
	Tetris *t = (Tetris*) d;
	
	gtk_timeout_remove(t->timeoutId);
	t->timeoutId = -1;
	blocknr_next = -1;
	t->endOfGame();
	return TRUE;
}

int
Tetris::gameQuit(GtkWidget *widget, void *d)
{
	Tetris *t = (Tetris*) d;

	if (t->w)
		gtk_widget_destroy(t->w);
	gtk_main_quit();

	return TRUE;
}

void
Tetris::generateTimer(int level)
{
	gtk_timeout_remove(timeoutId);

	int intv = 1000 - 100 * (level - 1);
	if (intv <= 0)
		intv = 100;
		
	timeoutId = gtk_timeout_add(intv, timeoutHandler, this);
}

void
Tetris::manageFallen()
{
	ops->fallingToLaying();

	int levelBefore = scoreFrame->getLevel();
	ops->checkFullLines(scoreFrame);
	int levelAfter = scoreFrame->getLevel();
	if ((levelBefore != levelAfter) || fastFall)
		generateTimer(levelAfter);
	
	generate();
}

int
Tetris::timeoutHandler(void *d)
{
	Tetris *t = (Tetris*) d;
	
	if (t->paused)
		return 1;

 	if (t->onePause)
 	{
		t->onePause = false;
		gtk_widget_draw(t->field->getWidget(), 0);
	}
 	else
	{
		bool res = t->ops->moveBlockDown();
		gtk_widget_draw(t->field->getWidget(), 0);

		if (res)
		{
			t->manageFallen();
			if(t->fastFall)
				t->scoreFrame->incScore(t->fastFallPoints);
			else
				t->fastFall = false;
		}
		else if(t->fastFall)
			++t->fastFallPoints;
	}

	return 1;	
}

gint
Tetris::eventHandler(GtkWidget *widget, GdkEvent *event, void *d)
{
	Tetris *t = (Tetris*) d;
	
	if (t->timeoutId == -1)
		return FALSE;
	
	if (event->type == GDK_KEY_PRESS)
	{
		if (((GdkEventKey*)event)->keyval == GDK_space)
		{
			t->togglePause();
			return TRUE;
		}
	}
	
	if (t->paused)
		return FALSE;
	
	bool res = false;
	bool keyEvent = false;
	
	switch (event->type)
	{
	case GDK_KEY_PRESS: 
	{
		GdkEventKey *e = (GdkEventKey*)event;
		keyEvent = true;
		
		switch(e->keyval)
		{
		case GDK_Left:
			res = t->ops->moveBlockLeft();
			break;
		case GDK_Right:
			res = t->ops->moveBlockRight();
			break;
		case GDK_Up:
			res = t->ops->rotateBlock(rotateCounterClockWise);
			break;
		case GDK_Down:
			if (!t->fastFall)
			{
				t->fastFall = true;
				t->fastFallPoints = 0;
				gtk_timeout_remove(t->timeoutId);
				t->timeoutId = gtk_timeout_add(10, timeoutHandler, t);
				res = true;
			}
			break;
		default:
			return FALSE;
		}
		break;
	}
	case GDK_KEY_RELEASE:
	{
		GdkEventKey *e = (GdkEventKey*)event;
		if (e->keyval == GDK_Down)
		{
			keyEvent = true;
			if (t->fastFall)
			{
				t->fastFall = false;
 				t->generateTimer(t->scoreFrame->getLevel());
			}
		}
		break;
	}
	default:
		break;
	}

	if (res)
		gtk_widget_draw(t->field->getWidget(), 0);

	return (keyEvent == true);
}

void
Tetris::togglePause()
{
	paused = !paused;
}

void
Tetris::generate()
{
	if (ops->generateFallingBlock())
	{
		ops->putBlockInField(false);
		gtk_widget_draw(preview->getWidget(), 0);
		onePause = true;
	}
	else
	{
		gtk_timeout_remove(timeoutId);
		timeoutId = -1;
		blocknr_next = -1;
		
		endOfGame();
	}
}

void
Tetris::endOfGame()
{
	gtk_widget_set_sensitive(gameMenuPtr[1].widget, FALSE);
	gtk_widget_set_sensitive(gameMenuPtr[4].widget, FALSE);
	gtk_widget_set_sensitive(gameSettingsPtr[0].widget, TRUE);
	color_next = -1;
	blocknr_next = -1;
	rot_next = -1;
	
	if (scoreFrame->getScore() > 0) 
	{
		int pos = gnome_score_log(scoreFrame->getScore(), 0, TRUE);
		showScores("Gnometris", pos);
	}
}

void
Tetris::showScores(gchar *title, guint pos)
{
	gnome_scores_display(title, "gnometris", 0, pos);
}

int
Tetris::gameNew(GtkWidget *widget, void *d)
{
	Tetris *t = (Tetris*) d;

	if (t->timeoutId) 
	{
		gtk_timeout_remove(t->timeoutId);
		t->timeoutId = -1;

		if (t->scoreFrame->getScore() > 0) 
			gnome_score_log(t->scoreFrame->getScore(), 0, TRUE);
	}

	t->fastFall = false;
	
	int level = t->cmdlineLevel ? t->cmdlineLevel :
		gnome_config_get_int_with_default("/gnometris/Properties/StartingLevel=1", 0);
	t->scoreFrame->setLevel(level);
	t->scoreFrame->setStartingLevel(level);
	random_block_colors = gnome_config_get_int_with_default(
		"/gnometris/Properties/RandomBlockColors=0", 0) != 0;
	do_preview = gnome_config_get_int_with_default(
		"/gnometris/Properties/DoPreview=0", 0) != 0;
	t->line_fill_height = gnome_config_get_int_with_default(
		"/gnometris/Properties/LineFillHeight=0",0);
	t->line_fill_prob = gnome_config_get_int_with_default(
		"/gnometris/Properties/LineFillProbability=5",0);

	t->timeoutId = gtk_timeout_add(1000 - 100 * (level - 1), timeoutHandler, t);
	t->ops->emptyField(t->line_fill_height,t->line_fill_prob);

	t->scoreFrame->resetLines();
	t->paused = false;
	
	t->ops->generateFallingBlock();
	gtk_widget_draw(t->field->getWidget(), 0);
	gtk_widget_draw(t->preview->getWidget(), 0);

	gtk_widget_set_sensitive(t->gameMenuPtr[1].widget, TRUE);
	gtk_widget_set_sensitive(t->gameMenuPtr[4].widget, TRUE);
	gtk_widget_set_sensitive(t->gameSettingsPtr[0].widget, FALSE);

	return TRUE;
}

int
Tetris::gameAbout(GtkWidget *widget, void *d)
{
	Tetris *t = (Tetris*) d;

	GtkWidget *about;

	const gchar *authors[] = {"J. Marcin Gorycki", 0};

	about = gnome_about_new("Gnometris", TETRIS_VERSION, "(C) 2000 J. Marcin Gorycki", 
							(const char **)authors,
							_("Written for my wife, Matylda\n"
							  "Send comments and bug reports to: "
							  "janusz.gorycki@intel.com"), 0);
	gnome_dialog_set_parent(GNOME_DIALOG(about), GTK_WINDOW(t->w));
	gtk_window_set_modal(GTK_WINDOW(about), TRUE);

	gtk_widget_show(about);

	return TRUE;
}

int
Tetris::gameTopTen(GtkWidget *widget, void *d)
{
	Tetris *t = (Tetris*) d;
	t->showScores("Gnometris", 0);

	return TRUE;
}











