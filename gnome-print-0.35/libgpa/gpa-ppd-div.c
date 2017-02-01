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
/*
   [ This first implementation in now SimpleDiv, as it proved to be limited in functionality
   after gpa_div was implemented it was renamed to gpa_simple_div. The current gpa_div implementation
   does not require the caller to specify the values to look for. The following explanation refers
   to the now gpa_simple_div method]

   This file constains the code to find the common parts between a set of postcript
   code fragments. (other kind of code can be used I guess, but it was designed for PS)

   So lets say we have this code fragments :
   1. "1 dict dup /Apple /Red /20 put put put"
   2. "1 dict dup /Grape /Green /1 put put put"
   3. "1 dict dup /Carrot /Orange /5 put put put"

   and we want to find the commont part of those code fragments and the variable
   part. So we can have like :

   "1 dict dup /{Fruit} /{Color} /{Price} put put put"

   along with the variable data :
   Apple, Red, 20
   Grape, Green, 1
   Carrot, Orange, 5

   this would allow us to replace the code with the value we need.
   (If you are wondering, why did Chema do all this mess for and not
   just save each's code fragment by itself. Here is why : by using
   a common and a variable data strucutre we are going to be able to
   share the code among printers that may have different number of options
   and values. So in theory we would only need to define each paper once
   and the rest of the code will be substituted. Dunno if it is worth it
   or not, but I am going to implement it anyways, since that is the only
   way to find out)

   a) So we would call this functions :

   GpaPpdDiv *  gpa_ppd_simple_div_new ([gint] 3, [gchar**] {"Fruit", "Color", "Price"});

   b) then to add  each element :

   gpa_ppd_simple_div ([GpaPpdDiv *] ppd_div, [gchar *] code,
                [gchar **] {"Apple", "Red", "20"});

   c) then to solve the puzzle :

   gpa_ppd_simple_div_solve ([GpaPpdDiv *] ppd_div, [gchar *] "{", [gchar *] "}", &code,
                      [gboolean] items_used );

   and code will contain :

   "1 dict dup /{Fruit} /{Color} /{Price} put put put"

   items used is a gboolean array that will be flaged as true for the items that it
   used.

 */

#include "gpa-defs.h"

#include <glib.h>

#include <string.h>

#include "gpa-ppd.h" /* for GPA_PPD_TAG_'s  */
#include "gpa-ppd-div.h"
#include "text-utils.h"

typedef struct _GpaPpdSimpleDivItem GpaPpdSimpleDivItem;
typedef struct _GpaPpdDivItem       GpaPpdDivItem;

struct _GpaPpdDiv
{
	GList *children;
};

struct _GpaPpdDivItem
{
	/* This points to the full fragment of code */
	gchar *code;
	GHashTable *hash;
};

struct _GpaPpdSimpleDiv
{
	gint size;
	gchar **labels;
	GList *children;
};

struct _GpaPpdSimpleDivItem
{
	gint replacements;
	gchar **tags;
	gchar *code;
	gchar *clean_code;
};

GpaPpdSimpleDiv *
gpa_ppd_simple_div_new (gint size, gchar **labels_)
{
	GpaPpdSimpleDiv *ppd_div;
	gchar **labels;
	gint n;
	
	debug (FALSE, "");

	labels = g_new (gchar *, size);
	for (n = 0; n < size; n++)
		labels [n] = g_strdup (labels_ [n]);
	
	ppd_div = g_new (GpaPpdSimpleDiv, 1);
	ppd_div->size     = size;
	ppd_div->labels   = labels;
	ppd_div->children = NULL;

	debug (FALSE, "end");

	return ppd_div;
}

void
gpa_ppd_simple_div_add (GpaPpdSimpleDiv *ppd_div, gchar **tags_, const gchar *code)
{
	GpaPpdSimpleDivItem *item;
	gchar **tags;
	gint size;
	gint n;
	
	g_return_if_fail (ppd_div != NULL);
	g_return_if_fail (code    != NULL);

	size = ppd_div->size;
	
	tags = g_new (gchar *, size);
	for (n = 0; n < size; n++)
		tags [n] = tags_ [n];

	item = g_new (GpaPpdSimpleDivItem, 1);
	item->tags = tags;
	item->code = g_strdup (code);
	item->clean_code = NULL;

	ppd_div->children = g_list_prepend (ppd_div->children, item);
}

