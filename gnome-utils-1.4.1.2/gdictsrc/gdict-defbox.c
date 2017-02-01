/* $Id: gdict-defbox.c,v 1.5 2001/06/22 22:15:42 kmaraas Exp $ */

/*
 *  Mike Hughes <mfh@psilord.com>
 *  Papadimitriou Spiros <spapadim+@cs.cmu.edu>
 *  Bradford Hovinen <hovinen@udel.edu>
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 *
 *  GDict main window
 *
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#include <gnome.h>

#include "gdict-defbox.h"
#include "gdict-app.h"
#include "gdict-pref.h"

#ifdef HAVE_GNOME_PRINT
#  include <libgnomeprint/gnome-printer-profile.h>
#  include <math.h>
#endif /* HAVE_GNOME_PRINT */

enum {
    WORD_LOOKUP_START_SIGNAL,
    WORD_LOOKUP_DONE_SIGNAL,
    WORD_NOT_FOUND_SIGNAL,
    SUBSTR_NOT_FOUND_SIGNAL,
    SOCKET_ERROR_SIGNAL,
    LAST_SIGNAL
};

static gint gdict_defbox_signals[LAST_SIGNAL] = { 0, 0, 0, 0 };

static void gdict_defbox_init (GDictDefbox *defbox);
static void gdict_defbox_class_init (GDictDefboxClass *class);

static void def_error_cb  (dict_command_t *command, DictStatusCode code, 
                           gchar *message, gpointer data);
static void def_status_cb (dict_command_t *command, DictStatusCode code, 
                           int num_found, gpointer data);
static void def_data_cb   (dict_command_t *command, dict_res_t *res,
                           gpointer data);

static gboolean is_xref   (gchar *str, int len);
static gboolean is_number (gchar *str, int len);
static gboolean is_part   (gchar *str, int len);
static void defbox_add    (GDictDefbox *defbox, gchar *def);

#ifdef HAVE_GNOME_PRINT
static GnomeTextLine *get_line (const char *fontlist, const char *string, 
                                double size);
#endif

/* gdict_defbox_get_type
 *
 * Register the GDictDefbox type with Gtk's type system if necessary and
 * return the type identifier code
 */

guint 
gdict_defbox_get_type (void) {
    static guint gdict_defbox_type = 0;
    
    if (!gdict_defbox_type) {
        GtkTypeInfo gdict_defbox_info = {
            "GDictDefbox",
            sizeof (GDictDefbox),
            sizeof (GDictDefboxClass),
            (GtkClassInitFunc) gdict_defbox_class_init,
            (GtkObjectInitFunc) gdict_defbox_init,
            (GtkArgSetFunc) NULL,
            (GtkArgGetFunc) NULL
        };
        
        gdict_defbox_type = 
            gtk_type_unique (gtk_text_get_type (), &gdict_defbox_info);
    }
    
    return gdict_defbox_type;
}

/* gdict_defbox_init
 *
 * Initialises an instance of a GDictDefbox object
 */

static void 
gdict_defbox_init (GDictDefbox *defbox) {
    defbox->context = NULL;
    defbox->def_cmd = NULL;
}

/* gdict_defbox_class_init
 *
 * Initialises a structure describing the GDictDefbox class; sets up signals
 * for defbox events in the Gtk signal management system
 */

