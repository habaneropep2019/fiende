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

#include "blockops.h"
#include "blocks.h"
#include "field.h"

GnomeCanvasItem *BlockOps::generateItem(int x, int y, int color)
{
	GnomeCanvasItem *	it = gnome_canvas_item_new(
  			gnome_canvas_root(GNOME_CANVAS(fieldDisplay->getWidget())),
  			gnome_canvas_image_get_type(),
  			"image", pic[color],
  			"x", (double) x * BLOCK_SIZE,
  			"y", (double) y * BLOCK_SIZE,
  			"width", (double) BLOCK_SIZE,
  			"height", (double) BLOCK_SIZE,
  			"anchor", GTK_ANCHOR_NW,
  			0);

	return it;
}

BlockOps::BlockOps(Field *f)
{
	field = new Block*[COLUMNS];
	fieldDisplay = f;
	
	for (int i = 0; i < COLUMNS; ++i)
		field[i] = new Block[LINES];

	field_initialized = false;
	emptyField();
}

BlockOps::~BlockOps()
{
	for (int i = 0; i < COLUMNS; ++i)
		delete[] field[i];
	
	delete[] field;
}

bool 
BlockOps::blockOkHere(int x, int y, int b, int r)
{
	x -= 2;
	
	for (int x1 = 0; x1 < 4; ++x1)
	{
		for (int y1 = 0; y1 < 4; ++y1)
		{
			if (blockTable[b][r][x1][y1] && (x1 + x < 0))
				return false;
			if (blockTable[b][r][x1][y1] && (x1 + x >= COLUMNS))
				return false;
			if (blockTable[b][r][x1][y1] && (y1 + y >= LINES))
				return false;
			if (blockTable[b][r][x1][y1] && field[x + x1][y1 + y].what == LAYING)
				return false;
		}
	}
	return true;	
}

int
BlockOps::getLinesToBottom()
{
	int lines = LINES;
	
	for (int x = 0; x < 4; ++x)
	{
		for (int y = 3; y >= 0; --y)
		{
			if (!blockTable[blocknr][rot][x][y])
				continue;
			int yy = posy + y;
			for (; yy < LINES; ++yy)
			{
				if (field[posx + x - 2][yy].what == LAYING)
					break;
			}
			int tmp = yy - posy - y;
			if (lines > tmp)
				lines = tmp;
		}
	}
	return lines;
}

bool 
BlockOps::moveBlockLeft()
{
	bool moved = false;

	if (blockOkHere(posx - 1, posy, blocknr, rot))
	{
		putBlockInField(true);
		--posx;
		putBlockInField(false);
		moved = true;
	}
	
	return moved;
}

bool 
BlockOps::moveBlockRight()
{
	bool moved = false;

	if (blockOkHere(posx + 1, posy, blocknr, rot))
	{
		putBlockInField(true);
		++posx;
		putBlockInField(false);
		moved = true;
	}
	
	return moved;
}

bool
BlockOps::rotateBlock(bool rotateCCW)
{
	bool moved = false;
	
	int r = rot;
	
	if ( rotateCCW )
	{
		if (--r < 0) r = 3;
	}
	else
	{
		if (++r >= 4) r = 0;
	}
		
	if (blockOkHere(posx, posy, blocknr, r))
	{
		putBlockInField(true);
		rot = r;
		putBlockInField(false);
		moved = true;
	}
	
	return moved;
}

bool 
BlockOps::moveBlockDown()
{
	bool fallen = false;

	if (!blockOkHere(posx, posy + 1, blocknr, rot))
		fallen = true;

	if (!fallen)
	{
		putBlockInField(true);
		++posy;
		putBlockInField(false);
	}
	
	return fallen;
}

void
BlockOps::dropBlock()
{
	while (!moveBlockDown())
		;
}

void
BlockOps::fallingToLaying()
{
	for (int x = 0; x < COLUMNS; ++x)
		for (int y = 0; y < LINES; ++y)
			if (field[x][y].what == FALLING)
				field[x][y].what = LAYING;
}

