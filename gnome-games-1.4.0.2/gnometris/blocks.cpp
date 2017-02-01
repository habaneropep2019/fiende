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

int blockTable[][4][4][4] =
{
	{
    {
    	{0, 1, 0, 0},
    	{0, 1, 0, 0},
    	{0, 1, 1, 0},
    	{0, 0, 0, 0},
    },
    {
    	{0, 0, 1, 0},
    	{1, 1, 1, 0},
    	{0, 0, 0, 0},
    	{0, 0, 0, 0},
    },
    {
    	{1, 1, 0, 0},
    	{0, 1, 0, 0},
    	{0, 1, 0, 0},
    	{0, 0, 0, 0},
    },
    {
    	{0, 0, 0, 0},
    	{1, 1, 1, 0},
    	{1, 0, 0, 0},
    	{0, 0, 0, 0},
    },
	},

	{
    {
    	{0, 1, 0, 0},
    	{0, 1, 0, 0},
    	{1, 1, 0, 0},
    	{0, 0, 0, 0},
    },
    {
    	{0, 0, 0, 0},
    	{1, 1, 1, 0},
    	{0, 0, 1, 0},
    	{0, 0, 0, 0},
    },
    {
    	{0, 1, 1, 0},
    	{0, 1, 0, 0},
    	{0, 1, 0, 0},
    	{0, 0, 0, 0},
    },
    {
    	{1, 0, 0, 0},
    	{1, 1, 1, 0},
    	{0, 0, 0, 0},
    	{0, 0, 0, 0},
    },
	},

	{
  	{
  		{0, 1, 0, 0},
  		{1, 1, 1, 0},
  		{0, 0, 0, 0},
  		{0, 0, 0, 0},
  	},
  	{
  		{0, 1, 0, 0},
  		{1, 1, 0, 0},
  		{0, 1, 0, 0},
  		{0, 0, 0, 0},
  	},
  	{
  		{0, 0, 0, 0},
  		{1, 1, 1, 0},
  		{0, 1, 0, 0},
  		{0, 0, 0, 0},
  	},
  	{
  		{0, 1, 0, 0},
  		{0, 1, 1, 0},
  		{0, 1, 0, 0},
  		{0, 0, 0, 0},
  	},
	},

	{
  	{
  		{0, 1, 1, 0},
  		{1, 1, 0, 0},
  		{0, 0, 0, 0},
  		{0, 0, 0, 0},
  	},
  	{
  		{1, 0, 0, 0},
  		{1, 1, 0, 0},
  		{0, 1, 0, 0},
  		{0, 0, 0, 0},
  	},
  	{
  		{0, 0, 0, 0},
  		{0, 1, 1, 0},
  		{1, 1, 0, 0},
  		{0, 0, 0, 0},
  	},
  	{
  		{0, 1, 0, 0},
  		{0, 1, 1, 0},
  		{0, 0, 1, 0},
  		{0, 0, 0, 0},
  	},
	},

	{
  	{
  		{1, 1, 0, 0},
  		{0, 1, 1, 0},
  		{0, 0, 0, 0},
  		{0, 0, 0, 0},
  	},
  	{
  		{0, 1, 0, 0},
  		{1, 1, 0, 0},
  		{1, 0, 0, 0},
  		{0, 0, 0, 0},
  	},
  	{
  		{0, 0, 0, 0},
  		{1, 1, 0, 0},
  		{0, 1, 1, 0},
  		{0, 0, 0, 0},
  	},
  	{
  		{0, 0, 1, 0},
  		{0, 1, 1, 0},
  		{0, 1, 0, 0},
  		{0, 0, 0, 0},
  	},
	},

	{
  	{
  		{0, 0, 0, 0},
  		{1, 1, 1, 1},
  		{0, 0, 0, 0},
  		{0, 0, 0, 0},
  	},
  	{
  		{0, 1, 0, 0},
  		{0, 1, 0, 0},
  		{0, 1, 0, 0},
  		{0, 1, 0, 0},
  	},
  	{
  		{0, 0, 0, 0},
  		{0, 0, 0, 0},
  		{1, 1, 1, 1},
  		{0, 0, 0, 0},
  	},
  	{
  		{0, 0, 1, 0},
  		{0, 0, 1, 0},
  		{0, 0, 1, 0},
  		{0, 0, 1, 0},
  	},
	},

	{
  	{
  		{0, 1, 1, 0},
  		{0, 1, 1, 0},
  		{0, 0, 0, 0},
  		{0, 0, 0, 0},
  	},
  	{
  		{0, 1, 1, 0},
  		{0, 1, 1, 0},
  		{0, 0, 0, 0},
  		{0, 0, 0, 0},
  	},
  	{
  		{0, 1, 1, 0},
  		{0, 1, 1, 0},
  		{0, 0, 0, 0},
  		{0, 0, 0, 0},
  	},
  	{
  		{0, 1, 1, 0},
  		{0, 1, 1, 0},
  		{0, 0, 0, 0},
  		{0, 0, 0, 0},
  	},
	}
};

int tableSize = sizeof(blockTable)/sizeof(blockTable[0]);

int sizeTable[][4][2] =
{
	{
		{2, 3},
		{3, 2},
		{2, 3},
		{3, 2},
	},
	{
		{2, 3},
		{3, 2},
		{2, 3},
		{3, 2},
	},
	{
		{3, 2},
		{2, 3},
		{3, 2},
		{2, 3},
	},
	{
		{3, 2},
		{2, 3},
		{3, 2},
		{2, 3},
	},
	{
		{3, 2},
		{2, 3},
		{3, 2},
		{2, 3},
	},
	{
		{4, 1},
		{1, 4},
		{4, 1},
		{1, 4},
	},
	{
		{2, 2},
		{2, 2},
		{2, 2},
		{2, 2},
	},
};
		
int sizeTSize = sizeof(sizeTable)/sizeof(sizeTable[0]);

int offsetTable[][4][2] = 
{
	{
		{1, 0},
		{0, 0},
		{0, 0},
		{0, 1},
	},
	{
		{0, 0},
		{0, 1},
		{1, 0},
		{0, 0},
	},
	{
		{0, 0},
		{0, 0},
		{0, 1},
		{1, 0},
	},
	{
		{0, 0},
		{0, 0},
		{0, 1},
		{1, 0},
	},
	{
		{0, 0},
		{0, 0},
		{0, 1},
		{1, 0},
	},
	{
		{0, 1},
		{1, 0},
		{0, 2},
		{2, 0},
	},
	{
		{1, 0},
		{1, 0},
		{1, 0},
		{1, 0},
	},
};

int offsetTSize = sizeof(offsetTable)/sizeof(offsetTable[0]);




