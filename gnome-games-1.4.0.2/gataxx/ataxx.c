/*
 * ataxx.c - gameplay code for gataxx
 * Written by Chris Rogers (gandalf@pobox.com)
 * Based on iagno code written by  Ian Peters (itp@gnu.org)
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

#include "config.h"
#include <gnome.h>

#include "ataxx.h"
#include "gataxx.h"

guint flip_final_id = 0;
gint flip_final;

extern guint black_computer_level;
extern guint white_computer_level;

extern gint animate;

extern guint whose_turn;
extern guint game_in_progress;

extern gint8 pixmaps[7][7];
extern gint8 board[7][7];
extern MoveHistory game[500]; /*since we can't make an acurate guess at a min, this should be a linked list*/

extern gint8 move_count;

extern gint bcount;
extern gint wcount;

extern gint timer_valid;

extern GtkWidget *time_display;

extern guint tiles_to_flip;


/* Wrapper for is_valid_move_board, to maintain API for CORBA stuff */

gint is_valid_piece(guint x, guint y, guint me) {
  return is_valid_piece_board(board, x, y, me);
}

gint is_valid_piece_board(gint8 board[7][7], guint x, guint y, guint me) {

  if (board[x][y] == me) return TRUE;
  return FALSE;
}

gint is_valid_move(guint x, guint y, guint selected_x, guint selected_y, guint me) {
  return is_valid_move_board(board, x, y, selected_x, selected_y, me);
}

/* Check if a given square is a valid move for one of the players */

gint is_valid_move_board(gint8 board[7][7], guint x, guint y, guint selected_x, guint selected_y, guint me){

  guint not_me;
  

  not_me = (me == WHITE_TURN) ? BLACK_TURN : WHITE_TURN;
  
  if(board[x][y] != 0)
    return(FALSE);
  
  /*basically make sure that the current and selected 
    positions are no more than 2 spaces away*/

  if ((abs(y-selected_y) <= 1) && (abs(x-selected_x) <= 1))
    return 1;
  
  if ((abs(y-selected_y) <= 2) && (abs(x-selected_x) <= 2)) 
    return 2;

  return(FALSE);
}

/* Wrapper for move_board, to maintain API for CORBA stuff */

gint move(guint x, guint y, guint selected_x, guint selected_y, guint me) {

  int retval;
  
  retval = move_board(board, x, y, selected_x, selected_y, me, 1);
  
  check_valid_moves();
  check_computer_players();
  
  return retval;
}


