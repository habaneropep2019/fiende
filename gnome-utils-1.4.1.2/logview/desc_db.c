#include <config.h>
#include <gnome.h>
#include "logview.h"
#include <string.h>
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>

#define MAX_NUM_MATCHES     10
#define DELIM               ":"

void free_database (GList **db);
void print_db (GList *db);
int match_line_in_db (LogLine *line, GList *db);
int read_regexp_db (char *filename, GList **db);

/* ----------------------------------------------------------------------
   NAME:        read_regexp_db
   DESCRIPTION: Reads the database with regular expressions to match.
   ---------------------------------------------------------------------- */

int
read_regexp_db (char *filename, GList **db)
{
  Description *item;
  ProcessDB *process_item;
  FILE *fp;
  char buffer[1024];
  char *c1, *tok;
  int done;

  /* Open regexp DB */
  fp = fopen (filename, "r");
  if (fp == NULL)
    {
      g_snprintf (buffer, sizeof (buffer),
		  _("Cannot open regexp data base <%s>! Open failed."), 
		  filename);
     ShowErrMessage (buffer);
     return(-1);
    }


  /* Start parsing file */
  done = FALSE;
  buffer[1023] = '\0';
  process_item = NULL;
  while (!done)
    {

      /* Read line */
      if (fgets( buffer, 1023, fp) == NULL)
	{
	  done = TRUE;
	  continue;
	}
      /* Ignore lines that begin with '#' */
      if (buffer[0] == '#')
	continue;

      /* Skip spaces */
      c1 = buffer;
      while (*c1 == ' ' || *c1 == '\t') c1++;
      if (*c1 == '\0' || *c1 == '\n')
	continue; /* Nothing to do here */

      /* Alloc memory for item */
      item = malloc (sizeof (Description));
      if (item == NULL)
	{
	  ShowErrMessage ("Error parsing regexp data base. Out of memory.");
	  exit (-1);
	}
      memset (item, 0, sizeof (Description));
      

      /* Read TAG */
      tok = strtok (c1, DELIM);
      if (tok == NULL)
	{
	  ShowErrMessage ("Error parsing regexp data base");
	  free_database (db);
	  return (-1);
	}
      strncpy (item->tag, tok, 49);
      item->tag[49] = '\0';

      /* Read process regexp. */
      tok = strtok (NULL, DELIM);
      if (tok[0] != '-') /* New process regexp */
	{
	  process_item = malloc (sizeof (ProcessDB));
	  if (process_item == NULL)
	    {
	      ShowErrMessage ("Error parsing regexp data base. \
                               Out of memory!");
	      exit (-1);
	    }
	  process_item->matching = NULL;
	  process_item->regexp = g_strdup (tok);
	  *db = g_list_append (*db, process_item);
	}

      /* Read regexp for this line */
      tok = strtok (NULL, DELIM);
      if (tok == NULL)
	{
	  ShowErrMessage ("Error parsing regexp data base");
	  free_database (db);
	  return (-1);
	}
      item->regexp = g_strdup (tok);
      
      /* Read level */
      item->level = 0;
      tok = strtok (NULL, "\n\0");
      if (tok != NULL)
	item->level = atoi(tok);

      /* Add item to list */
      if (process_item != NULL)
	process_item->matching = g_list_append (process_item->matching, item);
    }

  return TRUE;
}

/* ----------------------------------------------------------------------
   NAME:        read_descript_db
   DESCRIPTION: Reads the database with descriptions match.
   ---------------------------------------------------------------------- */

int
read_descript_db (char *filename, GList **db)
{
  DescriptionEntry *item;
  FILE *fp;
  char buffer[1024];
  char *c1, *tok;
  int done;

  /* Open description DB */
  fp = fopen (filename, "r");
  if (fp == NULL)
    {
      g_snprintf (buffer, sizeof (buffer),
		  _("Cannot open description data base <%s>! Open failed."), 
		  filename);
      ShowErrMessage (buffer);
      return(-1);
    }


  /* Start parsing file */
  done = FALSE;
  buffer[1023] = '\0';
  item = NULL;
  while (!done)
    {
      /* Read line */
      if (fgets( buffer, 1023, fp) == NULL)
	{
	  done = TRUE;
	  continue;
	}
      /* Ignore lines that begin with '#' */
      if (buffer[0] == '#')
	continue;

      /* Skip spaces */
      c1 = buffer;
      while (*c1 == ' ' || *c1 == '\t') c1++;
      if (*c1 == '\0' || *c1 == '\n')
	continue; /* Nothing to do here */

      /* Alloc memory for item */
      item = malloc (sizeof (Description));
      if (item == NULL)
	{
	  ShowErrMessage ("Error parsing description data base. Out of memory.");
	  exit (-1);
	}
      memset (item, 0, sizeof (Description));
      

      /* Read TAG */
      tok = strtok (c1, DELIM);
      if (tok == NULL)
	{
	  ShowErrMessage ("Error parsing description data base");
	  free_database (db);
	  return (-1);
	}
      strncpy (item->tag, tok, 49);
      item->tag[49] = '\0';

      /* Read description text. */
      tok = strtok (NULL, "\n\0");
      if (tok != NULL)
	item->text = g_strdup (tok);
      else
	{
	  ShowErrMessage ("Error parsing description data base");
	  return (-1);
	}
      
      /* Add item to list */
      if (item != NULL)
	*db = g_list_append (*db, item);
    }

  return TRUE;
}


