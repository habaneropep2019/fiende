/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/* Copyright (C) 1998 Redhat Software Inc. 
 * Authors: Jonathan Blandford <jrb@redhat.com>
 *
 * Support for i18n in the screenserver configuration file subsections.
 * 1999-03-22, Pablo Satxaga <srtxg@chanae.alphanet.ch>  -- srtxg 
 *
 */
#include <string.h>

#ifndef SIZEBUF
#define SIZEBUF 512
#endif

#include "parser.h"
#include "gnome.h"
#include "capplet-widget.h"
#include <libgnome/gnome-i18n.h>

#define get_lrange(ssd) (get_range (ssd, 0)) 
#define get_nrange(ssd) (get_range (ssd, 1)) 

typedef enum
{
        UNSET,
        ENTRY,
        LRANGE,
        NRANGE,
        CHECK,
        COLOR
} W_TYPES;
typedef struct _setup_data setup_data;
struct _setup_data
{
        gchar *name;
        W_TYPES widget_type;
        GtkWidget *widget;
        gchar *val_type;
        gchar *flag1;
        gchar *flag2; /* optional */
        gchar *label;
        gchar *right_label;
        gchar *left_label;
        gchar *comment;
        gchar *arg;
};

extern GtkWidget *capplet;

/* Callback prototypes for the generated widgets */
static void entry_callback (GtkEditable *editable, const gchar *text, gint length, gint *position, setup_data *ssd);
static void entry_delete_callback (GtkEditable *editable, gint length, gint *position, setup_data *ssd);
static void check_callback (GtkWidget *cbox, setup_data *ssd);
static void range_callback (GtkAdjustment *adj, setup_data *ssd);
static void color_callback (GnomeColorPicker *color_button, guint red, guint green, guint blue, guint alpha, setup_data *ssd);
static void set_entry (setup_data *ssd);
static void set_range (setup_data *ssd);
static void set_check (setup_data *ssd);
static void set_color (setup_data *ssd);


