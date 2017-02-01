#include <config.h>
#include "logview.h"
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <regex.h>
#include <gnome.h>

#define MAX_NUM_MATCHES     10
#define DELIM               ":"

void mon_edit_actions (GtkWidget *widget, gpointer data);

static GList *copy_actions_db (GList *db);
void free_actions_db (GList **db);
void free_action (Action *action);
void print_actions_db (GList *db);
void make_tree_from_actions_db (GList *db);
void clear_actions_db (GList *db);

static void edit_actions_entry_cb (GtkWidget *widget, gpointer data);
static void remove_actions_entry_cb (GtkWidget *widget, gpointer data);
static void add_actions_entry_cb (GtkWidget *widget, gpointer data);
static void edit_action_entry (Action *action);

int exec_action_in_db (Log *log, LogLine *line, GList *db);
int read_actions_db (char *filename, GList **db);
int write_actions_db (char *filename, GList *db);



extern ConfigData *cfg;
extern GList *actions_db;


static GList *local_actions_db = NULL;

GtkWidget *actions_dialog = NULL;
GtkCTree *ctree = NULL;
GtkCTreeNode *selected_node = NULL;
GList *ctree_parent = NULL;

static void
tree_select_row (GtkCTree     *ctree,
		 GtkCTreeNode *row,
		 gint          column)
{
	selected_node = row;
}

static void
tree_unselect_row (GtkCTree     *ctree,
		   GtkCTreeNode *row,
		   gint          column)
{
	if (selected_node == row)
		selected_node = NULL;
}


static void
apply_actions (GtkWidget *w, gpointer data)
{
	char *fname;

	free_actions_db (&actions_db);
	actions_db = local_actions_db;
	local_actions_db = NULL;

	fname = g_strdup_printf ("%s/.gnome/logview-actions.db",
				 g_get_home_dir ());
	if (write_actions_db (fname, actions_db)) {
		gtk_widget_destroy (actions_dialog);
	}
	g_free (fname);
}


/* ----------------------------------------------------------------------
   NAME:        mon_edit_actions
   DESCRIPTION: Create widow where actions are added and changed.
   ---------------------------------------------------------------------- */

void
mon_edit_actions (GtkWidget *widget, gpointer data)
{
  GtkWidget *hbox;
  GtkWidget *button;       
  GtkWidget *padding;       
  GtkWidget *swin;
  GtkBox *vbox;
  const gchar *title[] = {N_("Action database")};
  
  /* Create main window ------------------------------------------------  */
  actions_dialog = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width (GTK_CONTAINER (actions_dialog), 5);
  gtk_window_set_title (GTK_WINDOW (actions_dialog), _("Monitor options"));
  gtk_widget_set_style (actions_dialog, cfg->main_style);
  gtk_widget_set_usize (actions_dialog, 400, -1);
  gtk_signal_connect (GTK_OBJECT (actions_dialog), "destroy",
		      GTK_SIGNAL_FUNC (gtk_widget_destroyed),
		      &actions_dialog);
  
  vbox = (GtkBox *)gtk_vbox_new (FALSE, 2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 4);
  gtk_container_add (GTK_CONTAINER (actions_dialog), GTK_WIDGET (vbox));
  gtk_widget_show (GTK_WIDGET (vbox)); 
  
  /* List with actions */

  ctree = GTK_CTREE (gtk_ctree_new_with_titles (1, 0, (char **)title));
  gtk_signal_connect (GTK_OBJECT (ctree), "tree_select_row",
		      GTK_SIGNAL_FUNC (tree_select_row), NULL);
  gtk_signal_connect (GTK_OBJECT (ctree), "tree_unselect_row",
		      GTK_SIGNAL_FUNC (tree_unselect_row), NULL);
  selected_node = NULL;
  swin = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (swin), GTK_WIDGET (ctree));
  gtk_ctree_set_line_style (ctree, GTK_CTREE_LINES_DOTTED);
  gtk_clist_set_reorderable (GTK_CLIST(ctree), TRUE);
  gtk_widget_set_usize (GTK_WIDGET (ctree), -1, 300);

