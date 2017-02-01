#ifndef __TU_H__
#define __TU_H__

#ifdef __cplusplus 
extern "C" {
#endif
     
#include <glib.h>

gint tu_get_pos_of_last_delimiter (const gchar *buffer, gchar delimiter);

gint     gpa_tu_replace_all (const gchar *search_text,
				 const gchar *replace_text,
				 gchar **buffer, gboolean case_sensitive);

GList * gpa_tu_get_list_of_sufixes (const gchar *search_for,
					 const gchar *buffer,
					 gchar separator);
void gpa_tu_list_of_sufixes_free (GList *list);


gboolean gpa_tu_divide (const gchar *buffer,
			    const gchar* start_tag,
			    const gchar *end_tag,
			    gchar **pre, gchar **body, gchar **post);

gint gpa_tu_search (const gchar *buffer,
			const gchar *search_for,
			gboolean case_sensitive);

gint gpa_tu_search_real (const gchar *buffer, gint buffer_length,
					    const gchar *search_text, gint search_text_length,
					    gboolean case_sensitive);

guchar* gpa_tu_read_file (const gchar* file_name, gint *size_,
						gint initial_size, gint grow_size_);

void gpa_tu_xml_clean (gchar **code,
			   gint extra_clean);

gboolean gpa_tu_remove_trailing_spaces (gchar **buffer_);


gint gpa_tu_strnchr (const guchar *buffer, guchar c);


/* Tokens */
gboolean gpa_tu_token_next (const guchar *buffer, gint buffer_size, gint *offset);

gboolean gpa_tu_token_next_till_newline (const guchar *buffer,
							  gint buffer_size,
							  gint *offset);
gchar *  gpa_tu_token_next_dup_till_newline (const guchar *buffer,
								  gint buffer_size,
								  gint *offset);
gchar *  gpa_tu_token_next_dup (const guchar *buffer, gint buffer_length, gint *offset);

gchar *  gpa_tu_token_previous_dup (const guchar *buffer, gint buffer_length, gint *offset);

gboolean gpa_tu_token_next_verify (const guchar *buffer, gint buffer_length,
						 gint *offset, const gchar *label);

gchar *  gpa_tu_token_next_dup_till (const guchar *buffer, gint buffer_size,
						   gint *offset, guchar till_me);
gboolean gpa_tu_token_next_till (const guchar *buffer, gint buffer_size,
					    gint *offset, guchar till_me);
	
gboolean gpa_tu_token_next_search (const guchar *buffer, gint buffer_length, gint *offset,
						 const gchar *search_text, gboolean case_sensitive);


gboolean gpa_tu_string_contains_newline (const gchar *token);

#ifdef __cplusplus 
}
#endif


#endif /* __TU_H__ */