gint move_board(gint8 board[7][7], guint x, guint y, guint selected_x, guint selected_y, guint me, gint real) {
  guint not_me;
  gint count = 1;
  gint take_count = 0;
  
  /* Stuff to do if this is a ``real'' move */

  not_me = (me == WHITE_TURN) ? BLACK_TURN : WHITE_TURN;
 
  if(real) {
    
    /* Copy the old board and move info to the undo buffer */
    
    memcpy(game[move_count].board, board, sizeof(gint8) * 7 * 7);
    game[move_count].x = x;
    game[move_count].y = y;
    game[move_count].me = me;
    
    move_count++;
    
    if(whose_turn == WHITE_TURN) {
      whose_turn = BLACK_TURN;
      gui_message(_("Dark's move"));
      if(!white_computer_level) {
	gtk_clock_stop(GTK_CLOCK(time_display));
      }
    } else {
      whose_turn = WHITE_TURN;
      gui_message(_("Light's move"));
      if(!black_computer_level) {
	gtk_clock_stop(GTK_CLOCK(time_display));
      }
    }
    
    
    if (!((abs(y-selected_y) <= 1) && (abs(x-selected_x) <= 1))) 
      {
	pixmaps[selected_x][selected_y] = 0;
	gui_draw_pixmap(0, selected_x, selected_y);
      }
    pixmaps[x][y] = me;
    gui_draw_pixmap(me, x, y);
  }
  
 
  
  board[x][y] = me;
  if (!((abs(y-selected_y) <= 1) && (abs(x-selected_x) <= 1))) 
    {
      board[selected_x][selected_y] = 0;
      count--;
    }
  
  if (((gint) x - 1 >= 0) && (board[x-1][y] == not_me)) { board[x-1][y] = me; take_count++;}
  if ((x + 1 <= 6) && (board[x+1][y] == not_me)) { board[x+1][y] = me; take_count++; }
  
  if (((gint)y - 1 >= 0) && (board[x][y-1] == not_me)) {board[x][y-1] = me; take_count++; }
  if ((y + 1 <= 6) && (board[x][y+1] == not_me)) {board[x][y+1] = me; take_count++; }

  if ((x + 1 <= 6) && (y + 1 <= 6) && (board[x+1][y+1] == not_me)) {board[x+1][y+1] = me;take_count++; }
  if ((x + 1 <= 6) && ((gint)y - 1 >= 0) && (board[x+1][y-1] == not_me)) {board[x+1][y-1] = me;take_count++; }
  if (((gint)x - 1 >= 0) && (y + 1 <= 6) && (board[x-1][y+1] == not_me)) {board[x-1][y+1] = me;take_count++; }
  if (((gint)x - 1 >= 0) && (((gint)y - 1) >= 0) && (board[x-1][y-1] == not_me)) {board[x-1][y-1] = me;take_count++; }

  /* More stuff for a ``real'' move */

  if(real) {
    
    /* Update the statusbar counters */
    
    if(me == BLACK_TURN) {
      bcount = bcount + count + take_count;
      wcount -= take_count;
    } else {
      wcount += count + take_count;
      bcount -= take_count;
    }
    
    gui_status();
    
    if(not_me == BLACK_TURN && !black_computer_level && timer_valid) {
      gtk_clock_start(GTK_CLOCK(time_display));
    }
    if(not_me == WHITE_TURN && !white_computer_level && timer_valid) {
      gtk_clock_start(GTK_CLOCK(time_display));
    }
    
    tiles_to_flip = 1;
  }
  
  return(FALSE);
}


gint computer_move_3(guint me)
{
  guint xs[320], ys[320], xss[320], yss[320];
  guint weights[320];
  gint num_maxes = 0;
  gint maxpieces = -1;
  gint tmp;
  guint num_moves = 0;
  guint i, j, k, l;
  gint8 tboard[7][7];
  gint * tmplist;
  
  for(i = 0; i < 7; i++)
    for(j = 0; j < 7; j++)
      if(is_valid_piece(i, j, me)) 
	for(k = 0; k < 7; k++)
	  for(l = 0; l < 7; l++)
	    if(is_valid_move(k,l,i,j,me)){

	      memcpy(tboard, board, sizeof(gint8) * 7 * 7);
	      move_board(tboard, k, l, i, j, me, FALSE);
	      
	      tmp = count_pieces(tboard,me);
	      if (tmp == maxpieces) {
		xss[num_moves] = i;
		yss[num_moves] = j;
		xs[num_moves] = k;
		ys[num_moves] = l;
		weights[num_moves] = tmp;
		num_maxes++;
		num_moves++;
	      } 
	      else if (tmp > maxpieces) {
		xss[num_moves] = i;
		yss[num_moves] = j;
		xs[num_moves] = k;
		ys[num_moves] = l;
		weights[num_moves] = tmp;
		maxpieces = tmp;
		num_maxes = 1;
		num_moves++;
	      }
	    }


  if(num_moves) {
    tmplist = g_malloc(sizeof(gint) * num_maxes);
    j = 0;
    for (i = 0; i < num_moves; i++) 
      if (weights[i] == maxpieces) {
	tmplist[j++] = i;
      }
    j = (rand()>>3) % num_maxes;
    i = tmplist[j];
    move(xs[i], ys[i], xss[i], yss[i], me);
    g_free(tmplist);
  }
  
  return(FALSE);
}

