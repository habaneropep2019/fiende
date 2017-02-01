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

#include "gpa-defs.h"

#include <glib.h>
#include <libgnome/gnome-util.h>
#include <libgnomeui/gnome-dialog.h>
#include <libgnomeprint/gnome-print-encode.h>

#include "xml-utils.h"
#include "text-utils.h"

#include "gpa-private.h"
#include "gpa-include.h"
#include "gpa-backend.h"
#include "gpa-backend-private.h"
#include "gpa-code.h"
#include "gpa-code-private.h"
#include "gpa-model-private.h"
#include "gpa-option.h"
#include "gpa-option-private.h"
#include "gpa-options.h"
#include "gpa-options-private.h"
#include "gpa-printer-private.h"
#include "gpa-settings.h"
#include "gpa-settings-private.h"

/* --------- --------- --------- --------- --------- --------- --------- --------- --------- ---------*/
/* These are copied from gnome-print. To satisfy linking requriements for libgpa */
#include <glib.h>

#define GPC_MAX_CHARS_PER_LINE 80 /* Needs to be a multiple of 2 ! */

/* FIXME: Implement as a macro ? maybe */
static inline gint libgpa_hex_2_dec (guchar upper, guchar lower)
{
  if (upper > '9')
    upper -= 39;
  if (lower > '9')
    lower -= 39;
               
  return ((upper - '0')*16) + lower - '0';
}

static int
libgpa_decode_hex_wcs(int size)
{
  return ((size/2)+((size/2)/GPC_MAX_CHARS_PER_LINE)+4);
}

static int
libgpa_decode_hex (const guchar *in, guchar *out, gint in_size)
{
  gint p1; /* Points to where we are writing into the out buffer */
  gint p2; /* Points to where we are reading from the in buffer */

  p1 = 0;
  p2 = 0;
  for (p2=0; p2 < in_size; p2+=2)
    {
      if ((in [p2] == ' ') ||
	  (in [p2] == '\t') ||
	  (in [p2] == '\n'))
	continue;

      out [p1++] = libgpa_hex_2_dec (in[p2], in [p2+1]);
    }

  return p1;
}

/* --------- --------- --------- --------- --------- --------- --------- --------- --------- ---------*/











/*
  There are 5 and only 5 tags that we will understand .

  1. $ID
  2. $FOREACH
  3. $ENDFOREACH
  4. $VALUE_valuename
  5. $OPTIONSNUM   * For query code only *
  
*/


/**
 * gpa_code_get_string_from_encoding_type
 * @name: 
 * 
 * Translate from a enumed encoding type to a string
 * i.e. :  GPA_ENCODING_HEX -> "Hex" 
 *
 * Return Value: an GpaEncoding type
 **/
static gchar*
gpa_code_get_string_from_encoding_type (GpaEncoding encoding)
{
	switch (encoding) {
	case GPA_ENCODING_UNENCODED:
		return g_strdup (GPA_TAG_UNENCODED);
	case GPA_ENCODING_HEX:
		return g_strdup (GPA_TAG_HEX);
	default:
		gpa_error ("Unknown encoding enum (%i)", encoding);
		return NULL;
	}
}

/**
 * gpa_code_get_encoding_from_name:
 * @name: 
 * 
 * Translate from a gchar encoding to it's enumed type
 * i.e. :  "Hex" -> GPA_ENCODING_HEX
 *
 * Return Value: an GpaEncoding type
 **/
static GpaEncoding
gpa_code_get_encoding_type_from_string (const gchar *name)
{
	debug (FALSE, "no end, rets");

	if (name == NULL)
		return GPA_ENCODING_UNENCODED;
	else if (strcmp (name, GPA_TAG_UNENCODED) == 0)
		return GPA_ENCODING_UNENCODED;
	else if (strcmp (name, GPA_TAG_HEX) == 0)
		return GPA_ENCODING_HEX;
	else {
		gpa_error ("Unrecognized encoding %s\n", name);
		return  GPA_ENCODING_ERROR;
	}

	debug (FALSE, "end");
}

#if 0
/* XXX: libgnomeprint depends on gtk, so I had to paste these functions
 * directly in here.  Maybe I'll work them into macros in a little bit.  See
 * gpa-code.h for the wcs version of decode_hex.  -NickM
 */

