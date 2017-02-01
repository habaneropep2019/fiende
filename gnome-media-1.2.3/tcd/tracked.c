/* This file is part of TCD 2.0.
   tracked.c - Curses track editor module.

   Copyright (C) 1997-98 Tim P. Gerla <timg@rrv.net>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
                   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
                                   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
                                               
   Tim P. Gerla
   RR 1, Box 40
   Climax, MN  56523
   timg@rrv.net
*/
   

#include <fcntl.h>
#if defined(USE_NCURSES) && !defined(RENAMED_NCURSES)
#include <ncurses.h>
#else
#include <curses.h>
#endif
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <math.h>

#include <sys/ioctl.h>
#include <sys/types.h>

#include "linux-cdrom.h"
#include "tcd.h"
#include "tracked.h"

struct {
	char name[50];
        int track;
	int y;
        int num_tracks;
} pages[5][50];

int cur_page, maxlines, track, page, max_pages;
extern cd_struct *cd;
extern int maxx, maxy;
extern WINDOW *win;

void edit_trackdb( void )
{
	int stop=FALSE,i,res, j=2;
	char c;
	char tmp[5];

	maxlines = maxy-9;
	
	update_pages();

	cur_page = 0;

	werase(win);
	attron( C_BLUE );
	box( win, ACS_VLINE,ACS_HLINE );
	
	wmove( win, maxy-8,1 );
	whline( win, ACS_HLINE, maxx-2 );
	attroff( C_BLUE );
	attron(C_WHITE+A_BOLD);
	mvwprintw(win,0,2, "TCD Track List Editor" );	
	attroff(C_WHITE+A_BOLD);

	attron( C_YELLOW+A_BOLD );
	mvwprintw(win,maxy-3,2,"[E]" );
	mvwprintw(win,maxy-2,2,"[Q]" );

	mvwprintw(win,maxy-3,27,"[T]" );
	mvwprintw(win,maxy-2,27,"[C]" );

	mvwprintw(win,maxy-4,2,"[+]" );
	mvwprintw(win,maxy-4,27,"[-]" );

	mvwprintw(win,maxy-2,54,"[A]" );

	attroff( C_YELLOW+A_BOLD );

	mvwprintw(win,maxy-3,5," - Edit track name." );
	mvwprintw(win,maxy-2,5," - Exit to player." );

	mvwprintw(win,maxy-3,30," - Change disc title." );
	mvwprintw(win,maxy-2,30," - Clear all tracks." );

	mvwprintw(win,maxy-4,5, " - Next Page." );
	mvwprintw(win,maxy-4,30," - Prev. Page." );

	mvwprintw(win,maxy-2,57," - Edit All Tracks." );

	mvwprintw(win,maxy-5,2,"Page:" ); 
	mvwprintw(win,maxy-7,2,"Title:" );


	while( !stop )
	{
		/* Draw page */
		for( i=0; i < maxlines; i++ )
			mvwprintw( win, i+1, 5, "                                                       " );
		for( i=1; i <= pages[cur_page][0].num_tracks; i++ )
		{
			j=pages[cur_page][i].track;
			
			mvwprintw( win, i+1, 5, "                                                       " );
			mvwprintw( win, i+1, 5, "%2d|%2d:%02d - %s", 
						j,
						cd->trk[j].tot_min,
						cd->trk[j].tot_sec,
						pages[cur_page][i].name );
		}
		
		mvwprintw(win,maxy-5,2,"Page:" ); 
		attron( C_RED+A_BOLD );
		mvwprintw(win,maxy-7,10,cd->dtitle );
		mvwprintw(win,maxy-5,10,"%d/%d", cur_page+1, max_pages+1 );
		attroff( C_RED+A_BOLD );

		c = getch();
	
		switch( c )
		{
		case 'q':
		case 'Q':
			tcd_writediskinfo(cd);
			stop = TRUE;
			break;
		case 'e':
		case 'E':
		        cbreak();echo();leaveok( win, FALSE );
		
			mvwprintw(win, maxy-6, 2, "Which Track:   " );
			wmove(win, maxy-6, 15 );
			wgetnstr( win, tmp, 2 );
			mvwprintw(win, maxy-6, 2, "                 " );
			
		        nocbreak();noecho();leaveok( win, TRUE );
			halfdelay(5);
			
			track = atoi(tmp);

			/* Ugh, nasty if statement. This makes sure that
			   the entered number is within the correct bounds. */
			if( track >= pages[cur_page][1].track && 
			    track <= pages[cur_page][pages[cur_page][0].num_tracks].track )
			{
				/* Search for the correct track */
				for( i=1; i <= pages[cur_page][0].num_tracks; i++ )
				{
					if( pages[cur_page][i].track == track )
						name_track(track);
				}
			}
			update_pages();
			cd->needs_dbwrite=TRUE;
			break;
		case 'a':
		case 'A':
			for( i=1; i <= pages[cur_page][0].num_tracks; i++ )
			{
				name_track(pages[cur_page][i].track);
				update_pages();
			}
			break;
		case 't':
		case 'T':
			change_aort();
			cd->needs_dbwrite=TRUE;
			break;
		case 'c':
		case 'C':
			mvwprintw( win, maxy-6, 2, "Really clear all? (y/n)" );
			
			halfdelay(255);
			res = wgetch(win);
			halfdelay(5);
			
			mvwprintw(win, maxy-6, 2,  "                       " );
			if( res == 'y' || res =='Y' )
			{
				for( i = cd->first_t; i <= cd->last_t; i++ )
				{
					sprintf( cd->trk[i].name, "Track %d", i );
					cd->trk[i].titled=FALSE;
				} 
				update_pages();
			}
			break;
		case '+':
			if( cur_page < max_pages )
				cur_page++;
			break;
		case '-':
			if( cur_page > 0 )
				cur_page--;
			break;
		case ERR:
		default:
			wrefresh( win );
			break;
		}
	}

	werase(win);	
	return;
}