/* ----------------------------------------------------------------------
  gtk_signal_connect (GTK_OBJECT (ctree), "button_press_event",
  GTK_SIGNAL_FUNC (button_press), NULL);
  gtk_signal_connect_after (GTK_OBJECT (ctree), "button_press_event",
  GTK_SIGNAL_FUNC (after_press), NULL);
  gtk_signal_connect (GTK_OBJECT (ctree), "button_release_event",
  GTK_SIGNAL_FUNC (button_release), NULL);
  gtk_signal_connect_after (GTK_OBJECT (ctree), "button_release_event",
  GTK_SIGNAL_FUNC (after_press), NULL);
  gtk_signal_connect_after (GTK_OBJECT (ctree), "tree_move",
  GTK_SIGNAL_FUNC (after_move), NULL);
  -------------------------------------------------------------------- */

  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (swin), TRUE, TRUE, 0);
  gtk_clist_column_titles_passive (GTK_CLIST (ctree));
  gtk_clist_set_column_justification (GTK_CLIST (ctree), 1, GTK_JUSTIFY_RIGHT);
  gtk_clist_set_selection_mode (GTK_CLIST (ctree), GTK_SELECTION_SINGLE);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swin),
				  GTK_POLICY_ALWAYS, GTK_POLICY_AUTOMATIC);
  gtk_clist_set_column_width (GTK_CLIST (ctree), 0, 300);
  gtk_widget_show_all (GTK_WIDGET(swin));
      
      
  /* Make bottom part ------------------------------------------------ */
  hbox = gtk_hbox_new (FALSE, 2);
  gtk_box_pack_start (vbox, hbox, TRUE, TRUE, 0);
  gtk_widget_show (hbox);

  button = gtk_button_new_with_label (_("Add..."));
  gtk_signal_connect (GTK_OBJECT (button), "clicked", 
		      GTK_SIGNAL_FUNC (add_actions_entry_cb), NULL);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, TRUE, 0);
  gtk_widget_show (button);
  
  button = gtk_button_new_with_label (_("Edit..."));
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, TRUE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked", 
		      GTK_SIGNAL_FUNC (edit_actions_entry_cb), NULL);
  gtk_widget_show (button);
  
  button = gtk_button_new_with_label (_("Remove"));
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, TRUE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked", 
		      GTK_SIGNAL_FUNC (remove_actions_entry_cb), NULL);
  gtk_widget_show (button);

  hbox = gtk_hbox_new (FALSE, 2);
  gtk_box_pack_start (vbox, hbox, TRUE, TRUE, 0);
  gtk_widget_show (hbox);

  /* Add padding to right justify */
  padding = gtk_label_new (" ");
  gtk_widget_show (padding);
  gtk_box_pack_start (GTK_BOX (hbox), padding, TRUE, TRUE, 0);

  button = gnome_stock_button (GNOME_STOCK_BUTTON_CANCEL);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, TRUE, 0);
  gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
			     GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     GTK_OBJECT (actions_dialog));
  gtk_widget_show (button);

  button = gnome_stock_button (GNOME_STOCK_BUTTON_OK);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, TRUE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (apply_actions),
		      NULL);
  gtk_widget_show (button);

  /* Copy all actions */
  free_actions_db (&local_actions_db);
  local_actions_db = copy_actions_db (actions_db);

  /* Generate tree with action database */
  make_tree_from_actions_db (local_actions_db);

  gtk_widget_show (actions_dialog);
}


/* ----------------------------------------------------------------------
   NAME:          make_tree_from_actions_db
   DESCRIPTION:   Create the tree from the database.
   ---------------------------------------------------------------------- */