/* FIXME: Implement as a macro ? maybe */
static inline gint
libgpa_hex_2_dec (guchar upper, guchar lower)
{
        if (upper > '9')
                upper -= 39;
        if (lower > '9')
                lower -= 39;
         
        return ((upper - '0')*16) + lower - '0';
}

/**
 * gnome_print_decode_hex:
 * @in: a pointer to the buffer to encode
 * @out: a pointer to the prev. allocated out buffer
 * @in_size: the size of the input buffer
 * 
 * Implements hex decoding
 *
 * Return Value: size of the decoded buffer
 **/
static int 
libgpa_decode_hex (const guchar *in, guchar *out, gint in_size)
{
        gint p1; /* Points to where we are writing into the out buffer */
        gint p2; /* Points to where we are reading from the in buffer */

        p1 = 0;
        p2 = 0;
        for (p2=0; p2 < in_size; p2+=2)
        {
                if ((in [p2] == ' ') || 
                    (in [p2] == '\t') ||
                    (in [p2] == '\n'))
                        continue;

                out [p1++] = hex_2_dec (in[p2], in [p2+1]);
        }

        return p1;
}
#endif 

/**
 * gpa_code_decode:
 * @code: 
 * @encoding_type: 
 * 
 * decode "code" based on the "encoding_type"
 * 
 * Return Value: a malloced string, NULL on error. Caller is responsible
 *               of freeing the string
 **/
static gboolean
gpa_code_decode (gchar ** code_, GpaEncoding encoding_type)
{
	gchar *code;
	gchar *decoded;
	gint size;
	gint ret;
	
	debug (FALSE, "");
	
	code = *code_;

	if (code == NULL)
		return TRUE;
	
	switch (encoding_type) {
	case GPA_ENCODING_UNENCODED:
		decoded = g_strdup (code);
		break;
	case GPA_ENCODING_HEX:
		size = libgpa_decode_hex_wcs (strlen(code));
		decoded = g_new (gchar, size);
		ret = libgpa_decode_hex (code,
					 decoded,
					 strlen(code));

		if (ret < 1) {
			gpa_error ("The decoded string is empty");
			return FALSE;
		}
		decoded [ret] = 0;
		break;
	default:
		gpa_error ("Unrecognized encoding type\n");
		return FALSE;
	}

	g_free (*code_);
	*code_ = decoded;
	
	return TRUE;
}

#if 0
/**
 * gpa_code_divide:
 * @code: 
 * @pre: 
 * @body: 
 * @post: 
 * 
 * Divide "code" into "pre/body/post". The $FOREACH & $ENDFOREACH mark the
 * location of where the division takes place.
 * 
 * Return Value: FALSE on error, TRUE otherwise
 **/
static gboolean
gpa_code_divide (const gchar *code, gchar **pre, gchar **body, gchar **post)
{
	debug (FALSE, "");

	if (!gpa_tu_divide (code,
				GPA_TAG_CODE_FOREACH,
				GPA_TAG_CODE_ENDFOREACH,
				pre, body, post)) {
		gpa_error ("The querycode could not be divided in pre/code/post.\n"
			   "Verify the tags :\"%s\" and \"%s\"",
			   GPA_TAG_CODE_FOREACH,
			   GPA_TAG_CODE_ENDFOREACH);
		return FALSE;
	}

#if 0	
	g_print ("pre --->->%s<-<---\n", *pre);
	g_print ("bdy --->->%s<-<---\n", *body);
	g_print ("pre --->->%s<-<---\n", *post);
#endif	

	debug (FALSE, "end");
	
	return TRUE;
}
#endif

