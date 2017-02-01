/* turing.c - a Turing machine simulator.
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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "turing.h"

#define BLANK           '_'
#define END_TAG         '\n'
#define COMMENT_TAG     '#'
#define TOKEN_SEPARATOR " "

#define LEFT  'l'
#define RIGHT 'r'

/* An utility for fread_states: exhausts any spaces found in fd. */
static void read_spaces(FILE *fd)
{
	int buff;
	
	while (((buff = fgetc(fd)) != EOF) && isspace(buff))
		;
	
	ungetc(buff, fd);
}

static turing_tape *new_cell(void)
{
	turing_tape *temp;
	
	temp = malloc(sizeof(turing_tape));
	temp->value = BLANK;
	temp->next = NULL;
	temp->prev = NULL;
	
	return temp;
}

static turing_state *new_state(void)
{
	turing_state *temp;
	
	temp = malloc(sizeof(turing_state));
	
	temp->no = temp->read = temp->write = temp->move = temp->new = 0;
	temp->next = NULL;
	
	return temp;
}

turing *turing_new(void)
{
	turing *machine;
	
	machine = malloc(sizeof(turing));
	machine->state = 0;
	machine->pos = 0;
	machine->actualtape = machine->tapehead = NULL;
	machine->actualstate = machine->statehead = NULL;

	return machine;
}
	
static void turing_free_tape(turing *machine)
{
	turing_tape *temp, *next;
	
	for (temp = machine->tapehead; temp; temp = next)
		{
			next = temp->next;
			free (temp);
		}
	
	machine->tapehead = NULL;
}

/* Gets the tape from tape_string. */
void turing_set_tape(turing *machine, char *ptr)
{
	turing_tape *temp, *last;
	char buff;
	
	turing_free_tape(machine);
	
	last = machine->tapehead = NULL;
	
	while ((buff = *(ptr++)) != 0)
		{
			temp = new_cell();
			temp->value = (buff == ' ')? BLANK : buff;
			
			if (last)
				{
					last->next = temp;
					temp->prev = last;
					last = temp;
				}
			else
				last = machine->tapehead = temp;
		}
	 machine->actualtape = machine->tapehead;
}

char *turing_get_tape(turing *machine)
{
	char *buff, *tmpbuff;
	turing_tape *temp;
	int i;
	
	i = 1;
	for (temp = machine->tapehead; temp; temp = temp->next)
		i++;
	
	tmpbuff = buff = malloc(sizeof(char) * i);
	
	for (temp = machine->tapehead; temp; temp = temp->next, tmpbuff ++)
		*tmpbuff = (temp->value == BLANK)? ' ' : temp->value;
	
	*tmpbuff = 0;
	
	return buff;
}

/* Reads states from file. Supports #comments, empty lines and misc info
 * after the state declaration in the same line. fscanf sucks: discovered the
 * blessings of strtok (thanks to Federico Mena). */
int turing_fread_states(turing *machine, char *filename)
{
	FILE *fd;
	turing_state *temp;
	int count;
	char *token;
	char  line[1000];
	
	machine->statehead = temp = NULL;
	
	if ((fd = fopen(filename, "r")) == NULL)
		return -1;
		
	do
		{
			read_spaces(fd);
			
			if (fgets(line, 1000, fd) == NULL)
				break;
			
			if ((*line == COMMENT_TAG) || (*line == END_TAG))
				continue;
			
			token = strtok(line, TOKEN_SEPARATOR);

			count = 0;

			if (temp == NULL)
				temp = new_state();
			
			while (token) {
				switch (count) {
					case 0:
						temp->no = atoi(token);
						break;

					case 1:
						temp->read = *token;
						break;

					case 2:
						temp->write = *token;
						break;

					case 3:
						temp->move = *token;
						break;

					case 4:
						temp->new = atoi(token);
						break;

					default:
						fprintf(stderr, "turing_fread_states(): wrong states file format.\n");
						exit(1);
						break;
				} /* switch */

				token = strtok(NULL, TOKEN_SEPARATOR);

				if (count == 4)
					break;
				
				count++;
			} /* while */
			 
			if (count == 4)
				{
					temp->next = machine->statehead;
					machine->statehead = temp;
					temp = NULL;
				}
		}
	while (temp == NULL);
	
	if (temp != NULL)
		free (temp);
	
	fclose(fd);
	 
	if (machine->statehead)
		{
			for (temp = machine->statehead; temp->next; temp = temp->next)
				;
			machine->state = temp->no;
			machine->actualstate = temp;
		}
	
	return 0;
}

static char *str_copy_str (char *str, char *str2)
{
	char *c;
	int len, len2;
	
	len = strlen (str);
	len2 = strlen (str2);
	
	str = realloc (str, len + len2 + 1);
	c = str + len;
	
	while (*(c++) = *(str2++))
		;
	
	return str;
}

