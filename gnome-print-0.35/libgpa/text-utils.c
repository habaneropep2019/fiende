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

#include <glib.h>
#include <text-utils.h>
#include <stdio.h>

#define GPA_TU_EXTRA_REPLACEMENTS 2
/**
 * gpa_code_replace:
 * @search_text: 
 * @replace_text: 
 * @buffer: 
 * 
 * Code to "replace all", originaly written for gedit. 
 * 
 * Return Value: -1 o error, # of replacements otherwise
 **/
gint
gpa_tu_replace_all (const gchar *search_text, const gchar *replace_text,
		    gchar **buffer, gboolean case_sensitive)
{
	gint search_text_length;
	gint replace_text_length;
	guchar * buffer_in;
	guchar * buffer_out;
	guint buffer_in_length;
	guint buffer_out_length;
	gint delta;
	gint replacements = 0;
	guint p1, p2, p3, p4;
	gint grow_factor = GPA_TU_EXTRA_REPLACEMENTS;
	/* p1 = points to the index on buffer_in
	   p2 = pointer to the index on buffer_out
	   p3 = pointer to the index in search_text
	   p4 = pointer to the index in replace_text */

	g_return_val_if_fail (search_text  != NULL, -1);
	g_return_val_if_fail (replace_text != NULL, -1);
	g_return_val_if_fail (*buffer != NULL,      -1);

	search_text_length  = strlen (search_text);
	replace_text_length = strlen (replace_text);

	buffer_in        = *buffer;
	buffer_in_length = strlen (buffer_in);

	g_return_val_if_fail (buffer_in_length != 0, -1);

	delta = replace_text_length - search_text_length;
	if (delta > 0)
		buffer_out_length = buffer_in_length + (grow_factor) * delta;
	else
		buffer_out_length = buffer_in_length;

	buffer_out = g_malloc (++buffer_out_length);

	if (buffer_out==NULL)
	{
		g_warning ("Could not allocate the memory needed for search & replace\n");
		return -1;
	}

	p1 = 0;
	p2 = 0;
	p3 = 0;
	p4 = 0;


	/* Do the actual replace all */
	while (p1 < buffer_in_length)
	{
		if (p2 > buffer_out_length - (delta*2)) /* Allocate for at least 2 more replacements */
		{
			if (delta < 1) {
				g_warning ("This should not happen.\n");
				g_print ("Delta %i, Buffer_out_length:%i, p2:%i\n", delta, buffer_out_length, p2);
			}
			buffer_out_length = buffer_in_length + (replacements +
								(grow_factor <<= 1)) * delta;
			buffer_out = g_realloc (buffer_out, buffer_out_length);
			if (buffer_out == NULL)
			{
				g_warning ("Unable to realocate buffer");
				return -1;
			}
			
		}
		
		if ((buffer_in [p1]) == (search_text [p3]))
		{
			p3++;
			if (p3 == search_text_length)
			{
				p2 = p2 - search_text_length + 1;
				for (p4 = 0; p4 < replace_text_length; p4++)
					buffer_out [p2++] = replace_text [p4];
				replacements++;
				p3=0;
				p1++;
				continue;
			}
		}
		else
		{
			p3=0;
		}
		
		buffer_out [p2++] = buffer_in [p1++];
	}
	buffer_out [p2] = '\0';

	if (p2 >= buffer_out_length )
	{
		g_warning ("p2 > buffer_out_length");
		g_assert_not_reached();
	}

	g_free (*buffer);
	*buffer = buffer_out;

	return replacements;
}


/**
 * gpa_tu_search_for_prefix_and_get_sufix_with_lengths:
 * @search_for: 
 * @search_for_length: 
 * @buffer: 
 * @buffer_length: 
 * 
 * We need this function because we are going to need to search
 * buffers that contain Ascii "0"'s and a strlen will not work.
 *
 * Return Value: A pointer to the list. The caller is resposible for
 * freeing the list and it's elements
 **/