static gboolean
gpa_code_fragment_verify_with_option (GpaOption *option,
				      GList *values)
{
	GHashTable *hash;
	GList *list;
	gchar *key;
	gchar *content;

	debug (FALSE, "");

	if (values == NULL)
		return TRUE;
	
	hash = option->values;
	if (hash == NULL) {
		gpa_error ("The option \"%s\" does not contain a value "
			   "list. And the Set/Query code contains a\n"
			   "reference to a %s tag", option->name,
			   GPA_TAG_CODE_VALUE);
		return FALSE;
	}
		
	/* Verify that all of the Values that we need to replace exist
	   in the values hash */
	list = values;
	for (; list != NULL; list = list->next) {
		key = (gchar *) list->data;
		content = g_hash_table_lookup (hash, key);
		if (content == NULL) {
			gpa_error ("The Set/QueryCode contained a reference "
				   "to \"%s\" but it could not be found in "
				   "The option :\"%s\": hash table\n",
				   key, option->name);
			return FALSE;
		}
	}

	return TRUE;
}

/**
 * gpa_code_replace_values:
 * @code_: 
 * @option: 
 * 
 * Replace the refernces to $VALUE-x- inside "code_"
 * 
 * Return Value: TRUE on success, FALSE otherwise
 **/
#if 0
static gboolean
gpa_code_replace_values (gchar **code_, GpaOption *option)
{
	GHashTable *hash;
	GList *values;
	GList *list;
	gchar *code = *code_;
	gchar *key;
	gchar *content;
	gchar *search_for;
	gint replacements;

	debug (FALSE, "");

	if (code == NULL)
		return TRUE;
	
	/* Search For $VALUE_[x] and return a GList of x's */
	values = gpa_tu_get_list_of_sufixes (GPA_TAG_CODE_VALUE, code, '-');

	if (values == NULL)
		return TRUE;
	
	hash = option->values;

	/* Verify that all the values we are going to need are in "hash" */
	if (!gpa_code_fragment_verify_with_option (option, values)) {
		gpa_tu_list_of_sufixes_free (values);
		return FALSE;
	}

	/* Do the actual replace */
	list = values;
	for (; list != NULL; list = list->next) {
		key = (gchar *) list->data;
		content = g_hash_table_lookup (hash, key);
		search_for = g_strdup_printf (GPA_TAG_CODE_VALUE "-%s-", key);
		replacements = gpa_tu_replace_all (search_for,
						       content,
						       &code,
						       TRUE);
		g_free (search_for);
		if (replacements == -1) {
			gpa_tu_list_of_sufixes_free (values);
			return FALSE;
		}
	}

	*code_ = code;

	gpa_tu_list_of_sufixes_free (values);
	
	return TRUE;
}
#endif

#if 0
/**
 * gpa_code_querycode_emit_body:
 * @body: 
 * @options: 
 * 
 * Emit the "body" part of the querycode
 * 
 * Return Value: 
 **/
static gboolean
gpa_code_emit_for_options_body (gchar *body,
				GpaOptions *options,
				gboolean print)
{
	GpaOption *option;
	GList *list;
	gchar *temp_code;
	
	debug (FALSE, "");

	g_return_val_if_fail (options != NULL, FALSE);

	if (body == NULL)
		return TRUE;

	list = options->children;
	for (; list != NULL; list = list->next) {
		option = (GpaOption *)list->data;
		temp_code = g_strdup (body);
		if (gpa_tu_replace_all (GPA_TAG_CODE_ID, option->id, &temp_code, TRUE) == -1)
			return FALSE;        
		if (!gpa_code_replace_values (&temp_code, option))
			return FALSE;
		if (print)
			g_print ("%s", temp_code);
		g_free (temp_code);
	}

	return TRUE;
}
#endif

static GpaCodeFragment *
gpa_fragment_get_from_id (GList *fragments,
			  GpaBackend *backend,
			  const gchar* id)
{
	GpaCodeFragment *fragment;
	GList *list;

	debug (FALSE, "");
	
	g_return_val_if_fail (fragments != NULL, NULL);

	list = fragments;
	for (; list != NULL; list = list->next) {
		fragment = (GpaCodeFragment *)list->data;
		if (fragment->backend != backend)
			continue;
		if (strcmp (fragment->id, id) != 0)
			continue;
		return fragment;
	}

	return NULL;
}

/**
 * gpa_code_optionsnum_replace:
 * @options: 
 * @: 
 * @: 
 * @: 
 * 
 * Replace the reference to "$OPTIOSNUM" with the number of options
 * 
 * Return Value: 
 **/