static setup_data *
new_arg_vals ()
{
        setup_data *retval = g_malloc (sizeof (setup_data));

        retval->name = NULL;
        retval->widget_type = UNSET;
        retval->val_type = NULL;
        retval->flag1 = NULL;
        retval->flag2 = NULL;
        retval->label = NULL;
        retval->left_label = NULL;
        retval->right_label = NULL;
        retval->comment = NULL;
        retval->arg = NULL;
        return retval;
}
static void
free_setup_data (setup_data *dat)
{
        if (dat->name)
                g_free (dat->name);
        if (dat->val_type)
                g_free (dat->val_type);
        if (dat->flag1)
                g_free (dat->flag1);
        if (dat->flag2)
                g_free (dat->flag1);
        if (dat->label)
                g_free (dat->label);
        if (dat->right_label)
                g_free (dat->right_label);
        if (dat->left_label)
                g_free (dat->left_label);
        if (dat->comment)
                g_free (dat->comment);
        if (dat->arg)
                g_free (dat->arg);
        g_free (dat);
}
static void
print_data (setup_data *dat)
{
        g_print ("Name: %s\n", dat->name);
        switch (dat->widget_type) {
        case UNSET:
                g_print ("Type: UNSET\n");
                break;
        case ENTRY:
                g_print ("Type: ENTRY\n");
                break;
        case LRANGE:
                g_print ("Type: LRANGE\n");
                break;
        case NRANGE:
                g_print ("Type: NRANGE\n");
                break;
        case CHECK:
                g_print ("Type: CHECK\n");
                break;
        case COLOR:
                g_print ("Type: COLOR\n");
                break;
        }
        if (!dat->val_type)
                g_print ("ValType: NULL\n");
        else
                g_print ("ValType: %s\n", dat->val_type);
        g_print ("Flag: %s\n", dat->flag1);
        if (dat->flag2)
                g_print ("Flag2: %s\n", dat->flag2);
        else
                g_print ("Flag2: NULL\n");
        if (dat->label)
                g_print ("Label: %s\n", dat->label);
        else
                g_print ("Label: NULL\n");
        if (dat->comment)
                g_print ("Comment: %s\n", dat->comment);
        else
                g_print ("Comment: NULL\n");
        g_print ("Arg: %s\n", dat->arg);
        g_print ("\n\n");
}
static gboolean
validate_setup_data (setup_data *dat)
{
        /* We make sure that the entry is sane, and set up the arg field. */

        switch (dat->widget_type) {
        case UNSET:
                return FALSE;
        case ENTRY:
                if ((dat->val_type == NULL) ||
                    (dat->flag1 == NULL) ||
                    (dat->name == NULL))
                        return FALSE;
                return TRUE;
        case LRANGE:
                if ((dat->val_type == NULL) ||
                    (dat->flag1 == NULL) ||
                    (dat->name == NULL) ||
                    (dat->left_label == NULL) ||
                    (dat->right_label == NULL))
                        return FALSE;
                return TRUE;
        case NRANGE:
                if ((dat->val_type == NULL) ||
                    (dat->flag1 == NULL) ||
                    (dat->name == NULL))
                        return FALSE;
                return TRUE;
        case CHECK:
                if ((dat->flag1 == NULL) ||
                    (dat->flag2 == NULL) ||
                    (dat->name == NULL) ||
                    (dat->left_label == NULL))
                        return FALSE;
                return TRUE;
        case COLOR:
                if ((dat->val_type == NULL) ||
                    (dat->label == NULL) ||
                    (dat->flag1 == NULL) ||
                    (dat->name == NULL))
                        return FALSE;
                return TRUE;
        default:
                return FALSE;
        }
}
static void
parse_key (setup_data *arg_vals, gchar *key, gchar *value, int cpt_iter)
{
    GList *lang = NULL;
    char buf[BUFSIZ];
    char sfx[BUFSIZ];
    /* as the user can define several languages, and as parse_key()
     * can receive the data in another order thant the user preferred
     * languages order; a set of counters are defined to attach a
     * notion of "priority" to the translated labels. the various
     * cpt_* hold the priority level of the language used in a previous
     * matching entry; i holds the currently tryed language priority;
     * if i is lower then the previous translated string is replaced 
     * whith the one translated to the currently tested language.
     * cpt_iter however doesn't hold a priority but tells (if equal to 0)
     * that a new configuration section is parsed, and then reset
     * the others cpt_* counters.        -- srtxg
     */
    static int cpt_n,cpt_l,cpt_c,cpt_ll,cpt_rl;
    int i=0;

    /* we are in a new config section to parse; reset the priority
     * counters to a value high enough so they will allways be less
     * priority (I hope nobody has a $LANGUAGE variable whith 100+ 
     * preferences :)    -- srtxg
     */ 
    if (cpt_iter==0) {
	cpt_n=100;
	cpt_l=100;
	cpt_c=100;
	cpt_ll=100;
	cpt_rl=100;
    }

    lang = gnome_i18n_get_language_list ("LC_MESSAGE");
    do
    {
	if (lang) {
		if (strcmp("C",(gchar *) lang->data) !=0 && strcmp("POSIX",(gchar *)lang->data)!=0)
			snprintf(sfx,SIZEBUF,"[%s]",(gchar *)lang->data);
		else
			sfx[0]='\0';
		lang=lang->next;
	} else {
		sfx[0]='\0';
	}

#define TEST_FOR_I18N_KEY(a)	( (snprintf(buf,SIZEBUF,"%s%s",a,sfx)) && \
				  (strcmp (key, buf) == 0) )

        if TEST_FOR_I18N_KEY("Name") {
                if (cpt_n > i) {
			 arg_vals->name = g_strdup (value);
			 cpt_n=i;
		}
		break;
        } else if (strcmp (key, "Type") == 0) {
                if (strcmp (value, "Entry") == 0)
                        arg_vals->widget_type = ENTRY;
                else if (strcmp (value, "LRange") == 0)
                        arg_vals->widget_type = LRANGE;
                else if (strcmp (value, "NRange") == 0)
                        arg_vals->widget_type = NRANGE;
                else if (strcmp (value, "Check") == 0)
                        arg_vals->widget_type = CHECK;
                else if (strcmp (value, "Color") == 0)
                        arg_vals->widget_type = COLOR;
		break;
        } else if (strcmp (key, "ValType") == 0) {
                arg_vals->val_type = g_strdup (value); break;
        } else if (strcmp (key, "Flag") == 0) {
                arg_vals->flag1 = g_strdup (value); break;
        } else if (strcmp (key, "Flag2") == 0) {
                arg_vals->flag2 = g_strdup (value); break;
        } else if TEST_FOR_I18N_KEY("Label") {
                if (cpt_l > i) {
			arg_vals->label = g_strdup (value);
			cpt_l=i;
		}
		break;
        } else if TEST_FOR_I18N_KEY("Comment") {
		if (cpt_c > i) {
                	arg_vals->comment = g_strdup (value);
			cpt_c=i;
		}
		break;
        } else if TEST_FOR_I18N_KEY("LeftLabel") {
		if (cpt_ll > i) {
	                arg_vals->left_label = g_strdup (value);
			cpt_ll=i;
		}
		break;
        } else if TEST_FOR_I18N_KEY("RightLabel") {
		if (cpt_rl > i) {
        	        arg_vals->right_label = g_strdup (value);
			cpt_rl=i;
		}
		break;
	}

	i++;

    } while (lang);

}
static
void set_entry (setup_data *ssd)
{
        /* FIXME: set arg field */
        gnome_config_set_string (ssd->name, gtk_entry_get_text (GTK_ENTRY (ssd->widget)));
}
static
void set_range (setup_data *ssd)
{
        gchar val[100];
        if (ssd->arg)
                g_free (ssd->arg);
        if (ssd->val_type[0] == 'I') {
                snprintf (val, 100, " %d", (gint) (GTK_RANGE (ssd->widget))->adjustment->value);
                gnome_config_set_int (ssd->name, (gint) (GTK_RANGE (ssd->widget))->adjustment->value);
        } else {
                snprintf (val, 100, " %f", (GTK_RANGE (ssd->widget))->adjustment->value);
                gnome_config_set_float (ssd->name, (GTK_RANGE (ssd->widget))->adjustment->value);
        }
        ssd->arg = g_strconcat (ssd->flag1, val, NULL);
}
static
void set_check (setup_data *ssd)
{
        if (GTK_TOGGLE_BUTTON (ssd->widget)->active)
                ssd->arg = g_strdup (ssd->flag1);
        else 
                ssd->arg = g_strdup (ssd->flag2);
   
        gnome_config_set_bool (ssd->name,(GTK_TOGGLE_BUTTON (ssd->widget)->active));
}
static
void set_color (setup_data *ssd)
{
        gchar buffer[60];
        gushort red, green, blue;

        if (ssd->arg)
                g_free (ssd->arg);
        /* Get the colors and convert them to a string. */
        gnome_color_picker_get_i16 (GNOME_COLOR_PICKER (ssd->widget),
                                    &red,
                                    &green,
                                    &blue,
                                    NULL); 
        snprintf (buffer, sizeof(buffer), "#%02x%02x%02x",
                  red >> 8,
                  green >> 8,
                  blue >> 8);
        gnome_config_set_string (ssd->name, buffer);

        ssd->arg = g_strconcat (ssd->flag1, " ", buffer, NULL);
}