void
make_tree_from_actions_db (GList *db)
{
  char *text[2];
  char buffer[50];
  GList *item;
  GList *sibling = NULL;
  Action *action;

  text[0] = buffer;
  text[1] = NULL;
  
  /* Create first item */
  ctree_parent = NULL;
  buffer[0] = '\0';

  for(item = db; item != NULL; item=item->next)
    {
      action = (Action *)item->data;
      if (strncmp (buffer, action->tag, 49) != 0) {
	  strncpy (buffer, action->tag, 50);
	  buffer[49] = '\0';
	  /* this is the parent node */
	  sibling = (GList *)gtk_ctree_insert_node (ctree, NULL, NULL, (char **)text, 5, NULL, NULL,
				      NULL, NULL, FALSE, FALSE);
	  ctree_parent = sibling;
	}
      /* this is the actual node */
      sibling = (GList *)gtk_ctree_insert_node (ctree, (GtkCTreeNode *)ctree_parent, NULL, (char **)text, 5, NULL, NULL,
						NULL, NULL, FALSE, FALSE);
      /* Store data in tree */
      gtk_ctree_node_set_row_data (ctree, (GtkCTreeNode *)sibling,
				   (gpointer) action);
    }

  /* done. */
  return;

}

static int
action_compare (gconstpointer a, gconstpointer b)
{
	const Action *aa = a;
	const Action *bb = b;

	return strcmp (aa->tag, bb->tag);
}



/* ----------------------------------------------------------------------
   NAME:        read_action_db
   DESCRIPTION: Reads the database with regular expressions to match.
   ---------------------------------------------------------------------- */

int
read_actions_db (char *filename, GList **db)
{
  Action *item;
  FILE *fp;
  char buffer[1024];
  char *c1, *tok;
  int done;

  /* Open regexp DB */
  fp = fopen (filename, "r");
  if (fp == NULL)
    {
      g_snprintf (buffer, sizeof (buffer),
		  _("Cannot open action data base <%s>! Open failed."), 
		  filename);
      ShowErrMessage (buffer);
      return(-1);
    }


  /* Start parsing file */
  done = FALSE;
  buffer[1023] = '\0';
  while (!done)
    {

      /* Read line */
      if (fgets( buffer, 1023, fp) == NULL)
	{
	  done = TRUE;
	  continue;
	}
      /* Skip spaces */
      c1 = buffer;
      while (*c1 == ' ' || *c1 == '\t') c1++;
      if (*c1 == '\0' || *c1 == '\n')
	continue; /* Nothing to do here */
      /* Ignore lines that begin with '#' */
      if (*c1 == '#')
	continue;

      /* Alloc memory for item */
      item = g_new0 (Action, 1);

      /* Read TAG */
      tok = strtok (c1, DELIM);
      if (tok == NULL)
	{
	  ShowErrMessage (_("Error parsing actions data base"));
	  free_actions_db (db);
	  return (-1);
	}
      strncpy (item->tag, tok, 49);
      item->tag[49] = '\0';

      /* Read log regexp . */
      tok = strtok (NULL, DELIM);
      if (tok != NULL)
	item->log_name = g_strdup (tok);
      else
	{
	  ShowErrMessage (_("Error parsing actions data base"));
	  free_actions_db (db);
	  return (-1);
	}

      /* Read process regexp . */
      tok = strtok (NULL, DELIM);
      if (tok != NULL)
	item->process = g_strdup (tok);
      else
	{
	  ShowErrMessage (_("Error parsing actions data base"));
	  free_actions_db (db);
	  return (-1);
	}

      /* Read message regexp . */
      tok = strtok (NULL, DELIM);
      if (tok != NULL)
	item->message = g_strdup (tok);
      else
	{
	  ShowErrMessage (_("Error parsing actions data base"));
	  free_actions_db (db);
	  return (-1);
	}

      /* Read description regexp . */
      tok = strtok (NULL, DELIM);
      if (tok != NULL)
	item->description = g_strdup (tok);
      else
	{
	  ShowErrMessage (_("Error parsing actions data base"));
	  free_actions_db (db);
	  return (-1);
	}

      /* Read action regexp . */
      tok = strtok (NULL, "\0\n");
      if (tok != NULL)
	item->action = g_strdup (tok);
      else
	{
	  ShowErrMessage (_("Error parsing actions data base"));
	  free_actions_db (db);
	  return (-1);
	}

      
      /* Add item to list */
      if (item != NULL)
	*db = g_list_append (*db, item);
    }

  *db = g_list_sort (*db, action_compare);

  return TRUE;
}


/* ----------------------------------------------------------------------
   NAME:        write_regexp_db
   DESCRIPTION: This file generates the actions DB file. It assumes that
                all the entries in each action are valid.
   ---------------------------------------------------------------------- */