static GList *
gpa_tu_get_list_of_sufixes_full (const gchar *search_text, gint search_text_length,
				     const gchar *buffer_in, gint buffer_in_length,
				     gchar separator)
{
	GList *list = NULL;
	gchar *found_item;
	gint p1 = 0;
	gint p2;
	gint p3 = 0;
	gint p4;

	while (p1 < buffer_in_length)
	{
		if (buffer_in [p1] == search_text [p3])
		{
			p3++;
			if (p3 == search_text_length)
			{
				p1++;

				if (buffer_in [p1++] != separator) {
					g_warning ("The Escape tag does "
						   "not end with a separator. Tag %s "
						   "Code : \n*%s*",
						   search_text, buffer_in);
					return NULL;
				}

				p2 = p1;
				for (p4 = 0; ((buffer_in [p1] != separator) &&
					     ( p1 < buffer_in_length)); p4++)
					p1++;

				/* Add to the list */
				found_item = g_strndup (buffer_in + p2, p4);
				list = g_list_prepend (list, found_item);
				
				if ((buffer_in [p1] != separator) &&
				    (p1 < buffer_in_length )) {
					g_warning ("Unterminated Prefix");
					return NULL;
				}
				
				p3=0;
				p1++;
				continue;
			}
		}
		else
		{
			p3=0;
		}
		
		p1++;
	}

	return list;
}

/**
 * gpa_tu_get_list_of_sufixes:
 * @search_for: 
 * @buffer: 
 * 
 * If the buffer is "blah $ALFA-1-blah $ALFA-hello- blah ..."
 * it returns a GList that contain "1" & "hello"
 *
 * Return Value: 
 **/
GList *
gpa_tu_get_list_of_sufixes (const gchar *search_for, const gchar *buffer, gchar separator)
{
	GList *found = NULL;
	gint buffer_length;
	gint search_for_length;

	buffer_length     = strlen (buffer);
	search_for_length = strlen (search_for);

	found = gpa_tu_get_list_of_sufixes_full (search_for, search_for_length,
						     buffer, buffer_length,
						     separator);


	return found;
}

void
gpa_tu_list_of_sufixes_free (GList *list_in)
{
	GList *list;
	gchar *string;

	list = list_in;
	for (; list != NULL; list = list->next) {
		string = (gchar *) list->data;
		g_free (string);
	}

	g_list_free (list_in);
}


/**
 * gpa_tu_search_real:
 * @buffer: 
 * @buffer_length: 
 * @search_text: 
 * @search_text_length: 
 * @case_sensitive: 
 * 
 * 
 * 
 * Return Value: the location of the text
 **/
gint
gpa_tu_search_real (const gchar *buffer, gint buffer_length,
			const gchar *search_text, gint search_text_length,
			gboolean case_sensitive)
{
	gint p1;
	gint p2 = 0;
	gint case_sensitive_mask;

	case_sensitive_mask = case_sensitive?0:32;

	if (search_text_length == 0)
		search_text_length = strlen (search_text);
	
	for (p1=0; p1 < buffer_length; p1 ++)
	{
		if ((buffer[p1]     |case_sensitive_mask)==
		    (search_text[p2]|case_sensitive_mask))
		{
			p2++;
			if (p2==search_text_length)
				return p1 - search_text_length + 1;
		}
		else
			p2 = 0;
	}
	
	return -1;
}

/**
 * gpa_tu_search:
 * @buffer: 
 * @search_for: 
 * @case_sensitive: 
 * 
 * 
 * 
 * Return Value: the location of the "search_for" inside "buffer"
 **/
gint
gpa_tu_search (const gchar *buffer, const gchar *search_for, gboolean case_sensitive)
{
	gint buffer_length;
	gint search_for_length;
	gint ret;

	g_return_val_if_fail (buffer != NULL,     -1);
	g_return_val_if_fail (search_for != NULL, -1);

	buffer_length     = strlen (buffer);
	search_for_length = strlen (search_for);

	ret = gpa_tu_search_real (buffer, buffer_length,
				      search_for, search_for_length,
				      case_sensitive);

	return ret;	
}

/**
 * gpa_gpa_tu_divide_real:
 * @buffer: 
 * @buffer_length: 
 * @start_tag: 
 * @start_tag_length: 
 * @end_tag: 
 * @end_tag_length: 
 * @pre: 
 * @body: 
 * @post: 
 * 
 * we need this "real" variant since it can handle buffers that contain
 * ASCII 0's in them
 *
 * Return Value: 
 **/
