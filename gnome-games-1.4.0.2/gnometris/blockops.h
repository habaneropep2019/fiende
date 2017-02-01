#ifndef __blockops_h__
#define __blockops_h__

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
#include "scoreframe.h"

class BlockOps
{
public:
	BlockOps(Field *f);
	~BlockOps();
	
	bool moveBlockLeft();
	bool moveBlockRight();
	bool moveBlockDown();
	bool rotateBlock(bool);
	void dropBlock();
	void fallingToLaying();
	void checkFullLines(ScoreFrame *s);
	bool generateFallingBlock();
	void emptyField(void);
	void emptyField(int filled_lines, int fill_prob);
	void putBlockInField(bool erase);
	int getLinesToBottom();
	GnomeCanvasItem *generateItem(int x, int y, int color);

	const Block * const getFieldAt(int x, int y) {return &(field[x][y]);}

private:
	bool blockOkHere(int x, int y, int b, int r);
	Block **field;
	bool field_initialized;
	Field *fieldDisplay;
};

#endif //__blockops_h__