static void 
gdict_defbox_class_init (GDictDefboxClass *class) {
    GtkObjectClass *object_class;
    
    object_class = (GtkObjectClass *) class;
    
    gdict_defbox_signals[WORD_LOOKUP_START_SIGNAL] =
        gtk_signal_new ("word_lookup_start", GTK_RUN_FIRST, object_class->type,
                        GTK_SIGNAL_OFFSET (GDictDefboxClass, word_lookup_start),
                        gtk_signal_default_marshaller, GTK_TYPE_NONE, 0);
    
    gdict_defbox_signals[WORD_LOOKUP_DONE_SIGNAL] =
        gtk_signal_new ("word_lookup_done", GTK_RUN_FIRST, object_class->type,
                        GTK_SIGNAL_OFFSET (GDictDefboxClass, word_lookup_done),
                        gtk_signal_default_marshaller, GTK_TYPE_NONE, 0);
    
    gdict_defbox_signals[WORD_NOT_FOUND_SIGNAL] =
        gtk_signal_new ("word_not_found", GTK_RUN_FIRST, object_class->type,
                        GTK_SIGNAL_OFFSET (GDictDefboxClass, word_not_found),
                        gtk_signal_default_marshaller, GTK_TYPE_NONE, 0);
    
    gdict_defbox_signals[SUBSTR_NOT_FOUND_SIGNAL] =
        gtk_signal_new ("substr_not_found", GTK_RUN_FIRST, object_class->type,
                        GTK_SIGNAL_OFFSET (GDictDefboxClass, substr_not_found),
                        gtk_signal_default_marshaller, GTK_TYPE_NONE, 0);
    
    gdict_defbox_signals[SOCKET_ERROR_SIGNAL] =
        gtk_signal_new ("socket_error", GTK_RUN_FIRST, object_class->type,
                        GTK_SIGNAL_OFFSET (GDictDefboxClass, socket_error),
                        gtk_marshal_NONE__STRING, GTK_TYPE_NONE, 1,
                        GTK_TYPE_STRING);
    
    gtk_object_class_add_signals (object_class, gdict_defbox_signals,
                                  LAST_SIGNAL);
    
    class->word_lookup_start = NULL;
    class->word_lookup_done = NULL;
    class->word_not_found = NULL;
    class->substr_not_found = NULL;
}

/* gdict_defbox_new
 *
 * Creates a new GDictDefbox object
 */

GtkWidget *
gdict_defbox_new (void) {
    GDictDefbox *defbox;
    
    defbox = GDICT_DEFBOX (gtk_type_new (gdict_defbox_get_type ()));
    return GTK_WIDGET (defbox);
}

/* gdict_defbox_destroy
 *
 * Deallocates memory associated with the defbox
 */

void
gdict_defbox_destroy (GDictDefbox *defbox) {
    g_free (defbox->database);
}

/* gdict_defbox_lookup
 *
 * Sends the command to the server to commence looking up the definition
 * of a word and sets the callbacks so that the definition will be displayed
 * in this defbox
 *
 * Returns 0 on success and -1 on command invocation error
 */

gint
gdict_defbox_lookup (GDictDefbox *defbox, gchar *text) {
    g_return_val_if_fail (defbox != NULL, -1);
    g_return_val_if_fail (IS_GDICT_DEFBOX (defbox), -1);
    g_return_val_if_fail (text != NULL, -1);
    
    while (isspace (*text)) text++;
    
    if (*text == '\0')
        return 0;
    
    gtk_signal_emit (GTK_OBJECT (defbox), 
                     gdict_defbox_signals[WORD_LOOKUP_START_SIGNAL]);
    
    gdict_defbox_clear (defbox);
    
    if (defbox->database)
	    g_free (defbox->database);
    
    defbox->database = g_strdup (gdict_pref.database);
    
    defbox->def_cmd = dict_define_command_new (defbox->database, text);
    defbox->def_cmd->error_notify_cb = def_error_cb;
    defbox->def_cmd->status_notify_cb = def_status_cb;
    defbox->def_cmd->data_notify_cb = def_data_cb;
    defbox->def_cmd->user_data = defbox;

    if (dict_command_invoke (defbox->def_cmd, defbox->context) < 0)
	return -1;

    return 0;
}

/* gdict_defbox_clear
 *
 * Clears the text in a defbox and eliminates the current command structure
 */

void 
gdict_defbox_clear (GDictDefbox *defbox) {
    g_return_if_fail (defbox != NULL);
    g_return_if_fail (IS_GDICT_DEFBOX (defbox));
    
    gtk_text_set_point (GTK_TEXT(defbox), 0);
    gtk_text_forward_delete (GTK_TEXT(defbox), 
                             gtk_text_get_length (GTK_TEXT (defbox)));

    if (defbox->def_cmd) {
        dict_command_destroy (defbox->def_cmd);
        defbox->def_cmd = NULL;
    }
}

