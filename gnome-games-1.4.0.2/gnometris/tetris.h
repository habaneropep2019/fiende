#ifndef __tetris_h__
#define __tetris_h__

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

#include <config.h>
#include <gnome.h>

#define TETRIS_VERSION "1.2.4"

extern int LINES;
extern int COLUMNS;

extern int BLOCK_SIZE;

enum SlotType 
{
	EMPTY, 
	FALLING, 
	LAYING
};

struct Block
{
	SlotType what;
	int color;
	GnomeCanvasItem* item;
};

extern GdkPixmap *pix;
extern GdkImlibImage **pic;

extern int color_next;
extern int blocknr_next;
extern int rot_next;

extern int blocknr;
extern int rot;
extern int color;

extern int posx;
extern int posy;

extern int nr_of_colors;

extern bool random_block_colors;
extern bool do_preview;

class	Field;
class Preview;
class	BlockOps;
class ScoreFrame;

class Tetris
{
public:
	Tetris(int cmdlLevel);
	~Tetris();
	
	GtkWidget * getWidget()	{return w;}
	void togglePause();
	void generate();
	void endOfGame();
	void setupPixmap();
	
private:
	GtkWidget * w;

	char *blockPixmap;
	char *bgPixmap;
	
	Field *field;
	Preview *preview;
	BlockOps *ops;
	ScoreFrame *scoreFrame;
	
	bool paused;
	int timeoutId;
	bool onePause;

	void generateTimer(int level);
	
	static gint eventHandler(GtkWidget *widget, GdkEvent *event, void *d);
	static int timeoutHandler(void *d);
	static int gameQuit(GtkWidget *widget, void *d);
	static int gameNew(GtkWidget *widget, void *d);
	static int gamePause(GtkWidget *widget, void *d);
	static int gameEnd(GtkWidget *widget, void *d);
	static int gameAbout(GtkWidget *widget, void *d);
	static int gameTopTen(GtkWidget *widget, void *d);
	static int gameProperties(GtkWidget *widget, void *d);
	static void setupdialogDestroy(GtkWidget *widget, void *d);
	static void doSetup(GtkWidget *widget, void *d);
 	static void setRotateCounterClockWise(GtkWidget *widget, void *d);
	static void setSelectionPreview(GtkWidget *widget, void *d);
	static void setSelectionBlocks(GtkWidget *widget, void *d);
	static void setSelection (GtkWidget *widget, void *data);
	static void setBGSelection (GtkWidget *widget, void *data);
	static void freeStr (GtkWidget *widget, void *data);
	
	static char *blockPixmapTmp;
	static char *bgPixmapTmp;

	GdkImlibImage *image;
	GdkImlibImage *bgimage;

	void fillMenu(GtkWidget *menu, char *pixname, char *dirname, GtkSignalFunc selectFunc, bool addnone = false);
	
	GtkWidget *setupdialog;
	GtkWidget *sentry;
	int startingLevel;
	int cmdlineLevel;
	bool doPreviewTmp;
	bool randomBlocksTmp;
 	bool rotateCounterClockWiseTmp;

	GtkWidget * fill_height_spinner;
	int line_fill_height;
	GtkWidget * fill_prob_spinner;
	int line_fill_prob;
	
	GnomeUIInfo *gameMenuPtr; 
	GnomeUIInfo *gameSettingsPtr; 

	void manageFallen();
	void showScores(gchar *title, guint pos);

	bool fastFall;
	int fastFallPoints;
};

#endif // __tetris_h__