static GtkAdjustment *
get_adjustment (gchar *type)
{
        GtkObject *retval;
        gchar *temp;
        gchar *temp2;
        gchar **temp3; /*sheesh, i should be more creative in my names */
        gint low;
        gint high;
        gint val;

        if (strlen (type) < 2)
                return GTK_ADJUSTMENT (gtk_adjustment_new (0.5, 0.0, 1.0, 0.1, 0.1, 0.1));

        if (type[0] == 'I') {
                if (type[1] == '[') {
                        temp = strdup (type + 2);
                        temp2 = strstr (temp, "]");
                        temp2[0] = '\000';
                        temp3 = g_strsplit (temp, "-", -1);
                        /* FIXME: This will segv if incorrectly entered. */
                        low = atoi (temp3[0]);
                        high = atoi (temp3[1]);
                        if (temp2[1]=='=') 
                                val = atoi (temp2+2);
                        else 
                                val = low;
                        
                        g_strfreev (temp3);
                        free (temp);
                        /* we add a little to the high value, b/c otherwise the adjustment
                         * won't display the higher value
                         */
                        return GTK_ADJUSTMENT (gtk_adjustment_new (val, low, high + 1, 1, 1, 1));
                }
                else
                        return GTK_ADJUSTMENT (gtk_adjustment_new (5, 0, 10, 1, 1, 1));
        }
        return GTK_ADJUSTMENT (gtk_adjustment_new (5, 0, 10, 1, 1, 1));
}
static GtkWidget *
get_entry (setup_data *ssd)
{
        GtkWidget *frame = NULL;
        GtkWidget *entry;
        GtkWidget *box;
        gchar *prefix;
        gchar *val;
                
        if (ssd->label)
                frame = gtk_frame_new (ssd->label);
        box = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
        gtk_container_set_border_width (GTK_CONTAINER (box), GNOME_PAD_SMALL);

        /* make the entry */
        entry = gtk_entry_new ();
        ssd->widget = entry;
        
        prefix = g_strconcat (ssd->name, "=", ssd->val_type, NULL);
        val = gnome_config_get_string_with_default (prefix, NULL);

        g_free (prefix);
        
        if (val)
                gtk_entry_set_text (GTK_ENTRY (entry), val);
        gtk_signal_connect_after (GTK_OBJECT (entry), "insert_text", GTK_SIGNAL_FUNC (entry_callback), ssd);
        gtk_signal_connect_after (GTK_OBJECT (entry), "delete_text", GTK_SIGNAL_FUNC (entry_delete_callback), ssd);
        if (ssd->left_label)
                gtk_box_pack_start (GTK_BOX (box), gtk_label_new (ssd->left_label), FALSE, FALSE, 0);
        gtk_box_pack_start (GTK_BOX (box), entry, TRUE, TRUE, 0);
        if (ssd->right_label)
                gtk_box_pack_start (GTK_BOX (box), gtk_label_new (ssd->right_label), FALSE, FALSE, 0);

        /* set up the args */
        if (ssd->val_type)
                ssd->arg = g_strconcat (ssd->flag1, " \"", ssd->val_type, "\"", NULL);
        else
                ssd->arg = g_strconcat (ssd->flag1, " \"\"", NULL);

        if (frame) {
                gtk_container_add (GTK_CONTAINER (frame), box);
                return frame;
        } else
                return box;
}
static GtkWidget *
get_range (setup_data *ssd, gint type)
{
        GtkAdjustment *adj;
        GtkWidget *label;
        GtkWidget *align;
        GtkWidget *frame = NULL;
        GtkWidget *box;
        gchar *prefix;
        gchar temp[25];
        GtkWidget *range;

        if (ssd->label)
                frame = gtk_frame_new (ssd->label);
        box = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
        gtk_container_set_border_width (GTK_CONTAINER (box), GNOME_PAD_SMALL);

        /* set up the adjustment */
        adj = get_adjustment (ssd->val_type);
        if (ssd->val_type[0] == 'I')
                snprintf (temp, 24, "%d", (gint) adj->value);
        else
                snprintf (temp, 24, "%f", adj->value);
        
        prefix = g_strconcat (ssd->name, "=", temp, NULL);
        
        adj->value = gnome_config_get_int_with_default (prefix, NULL);
        gtk_signal_connect (GTK_OBJECT (adj), "value_changed", (GtkSignalFunc) range_callback, ssd);
        g_free (prefix);

        /* set up the range */
        range = gtk_hscale_new (adj);
        ssd->widget = range;
        
        if (ssd->val_type[0] == 'I')
                gtk_scale_set_digits (GTK_SCALE (range), 0);
        
        if (type == 0)
                gtk_scale_set_draw_value (GTK_SCALE (range), FALSE);
        if (ssd->left_label) {
                label = gtk_label_new (ssd->left_label);
                align = gtk_alignment_new (0.5, 1.0, 0.0, 0.0);
                gtk_container_add (GTK_CONTAINER (align), label);
                gtk_box_pack_start (GTK_BOX (box), align, FALSE, FALSE, 0); 
        }
        gtk_box_pack_start (GTK_BOX (box), range, TRUE, TRUE, 0);
        if (ssd->right_label) {
                label = gtk_label_new (ssd->right_label);
                align = gtk_alignment_new (0.5, 1.0, 0.0, 0.0);
                gtk_container_add (GTK_CONTAINER (align), label);
                gtk_box_pack_start (GTK_BOX (box), align, FALSE, FALSE, 0); 
        }

        /* finally, we set the arg */
        ssd->arg = g_strconcat (ssd->flag1, " ", temp, NULL);

        /* and return the widget */
        if (frame) {
                gtk_container_add (GTK_CONTAINER (frame), box);
                return frame;
        } else
                return box;
}
static GtkWidget *
get_check (setup_data *ssd)
{
        GtkWidget *frame = NULL;
        GtkWidget *check;
        GtkWidget *box;
        gchar *prefix;
        gboolean set;
        
        if (ssd->label)
                frame = gtk_frame_new (ssd->label);
        prefix = g_strconcat (ssd->name, "=FALSE", NULL);
        box = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
        gtk_container_set_border_width (GTK_CONTAINER (box), GNOME_PAD_SMALL);
        check = gtk_check_button_new_with_label (ssd->left_label);
        ssd->widget = check;
        
        set = gnome_config_get_bool_with_default (prefix, NULL);

        g_free (prefix);

        gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON (check), set);
        gtk_signal_connect (GTK_OBJECT (check), "toggled", (GtkSignalFunc) check_callback, ssd);
        gtk_box_pack_start (GTK_BOX (box), check, TRUE, TRUE, 0);
        if (set)
                ssd->arg = g_strdup (ssd->flag1);
        else
                ssd->arg = g_strdup (ssd->flag2);
        if (frame) {
                gtk_container_add (GTK_CONTAINER (frame), box);
                return frame;
        } else
                return box;
}
/* This should prolly honor left_label, but I want to keep the color
 * buttons aligned vertically. Maybe pack them into a global
 * gtk_table? Icky. - alh */