gint computer_move_2(guint me) {

  guint xs[1000], ys[1000], xss[1000], yss[1000]; /*another canidate for linked lists*/
  gint weights[1000];
  gint maxpieces = 0;
  gint maxwhite = 0;
  gint maxwhite2 = -50;
  gint tmp,tmp2, tmp3;
  guint num_moves = 0;
  guint i, j, k, l;
  guint i2,j2,k2,l2;
  gint8 tboard[7][7],t2board[7][7] ;
  guint not_me;

 
  gint * tmplist;
  gint num_maxes = 0;

  
  not_me = (me == WHITE_TURN) ? BLACK_TURN : WHITE_TURN;

  for(i = 0; i < 7; i++)
    for(j = 0; j < 7; j++)
      if(is_valid_piece(i, j, me)) 
	for(k = 0; k < 7; k++)
	  for(l = 0; l < 7; l++)
	    if(is_valid_move(k,l,i,j,me)){

	      maxpieces = 0;
	      memcpy(tboard, board, sizeof(gint8) * 7 * 7);
	      move_board(tboard, k, l, i, j, me, FALSE);
	      maxwhite = 0;
	      for(i2 = 0; i2 < 7; i2++)
		for(j2 = 0; j2 < 7; j2++)
		  if(is_valid_piece(i2, j2, not_me)) 
		    for(k2 = 0; k2 < 7; k2++)
		      for(l2 = 0; l2 < 7; l2++)
			if(is_valid_move(k2,l2,i2,j2,not_me)){

			  memcpy(t2board, tboard, sizeof(gint8) * 7 * 7);
			  move_board(t2board, k2, l2, i2, j2, not_me, FALSE);
			  
			  tmp = count_pieces(t2board,not_me);
			  tmp3 = count_pieces(t2board, me);
			  
			  /*			  g_print("maxblack:%d maxwhite:%d\n",tmp,tmp3);  */
			  if (tmp > maxpieces) {
			    maxwhite = tmp3;
			    maxpieces = tmp;
			  } 
			  else if ((tmp == maxpieces) && (tmp3 < maxwhite))
			    maxwhite = tmp3;
			}
			  
	      tmp2 = (maxwhite);
	      /*	      g_print("weight:%d\n",tmp2); */
	      if ( tmp2 == maxwhite2) {
		
		xss[num_moves] = i;
		yss[num_moves] = j;
		xs[num_moves] = k;
		ys[num_moves] = l;
		weights[num_moves] = maxwhite2;
		num_maxes++;
		num_moves++;
		/*		g_print("num_moves:%d weight:%d\n",num_moves,maxwhite2);*/
	      } 
	      else if (tmp2> maxwhite2){
		xss[num_moves] = i;
		yss[num_moves] = j;
		xs[num_moves] = k;
		ys[num_moves] = l;
		maxwhite2 = tmp2;
		weights[num_moves] = maxwhite2;

		num_maxes = 1;
		num_moves++;
		/*		g_print("num_moves:%d weight:%d\n",num_moves,maxwhite2);*/
	      }
	    }
  
  if(num_moves) {
    tmplist = g_malloc(sizeof(gint) * num_maxes);
   /* g_print("num_maxes:%d\n",num_maxes); */
    j = 0;
    for (i = 0; i < num_moves; i++) {
/*      g_print("weight:%d, max:%d\n",weights[i],maxwhite2); */
      if (weights[i] == maxwhite2) {
	tmplist[j++] = i;
      }
    }
/*    g_print("maxes assigned:%d\n",j); */
    j = (rand()>>3) % num_maxes;
/*    g_print("index into tmplist:%d\n",j); */
    i = tmplist[j];
/*    g_print("i:%d\n",i); */
    move(xs[i], ys[i], xss[i], yss[i], me);
    g_free(tmplist);
  }

  return(FALSE);

}