#if 0
static gboolean
gpa_code_optionsnum_replace (GpaOptions *options,
			     gchar **buffer_1, gchar **buffer_2, gchar **buffer_3)
{
	gchar *options_num;

	debug (FALSE, "");
	
	options_num = g_strdup_printf ("%i", g_list_length (options->children));

	if (*buffer_1 != NULL)
		if (gpa_tu_replace_all (GPA_TAG_CODE_OPTIONSNUM,
					    options_num,
					    buffer_1,
					    TRUE) == -1)
			return FALSE;
	if (*buffer_2 != NULL)
		if (gpa_tu_replace_all (GPA_TAG_CODE_OPTIONSNUM,
					    options_num,
					    buffer_2,
					    TRUE) == -1)
			return FALSE;
	if (*buffer_3 != NULL)
		if (gpa_tu_replace_all (GPA_TAG_CODE_OPTIONSNUM,
					    options_num,
					    buffer_3,
					    TRUE) == -1)
			return FALSE;
	
	g_free (options_num);

	debug (FALSE, "end");
	
	return TRUE;
}
#endif

#if 0
/**
 * gpa_code_querycode_emit:
 * @options: 
 * @print: 
 * 
 * Emit & replace the QueryCode
 * 
 * Return Value: 
 **/
static gboolean
gpa_code_emit_for_options (GpaSettings *settings,
			   GpaOptions *options,
			   const gchar * id,
			   gboolean print)
{
	GpaCodeFragment *fragment;
	GpaOption *option;
	GpaEncoding encoding;
	gchar *code;
	gchar *pre;
	gchar *body;
	gchar *post;
	
	debug (FALSE, "");
	
	g_return_val_if_fail (options != NULL, FALSE);
	g_return_val_if_fail (id != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_SETTINGS (settings), FALSE);

	if (options->code_fragments == NULL) {
		gpa_error ("The option %s does not contain code fragments\n"
			   "A request was made to emit the \"%s\" code fragment.\n",
			   id, options->name);
		return TRUE;
	}
	
	option = gpa_options_get_selected_option (settings, options);
	
	fragment = gpa_fragment_get_from_id (options->code_fragments,
					     NULL,
					     id);
	
	if (fragment == NULL)
		return FALSE;

	g_return_val_if_fail (fragment != NULL,          FALSE);
	g_return_val_if_fail (fragment->content != NULL, FALSE);
	g_return_val_if_fail (option != NULL,            FALSE);
		

	/* Here is the recipie for emiting code :
	   1. clean the tabs added by the xml,
	   2. divide the code in pre,body & post for $FOREACH looping
	   3. replace $OPTIONSNUM
	   4. emit pre
	   5. emit body
	   6. emit post
	*/
	
	code = g_strdup (fragment->content);
	encoding = fragment->encoding;
	
	if (!gpa_code_decode             (&code, fragment->encoding))
		return FALSE;
	if (!gpa_code_divide             (code, &pre, &body, &post))
		return FALSE;
	if (!gpa_code_optionsnum_replace (options, &pre, &body, &post))
		return FALSE;
	if (!gpa_code_replace_values (&pre, option))
		return FALSE;
	/* TEMP */
	if (print && pre != NULL)
		g_print ("%s", pre);
	if (!gpa_code_emit_for_options_body (body, options, print))
		return FALSE;
	if (!gpa_code_replace_values (&post, option))
		return FALSE;
	/* TEMP */
	if (print && post != NULL)
		g_print ("%s", post);

	/* Free everything */
	g_free (code);
	if (pre != NULL)
		g_free (pre);
	if (post != NULL)
		g_free (post);
	if (body != NULL)
		g_free (body);
	
	debug (FALSE, "end");

	return TRUE;
}
#endif