int
write_actions_db (char *filename, GList *db)
{
  GList *item;
  Action *action;
  FILE *fp;

  
  fp = fopen (filename, "w");
  if (fp == NULL)
    {
      ShowErrMessage (_("Can't write to actions database!"));
      return FALSE;
    }
  
  /* Write a  header to the DB file */
  fprintf (fp, "#\n# Actions DB\n#\n# Generated by logview. Don't edit by hand\n");
  fprintf (fp, "# unless you know what you are doing!\n#\n\n");
  
  /* Search for daemon in our list */
  item = g_list_first (db);
  for (item = g_list_first (db); item != NULL; item=item->next)
    {
      action = (Action *)item->data;
      /* Check that there is a valid entry */
      if (*action->tag == '\0' || action->log_name == NULL ||
	   action->process == NULL || action->message == NULL ||
	   action->description == NULL ||
	   action->action == NULL)
	continue;

      /* write entry */
      fprintf (fp, "%s:%s:%s:%s:%s:%s\n", action->tag,
	       action->log_name, action->process, 
	       action->message, action->description,
	       action->action);
    }

  fclose (fp);

  return TRUE;
}

/* ----------------------------------------------------------------------
   NAME:        free_actions_db
   DESCRIPTION: 
   ---------------------------------------------------------------------- */

void
free_actions_db (GList **db)
{
	if (*db != NULL) {
		clear_actions_db (*db);
		g_list_free (*db);
		*db = NULL;
	}
}

/* ----------------------------------------------------------------------
   NAME:        exec_action_in_db
   DESCRIPTION: Try to find the error message in line in the database.
   ---------------------------------------------------------------------- */

int
exec_action_in_db (Log *log, LogLine *line, GList *db)
{
  GList *item;
  Action *cur_action = NULL;
  regex_t preg;
  regmatch_t matches[MAX_NUM_MATCHES];
  int doesnt_match;

  /* Search for daemon in our list */
  for (item = db; item != NULL; item = item->next)
    {
      doesnt_match = FALSE;
      cur_action = (Action *)item->data;
      regcomp (&preg, cur_action->log_name, REG_EXTENDED);
      doesnt_match = regexec (&preg, log->name, MAX_NUM_MATCHES, matches, 0);
      regfree (&preg);
      if (doesnt_match)
	continue;
      regcomp (&preg, cur_action->process, REG_EXTENDED);
      doesnt_match = regexec (&preg, line->process, MAX_NUM_MATCHES, matches, 0);
      regfree (&preg);
      if (doesnt_match)
	continue;
      regcomp (&preg, cur_action->message, REG_EXTENDED);
      doesnt_match = regexec (&preg, line->message, MAX_NUM_MATCHES, matches, 0);
      regfree (&preg);
      if (doesnt_match)
	continue;
      
      break;
    }

  if (item == NULL)
      return FALSE; /* not in our list */

  /* If there is a non-null action execute it */
  if (cur_action != NULL)
    system (cur_action->action);

  return TRUE;
}

/* ----------------------------------------------------------------------
   NAME:        add_actions_entry_cb
   DESCRIPTION: Add an entry to the actions DB.
   ---------------------------------------------------------------------- */

static void
add_actions_entry_cb (GtkWidget *widget, gpointer data)
{
  Action *action;

  action = g_new (Action, 1);
  
  g_snprintf (action->tag, sizeof (action->tag), _("<empty>"));
  action->log_name = g_strdup (_("log name regexp"));
  action->process = g_strdup (_("process regexp"));
  action->message = g_strdup (_("message regexp"));
  action->action = g_strdup (_("action to execute when regexps are TRUE"));
  action->description = g_strdup (_("description"));

  edit_action_entry (action);

}

static void
remove_actions_entry_cb (GtkWidget *widget, gpointer data)
{
  Action *action;

  if (selected_node == NULL)
	  return;

  action = (Action *) gtk_ctree_node_get_row_data (GTK_CTREE (ctree),
						   selected_node);

  if (action != NULL) {
	  local_actions_db = g_list_remove (local_actions_db, action);
	  gtk_clist_clear (GTK_CLIST (ctree));
	  selected_node = NULL;
	  make_tree_from_actions_db (local_actions_db);
  }
}