/* ----------------------------------------------------------------------
   NAME:        free_database
   DESCRIPTION: 
   ---------------------------------------------------------------------- */

void
free_database (GList **db)
{
}

/* ----------------------------------------------------------------------
   NAME:        match_line_in_db
   DESCRIPTION: Try to find the error message in line in the database.
   ---------------------------------------------------------------------- */

int
match_line_in_db (LogLine *line, GList *db)
{
  GList *process, *item;
  ProcessDB *cur_proc = NULL;
  Description *cur_desc = NULL;
  regex_t preg;
  regmatch_t matches[MAX_NUM_MATCHES];

  /* Lets assume it doesn't appear */
  line->description = NULL;

  /* Search for daemon in our list */
  process = g_list_first (db);
  while (process != NULL)
    {
      cur_proc = (ProcessDB *)process->data;
      regcomp (&preg, cur_proc->regexp, REG_EXTENDED);
      if (regexec (&preg, line->process, MAX_NUM_MATCHES, matches, 0) == 0)
	break;
      regfree (&preg);
      process = process->next;
    }

  if (process == NULL || cur_proc == NULL || cur_proc->matching == NULL)
      return FALSE; /* not in our list */

  /* start search */
  item = g_list_first (cur_proc->matching);
  while (item != NULL)
    {
      cur_desc = item->data;
      regcomp (&preg, cur_desc->regexp, REG_EXTENDED);
      if (regexec (&preg, line->message, MAX_NUM_MATCHES, matches, 0)==0)
	break;
      regfree (&preg);
      item = item->next;
    }
  
  if (item == NULL)
    return FALSE;

  line->description = cur_desc;

  return TRUE;
}

/* ----------------------------------------------------------------------
   NAME:        find_tag
   DESCRIPTION: Find tag in database and fill description.
   ---------------------------------------------------------------------- */

int
find_tag_in_db (LogLine *line, GList *db)
{
  GList *item;
  DescriptionEntry *cur_desc = NULL;

  if (line->description == NULL)
    return FALSE;

  /* Check if entry is already updated */
  if (line->description->description != NULL)
    return TRUE;

  /* Search for tag in our list */
  item = g_list_first (db);
  while (item != NULL)
    {
      cur_desc = (DescriptionEntry *)item->data;
      if (strcmp (cur_desc->tag, line->description->tag) == 0)
	break;
      item = item->next;
    }

  if (item == NULL )
      return FALSE; /* not in our list */

  line->description->description = cur_desc->text;

  return TRUE;
}


/* ----------------------------------------------------------------------
   NAME:        print_db
   DESCRIPTION: Prints the database (for debbuging purposes).
   ---------------------------------------------------------------------- */

void
print_db (GList *db)
{
  GList *process, *item;
  ProcessDB *cur_proc;
  Description *cur_desc;

  /* Search for daemon in our list */
  process = g_list_first (db);
  while (process != NULL)
    {
      cur_proc = (ProcessDB *)process->data;
      printf ("%s\n", cur_proc->regexp);
      item = g_list_first (cur_proc->matching);
      while (item != NULL)
	{
	  cur_desc = item->data;
	  if (cur_desc == NULL)
	    printf ("somethings wrong!\n");
	  else
	    printf ("    [%.50s] [%s] [%d]\n", cur_desc->tag,
		    cur_desc->regexp, cur_desc->level);
	  item = item->next;
	}
      process = process->next; 
    }
      

}


/* ----------------------------------------------------------------------
   NAME:        
   DESCRIPTION: 
   ---------------------------------------------------------------------- */

