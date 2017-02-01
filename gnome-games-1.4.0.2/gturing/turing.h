/* turing.h - a Turing machine simulator.
 * Copyright (C) 1998 The Free Software Foundation
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* The tape is dynamic as it is supposed to be 'infinite'. Check out move_tape
 * to see how it will grow. */
typedef struct _tape
{
	char value;
	struct _tape *next;
	struct _tape *prev;
}
turing_tape;

/* The machine contains a list of states... maybe it could have been faster if
 * I had implemented a graph. */
typedef struct _state
{
	int no;
	char read;
	char write;
	char move;
	int new;
	
	struct _state *next;
}
turing_state;

typedef struct
{
	int state;
	int pos;
	
	turing_state *statehead;
	turing_state *actualstate;
	turing_tape *tapehead;
	turing_tape *actualtape;
}
turing;

extern char states_fname[1024];
extern char tape_string[1024];

extern turing *turing_new(void);
extern char *turing_states_to_string(turing_state *state);
extern int turing_fread_states(turing *machine, char *filename);
extern int turing_fwrite_states(turing_state *state, char *filename, char *comment);
extern char *turing_fread_comments(char *filename);
extern int turing_run_state(turing *machine);
extern void turing_set_state(turing *machine, turing_state state);

extern void turing_set_tape(turing *machine, char *ptr);
extern char *turing_get_tape(turing *machine);