gint computer_move_1(guint me) {

  guint xs[1000], ys[1000], xss[1000], yss[1000]; /*another canidate for linked lists*/
  gint weights[1000];
  gint maxpieces = 0;
  gint maxwhite = 0;
  gint maxwhite2 = -50;
  gint tmp,tmp2, tmp3;
  guint num_moves = 0;
  guint i, j, k, l;
  guint i2,j2,k2,l2;
  gint8 tboard[7][7],t2board[7][7] ;
  guint not_me;

 
  gint * tmplist;
  gint num_maxes = 0;

  
  not_me = (me == WHITE_TURN) ? BLACK_TURN : WHITE_TURN;

  for(i = 0; i < 7; i++)
    for(j = 0; j < 7; j++)
      if(is_valid_piece(i, j, me)) 
	for(k = 0; k < 7; k++)
	  for(l = 0; l < 7; l++)
	    if(is_valid_move(k,l,i,j,me)){

	      maxpieces = 0;
	      memcpy(tboard, board, sizeof(gint8) * 7 * 7);
	      move_board(tboard, k, l, i, j, me, FALSE);
	      maxwhite = 0;
	      for(i2 = 0; i2 < 7; i2++)
		for(j2 = 0; j2 < 7; j2++)
		  if(is_valid_piece(i2, j2, not_me)) 
		    for(k2 = 0; k2 < 7; k2++)
		      for(l2 = 0; l2 < 7; l2++)
			if(is_valid_move(k2,l2,i2,j2,not_me)){

			  memcpy(t2board, tboard, sizeof(gint8) * 7 * 7);
			  move_board(t2board, k2, l2, i2, j2, not_me, FALSE);
			  
			  tmp = count_pieces(t2board,not_me);
			  tmp3 = count_pieces(t2board, me);
			  
			  /*			  g_print("maxblack:%d maxwhite:%d\n",tmp,tmp3);  */
			  if (tmp > maxpieces) {
			    maxwhite = tmp3;
			    maxpieces = tmp;
			  } 
			  else if ((tmp == maxpieces) && (tmp3 < maxwhite))
			    maxwhite = tmp3;
			}
			  
	      tmp2 = (maxwhite - maxpieces);
	/*	      g_print("weight:%d\n",tmp2); */
	      if ( tmp2 == maxwhite2) {
		xss[num_moves] = i;
		yss[num_moves] = j;
		xs[num_moves] = k;
		ys[num_moves] = l;
		weights[num_moves] = maxwhite2;
		num_maxes++;
		num_moves++;
	/*	g_print("num_moves:%d weight:%d\n",num_moves,maxwhite2);
*/
	      } 
	      else if (tmp2> maxwhite2){
		xss[num_moves] = i;
		yss[num_moves] = j;
		xs[num_moves] = k;
		ys[num_moves] = l;
		maxwhite2 = tmp2;
		weights[num_moves] = maxwhite2;

		num_maxes = 1;
		num_moves++;
	/*	g_print("num_moves:%d weight:%d\n",num_moves,maxwhite2);
*/
	      }
	    }
  
  if(num_moves) {
    tmplist = g_malloc(sizeof(gint) * num_maxes);
 /*   g_print("num_maxes:%d\n",num_maxes); */
    j = 0;
    for (i = 0; i < num_moves; i++) {
    /*   g_print("weight:%d, max:%d\n",weights[i],maxwhite2); */
      if (weights[i] == maxwhite2) {
	tmplist[j++] = i;
      }
    }
   /* g_print("maxes assigned:%d\n",j); */
    j = (rand()>>3) % num_maxes;
   /* g_print("index into tmplist:%d\n",j); */
    i = tmplist[j];
/*    g_print("i:%d\n",i); */
    move(xs[i], ys[i], xss[i], yss[i], me);
    g_free(tmplist);
  }

  return(FALSE);

}



gint count_pieces(gint8 board[7][7], gint me) {
  guint tmp = 0;
  guint i, j;
  
  for(i = 0; i < 7; i++)
    for(j = 0; j < 7; j++)
      if(board[i][j] == me)
	tmp++;
  
  return(tmp);
}