/* gdict_defbox_find
 *
 * Finds a string of text in the current definition
 */

void 
gdict_defbox_find (GDictDefbox *defbox, gchar *text, gboolean start) {
    static gchar *lastp = NULL;
    static gchar *deftext = NULL;
    gchar *findp;

    g_return_if_fail (defbox != NULL);
    g_return_if_fail (IS_GDICT_DEFBOX (defbox));
    
    if (!lastp || start) {
        g_free (deftext);
        deftext = gtk_editable_get_chars (GTK_EDITABLE (defbox), 0, 
                                	  gtk_text_get_length (GTK_TEXT (defbox)));
        lastp = deftext;
    }

    if (deftext == NULL)
        return;

    findp = strstr(lastp, text);

    if (findp) {
        gtk_editable_select_region (GTK_EDITABLE(defbox),
				    (findp - deftext),
				    (findp - deftext) + strlen(text));
        lastp = findp + 1;
    } else {
        gtk_signal_emit (GTK_OBJECT (defbox), 
                         gdict_defbox_signals[SUBSTR_NOT_FOUND_SIGNAL]);
    }
}

/* gdict_defbox_reset
 *
 * Resets the defbox by re-invoking the define command on a new database
 * and/or server
 */

void
gdict_defbox_reset (GDictDefbox *defbox, dict_context_t *context) {
    gchar *word;
    
    if (context != defbox->context || 
        defbox->database == NULL ||
        strcmp (defbox->database, gdict_pref.database))
    {
        defbox->context = context;
        
        if (defbox->def_cmd) {
            word = g_strdup (defbox->def_cmd->search_term);
            dict_command_destroy (defbox->def_cmd);
            defbox->def_cmd = NULL;
            gdict_defbox_lookup (defbox, word);
            g_free (word);
        }
    }
}

#ifdef HAVE_GNOME_PRINT

/* gdict_defbox_print
 * 
 * Prints the current definition
 */

void 
gdict_defbox_print (GDictDefbox *defbox) {
    GnomePrintContext *pc;
    GList *node;
    dict_res_t *res;
    int x = 32, y = 700;

    g_return_if_fail (defbox != NULL);
    g_return_if_fail (IS_GDICT_DEFBOX (defbox));
    g_return_if_fail (gdict_printer != NULL);

    if (defbox->def_cmd == NULL)
        return;

    pc = gnome_print_context_new (gdict_printer);
    g_return_if_fail(pc != NULL);

    for (node = defbox->def_cmd->res_list; node; node = g_list_next (node)) {
        gchar *linep, line[1024];  /* FIXME */
        GnomeTextLine *textline;
        
        res = (dict_res_t *) node->data;
        linep = res->desc;

        while (linep && *linep) {
            gchar *endp = strchr(linep, '\n') - 1;

            bzero(line, sizeof(line));
            strncpy(line, linep, endp - linep);
            textline = get_line("Times", line, 12.0);
            gnome_print_moveto(pc, x, y);
            gnome_print_textline(pc, textline);
            gnome_text_line_free(textline);
            y -= 14;
            linep = endp + 2;
        }
        textline = get_line("Times", "", 12.0);
        gnome_print_moveto(pc, x, y);
        gnome_print_textline(pc, textline);
        gnome_text_line_free(textline);
        y -= 14;
    }

    gnome_print_context_close(pc);
}

#endif /* HAVE_GNOME_PRINT */

/* gdict_defbox_get_word
 *
 * Returns the word defined in the defbox, if any
 */

gchar *
gdict_defbox_get_word (GDictDefbox *defbox) {
    g_return_val_if_fail (defbox != NULL, NULL);
    g_return_val_if_fail (IS_GDICT_DEFBOX (defbox), NULL);
    
    return defbox->def_cmd ? defbox->def_cmd->search_term : NULL;
}

/* is_xref
 *
 * Returns true if the substring str[0..len-1] is a cross reference
 */