void name_track( int track )
{
	char tmp[TRK_NAME_LEN];

	cbreak();
	echo();
	leaveok(win, FALSE);

	mvwprintw(win, maxy-6, 1, "Editing %2d:                                                     ", 
	 	track );
	wmove(win, maxy-6, 13);
	wgetnstr( win, tmp, TRK_NAME_LEN-1 );
	mvwprintw(win, maxy-6, 1, "                                                                 " );

	if( !strcmp( tmp, "" ) )
	{
	        nocbreak();
		noecho();
	        halfdelay(5);
	        leaveok( win, TRUE );
		return;
	}		
	cd->trk[track].titled = TRUE;
	strcpy( cd->trk[track].name, tmp );
	strcpy( pages[cur_page][track].name, tmp );
	
        nocbreak();
	noecho();
        halfdelay(5);
        leaveok( win, TRUE );
                                
	return;
}

void update_pages( void )
{
	int i;
	track = 1;
	page = 0;
	
	pages[0][0].num_tracks = maxlines+1;
	for( i=1; i <= cd->last_t; i++ )
	{
		if( i%maxlines==0 ) {
			pages[page][0].num_tracks = track-1;
			page++;
			track=1;
		}
		strcpy( pages[page][track].name, cd->trk[i].name );
		pages[page][track].track=i;
		track++;
	}
	pages[page][0].num_tracks = track-1;
	max_pages = page;
}

void change_aort( void )
{
	cbreak();
	echo();
	leaveok( win, FALSE );

	mvwprintw( win,maxy-7,10,"                                           " );
	mvwprintw( win,maxy-6,2,"Please use Artist / Album format." );
	

	wmove( win, maxy-7,10 );
	if( (maxx-11) > DISC_INFO_LEN )
		wgetnstr(win, cd->dtitle, DISC_INFO_LEN-2);
	else
		wgetnstr(win, cd->dtitle, maxx-11 );	
/*	mvwprintw( win,maxy-6,2,"                                           " ); */


	nocbreak();
	noecho();
	halfdelay(5);
	leaveok( win, TRUE );
}