gint flip_final_results()
{
  guint i;
  guint white_pieces;
  guint black_pieces;
  guint adder = 0;
  
  white_pieces = count_pieces(board, WHITE_TURN);
  black_pieces = count_pieces(board, BLACK_TURN);
  
  for(i = 0; i < black_pieces; i++) {
    board[i % 7][i / 7] = BLACK_TURN;
    if(pixmaps[i % 7][i / 7] < 1)
      pixmaps[i % 7][i / 7] = WHITE_TURN;
    if(pixmaps[i % 7][i / 7] == WHITE_TURN) {
      pixmaps[i % 7][i / 7] += adder;
    }
  }
  for(i = black_pieces; i < 49 - white_pieces; i++) {
    board[i % 7][i / 7] = 0;
    pixmaps[i % 7][i / 7] = 100;
  }
  for(i = 49 - white_pieces; i < 49; i++) {
    board[i % 7][i / 7] = WHITE_TURN;
    if(pixmaps[i % 7][i / 7] == 0)
      pixmaps[i % 7][i / 7] = BLACK_TURN;
    if(pixmaps[i % 7][i / 7] == BLACK_TURN) {
      pixmaps[i % 7][i / 7] -= adder;
    }
  }
  
  tiles_to_flip = 1;
  return(FALSE);
}

gint check_valid_moves()
{
  
  guint i, j, k, l;
  guint white_moves = 0;
  guint black_moves = 0;


  if(!game_in_progress)
    return(TRUE);

  switch(whose_turn) {
  case WHITE_TURN:
    for(i = 0; i < 7; i++)
      for(j = 0; j < 7; j++)
	if(is_valid_piece(i, j, WHITE_TURN)) 
	  for(k = 0; k < 7; k++)
	    for(l = 0; l < 7; l++)
	      if(is_valid_move(k,l,i,j,WHITE_TURN))
		return(TRUE);
    break;
  case BLACK_TURN:
    for(i = 0; i < 7; i++)
      for(j = 0; j < 7; j++)
	if(is_valid_piece(i, j, BLACK_TURN)) 
	  for(k = 0; k < 7; k++)
	    for(l = 0; l < 7; l++)
	      if(is_valid_move(k,l,i,j,BLACK_TURN))
		return(TRUE);
    break;
  }

  switch(whose_turn) {
  case WHITE_TURN:
    for(i = 0; i < 7; i++)
      for(j = 0; j < 7; j++)
	if(is_valid_piece(i, j, BLACK_TURN)) 
	  for(k = 0; k < 7; k++)
	    for(l = 0; l < 7; l++)
	      if(is_valid_move(k,l,i,j,BLACK_TURN))
		black_moves++;
    break;
  case BLACK_TURN:
    for(i = 0; i < 7; i++)
      for(j = 0; j < 7; j++)
	if(is_valid_piece(i, j, WHITE_TURN)) 
	  for(k = 0; k < 7; k++)
	    for(l = 0; l < 7; l++)
	      if(is_valid_move(k,l,i,j,WHITE_TURN))
		white_moves++;
    break;
  }

  if(!white_moves || !black_moves) {
    gtk_clock_stop(GTK_CLOCK(time_display));
    white_moves = count_pieces(board, WHITE_TURN);
    black_moves = count_pieces(board, BLACK_TURN);
    if(white_moves > black_moves)
      gui_message(_("Light player wins!"));
    if(black_moves > white_moves)
      gui_message(_("Dark player wins!"));
    if(white_moves == black_moves)
      gui_message(_("The game was a draw."));
    whose_turn = 0;
    game_in_progress = 0;
    if (flip_final)
      flip_final_id = gtk_timeout_add(3000,flip_final_results, NULL);
    return(TRUE);
  }

  if(whose_turn == WHITE_TURN) {
    gui_message(_("Light must pass, Dark's move"));
    whose_turn = BLACK_TURN;
    if(white_computer_level ^ black_computer_level) {
      if(!black_computer_level && timer_valid)
	gtk_clock_start(GTK_CLOCK(time_display));
      else
	gtk_clock_stop(GTK_CLOCK(time_display));
    }
    return(TRUE);
  }

  if(whose_turn == BLACK_TURN) {
    gui_message(_("Dark must pass, Light's move"));
    whose_turn = WHITE_TURN;
    if(white_computer_level ^ black_computer_level) {
      if(!white_computer_level && timer_valid)
	gtk_clock_start(GTK_CLOCK(time_display));
      else
	gtk_clock_stop(GTK_CLOCK(time_display));
    }
    return(TRUE);
  }
  
  return(TRUE);
}