static GtkWidget *
get_color (setup_data *ssd) {
        GtkWidget *frame = NULL;
        GtkWidget *color_button;
        GtkWidget *alignment;
        GtkWidget *label;
        GtkWidget *align;
        GdkColor selected_color;
        gchar *prefix;
        gchar **temp_ptr;
        gchar *color_string;

        if (ssd->label) 
                frame = gtk_frame_new (ssd->label);

        /* This will center the color picker in the frame. */
        alignment = gtk_alignment_new (0.5, 1.0, 0.25, 1.0);

        /* make the color picker with its default (if any) */
        color_button = gnome_color_picker_new();
        ssd->widget = color_button;
        /* ssd->val_type is currently "Color=<#color>", seperate out
         * <#color> */
        temp_ptr = g_strsplit(ssd->val_type, "=", -1);
        prefix = g_strconcat (ssd->name, "=", temp_ptr[1], NULL);
        color_string = gnome_config_get_string_with_default (prefix, NULL);

        g_free (prefix);

        gdk_color_parse(color_string, &selected_color);
        gnome_color_picker_set_i16(GNOME_COLOR_PICKER( color_button ),
                                   selected_color.red,
                                   selected_color.green,
                                   selected_color.blue,
                                   0);
        gtk_signal_connect (GTK_OBJECT (color_button), "color_set", (GtkSignalFunc) color_callback, ssd);
        gtk_container_add (GTK_CONTAINER (alignment), color_button);

        /* set the arg to be the current color value */
        ssd->arg = g_strconcat (ssd->flag1, " ", color_string, NULL);
        
        if (frame) {
                gtk_container_add (GTK_CONTAINER (frame), alignment);
                return frame;
        } else
                return alignment;
}
static void
write_field (setup_data *ssd)
{
        switch (ssd->widget_type) {
        case ENTRY:
                set_entry (ssd);
                break;
        case LRANGE:
        case NRANGE:
                set_range (ssd);
                break;
        case CHECK:
                set_check (ssd);
                break;
        case COLOR:
                set_color (ssd);
                break;
        default:
		break;
        }
}
static void
entry_callback (GtkEditable *editable, const gchar *text, gint length, gint *position, setup_data *ssd)
{
        if (ssd->arg)
                g_free (ssd->arg);
        
        ssd->arg = g_strconcat (ssd->flag1, " \"", gtk_entry_get_text (GTK_ENTRY (editable)),"\"", NULL);
        capplet_widget_state_changed(CAPPLET_WIDGET (capplet), TRUE);
}
static void
entry_delete_callback (GtkEditable *editable, gint length, gint *position, setup_data *ssd)
{
        if (ssd->arg)
                g_free (ssd->arg);
        
        if (strlen (gtk_entry_get_text (GTK_ENTRY (editable))) > 0)
                ssd->arg = g_strconcat (ssd->flag1, " \"", gtk_entry_get_text (GTK_ENTRY (editable)),"\"", NULL);
        else
                ssd->arg = g_strconcat (ssd->flag1, " \"\"", NULL);
        capplet_widget_state_changed(CAPPLET_WIDGET (capplet), TRUE);

}
static void
check_callback (GtkWidget *cbox, setup_data *ssd)
{
        if (ssd->arg)
                g_free (ssd->arg);

        if (GTK_TOGGLE_BUTTON (cbox)->active)
                ssd->arg = g_strdup (ssd->flag1);
        else 
                ssd->arg = g_strdup (ssd->flag2);
}
static void
range_callback (GtkAdjustment *adj, setup_data *ssd)
{
        gchar val[100];
        if (ssd->arg)
                g_free (ssd->arg);
        if (ssd->val_type[0] == 'I')
                snprintf (val, 100, " %d", (gint) adj->value);
        else
                snprintf (val, 100, " %f", adj->value);
        ssd->arg = g_strconcat (ssd->flag1, val, NULL);
        capplet_widget_state_changed(CAPPLET_WIDGET (capplet), TRUE);
}
static void
color_callback (GnomeColorPicker *color_button, guint red, guint blue,
                guint green, guint alpha, setup_data *ssd)
{
        gchar buffer[60];

        if (ssd->arg)
                g_free (ssd->arg);

        /* Get the colors and convert them to a string. */
        snprintf (buffer, sizeof(buffer), "#%02x%02x%02x",
                  red >> 8,
                  green >> 8,
                  blue >> 8);

        ssd->arg = g_strconcat (ssd->flag1, " ", buffer, NULL);
        capplet_widget_state_changed(CAPPLET_WIDGET (capplet), TRUE);
}