static gboolean
gpa_code_emit_for_settings (GpaSettings *settings,
			    const gchar * id,
			    gboolean print)
{
	GpaBackend *backend;
	GpaCodeFragment *fragment = NULL;
	GpaEncoding encoding;
	gchar *code;

	debug (FALSE, "");

	g_return_val_if_fail (GPA_IS_SETTINGS (settings), FALSE);

	if (settings->printer->model->code_fragments == NULL) {
		gpa_error ("The printer %s does not contain code fragments\n"
			   "A request was made to emit the \"%s\" code fragment\n",
			   settings->printer->name, id);
		return TRUE;
	}

	backend = gpa_backend_get_selected (settings);
	if (backend == NULL)
		return FALSE;

	fragment = gpa_fragment_get_from_id (settings->printer->model->code_fragments,
					     backend,
					     id);
	
	if (fragment == NULL)
		return FALSE;

	g_return_val_if_fail (fragment != NULL,          FALSE);
	g_return_val_if_fail (fragment->content != NULL, FALSE);

	code = g_strdup (fragment->content);
	encoding = fragment->encoding;

	if (!gpa_code_decode             (&code, fragment->encoding))
		return FALSE;

	if (print && code)
		g_print ("%s", code);
	
	g_free (code);
	
	debug (FALSE, "end");

	return TRUE;
}

/**
 * gpa_code_emit_all:
 * @printer: 
 * 
 * This is a temp function to test the emision of setcode & querycode
 **/
void
gpa_code_emit_all (GpaSettings *settings)
{
	GpaOptions *options;
	GList *list;

	debug (FALSE, "");

	g_return_if_fail (GPA_IS_SETTINGS (settings));

	list = settings->printer->model->options_list;
	options = (GpaOptions *) list->data;
	
	g_print ("Emiting code for Printer Settings \"%s\"\n", settings->name);
	
	g_print ("--------------------------------------\n");
	if (!gpa_code_emit_for_settings (settings, "Reset", TRUE))
		return;
	
	g_print ("--------------------------------------\n");
#if 0	
	g_print ("Emiting code for Option \"%s\"\n", options->name);
	g_print ("--------------------------------------");

	if (!gpa_code_emit_for_options (settings, options, GPA_TAG_SET, TRUE))
		return;
	
	g_print ("--------------------------------------");
	
	if (!gpa_code_emit_for_options (settings, options, GPA_TAG_QUERY, TRUE))
		return;

	g_print ("--------------------------------------\n");
#endif	

}





#if 0 /* Generalize */
/**
 * gpa_code_setcode_emit:
 * @options: 
 * @print: 
 * 
 * Emit the Setcode
 * 
 * Return Value: FALSE on error, TRUE otherwise
 **/
static gboolean
gpa_code_setcode_emit (GpaOptions *options, gboolean print)
{
	GpaOption *option;
	gchar *temp_code;
	gint replacements;
	
	debug (FALSE, "");
	
	g_return_val_if_fail (options != NULL,          FALSE);
	g_return_val_if_fail (options->setcode != NULL, FALSE);

	/* Get the selected option */
	option = gpa_options_get_selected_option (options);
	if (option == NULL)
		return FALSE;

	/* Decode the setcode */
	temp_code = gpa_code_decode (options->setcode, options->setcode_encoding);
	if (temp_code == NULL)
		return FALSE;

	/* Replace the "$ID" with the selected option ID */
	replacements = gpa_tu_replace_all (GPA_TAG_CODE_ID, option->id,
					       &temp_code, TRUE);
	if (replacements == -1)
		return FALSE;

	/* Replace the values from the option's hash */
	if (!gpa_code_replace_values (&temp_code, option))
		return FALSE;

	/* This is temp, till we find something better to do with temp_code */
	if (print)
		g_print ("->>%s<<-", temp_code);
	
	g_free (temp_code);

	debug (FALSE, "end");
	
	return TRUE;
}
#endif


#if 0
#define GPA_HEX_CHARS_PER_LINE 60
static gchar *
gpa_code_hex_blockify (gchar *in)
{
	gchar *out;
	gint size;
	gint p1;
	gint p2;
	gint col;

	g_return_val_if_fail (in != NULL, NULL);

	size = strlen (in);

	out = g_new (gchar, size+((size+1)/GPA_HEX_CHARS_PER_LINE)+2);

	p1 = 0;
	p2 = 0;
	col = 0;

	out [p2++] = '\n';
	
	for (; p1 < size; p1++) {
		out [p2++] = in [p1];
		
		if (col++==GPA_HEX_CHARS_PER_LINE) {
			out [p2++] = '\n';
			col = 0;
		}
	}

	out [p2] = 0;

	return out;
}
#endif