static gboolean 
is_xref (gchar *str, int len) {
    gint i;
    for (i = 0;  i < len;  i++)
        if (!isupper(str[i]) && !ispunct(str[i]))
            return FALSE;
    return TRUE;
}

/* is_number
 *
 * Returns true if the substring given by str[0..len-1] is a heading number
 */

static gboolean 
is_number (gchar *str, int len) {
    gint i;

    if (str[len-1] != ':')
        return FALSE;

    for (i = 0;  i < len - 1;  i++)
        if (!islower(str[i]) && !isdigit(str[i]))
            return FALSE;

    return TRUE;
}

/* is_part
 *
 * Returns true if the substring given by str[0..len-1] is a part of speech
 */

static gboolean 
is_part (gchar *str, int len) {
    gchar buf[3];

    if ((len < 1) || (len > 2))
        return FALSE;

    strncpy(buf, str, 2);
    buf[len] = 0;

    return (strcmp(buf, "n") == 0) || \
           (strcmp(buf, "vt") == 0) || \
           (strcmp(buf, "vi") == 0) || \
           (strcmp(buf, "vb") == 0) || \
           (strcmp(buf, "aj") == 0) || \
           (strcmp(buf, "av") == 0);
}

/* defbox_add
 *
 * Adds a definition to the defbox, performing all necessary formatting
 */

static void 
defbox_add (GDictDefbox *defbox, gchar *def) {
    gchar *p;
    gint len;
    GdkFont *font;
    GdkColor *color;

    gtk_text_freeze (GTK_TEXT (defbox));

    font = gdict_pref.typefaces[TYPEFACE_HEADWORD].font;
    color = gdict_pref.typefaces[TYPEFACE_HEADWORD].color;
    p = def;
    len = strcspn(def, " ");

    while (*p) {
        /* Handle word token */
        gtk_text_insert (GTK_TEXT (defbox), font, color, NULL, p, len);
        if ((font == gdict_pref.typefaces[TYPEFACE_HEADWORD].font) || \
            (font == gdict_pref.typefaces[TYPEFACE_NUMBER].font) || \
            (font == gdict_pref.typefaces[TYPEFACE_PART].font) || \
            (font == gdict_pref.typefaces[TYPEFACE_XREF].font))
        {
            font = gdict_pref.typefaces[TYPEFACE_BODY].font;
            color = gdict_pref.typefaces[TYPEFACE_BODY].color;
        }
        p += len;
        
        /* ... then handle spaces ... */
        len = strspn(p, " ");
        if (len > 0)
            gtk_text_insert (GTK_TEXT (defbox),  font, color, NULL, p, len);
        p += len;

        /* ... handle special characters ... */
        if (*p == '\\') {
            font = (font == gdict_pref.typefaces[TYPEFACE_BODY].font) ? \
                gdict_pref.typefaces[TYPEFACE_PRONUNCIATION].font : \
                gdict_pref.typefaces[TYPEFACE_BODY].font;
            color = (color == gdict_pref.typefaces[TYPEFACE_BODY].color) ? \
                gdict_pref.typefaces[TYPEFACE_PRONUNCIATION].color : \
                gdict_pref.typefaces[TYPEFACE_BODY].color;
            ++p;
        } else if (*p == '[') {
            font = gdict_pref.typefaces[TYPEFACE_ETYMOLOGY].font;
            color = gdict_pref.typefaces[TYPEFACE_ETYMOLOGY].color;
            ++p;
        } else if (*p == '{') {
            font = gdict_pref.typefaces[TYPEFACE_EXAMPLE].font;
            color = gdict_pref.typefaces[TYPEFACE_EXAMPLE].color;
            ++p;
        } else if ((*p == ']') || (*p == '}')) {
            font = gdict_pref.typefaces[TYPEFACE_BODY].font;
            color = gdict_pref.typefaces[TYPEFACE_BODY].color;
            ++p;
        }
        
        len = strcspn (p, " \\[]{}");

        if (font == gdict_pref.typefaces[TYPEFACE_BODY].font) {
            if (is_xref(p, len)) {
                font = gdict_pref.typefaces[TYPEFACE_XREF].font;
                color = gdict_pref.typefaces[TYPEFACE_XREF].color;
            } else if (is_number(p, len)) {
                font = gdict_pref.typefaces[TYPEFACE_NUMBER].font;
                color = gdict_pref.typefaces[TYPEFACE_NUMBER].color;
            } else if (is_part(p, len)) {
                font = gdict_pref.typefaces[TYPEFACE_PART].font;
                color = gdict_pref.typefaces[TYPEFACE_PART].color;
            }
        }
    }

    gtk_text_insert (GTK_TEXT (defbox), 
                     gdict_pref.typefaces[TYPEFACE_BODY].font, 
                     gdict_pref.typefaces[TYPEFACE_BODY].color, NULL, "\n", 1);
    gtk_text_thaw (GTK_TEXT (defbox));
}