static void
gpa_ppd_simple_div_free_item (GpaPpdSimpleDivItem *item, gint size)
{
	gint n;
	
	g_return_if_fail (item != NULL);


	for (n = 0; n < size; n++)
		g_free (item->tags [n]);
	
	g_free (item->code);
	g_free (item->clean_code);
	g_free (item);
	
}

static void
gpa_ppd_simple_div_free (GpaPpdSimpleDiv *ppd_div)
{
	GpaPpdSimpleDivItem *item;
	GList *list;
	gint size;
	gint n;

	g_return_if_fail (ppd_div != NULL);

	size = ppd_div->size;
	
	for (list = ppd_div->children; list != NULL; list = list->next) {
		item = list->data;
		gpa_ppd_simple_div_free_item (item, size);
	}

	for (n = 0; n < size; n++)
		g_free (ppd_div->labels [n]);

	g_free (ppd_div);
		
}

gboolean
gpa_ppd_simple_div_solve (GpaPpdSimpleDiv *ppd_div,
			  const gchar *start_tag,
			  const gchar *end_tag,
			  gchar **cleaned_code)
{
	GpaPpdSimpleDivItem *item;
	GpaPpdSimpleDivItem *first_item;
	GList *list;
	gchar **labels_real = NULL;
	gint size;
	gint n;
	
	g_return_val_if_fail (ppd_div   != NULL, FALSE);
	g_return_val_if_fail (start_tag != NULL, FALSE);
	g_return_val_if_fail (end_tag   != NULL, FALSE);

	size = ppd_div->size;
	list = ppd_div->children;

	/* Generate from labels, the real labels that will go in the code */
	labels_real = g_new (gchar *, size);
	for (n = 0; n < size; n++)
		labels_real [n] = g_strdup_printf ("%s%s%s",
						   start_tag,
						   ppd_div->labels [n],
						   end_tag);
	
	/* Generate the cleaned code for each item */
	for (; list != NULL; list = list->next) {
		item = list->data;
		item->clean_code = g_strdup (item->code);
		for (n = 0; n < size; n++)
			item->replacements = gpa_tu_replace_all (item->tags [n],
								 labels_real [n],
								 &item->clean_code,
								 TRUE);
	}

	/* For each code fragment, verify that it matches the code fragment
	   of the first element, even itself. */
	list = ppd_div->children;
	first_item = list->data;
	for (; list != NULL; list = list->next) {
		item = list->data;
		if (strcmp (item->clean_code, first_item->clean_code) != 0)
			break;
	}

	if (list != NULL) {
		gpa_ppd_error ("Could not find the common code in :\n"
				 "-->%s<--\n-->%s<--\n",
				 item->clean_code,
				 first_item->clean_code);
		return FALSE;
	}


	*cleaned_code = g_strdup (first_item->clean_code);

	gpa_ppd_simple_div_free (ppd_div);
	g_free (labels_real);
	
	return TRUE;	
}

			  



GpaPpdDiv *
gpa_ppd_div_new (void)
{
	GpaPpdDiv *ppd_div;
	
	debug (FALSE, "");
	
	ppd_div = g_new (GpaPpdDiv, 1);
	
	ppd_div->children = NULL;

	debug (FALSE, "end");

	return ppd_div;
}

void
gpa_ppd_div_add (GpaPpdDiv *ppd_div, const gchar *code, GHashTable *hash)
{
	GpaPpdDivItem *item;

	g_return_if_fail (ppd_div != NULL);

	if (code == NULL)
		return;
	
	item = g_new (GpaPpdDivItem, 1);
	item->code = g_strdup (code);
	item->hash = hash;

	ppd_div->children = g_list_prepend (ppd_div->children, item);
}


/* Return the number of bytes that are equal */
static gint
gpa_strncmp (const gchar *a, const gchar *b, gint max_n)
{
	gint n;

	for (n = 0; n < max_n; n++)
		if (a [n] != b [n])
			return n;
	return n;
}