/* ----------------------------------------------------------------------
   NAME:        edit_actions_entry_cb
   DESCRIPTION: Function called when the edit button is pressed.
   ---------------------------------------------------------------------- */

static void
edit_actions_entry_cb (GtkWidget *widget, gpointer data)
{
  Action *action;

  if (selected_node == NULL)
	  return;

  action = (Action *) gtk_ctree_node_get_row_data (GTK_CTREE (ctree),
						   selected_node);

  edit_action_entry (action);
}

static Action *edited_action = NULL;

static void
apply_edit (GtkWidget *w, gpointer data)
{
	char *text;
	GtkWidget *action_record = data;
	GtkWidget *tag =
		gtk_object_get_data (GTK_OBJECT (action_record), "tag");
	GtkWidget *log_name =
		gtk_object_get_data (GTK_OBJECT (action_record), "log_name");
	GtkWidget *process =
		gtk_object_get_data (GTK_OBJECT (action_record), "process");
	GtkWidget *message =
		gtk_object_get_data (GTK_OBJECT (action_record), "message");
	GtkWidget *action =
		gtk_object_get_data (GTK_OBJECT (action_record), "action");
	GtkWidget *description =
		gtk_object_get_data (GTK_OBJECT (action_record), "description");

	text = gtk_editable_get_chars (GTK_EDITABLE (tag), 0, -1);
	strncpy (edited_action->tag, text, 50);
	edited_action->tag[49] = '\0';

	g_free (edited_action->log_name);
	edited_action->log_name =
		gtk_editable_get_chars (GTK_EDITABLE (log_name), 0, -1);
	g_free (edited_action->process);
	edited_action->process =
		gtk_editable_get_chars (GTK_EDITABLE (process), 0, -1);
	g_free (edited_action->message);
	edited_action->message =
		gtk_editable_get_chars (GTK_EDITABLE (message), 0, -1);
	g_free (edited_action->action);
	edited_action->action =
		gtk_editable_get_chars (GTK_EDITABLE (action), 0, -1);
	g_free (edited_action->description);
	edited_action->description =
		gtk_editable_get_chars (GTK_EDITABLE (description), 0, -1);

	if ( ! g_list_find (local_actions_db, edited_action))
		local_actions_db = g_list_prepend (local_actions_db,
						   edited_action);

	local_actions_db = g_list_sort (local_actions_db,
					action_compare);

	gtk_clist_clear (GTK_CLIST (ctree));
	selected_node = NULL;
	make_tree_from_actions_db (local_actions_db);
}

/* ----------------------------------------------------------------------
   NAME:        edit_action_entry
   DESCRIPTION: Edit a DB record on a window.
   ---------------------------------------------------------------------- */

