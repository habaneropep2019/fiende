/*
 * Gnome-Mahjonggg-solubility header file
 * (C) 1998 the Free Software Foundation
 *
 *
 * Author: Michael Meeks.
 *
 *
 * http://www.imaginator.com/~michael
 * michael@imaginator.com
 *
 */

/* If defined this allows graphical placement debugging.
 * define, then run and hit new game. Then press keys as
 * in keyPress to change things */
/* #define PLACE_DEBUG */

extern void generate_game () ;
extern void generate_dependancies () ;
extern int tile_free (int) ;