static GList *
gpa_ppd_div_get_common_tokens (GpaPpdDiv *ppd_div, gint num)
{
	GpaPpdDivItem *item;
	GHashTable **hash;
	GList *tokens = NULL;
	GList *child;
	gchar *test;
	gchar **code;
	gchar *token;
	gchar *temp;
	gchar *key;
	gint size = 0;
	gint test_size;
	gint size_min;
	gint n;
	gint m = 0;
	gint offset;

	/* Put the list into an array of gchar pointers, since we need to
	 advance the pointer while we analize it */
	code = g_new (gchar *, num);
	hash = g_new (GHashTable *, num);
	n = 0;
	for (child = ppd_div->children; child != NULL; child = child->next) {
		item = child->data;
		g_return_val_if_fail (item->hash != NULL, NULL);
		g_return_val_if_fail (item->code != NULL, NULL);
		code [n] = item->code;
		hash [n] = item->hash;
		n++;
	}
	
	test  = code [0];
	test_size = strlen (test);

	
	while (TRUE) {
		size_min  = strlen (test);

		/* Look for the common part in all the codes */
		for (n = 1; n < num; n++) {
			size = gpa_strncmp (code [n], test, size_min);
			size_min = MIN (size, size_min);
		}
		token = g_strndup (test, size_min);
		tokens = g_list_prepend (tokens, token);

		/* Now, advance 1 token from test, which will be
		 our variable part */
		test      += size_min;
		test_size -= size_min;

		/* If we are done, break; */
		if (*test == 0)
			break; 
		offset = 0;
		
		/* Advance 1 token and add it to the first hash */
		temp = gpa_tu_token_next_dup (test, test_size, &offset);
		key = g_strdup_printf ("%s%.2i%s", GPA_PPD_TAG_PRE_VAR, m, GPA_PPD_TAG_POST_VAR);
		g_hash_table_insert (hash [0], key, temp);
		test += offset;
		test_size -= offset;

		/* Now advance 1 token from each item */
		for (n = 1; n < num; n++) {
			gchar *item_code;
			gint item_code_size = 200;
			item_code = code [n] + size_min;;
			offset = 0;
			temp = gpa_tu_token_next_dup (item_code, item_code_size, &offset);
			code [n] = item_code + offset;
			key = g_strdup_printf ("%s%.2i%s",  GPA_PPD_TAG_PRE_VAR, m, GPA_PPD_TAG_POST_VAR);
			g_hash_table_insert (hash [n], key, temp);
		}
		m++;
	}
	tokens = g_list_reverse (tokens);

	return tokens;
}

static gint
gpa_ppd_div_determine_code_length (GList *tokens)
{
	GList *list;
	gchar *token;
	gint length;
	gint tags_length;
	
	g_return_val_if_fail (tokens != NULL, 0);
	
	length = 0;
	tags_length = strlen (GPA_PPD_TAG_PRE_VAR) + strlen (GPA_PPD_TAG_POST_VAR) + 2;
	
	for (list = tokens; list != NULL; list = list->next) {
		token = list->data;
		length += strlen (token) + tags_length;
	}
	
	length -= tags_length;

	return length;
}

gboolean
gpa_ppd_div_solve (GpaPpdDiv *ppd_div, gchar **set_code_)
{
	GpaPpdDivItem *item;
	GList *tokens;
	GList *list;
	gchar *token;
	gchar *setcode;
	gchar *key;
	gint setcode_length;
	gint token_length;
	gint num;
	gint used;
	gint n = 0;

	num = g_list_length (ppd_div->children);
	
	*set_code_ = NULL;
	
	if (num < 2)
		return TRUE;

	tokens = gpa_ppd_div_get_common_tokens (ppd_div, num);

	if (tokens == NULL) {
		gpa_ppd_error ("Could not get determine the code for options, because "
			       "the code could not be tokenized");
		return FALSE;
	}

	/* Determine how much memory do we need for the setcode, and allocate it */
	setcode_length = gpa_ppd_div_determine_code_length (tokens);
	setcode = g_new (gchar, setcode_length + 1 /*Add a Null cero */);
	used = 0;

	/* Now compose the setcode from, a token + var + token + var ... */	  
	for (list = tokens; list != NULL; list = list->next) {
		token = list->data;
		token_length = strlen (token);
		memcpy (setcode + used,	token,token_length);
		used += token_length;
		if (list->next == NULL)
			continue;
		key = g_strdup_printf ("%s%.2i%s", GPA_PPD_TAG_PRE_VAR,
				       n++, GPA_PPD_TAG_POST_VAR);
		token_length = strlen (key);
		memcpy (setcode + used,	key,token_length);
		used += token_length;
		g_free (key);
	}

	setcode [used] = 0;
	
	/* Now remove  from the hash the Setcode items */
	for (list = ppd_div->children; list != NULL; list = list->next) {
		item = list->data;
		g_hash_table_remove (item->hash, GPA_PPD_TAG_SET_CODE);
	}
		
	/* Verify that we alloceted the correct amount of memory */
	if ((setcode_length != used) ||
	    (setcode_length != strlen (setcode)))
		g_warning ("Dude, you need to learn how to add, substract and multiply\n");

	*set_code_ = setcode;

	return TRUE;
}