/**
 * gpa_code_querycode_verify_nesting:
 * @options: 
 * 
 * Verify that the querycode $FOREACH and $ENDFOREACH nesting is 0K
 * 
 * Return Value: 
 **/
static gboolean
gpa_code_fragment_verify_nesting (GpaCodeFragment *fragment)
{
	gchar *start_tag;
	gchar *end_tag;
	gchar *code;
	gint ret;

	debug (FALSE, "");

	g_return_val_if_fail (fragment != NULL, FALSE);

	code = g_strdup (fragment->content);
	
	if (code == NULL)
		return TRUE;

	gpa_code_decode (&code, fragment->encoding);

	start_tag = g_strdup (GPA_TAG_CODE_FOREACH);
	end_tag   = g_strdup (GPA_TAG_CODE_ENDFOREACH);

	/* Is there a start_tag ?. If there is, verify that there isn't and end tag */
	ret = gpa_tu_search (code, start_tag, TRUE);
	if (ret == -1) {
		ret = gpa_tu_search (code, end_tag, TRUE);
		if (ret != -1) {
			gpa_error ("The \"%s\" code fragment contained a %s without "
				   "a corresponding %s",
				   fragment->id, end_tag, start_tag);
			goto gpa_code_nesting_error;
			return FALSE;
		}
		goto gpa_code_nesting_success;
	}

	/* Search for a second start tag. If found, emit an error */
	ret = gpa_tu_search (code+ret+1, start_tag, TRUE);
	if (ret != -1) {
		gpa_error ("The querycode contained more than one \"%s\" tags.",
			   start_tag);
		goto gpa_code_nesting_error;
	}

	/* Search for an end_tag. If not found, emit an error */
	ret = gpa_tu_search (code, end_tag, TRUE);
	if (ret == -1) {
		gpa_error ("The querycode contained a \"%s\" tag without a \"%s\" tag.",
			   start_tag, end_tag);
		goto gpa_code_nesting_error;
	}

	/* Search for a second end tag. If found, emit an error */
	ret = gpa_tu_search (code+ret+1, end_tag, TRUE);
	if (ret != -1) {
		gpa_error ("The querycode contained more than one \"%s\" tags.",
			   end_tag);
		goto gpa_code_nesting_error;
	}

	/* Verify the the start tag appears before the end tag, if not emit and error */
	ret = gpa_tu_search (code, end_tag, TRUE);
	if (ret < gpa_tu_search (code, start_tag, TRUE)) {
		gpa_error ("The querycode contained the \"%s\" and  \"%s\" tags in the "
			   "wrong order", start_tag, end_tag);
		goto gpa_code_nesting_error;
	}

gpa_code_nesting_success:
	g_free (code);
	g_free (start_tag);
	g_free (end_tag);
	
	return TRUE;

gpa_code_nesting_error:
	g_free (code);
	g_free (start_tag);
	g_free (end_tag);

	return FALSE;
}


static gboolean
gpa_code_fragment_verify_with_options (GpaCodeFragment *fragment,
				       GpaOptions *options)
{
	GpaOption *option;
	GList *values;
	GList *list;
	gchar *code;
	
	debug (FALSE, "");

	/* Fragments need to be either "Set" or "Query" till we
	   find that there needs to be generic fragments inside options */
	if ( (strcmp (fragment->id, GPA_TAG_SET) != 0) &&
	     (strcmp (fragment->id, GPA_TAG_QUERY) != 0)) {
		g_warning ("Code fragment inside option from invalid type.\n"
			   "Valid types are %s and %s",
			   GPA_TAG_SET, GPA_TAG_QUERY);
		return FALSE;
	}

	if (fragment->backend == NULL) {
		gpa_error ("The fragment \"%s\" does not contain a backend\n",
			   fragment->id);
		return FALSE;
	}

	code = fragment->content;
	values = gpa_tu_get_list_of_sufixes (GPA_TAG_CODE_VALUE, code, '-');
	list = options->children;
	for (; list != NULL; list = list->next) {
		option = (GpaOption *) list->data;
		if (!gpa_code_fragment_verify_with_option (option, values)) {
			gpa_tu_list_of_sufixes_free (values);
			return FALSE;
		}
	}
	gpa_tu_list_of_sufixes_free (values);

	if (!gpa_code_fragment_verify_nesting (fragment))
		return FALSE;

#if 0	/* FIXME, we are not verifying that the emission will be successful */
	if (setcode) {
		if (!gpa_code_setcode_emit (options, FALSE))
			return FALSE;
	} else {
		if (!gpa_code_querycode_verify_nesting (options))
			return FALSE;
		if (!gpa_code_querycode_emit (options, FALSE))
			return FALSE;
	}
#endif	

	return TRUE;
}

