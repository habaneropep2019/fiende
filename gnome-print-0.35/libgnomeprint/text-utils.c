/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2000 Jose M Celorio
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
 *
 * Authors :
 *  Chema Celorio <chema@celorio.com>
 *
 */

#include "config.h"

#include <glib.h>
#include <text-utils.h>
#include <stdio.h>

#ifdef HAVE_STRING_H
#include <string.h>
#endif

/* Tokens Stuff */
#define is_spacer(x) (((x == ' ')  || \
                       (x == '\t') || \
                       (x == '\n') || \
                       (x == '\r') || \
                       (x == 0))        ?TRUE:FALSE)
#define is_newline(x) (((x=='\n') || \
                       (x== '\r'))      ?TRUE:FALSE)

gboolean
tu_token_next_till_newline (const guchar *buffer, gint buffer_size, gint *offset)
{
	gint n = 0;

	buffer += *offset;

	for (n = 0; n < buffer_size; n++)
		if (is_newline (buffer[n]))
			break;

	if (n > buffer_size - 1) {
		g_warning ("token bigger than buffer. Error");
		return FALSE;
	}
	
	*offset += n;
	
	return TRUE;
}


gchar *
tu_token_next_dup_till_newline (const guchar *buffer, gint buffer_size, gint *offset)
{
	guchar *temp;
	gint temp_size_max = 32;
	gint temp_size;
	gint n = 0;
	gboolean extra = 0;

	buffer += *offset;

	temp_size = 0;
	temp = g_new (guchar, temp_size_max);
	
	if (is_spacer (buffer[n])) {
		extra = 1;
		buffer++;
	}

	for (n = 0; n < buffer_size - extra; n++) {
		if (temp_size == temp_size_max)
			temp = g_realloc (temp, temp_size_max<<=1);
		if (is_newline (buffer[n]))
			break;
		temp [n] = buffer [n];
		temp_size ++;
	}

	if (n > buffer_size - 1) {
		g_warning ("token bigger than buffer. Error");
		return NULL;
	}
	
	temp [n] = 0;
		
	*offset += n + extra;

	return g_strdup (temp);
}


gchar *
tu_token_next_dup (const guchar *buffer, gint buffer_length, gint *offset)
{
	/* Limit the size of the tokens for buffer overflows */
	guchar temp [36];
	gint n = 0;
	gboolean extra = 0;

	/* We can't reach to offset -1 since in the buffer start we would
	   be accesing outside of the buffer memory */
	if (*offset > 0)
		if (buffer [*offset] == 0) {
			g_warning ("Can't tokenize, bufffer starts with 0\n");
			return NULL;
		}
	
	buffer += *offset;
	buffer_length -= *offset;

	while ( is_spacer (buffer [0]) && buffer_length > 0 ) {
		extra++;
		buffer++;
		buffer_length--;
	}
	       
	for (n = 0; n < buffer_length; n++) {
		if (is_spacer (buffer[n]))
			break;
		temp [n] = buffer [n];
	}
	
	if (n > buffer_length) {
		g_warning ("token bigger than buffer. Error (3.1)");
		return NULL;
	}

	if (n == 0)
		return NULL;
		
	temp [n] = 0;

	*offset += n + extra;

	return g_strdup (temp);
}


gchar *
tu_token_previous_dup (const guchar *buffer, gint buffer_length, gint *offset)
{
	/* Limit the size of the tokens for buffer overflows */
	guchar temp [36];
	guchar tmp;
	gint n = 0, m;
	gboolean extra = 0;

	buffer += *offset;
	
	if (is_spacer (buffer[n])) {
		extra = -1;
		buffer--;
	}

	for (n = 0; n < buffer_length; n++) {
		if (is_spacer (buffer[-n]))
			break;
		temp [n] = buffer [-n];
	}

	if (n > buffer_length) {
		g_warning ("token bigger than 34. Error");
		return NULL;
	}
	temp [n] = 0;

	/* Now reverse the buffer */
	for (m = 0; m < (n/2); m++) {
		tmp = temp [m];
		temp [m] = temp [n-m-1];
		temp [n-m-1] = tmp;
	}

	*offset += n + extra;

	return g_strdup (temp);
}


gchar *
tu_token_next_dup_till (const guchar *buffer, gint buffer_size,
			gint *offset, guchar till_me)
{
	/* Limit the size of the tokens for buffer overflows */
	guchar *temp;
	gint temp_size_max = 32;
	gint temp_size;
	gint n = 0;
	gboolean extra = 0;

	g_return_val_if_fail (*offset < buffer_size, NULL);
	
	buffer += *offset;

	temp_size = 0;
	temp = g_new (guchar, temp_size_max);
	
	if (is_spacer (buffer[n])) {
		extra = 1;
		buffer++;
	}

	for (n = 0; n < buffer_size - extra; n++) {
		temp [n] = buffer [n];
		temp_size ++;
		if (temp_size == temp_size_max)
			temp = g_realloc (temp, temp_size_max<<=1);
		if (buffer[n] == till_me)
			break;
	}
	n++;

	if (n > buffer_size - extra) {
		g_warning ("Token bigger than buffer. "
			   "Token not found [till %c].",
			   till_me);
		return NULL;
	}
	
	temp [n-1] = 0; /* We substract 1, since we don't want the till_me
			   character in the string */

	*offset += n + extra;
	
	return g_strdup (temp);
}


gboolean
tu_token_next_till (const guchar *buffer, gint buffer_size,
		    gint *offset, guchar till_me)
{
	gint n = 0;
	gboolean extra = 0;

	buffer += *offset;

	if (is_spacer (buffer[n])) {
		extra = 1;
		buffer++;
	}

	for (n = 0; n < buffer_size - extra; n++) {
		if (buffer[n] == till_me)
			break;
	}
	n++;

	if (n > buffer_size - extra) {
		g_warning ("Token bigger than buffer. "
				 "Token not found [till %c].",
				 till_me);
		return FALSE;
	}
	
	*offset += n + extra;

	return TRUE;
}

gboolean 
tu_token_next_verify (const guchar *buffer, gint *offset, const gchar *label)
{
	guchar temp [36];
	gint extra = 0;
	gint n = 0;

	buffer += *offset;
	
	if (is_spacer (buffer[n])) {
		extra = 1;
		buffer++;
	}
	for (n = 0; n < 37; n++) {
		if (is_spacer (buffer[n]))
			break;
		temp [n] = buffer [n];
	}
	if (n > 35) {
		g_warning ("token bigger than 34. Error");
		return FALSE;
	}
	
	temp [n] = 0;
	*offset += n + extra;
	
	if (strcmp (temp, label) == 0)
		return TRUE;
	
	g_warning ("Token expected not found. Expected \"%s\", found \"%s\"",
		   label, temp);
	
	return FALSE;
}


gboolean 
tu_token_next (const guchar *buffer, gint buffer_size, gint *offset)
{
	gint extra = 0;
	gint n = 0;

	buffer += *offset;
	
	while (is_spacer (buffer[n])) {
		extra++;
		buffer++;
	}
	
	for (n = 0; n < buffer_size; n++)
		if (is_spacer (buffer[n]))
			break;
		
	if (n > buffer_size) {
		g_warning ("token bigger than buffer. Error (5.1)");
		return FALSE;
	}
	
	*offset += n + extra;
	
	return TRUE;
}

