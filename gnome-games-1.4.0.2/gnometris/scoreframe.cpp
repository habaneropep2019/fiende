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
#include "scoreframe.h"

ScoreFrame::ScoreFrame(int cmdlLevel)
	: score(0), lines(0)
{
  startingLevel = cmdlLevel ? cmdlLevel : gnome_config_get_int_with_default("/gnometris/Properties/StartingLevel=1", 0);
	level = startingLevel;
	
	w = gtk_frame_new(_("Game Status"));

	scoreLabel = gtk_label_new(_("Score: "));
	sprintf(b, "%7d", 0);
	scorew = gtk_label_new(b);

	linesLabel = gtk_label_new(_("Lines: "));
	sprintf(b, "%7d", 0);
	linesw = gtk_label_new(b);

	levelLabel = gtk_label_new(_("Level: "));
	sprintf(b, "%7d", level);
	levelw = gtk_label_new(b);

	vb = gtk_vbox_new(FALSE, 0);
	hbScore = gtk_hbox_new(FALSE, 0);
	hbLines = gtk_hbox_new(FALSE, 0);
	hbLevel = gtk_hbox_new(FALSE, 0);

	gtk_container_add(GTK_CONTAINER(w), vb);
  gtk_container_border_width(GTK_CONTAINER(vb), 10);

	gtk_box_pack_start(GTK_BOX(vb), hbScore, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(vb), hbLines, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(vb), hbLevel, 0, 0, 0);

	gtk_box_pack_start(GTK_BOX(hbScore), scoreLabel,  0, 0, 0);
	gtk_box_pack_end(GTK_BOX(hbScore), scorew, 0, 0, 0);

	gtk_box_pack_start(GTK_BOX(hbLines), linesLabel,  0, 0, 0);
	gtk_box_pack_end(GTK_BOX(hbLines), linesw, 0, 0, 0);

	gtk_box_pack_start(GTK_BOX(hbLevel), levelLabel,  0, 0, 0);
	gtk_box_pack_end(GTK_BOX(hbLevel), levelw, 0, 0, 0);
}

void
ScoreFrame::show()
{
	gtk_widget_show(scoreLabel);
	gtk_widget_show(linesLabel);
	gtk_widget_show(levelLabel);
	gtk_widget_show(scorew);
	gtk_widget_show(linesw);
	gtk_widget_show(levelw);
	gtk_widget_show(hbScore);
	gtk_widget_show(hbLines);
	gtk_widget_show(hbLevel);
	gtk_widget_show(vb);
	gtk_widget_show(w);
}

void 
ScoreFrame::setScore(int s)
{
	score = s;
	
	sprintf(b, "%7d", score);
	gtk_label_set(GTK_LABEL(scorew), b);
}

void
ScoreFrame::incScore(int s)
{
	score += s;
	
	sprintf(b, "%7d", score);
	gtk_label_set(GTK_LABEL(scorew), b);
}

void
ScoreFrame::resetLines()
{
	lines = 0;
	score = 0;
	sprintf(b, "%7d", lines);
	gtk_label_set(GTK_LABEL(scorew), b);
	gtk_label_set(GTK_LABEL(linesw), b);
}

void 
ScoreFrame::incLines(int newlines)
{
	int linescore = 0;

	lines += newlines;
	switch(newlines)
	{
		case 1:
			linescore = 40;
			break;
		case 2:
			linescore = 100;
			break;
		case 3:
			linescore = 300;
			break;
		case 4:
			linescore = 1200;
			break;
	}
	incScore(linescore * level);
	sprintf(b, "%7d", lines);
	gtk_label_set(GTK_LABEL(linesw), b);
}

void 
ScoreFrame::checkLevel()
{
	int l = startingLevel + lines / 10;
	if ((l > level) && (l <= 10))
	{
		level = l;
		sprintf(b, "%7d", level);
		gtk_label_set(GTK_LABEL(levelw), b);
	}
}

void 
ScoreFrame::setLevel(int l)
{
	level = l;
	sprintf(b, "%7d", level);
	gtk_label_set(GTK_LABEL(levelw), b);
}

void
ScoreFrame::setStartingLevel(int l)
{
	startingLevel = l;
}