gboolean
gpa_code_fragments_verify_with_options (GpaOptions *options)
{
	GpaCodeFragment *fragment;
	GList *list;
	
	debug (FALSE, "");

	g_return_val_if_fail (options != NULL, FALSE);

	list = options->code_fragments;

	if (list == NULL)
		return TRUE;

	for (;list != NULL; list = list->next) {
		fragment = (GpaCodeFragment *)list->data;
		if (!gpa_code_fragment_verify_with_options (fragment, options))
			return FALSE;
	}

	return TRUE;
}

gboolean
gpa_code_fragments_verify_with_printer (GList *list_, GpaPrinter *printer)
{
	GList *list;
	
	debug (FALSE, "");

	g_return_val_if_fail (printer != NULL, FALSE);

	list = list_;

	if (list == NULL)
		return TRUE;

	return TRUE;
}

GpaCodeFragment *
gpa_code_fragment_new (void)
{
	GpaCodeFragment *fragment;
	
	fragment = g_new0 (GpaCodeFragment, 1);

	fragment->id       = NULL;
	fragment->encoding = GPA_ENCODING_UNENCODED;
	fragment->content  = NULL;
	fragment->backend  = NULL;

	return fragment;
}
		

static GpaCodeFragment *
gpa_code_fragment_new_from_node (xmlNodePtr tree_,
				 GpaModel *model)
{
	GpaCodeFragment *fragment;
	GpaBackend *backend;
	GpaEncoding encoding_type;
	xmlNodePtr tree;
	gchar *encoding;
	gchar *content;
	gchar *backend_id;
	gchar *id;
	
	debug (FALSE, "");

	tree = gpa_include_node (tree_);
	
	if (!gpa_xml_node_verify (tree, GPA_TAG_CODE_FRAGMENT))
		return NULL;
	id = gpa_xml_get_value_string_required (tree, GPA_TAG_ID, NULL);
	if (id == NULL)
		return NULL;
	encoding = gpa_xml_get_value_string_required (tree, GPA_TAG_ENCODING, NULL);
	if (encoding == NULL)
		return NULL;
	content = gpa_xml_get_value_string_required (tree, GPA_TAG_CONTENT, NULL);
	if (content  == NULL)
		return NULL;
	backend_id = gpa_xml_get_value_string_required (tree, GPA_TAG_BACKEND, NULL);
	if (backend_id  == NULL)
		return NULL;

	backend = gpa_backend_get_from_id (model, backend_id);
	g_free (backend_id);
	if (backend == NULL)
		return NULL;

	encoding_type = gpa_code_get_encoding_type_from_string (encoding);
	if (encoding_type == GPA_ENCODING_ERROR)
		return NULL;
	g_free (encoding);


	fragment = gpa_code_fragment_new ();
	
	fragment->id       = id;
	gpa_tu_xml_clean (&content,(encoding_type==GPA_ENCODING_UNENCODED)?FALSE:TRUE);
	fragment->content  = content;
	fragment->encoding = encoding_type;
	fragment->backend  = backend;

	debug (FALSE, "end");
	
	return fragment;
}