/* def_error_cb
 *
 * Callback invoked when there was an error in the last query
 */

static void
def_error_cb (dict_command_t *command, DictStatusCode code,
              gchar *message, gpointer data)
{
    GtkObject *defbox;
    dict_command_t *sec_cmd;
    gchar *string;
    
    defbox = GTK_OBJECT (data);
    
    if (code != DICT_SOCKET_ERROR) {
        string = g_strdup_printf (_("Error invoking query: %s"), message);
        gnome_error_dialog (string);
        sec_cmd = dict_disconnect_command_new ();
        dict_command_invoke (sec_cmd, command->context);
        
        gtk_signal_emit (defbox, gdict_defbox_signals[WORD_LOOKUP_DONE_SIGNAL]);
    }
    else {
        gtk_signal_emit (defbox, gdict_defbox_signals[SOCKET_ERROR_SIGNAL],
                         message);
    }
}

/* def_data_cb
 *
 * Callback used when a new definition has arrived over the link
 */

static void 
def_data_cb (dict_command_t *command, dict_res_t *res, gpointer data) {
    GDictDefbox *defbox;
    
    defbox = GDICT_DEFBOX (data);
    defbox_add (defbox, res->desc);
}

/* def_status_cb
 *
 * Callback used when a status code has arrived over the link
 */

static void 
def_status_cb (dict_command_t *command, DictStatusCode code, 
               int num_found, gpointer data)
{
    GtkObject *defbox;
    
    defbox = GTK_OBJECT (data);
    
    if (code == DICT_STATUS_OK)
      gtk_signal_emit (defbox, gdict_defbox_signals[WORD_LOOKUP_DONE_SIGNAL]);
    else if (code == DICT_STATUS_NO_MATCH)
      gtk_signal_emit (defbox, gdict_defbox_signals[WORD_NOT_FOUND_SIGNAL]);
}

#ifdef HAVE_GNOME_PRINT

/* get_line
 *
 * Formats a single line of text for printing and returns it
 */

static GnomeTextLine *
get_line (const char *fontlist, const char *string, double size) {
    GnomeTextAttrEl attrs[4];
    int n_chars, i;
    GnomeTextLayout *layout;
    GnomeTextLine *line;

    n_chars = 0;
    for (i = 0; string[i]; i++)
      if ((string[i] & 0xc0) != 0x80)
        n_chars++;

    i = 0;
    attrs[i].char_pos = 0;
    attrs[i].attr = GNOME_TEXT_FONT_LIST;
    attrs[i++].attr_val = gnome_text_intern_font_list (fontlist);
    attrs[i].char_pos = 0;
    attrs[i].attr = GNOME_TEXT_SIZE;
    attrs[i++].attr_val = floor (size * 1000 + 0.5);
    attrs[i].char_pos = n_chars;
    attrs[i].attr = GNOME_TEXT_END;
    attrs[i++].attr_val = 0;

    layout = gnome_text_layout_new (string, attrs);

    line = gnome_text_line_from_layout (layout);

    gnome_text_layout_free (layout);

    return line;
}

#endif /* HAVE_GNOME_PRINT */
