#ifndef __preview_h__
#define __preview_h__

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

#include <gnome.h>
#include "tetris.h"

#define PREVIEW_SIZE 5

class Preview
{
public:
	Preview();
	
	void show();
	void updateSize();
	
	GtkWidget * getWidget()	{return w;}
	
private:
	GtkWidget * w;

	void paint(GdkRectangle *area);

	static gint eventHandler(GtkWidget *widget, GdkEvent *event, void *d);
};

#endif //__preview_h__