/* public functions */
void
init_screensaver_data (screensaver_data *sd)
{
        /* this gets the stuff that's not part of the desktop-entry */
        /* it will look in the .gnome dir for some of this stuff */
        gchar *prefix;
        gchar *tempstring;
        gchar *tempstring2;
        gchar *sec;
        gchar *key;
        gchar *value;
        void *sec_iter;
        void *val_iter;
        setup_data *arg_vals;
	/* I needed some flag to tell parse_key wheter there is a new
	 * section or not; so it can reinitialize its own counters -- srtxg
	 */
	int cpt_iter;

        /* We set up the prefix for most of our gnome_config stuff. */
        prefix = g_strconcat ("=", sd->desktop_filename, "=", NULL);
        gnome_config_push_prefix (prefix);
        /* We determine the current args */
        tempstring = gnome_config_get_string_with_default ("Screensaver Data/DefaultFlags", NULL);
        gnome_config_pop_prefix ();
        tempstring2 = g_strconcat ("/Screensaver/", sd->name, "/args=", sd->tryexec, " ", tempstring ,NULL);
        g_free (tempstring);
        sd->args = gnome_config_get_string_with_default (tempstring2, NULL);
        g_free (tempstring2);

        /* We determine the setup layout. */
        gnome_config_drop_file (sd->desktop_filename);
        gnome_config_drop_file (prefix);
        sec_iter = gnome_config_init_iterator_sections (prefix);
        sd->setup_data = NULL;
        while ((sec_iter = gnome_config_iterator_next (sec_iter, &sec, NULL))) {
                if (strncmp ("Arg", sec, 3) == 0) {
			cpt_iter=0; /* there is a new section */
                        arg_vals = new_arg_vals();
                        tempstring = g_strconcat (prefix, "/", sec, "/", NULL);
                        val_iter = gnome_config_init_iterator (tempstring);
                        while ((val_iter = gnome_config_iterator_next (val_iter, &key, &value))) {
                                parse_key (arg_vals, key, value, cpt_iter);
				cpt_iter++;
                                g_free (key);
                                g_free (value);
                        }
                        if (validate_setup_data (arg_vals))
                                sd->setup_data = g_list_prepend (sd->setup_data, arg_vals);
                        /* FIXME: THIS IS REALLY ANNOYING!!! */
                        /* I don't know why this breaks, but this line here
                         * seems to break the g_strconcat a few lines earlier
                         * on the next pass!!! */
                        /*else
                          free_setup_data (arg_vals);*/
                        g_free (tempstring);
                }
                g_free (sec);
        } 
        g_free (prefix);
}
GtkWidget *
get_screensaver_widget (screensaver_data *sd)
{
        GtkWidget *vbox;
        setup_data *ssd;
        GList *list;
        gchar *prefix;
        
        vbox = gtk_vbox_new (FALSE, GNOME_PAD);
        prefix = g_strconcat ("/Screensaver/", sd->name, "/", NULL);
        gnome_config_push_prefix (prefix);
        g_free (prefix);
        
        for (list = sd->setup_data; list; list = list->next) {
                ssd = ((setup_data *)list->data);
                switch (ssd->widget_type) {
                case ENTRY:
                        gtk_box_pack_start (GTK_BOX (vbox), get_entry (ssd), FALSE, FALSE, 0);
                        break;
                case LRANGE:
                        gtk_box_pack_start (GTK_BOX (vbox), get_lrange (ssd), FALSE, FALSE, 0);
                        break;
                case NRANGE:
                        gtk_box_pack_start (GTK_BOX (vbox), get_nrange (ssd), FALSE, FALSE, 0);
                        break;
                case CHECK:
                        gtk_box_pack_start (GTK_BOX (vbox), get_check (ssd), FALSE, FALSE, 0);
                        break;
                case COLOR:
                        gtk_box_pack_start (GTK_BOX (vbox), get_color (ssd), FALSE, FALSE, 0);
                        break;
                default:
                        g_warning ("unknown type trying to be realized");
                }
                
        }
        gnome_config_pop_prefix ();
        gtk_container_set_border_width (GTK_CONTAINER (vbox), GNOME_PAD_SMALL);
        return vbox;
}
/* we go through the arg list and generate our arg field */
void
store_screensaver_data (screensaver_data *sd)
{
        gchar *temp;
        GList *list;
        GString *arg;
        gchar *prefix;

        if (sd == NULL)
                return;

        prefix = g_strconcat ("/Screensaver/", sd->name, "/", NULL);
        gnome_config_push_prefix (prefix);
        g_free (prefix);
        if (sd->args)
                g_free (sd->args);
        arg = g_string_new (sd->tryexec);
        /*        if (sd->root){
                g_string_append_c (arg, ' ');
                g_string_append (arg, sd->root);
                } */       
        for (list = sd->setup_data; list; list=list->next) {
                write_field ((setup_data *)list->data);

                if (strlen (((setup_data *)list->data)->arg) > 0) {
                        /* we do this b/c if there is no flag
                         * (ie. a default setting) 
                         * we don't want to add an extra space */
                        g_string_append_c (arg, ' ');
                        g_string_append (arg, ((setup_data *)list->data)->arg);
                }
        }
        gnome_config_set_string ("ARGS", arg->str);
        sd->args = arg->str;
        g_string_free (arg, FALSE);
        gnome_config_pop_prefix ();
}
        
void
free_screensaver_data (screensaver_data *sd)
{
        /* FIXME: hmmm... I should write this (: */
}