static gboolean
gpa_gpa_tu_divide_real (const gchar *buffer,    gint buffer_length,
			const gchar* start_tag, gint start_tag_length,
			const gchar *end_tag,   gint end_tag_length,
			gchar **pre, gchar **body, gchar **post)
{
	gint p1; /* Pointer where pre_code ends */
	gint p2; /* Pointer where body_code starts */
	gint p3; /* Pointer where body_code ends */
	gint p4; /* Pointer where post_code starts */
	
	p1 = gpa_tu_search (buffer, start_tag, TRUE);
	p3 = gpa_tu_search (buffer, end_tag, TRUE);
	if (p3 < p1) {
		return FALSE;
	}
		
	/* This is correct, just that there isn't a division */
	if ((p1 == -1) && (p3 == -1)) {
		*pre = g_strndup (buffer, buffer_length);
		*body = NULL;
		*post = NULL;
		return TRUE;
	}
	/* If one of the tags was found, but not the other. Error */
	if (p1 == -1)
		return FALSE;
	if (p3 == -1)
		return FALSE;


	p2 = p1 + start_tag_length + 1;
	p4 = p3 + end_tag_length + 1;

	if (p1 > 0)
		*pre  = g_strndup (buffer, p1);
	else
		*pre = NULL;
	*body = g_strndup (buffer + p2, p3-p2);
	if ((buffer_length - p4) > 0)
		*post = g_strndup (buffer + p4, buffer_length-p4);
	else
		*post = NULL;

	return TRUE;
}

gboolean
gpa_tu_divide (const gchar *buffer,
		   const gchar* start_tag,
		   const gchar *end_tag,
		   gchar **pre,
		   gchar **body,
		   gchar **post)
{
	gint buffer_length;
	gint start_tag_length;
	gint end_tag_length;
	gint ret;

	g_return_val_if_fail (buffer != NULL,    -1);
	g_return_val_if_fail (start_tag != NULL, -1);
	g_return_val_if_fail (end_tag != NULL,   -1);

	buffer_length    = strlen (buffer);
	start_tag_length = strlen (start_tag);
	end_tag_length   = strlen (end_tag);

	ret = gpa_gpa_tu_divide_real (buffer, buffer_length,
				      start_tag, start_tag_length,
				      end_tag, end_tag_length,
				      pre, body, post);
	
	return ret;
}

/**
 * gpa_tu_xml_clean:
 * @code: 
 * @extra_clean: 
 * 
 * remove the spaces/tabs added because of XML levels.
 * if extra_clean is true, remove newlines too
 * The cleaning method is :
 * after a new_line, remove the ' ' & '\t' till you find
 * a non ' '/'\t' char.
 **/
void
gpa_tu_xml_clean (gchar **code, gint extra_clean)
{
	gchar *in = *code;
	gchar *out;
	gint length;
	gint p1;
	gint n = 0;

	length = strlen (*code);
	out    = g_malloc (length+1);

	for (p1 = 0; p1 < length; p1++) {
		if ( (extra_clean == FALSE) ||
		     (in [p1] != '\n')) {
			out [n] = in [p1];
			n++;
		}
		if (in [p1] == '\n') {
			while ( ((in [p1+1] == ' ')||
				 (in [p1+1] == '\t')) &&
				(p1 < length) )
				p1++;
		}
		
	}
	out [n] = 0;

	g_free (in);

	*code = out;
}


/**
 * gpa_tu_read_file:
 * @file_name: full path of the file to read
 * @size_: return here the size read
 * @initial_size: initial size of the buffer to be malloced
 * @grow_size_: grow size of the buffer once the malloced buffer is full
 * 
 * Read a file from disk into a gchar buffer
 * 
 * Return Value: a pointer to the malloc'ed buffer, NULL on error
 **/