extern char *turing_states_to_string(turing_state *state)
{
	turing_state *s;
	char *ret, buff[1024];
	
	ret = calloc (1, 1);
	
	for (s = state; s; s = s->next)
	{
		snprintf (buff, 1024, "%d %c %c %c %d\n", s->no, s->read, s->write, s->move, s->new);
		ret = str_copy_str (ret, buff);
	}
	
	return ret;
}

extern int turing_fwrite_states(turing_state *state, char *filename, char *comment)
{
	FILE *fd;
	char *str, *c, *c2, *comment2;
	
	if ((fd = fopen (filename, "w")) == NULL)
		return -1;
	
	if (comment)
	{
		c2 = comment2 = strdup (comment);
		
		while ((c = strchr (c2, '\n')))
		{
			*c = 0;
			fprintf (fd, "#%s\n", c2);
			c2 = c + 1;
		}
		
		if (*c2)
			fprintf (fd, "#%s\n", c2);
		
		free (comment2);
	}
	
	str = turing_states_to_string (state);
	fprintf (fd, str);
	free (str);
	
	fclose (fd);
	
	return 0;
}

char *turing_fread_comments(char *filename) {
	int c;
	int size;
	char line[1000];
	char *ret, *tmp;
	FILE *fd;
	
	if ((fd = fopen(filename, "r")) == NULL)
		return NULL;
	
	ret = malloc(1);
	*ret = 0;
	size = 1;

	while ((c = fgetc(fd)) != EOF) {
		
		fgets(line, 1000, fd);
		
		if (c == '#') {
			ret = realloc(ret, size + strlen(line));
			tmp = ret + size - 1;
			size += strlen(line);
			strcpy (tmp, line);
		}
	}

	return ret;
}

/* Search the next state to execute depending on what we are reading and the
 * state field of the turing *machine. */
static turing_state *turing_search_state(turing *machine)
{
	turing_state *temp;
	
	for (temp = machine->actualstate; temp; temp = temp->next)
		if (temp->no == machine->state)
			return temp;
	
	return NULL;
}

/* Moves the tape. If we try to get out of the range of the list, we create a
 * new cell with a blank in it (giving the impression that it is infinite. */
static void turing_move_tape(turing *machine)
{
	if (tolower(machine->actualstate->move == LEFT))
		(machine->pos > 0)? machine->pos-- : 0;
	else
		machine->pos++;
			
	if (tolower(machine->actualstate->move == LEFT))
		{
			if (machine->actualtape->prev == NULL)
				{
					machine->pos = 0;
					machine->actualtape->prev = new_cell();
					machine->actualtape->prev->next = machine->actualtape;
					machine->tapehead = machine->actualtape->prev;
				}
			machine->actualtape = machine->actualtape->prev;
		}
	else
		{
			if (machine->actualtape->next == NULL)
				{
					machine->actualtape->next = new_cell();
					machine->actualtape->next->prev = machine->actualtape;
				}
			machine->actualtape = machine->actualtape->next;
		}
}

/* Runs the next state to execute. */
int turing_run_state(turing *machine)
{
	machine->actualstate = machine->statehead;
	
	while (machine->actualstate && 
				 ((machine->actualstate->no != machine->state) || 
					(machine->actualstate->read != machine->actualtape->value)))
		{
			machine->actualstate = machine->actualstate->next;
			machine->actualstate = turing_search_state(machine);
		}
	
	if (machine->actualstate)
		{
			machine->actualtape->value = machine->actualstate->write;
			turing_move_tape(machine);
			machine->state = machine->actualstate->new;
			return 1;
		}
	
	return 0;
}

/* add or modify a state to the list. 
 * Returns the position where the state was added/modified. 
 * flag is true if an insertion was made. */
extern void turing_set_state(turing *machine, turing_state state)
{
	turing_state *temp, *node, *last;
	
	node = last = NULL;
	for (temp = machine->statehead; temp; last = temp, temp = temp->next)
		if (temp->no == state.no)
	  {
			if (!node) 
				node = last;
			
			if (temp->read == state.read)
			 {
				 temp->write = state.write;
				 temp->move = state.move;
				 temp->new = state.new;
				 break;
			 }
		}

	if (!temp) /* Values were not substituted: insert new state. */
	{
		temp = malloc (sizeof (turing_state));
		
		*temp = state;
		if (!node) /* A completly new state number. Add at the bottom. */
		{
			temp->next = machine->statehead;
			machine->statehead = temp;
			return;
		}
		
		temp->next = node->next;
		node->next = temp;
	}
	
	return;
}



