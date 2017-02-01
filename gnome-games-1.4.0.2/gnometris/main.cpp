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
#include "tetris.h"
#include <libgnomeui/gnome-window-icon.h>

int nr_of_colors = 0;

int
main(int argc, char *argv[])
{
	bindtextdomain(PACKAGE, GNOMELOCALEDIR);
	textdomain(PACKAGE);
	
	int cmdlineLevel = 0;

	poptOption options[] = 
	{
		{"level", 'l', POPT_ARG_INT, &cmdlineLevel, 0, N_("Set starting level (1-10)"), N_("LEVEL")},
		{0, '\0', 0, 0, 0}
	};

	srand(time(0));

	gnome_score_init("gnometris");

	gnome_init_with_popt_table("gnometris", TETRIS_VERSION, argc, argv, options, 0, 0);
	gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-gtetris.png");

//	GnomeClient *client= gnome_master_client();
	
	Tetris * t = new Tetris(cmdlineLevel);

	gtk_main();

	delete t;
	
	return 0;
}