guchar*
gpa_tu_read_file (const gchar* file_name, gint *size_,
	      gint initial_size, gint grow_size_)
{
	FILE *f;
	gint size;
	gint size_max;
	gint grow_size;
	gint bytes_read;
	guchar *buffer;

	*size_ = 0;
	size = 0;
	
	g_return_val_if_fail (file_name != NULL, NULL);
	
	f = fopen (file_name, "r");
	if (f == NULL) {
		g_warning ("Couldn't open \"%s\"", file_name);
		return NULL;
	}
	
	size = 0;
	size_max = (initial_size == 0)? 1024*1 : initial_size;
	grow_size = (grow_size_ == 0)? 1024*1 : grow_size_;
	buffer = g_new (guchar, size_max);
	while (TRUE) {
		bytes_read = fread (buffer + size, 1, size_max - size, f);
		if (bytes_read == 0)
			break;
		size += bytes_read;
		if (size_max - size != 0)
			continue;
		size_max += (size_max - size == 0)?grow_size:size_max-size;
		buffer = g_realloc (buffer, size_max);
	}

	/* Return the memory we didn't use */
	buffer = g_realloc (buffer, size);
	
	*size_ = size;

	if (0 != fclose (f))
		g_warning ("Could not close %s", file_name);
	
	return buffer;
}


gboolean
gpa_tu_remove_trailing_spaces (gchar **buffer_)
{
	gchar *buffer;
	gint length;
	gint n;

	buffer = *buffer_;
	
	g_return_val_if_fail (buffer != NULL, FALSE);

	length = strlen (buffer);
	for (n = length - 1; n != 0; n--)
		if (buffer [n] == ' ') {
			buffer [n] = 0;
		} else {
			break;
		}
	
	*buffer_ = buffer;

	return TRUE;
}
	


gint
gpa_tu_strnchr (const guchar *buffer, guchar c)
{
	gint n = 0;

	while (buffer [n] != 0)
		if (buffer [n++] == c)
			return n;
	
	return -1;
}


/**
 * tu_get_pos_of_last_char:
 * @buffer: 
 * @delimiter: 
 * 
 * given "foo-barr-fra-fra-ole" returns the posistion of the list '-'
 * in the string.
 * 
 * Return Value: the position, -1 if it was not found
 **/
gint
tu_get_pos_of_last_delimiter (const gchar *buffer, gchar delimiter)
{
	gint length;
	gint n;

	g_return_val_if_fail (buffer != NULL, -1);

	length = strlen (buffer);
	for (n = length - 1; n != 0; n--)
		if (buffer [n] == delimiter)
			return n;

	return -1;
}


/* Tokens Stuff */
#define is_spacer(x) (((x == ' ')  || \
                       (x == '\t') || \
                       (x == '\n') || \
                       (x == '\r') || \
                       (x == 0))        ?TRUE:FALSE)
#define is_newline(x) (((x=='\n') || \
                       (x== '\r'))      ?TRUE:FALSE)

gboolean
gpa_tu_token_next_search (const guchar *buffer, gint buffer_length, gint *offset,
		      const gchar *search_text, gboolean case_sensitive)
{
	gint p1;
	gint p2 = 0;
	gchar case_sensitive_mask;

	case_sensitive_mask = case_sensitive?0:32;

	buffer += *offset;
	buffer_length -= *offset;

	for (p1 = 0; p1 < buffer_length; p1 ++)
	{
		if ((buffer[p1]     |case_sensitive_mask)==
		    (search_text[p2]|case_sensitive_mask))
		{
			p2++;
			if (search_text [p2] == 0)
			{
				*offset += p1 + 1;
				return TRUE;
			}
		}
		else
		{
			p2 = 0;
		}
	}

	*offset += p1;

	return FALSE;
}

gboolean
gpa_tu_token_next_till_newline (const guchar *buffer, gint buffer_size, gint *offset)
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
gpa_tu_token_next_dup_till_newline (const guchar *buffer, gint buffer_size, gint *offset)
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
gpa_tu_token_next_dup (const guchar *buffer, gint buffer_length, gint *offset)
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
gpa_tu_token_previous_dup (const guchar *buffer, gint buffer_length, gint *offset)
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
gpa_tu_token_next_dup_till (const guchar *buffer, gint buffer_size,
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
gpa_tu_token_next_till (const guchar *buffer, gint buffer_size,
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
gpa_tu_token_next_verify (const guchar *buffer, gint buffer_length, gint *offset, const gchar *label)
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
gpa_tu_token_next (const guchar *buffer, gint buffer_size, gint *offset)
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



gboolean
gpa_tu_string_contains_newline (const gchar *token)
{
	gint i;
	gint len;

	len = strlen (token);

	for (i = 0; i < len; i++)
		if (is_newline (token [i]))
			return TRUE;
	
	return FALSE;	
}