static void
edit_action_entry (Action *action)
{
  GtkWidget *hbox;
  GtkWidget *label;
  GtkWidget *text;
  GtkWidget *entry;
  GtkWidget *action_record;
  GtkWidget *button;       
  GtkWidget *padding;       
  GtkBox *vbox;
  GtkTooltips *tips;

  if (!action)
    return;

  edited_action = action;
  
  /* Create main window ------------------------------------------------  */
  action_record = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_modal (GTK_WINDOW (action_record), TRUE);
  gtk_window_set_transient_for (GTK_WINDOW (action_record),
				GTK_WINDOW (actions_dialog));
  gtk_container_set_border_width (GTK_CONTAINER (action_record), 5);
  gtk_window_set_title (GTK_WINDOW (action_record), "Edit action record");
  gtk_widget_set_style (action_record, cfg->main_style);
  gtk_widget_set_usize (action_record, 400, 400);
  
  vbox = (GtkBox *)gtk_vbox_new (FALSE, 2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 4);
  gtk_container_add (GTK_CONTAINER (action_record), GTK_WIDGET (vbox));
  gtk_widget_show (GTK_WIDGET (vbox)); 
  
  /* Show fields to edit ----------------------------------------------- */
  tips = gtk_tooltips_new ();

  /* Tag */
  hbox = gtk_hbox_new (FALSE, 2);
  gtk_box_pack_start (vbox, hbox, TRUE, TRUE, 0);
  gtk_widget_show (hbox);
  label = gtk_label_new ("Tag:");
  gtk_widget_set_usize (label, 60, -1);
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
  gtk_widget_show (label);
  entry = gtk_entry_new ();
  gtk_widget_set_usize (entry, 200, -1);
  gtk_entry_set_editable (GTK_ENTRY (entry), TRUE);
  gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, TRUE, 0);
  gtk_entry_set_text (GTK_ENTRY (entry), action->tag);
  gtk_tooltips_set_tip (tips, entry, "Tag that identifies the log file.", NULL);
  gtk_widget_show (entry); 
  gtk_object_set_data (GTK_OBJECT (action_record), "tag", entry);

  /* log name */
  hbox = gtk_hbox_new (FALSE, 2);
  gtk_box_pack_start (vbox, hbox, TRUE, TRUE, 0);
  gtk_widget_show (hbox);
  label = gtk_label_new ("Log name:");
  gtk_widget_set_usize (label, 60, -1);
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
  gtk_widget_show (label);
  entry = gtk_entry_new ();
  gtk_widget_set_usize (entry, 200, -1);
  gtk_entry_set_editable (GTK_ENTRY (entry), TRUE);
  gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, TRUE, 0);
  gtk_entry_set_text (GTK_ENTRY (entry), action->log_name);
  gtk_tooltips_set_tip (tips, entry, "Regular expression that will match the log name.", 
			NULL);
  gtk_widget_show (entry); 
  gtk_object_set_data (GTK_OBJECT (action_record), "log_name", entry);
      
  /* Process */
  hbox = gtk_hbox_new (FALSE, 2);
  gtk_box_pack_start (vbox, hbox, TRUE, TRUE, 0);
  gtk_widget_show (hbox);
  label = gtk_label_new ("process:");
  gtk_widget_set_usize (label, 60, -1);
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
  gtk_widget_show (label);
  entry = gtk_entry_new ();
  gtk_widget_set_usize (entry, 200, -1);
  gtk_entry_set_editable (GTK_ENTRY (entry), TRUE);
  gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, TRUE, 0);
  gtk_entry_set_text (GTK_ENTRY (entry), action->process);
  gtk_tooltips_set_tip (tips, entry, "Regular expression that will match process part of message.", 
			NULL);
  gtk_widget_show (entry); 
  gtk_object_set_data (GTK_OBJECT (action_record), "process", entry);

  /* Message */
  hbox = gtk_hbox_new (FALSE, 2);
  gtk_box_pack_start (vbox, hbox, TRUE, TRUE, 0);
  gtk_widget_show (hbox);
  label = gtk_label_new ("message:");
  gtk_widget_set_usize (label, 60, -1);
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
  gtk_widget_show (label);
  entry = gtk_entry_new ();
  gtk_widget_set_usize (entry, 200, -1);
  gtk_entry_set_editable (GTK_ENTRY (entry), TRUE);
  gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, TRUE, 0);
  gtk_entry_set_text (GTK_ENTRY (entry), action->message);
  gtk_tooltips_set_tip (tips, entry, "Regular expression that will match the message.", 
			NULL);
  gtk_widget_show (entry); 
  gtk_object_set_data (GTK_OBJECT (action_record), "message", entry);

  /* Action */
  hbox = gtk_hbox_new (FALSE, 2);
  gtk_box_pack_start (vbox, hbox, TRUE, TRUE, 0);
  gtk_widget_show (hbox);
  label = gtk_label_new ("action:");
  gtk_widget_set_usize (label, 60, -1);
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
  gtk_widget_show (label);
  entry = gtk_entry_new ();
  gtk_widget_set_usize (entry, 200, -1);
  gtk_entry_set_editable (GTK_ENTRY (entry), TRUE);
  gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, TRUE, 0);
  gtk_entry_set_text (GTK_ENTRY (entry), action->action);
  gtk_tooltips_set_tip (tips, entry, "Action that will be executed if all previous regexps. are matched. This is executed by a system command: system (action).", 
			NULL);
  gtk_widget_show (entry); 
  gtk_object_set_data (GTK_OBJECT (action_record), "action", entry);

  /* Description */
  hbox = gtk_hbox_new (FALSE, 2);
  gtk_box_pack_start (vbox, hbox, TRUE, TRUE, 0);
  gtk_widget_show (hbox);
  label = gtk_label_new (_("description:"));
  gtk_widget_set_usize (label, 60, -1);
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
  gtk_widget_show (label);
  text = gtk_text_new (NULL, NULL);
  gtk_widget_set_usize (text, 200, -1);
  gtk_text_set_editable (GTK_TEXT (text), TRUE);
  gtk_box_pack_start (GTK_BOX (hbox), text, TRUE, TRUE, 0);
  gtk_tooltips_set_tip (tips, text, _("Description of this entry."), NULL);
  gtk_widget_show (text); 
  gtk_object_set_data (GTK_OBJECT (action_record), "description", text);


  /* Make bottom part ------------------------------------------------ */
  hbox = gtk_hbox_new (FALSE, 2);
  gtk_box_pack_start (vbox, hbox, TRUE, TRUE, 0);
  gtk_widget_show (hbox);

  /* Add padding to right justify */
  padding = gtk_label_new (" ");
  gtk_widget_show (padding);
  gtk_box_pack_start (GTK_BOX (hbox), padding, TRUE, TRUE, 0);

  button = gnome_stock_button (GNOME_STOCK_BUTTON_CANCEL);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, TRUE, 0);
  gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
			     GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     GTK_OBJECT (action_record));
  gtk_widget_show (button);

  button = gnome_stock_button (GNOME_STOCK_BUTTON_OK);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, TRUE, 0);
  gtk_widget_show (button);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (apply_edit),
		      action_record);
  gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
			     GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     GTK_OBJECT (action_record));


  gtk_widget_show (action_record);

  /* Insert text into text widget */
  gtk_text_insert (GTK_TEXT (text), NULL, NULL, NULL, action->description, 
		   strlen (action->description));
}