void
BlockOps::checkFullLines(ScoreFrame *s)
{
	bool found;
	int numFullLines = 0;
	
	do
	{
		found = false;
		
		for (int y = LINES - 1; y >= 0; --y)
		{
			bool f = true;
			for (int x = 0; x < COLUMNS; ++x)
			{
				if (field[x][y].what != LAYING)
				{
					f = false;
					break;
				}
			}
			if (f)
			{
				++numFullLines;
				s->checkLevel();
				
				found = true;
				for (int y1 = y; y1 >= 0; --y1)
				{
					for (int x = 0; x < COLUMNS; ++x)
					{
						if (y1 < y)
							field[x][y1 + 1] = field[x][y1];
						field[x][y1].what = EMPTY;
						if (field[x][y1].item)
						{
							gtk_object_destroy(GTK_OBJECT(field[x][y1].item));
							field[x][y1].item = 0;
							if (y1 < y)
								field[x][y1 + 1].item = 
									generateItem(x, y1 + 1, field[x][y1 + 1].color);
						}
						
					}
				}
				break;
			}
		}
	}
	while (found);
	if(numFullLines > 0)
		s->incLines(numFullLines);
}

bool
BlockOps::generateFallingBlock()
{
	posx = COLUMNS / 2 + 1;
	posy = 0;

	blocknr = blocknr_next == -1 ? rand() % tableSize : blocknr_next;
	rot = rot_next == -1 ? rand() % 4 : rot_next;
	int cn = random_block_colors ? rand() % nr_of_colors : blocknr % nr_of_colors;
	color = color_next == -1 ? cn : color_next;
	
	blocknr_next = rand() % tableSize;
	rot_next = rand() % 4;
	color_next = random_block_colors ? rand() % nr_of_colors : 
		blocknr_next % nr_of_colors;
	
	if (!blockOkHere(posx, posy, blocknr, rot))
		return false;

	return true;
}

void
BlockOps::emptyField(int filled_lines, int fill_prob)
{
	int blank;

	for (int y = 0; y < LINES; ++y)
	{
		blank = rand() % COLUMNS; // Allow for at least one blank per line
		for (int x = 0; x < COLUMNS; ++x)
		{
			if (field_initialized && field[x][y].item != 0)
			{
				gtk_object_destroy(GTK_OBJECT(field[x][y].item));
			}
			field[x][y].item = 0;
			field[x][y].what = EMPTY;
			if ((y>=(LINES - filled_lines)) && (x != blank) &&
			    ((rand() % 10) < fill_prob)) { 
				field[x][y].what = LAYING;
				if (nr_of_colors)
					field[x][y].color = rand() % nr_of_colors;
				else
					// This is in case we're called
					// before the widgets are set 
					// up and nr_of_colors is
					// defined.
					field[x][y].color = 0; 
				field[x][y].item = generateItem(x,y,field[x][y].color);
			}
		}
	}
	field_initialized = true;
}

void
BlockOps::emptyField(void)
{
	emptyField(0,5);
}

void
BlockOps::putBlockInField(bool erase)
{
	for (int x = 0; x < 4; ++x)
	{
		for (int y = 0; y < 4; ++y)
		{
			if (blockTable[blocknr][rot][x][y])
			{
				field[posx - 2 + x][y + posy].what = erase ? EMPTY : FALLING;
				if (erase)
				{
					GnomeCanvasItem *o = field[posx - 2 + x][y + posy].item;
					if (o)
						gtk_object_destroy(GTK_OBJECT(o));
					field[posx - 2 + x][y + posy].item = 0;
				}
				else
				{
					field[posx - 2 + x][y + posy].color = color;
					field[posx - 2 + x][y + posy].item = 
						generateItem(posx - 2 + x, y + posy, 
									 field[posx - 2 + x][y + posy].color);
				}
			}
		}
	}
}