static xmlNodePtr
gpa_code_fragment_write (GpaCodeFragment *fragment,
			 XmlParseContext *context)
{
	xmlNodePtr node;
	gchar *encoding;

	debug (FALSE, "");

	g_return_val_if_fail (fragment != NULL, NULL);
			      
	node = xmlNewDocNode (context->doc, context->ns, GPA_TAG_CODE_FRAGMENT, NULL);

	/* Id */
	xmlNewChild (node, context->ns, GPA_TAG_ID,       fragment->id);
	/* Backend  */
	xmlNewChild (node, context->ns, GPA_TAG_BACKEND,  fragment->backend->id);
	/* Encoding */
	encoding = gpa_code_get_string_from_encoding_type (fragment->encoding);
	if (encoding == NULL)
		return NULL;
	xmlNewChild (node, context->ns, GPA_TAG_ENCODING, encoding);
	g_free (encoding);
	/* Content */
	xmlNewChild (node, context->ns, GPA_TAG_CONTENT,  fragment->content);


	return node;
}

xmlNodePtr
gpa_code_fragments_write (XmlParseContext *context, GList *list_)
{
	GpaCodeFragment *fragment;
	xmlNodePtr node;
	xmlNodePtr child;
	GList *list;
	
	debug (FALSE, "");

	g_return_val_if_fail (context != NULL, NULL);

	list = list_;

	if (list == NULL)
		return NULL;

	node = xmlNewDocNode (context->doc, context->ns, GPA_TAG_CODE_FRAGMENTS, NULL);

	for (; list != NULL; list = list->next) {
		fragment = (GpaCodeFragment *) list->data;
		child = gpa_code_fragment_write (fragment, context);
		if (child == NULL)
			return NULL;
		xmlAddChild (node, child);
	}

	return node;
}

GList *
gpa_code_fragments_new_from_node (xmlNodePtr tree_,
				  GpaModel *model)
{
	GpaCodeFragment *fragment;
	xmlNodePtr tree;
	xmlNodePtr node;
	GList *list;

	debug (FALSE, "");

	tree = gpa_include_node (tree_);
	
	if (!gpa_xml_node_verify (tree, GPA_TAG_CODE_FRAGMENTS))
		return NULL;

	list = NULL;
	node = tree->childs;

	while (node != NULL) {
		skip_text (node);
		if (!gpa_xml_node_verify (node, GPA_TAG_CODE_FRAGMENT))
			return NULL;
		fragment = gpa_code_fragment_new_from_node (node,
							    model);
		if (fragment == NULL)
			return NULL;
		list = g_list_prepend (list, fragment);
		node = node->next;
	}
	
	return g_list_reverse (list);
}


static gboolean
gpa_code_fragment_free (GpaCodeFragment *fragment)
{
	debug (FALSE, "");

	g_return_val_if_fail (fragment          != NULL, FALSE);
	g_return_val_if_fail (fragment->id      != NULL, FALSE);
	g_return_val_if_fail (fragment->content != NULL, FALSE);

	g_free (fragment->id);
	g_free (fragment->content);
	g_free (fragment);

	return TRUE;
}

gboolean
gpa_code_fragments_free (GList *list_)
{
	GpaCodeFragment *fragment;
	GList *list;

	list = list_;
	for (; list != NULL; list = list->next) {
		fragment = (GpaCodeFragment *) list->data;
		if (!gpa_code_fragment_free (fragment))
			return FALSE;
	}

	g_list_free (list_);

	return TRUE;
}
	

static GpaCodeFragment *
gpa_code_fragment_copy (GpaCodeFragment *fragment)
{
	GpaCodeFragment *new_fragment;

	debug (FALSE, "");

	g_return_val_if_fail (fragment != NULL, NULL);

	new_fragment = gpa_code_fragment_new ();
	new_fragment->id       = g_strdup (fragment->id);
	new_fragment->content  = g_strdup (fragment->content);
	new_fragment->encoding = fragment->encoding;
	new_fragment->backend  = fragment->backend;

	return new_fragment;
}

GList *
gpa_code_fragments_copy (GList *list_)
{
	GpaCodeFragment *fragment;
	GpaCodeFragment *new_fragment;
	GList *new_list = NULL;
	GList *list;

	debug (FALSE, "");

	list = list_;
	for (; list != NULL; list = list->next) {
		fragment = (GpaCodeFragment *) list->data;
		new_fragment = gpa_code_fragment_copy (fragment);
		if (new_fragment == NULL)
			return NULL;
		new_list = g_list_prepend (new_list, new_fragment);
	}

	return g_list_reverse (new_list);
}