/* ----------------------------------------------------------------------
   NAME:        print_actions_db
   DESCRIPTION: Prints the database (for debbuging purposes).
   ---------------------------------------------------------------------- */

void
print_actions_db (GList *db)
{
  GList *item;
  Action *action;

  /* Search for daemon in our list */
  
  for (item = g_list_first (db); item != NULL; item = item->next)
    {
      action = (Action *)item->data;
      printf (_("tag: [%s]\nlog_name: [%s]\nprocess: [%s]\nmessage: [%s]\n"
		"description: [%s]\naction: [%s]\n"), 
	      action->tag,
	      action->log_name,
	      action->process,
	      action->message,
	      action->description,
	      action->action);
    }
      
  return;

}


/* ----------------------------------------------------------------------
   NAME:        clear_actions_db
   DESCRIPTION: Free all memory used by actions DB.
   ---------------------------------------------------------------------- */

void
clear_actions_db (GList *db)
{
  GList *item;
  Action *action;

  for (item = g_list_first (db); item != NULL; item = item->next)
    {
      action = (Action *)item->data;
      free_action (action);
      item->data = NULL;
    }
   
  return;

}

/* ----------------------------------------------------------------------
   NAME:        free_action
   DESCRIPTION: Given an action struct, clear it.
   ---------------------------------------------------------------------- */

void
free_action (Action *action)
{
	if (action) {
		g_free (action->log_name);
		g_free (action->process);
		g_free (action->message);

		g_free (action->action);
		g_free (action->description);

		g_free (action);
	}
}

static GList *
copy_actions_db (GList *db)
{
	GList *li;
	GList *copy;

	if (db == NULL)
		return NULL;

	copy = g_list_copy (db);
	for (li = copy; li != NULL; li = li->next) {
		Action *old = li->data;
		Action *new = g_new0 (Action, 1);

		/* these are the same size so strcpy is safe */
		strcpy (new->tag, old->tag);

		new->log_name = g_strdup (old->log_name);
		new->process = g_strdup (old->process);
		new->message = g_strdup (old->message);
		new->action = g_strdup (old->action);
		new->description = g_strdup (old->description);
		li->data = new;
	}

	return copy;
}
