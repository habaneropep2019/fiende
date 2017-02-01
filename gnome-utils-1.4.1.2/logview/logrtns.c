
/*  ----------------------------------------------------------------------

    Copyright (C) 1998  Cesar Miquel  (miquel@df.uba.ar)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    ---------------------------------------------------------------------- */

#include <config.h>
#include <gnome.h>
#include <locale.h>
#include "logview.h"
#include "logrtns.h"

/*
 * -----------------
 * Local definitions
 * ------------------
 */

#define LOG_COL1                 7
#define LOG_COL2                 70
#define LOG_COL3                 170
#define LOG_COL4                 200
#define LOG_COL5                 300
#define LOG_COL6                 340
#define LOG_COL7                 400
#define LOG_TITLEY               10

#define MAX_NUM_LOGS             10
#define MAX_NUM_DPY_LINES        300
#define MAX_DPY_NUM_CHARS        200
#define DEF_NUM_LOGS             3

#define FRMT_HEADING             1
#define FRMT_NORMAL              2


/*
 * -------------------
 * Function prototypes 
 * -------------------
 */

int IsLeapYear (int);
int match_line_in_db (LogLine *line, GList *db);
void UpdateLastPage (Log *log);

extern GList *regexp_db;
extern GList *actions_db;

/*
 * -------------------
 * Module variables 
 * -------------------
 */

const char *
C_monthname[12] =
{ "January",
  "February",
  "March",
  "April",
  "May",
  "June",
  "July",
  "August",
  "September",
  "October",
  "November",
  "December" };

/* space separated list of locales to check, must be space separated, no
 * newlines */
/* FIXME: This should be checked for at configure time or something to
 * that effect */
char *all_locales = "af_ZA ar_SA bg_BG ca_ES cs_CZ da_DK de_AT de_BE de_CH de_DE de_LU el_GR en_AU en_CA en_DK en_GB en_IE en_US es_ES et_EE eu_ES fi_FI fo_FO fr_BE fr_CA fr_CH fr_FR fr_LU ga_IE gl_ES gv_GB hr_HR hu_HU in_ID is_IS it_CH it_IT iw_IL kl_GL kw_GB lt_LT lv_LV mk_MK nl_BE nl_NL no_NO pl_PL pt_BR pt_PT ro_RO ru_RU.KOI8-R ru_RU ru_UA sk_SK sl_SI sr_YU sv_FI sv_SE th_TH tr_TR uk_UA";

static int
get_month (const char *str)
{
	int i, j;
	static char *monthname[12] = { 0 };
	static GHashTable *locales_monthnames = NULL;
	static char **locales = NULL;

	for (i = 0; i < 12; i++) {
		if (g_strncasecmp (str, C_monthname[i], 3) == 0) {
			return i;
		}
	}

	for (i = 0; i < 12; i++) {
		if (monthname[i] == NULL) {
			GDate *date;
			char buf[256];

			date = g_date_new_dmy (1, i+1, 2000 /* bogus */);

			if (g_date_strftime (buf, sizeof (buf), "%b", date) <= 0) {
				/* eek, just use C locale cuz we're screwed */
				monthname[i] = g_strndup (C_monthname[i], 3);
			} else {
				monthname[i] = g_strdup (buf);
			}

			g_date_free (date);
		}

		if (g_strcasecmp (str, monthname[i]) == 0) {
			return i;
		}
	}

	if (locales == NULL)
		locales = g_strsplit (all_locales, " ", 0);

	if (locales_monthnames == NULL)
		locales_monthnames = g_hash_table_new (g_str_hash, g_str_equal);

	/* Try all known locales */
	for (j = 0; locales != NULL && locales[j] != NULL; j++) {
		for (i = 0; i < 12; i++) {
			char *key = g_strdup_printf ("%s %d", locales[j], i);
			char *name = g_hash_table_lookup (locales_monthnames, key);
			if (name == NULL) {
				char buf[256];
				char *old_locale = g_strdup (setlocale (LC_TIME, NULL));
				
				if (setlocale (LC_TIME, locales[j]) == NULL) {
					strcpy (buf, "");
				} else {
					GDate *date;

					date = g_date_new_dmy (1, i+1, 2000 /* bogus */);

					if (g_date_strftime (buf, sizeof (buf), "%b", date) <= 0) {
						strcpy (buf, "");
					}

					if (old_locale != NULL) {
						setlocale (LC_TIME, old_locale);
						g_free (old_locale);
					}
				}

				name = g_strdup (buf);
				g_hash_table_insert (locales_monthnames, g_strdup (key), name);
			}
			g_free (key);

			if (name != NULL &&
			    name[0] != '\0' &&
			    g_strcasecmp (str, name) == 0) {
				return i;
			}
		}
	}

	return 12;
}



/* ----------------------------------------------------------------------
   NAME:          OpenLogFile
   DESCRIPTION:   Open a log file and read several pages.
   ---------------------------------------------------------------------- */

Log *
OpenLogFile (char *filename)
{
   Page *prev, *first, *cur;
   Log *tlog;
   int j;

   /* Check that the file exists and is readable and is a
      logfile */
   if (!isLogFile (filename))
      return (Log *) NULL;

   /* Alloc memory for  log structures -------------------- */
   tlog = malloc (sizeof (Log));
   if (tlog == NULL)
   {
      ShowErrMessage (_("Not enough memory!\n"));
      return ((Log *) NULL);
   }
   memset (tlog, 0, sizeof (Log));
   /* Alloc memory for pages ------------------------------ */
   tlog->currentpg = (Page *) malloc (sizeof (Page));
   if (tlog->currentpg == NULL)
   {
      ShowErrMessage (_("Out of memory!\n"));
      free (tlog);
      return (Log *) NULL;
   }
   prev = NULL;
   cur = tlog->currentpg;
   tlog->firstpg = first = cur;
   for (j = 1; j < NUM_PAGES; j++)
   {
      cur->next = (Page *) malloc (sizeof (Page));
      cur->prev = prev;
      prev = cur;
      cur = cur->next;
   }

   tlog->lastpg = cur;
   cur->prev = prev;
   cur->next = first;
   first->prev = cur;

   /* Open log files ------------------------------------- */
   strncpy (tlog->name, filename, sizeof (tlog->name)-1);
   tlog->name[sizeof (tlog->name) - 1] = '\0';
   tlog->fp = fopen (filename, "r");
   if (tlog->fp == NULL)
   {
      ShowErrMessage (_("Unable to open logfile!\n"));
      return (Log *) NULL;
   }
   fseek (tlog->fp, -1L, SEEK_END);

   /* Read pages into buffers --------------------------- */
   cur = tlog->lastpg;
   for (j = 0; j < NUM_PAGES; j++)
   {
      if (ReadPageUp (tlog, cur) == TRUE)
      {
	 tlog->firstpg = cur;
	 break;
      }
      cur = cur->prev;
   }
   tlog->lastpg->ll = LINES_P_PAGE-1;
   tlog->lastpg->islastpage = TRUE;
   tlog->currentpg = tlog->lastpg;
   tlog->firstline = tlog->lastpg->fl;
   tlog->pointerln = tlog->firstline;
   tlog->pointerpg = tlog->currentpg;
   tlog->mon_on = FALSE;

   /* Read log stats */
   ReadLogStats (tlog);

   /* Move end of file marker to last char on file */
   fseek(tlog->fp, -1L, SEEK_END);
   tlog->offset_end = ftell(tlog->fp);
   tlog->ln = tlog->lstats.numlines - LINES_P_PAGE;

   return tlog;
}

/* ----------------------------------------------------------------------
   NAME:          isLogFile
   DESCRIPTION:   Check that the given file is indeed a logfile and 
   that it is readable.
   ---------------------------------------------------------------------- */
int
isLogFile (char *filename)
{
   struct stat fstatus;
   char buff[1024];
   char *token;
   int i;
   FILE *fp;

   stat (filename, &fstatus);

   /* Check that its a regular file       */
   if (!S_ISREG (fstatus.st_mode))
   {
      g_snprintf (buff, sizeof (buff),
		  _("%s is not a regular file."), filename);
      ShowErrMessage (buff);
      return FALSE;
   }
   /* File unreadable                     */
   if ((fstatus.st_mode & S_IRUSR) == 0)
   {
      g_snprintf (buff, sizeof (buff),
		  _("%s is not user readable. "
      "Either run the program as root or ask the sysadmin to "
      "change the permissions on the file."), filename);
      ShowErrMessage (buff);
      return FALSE;
   }
   /* Read first line and check that it has the format
      of a log file: Date ...
    */
   fp = fopen (filename, "r");
   if (fp == NULL)
   {
      g_snprintf (buff, sizeof (buff),
		  _("%s could not be opened."), filename);
      ShowErrMessage (buff);
      return FALSE;
   }
   fgets (buff, sizeof (buff), fp);
   fclose (fp);
   token = strtok (buff, " ");
   /* This is not a good assumption I don't think, especially
    * if log is internationalized
    * -George
   if (strlen (buff) != 3)
   {
      g_snprintf (buff, sizeof (buff),
		  _("%s not a log file."), filename);
      ShowErrMessage (buff);
      return FALSE;
   }
   */

   i = get_month (token);

   if (i == 12)
   {
      g_snprintf (buff, sizeof (buff), _("%s not a log file."), filename);
      ShowErrMessage (buff);
      return FALSE;
   }
   /* It looks like a log file... */
   return TRUE;
}

/* ----------------------------------------------------------------------
   NAME:        ReadNPagesUp
   DESCRIPTION: 
   ---------------------------------------------------------------------- */

int
ReadNPagesUp (Log * lg, Page * pg, int n)
{
   Page *cp;
   int i;

   /* Go to place on file. */
   fseek (lg->fp, pg->firstchpos, SEEK_SET);

   cp = pg->prev;
   /* Read n pages up.             */
   for (i = 0; i < n; i++)
   {
      ReadPageUp (lg, cp);
      lg->firstpg = cp;
      lg->lastpg = lg->lastpg->prev;
      if (cp->isfirstpage == TRUE)
	 return i+1;
      cp = cp->prev;
   }

   return n;
}

/* ----------------------------------------------------------------------
   NAME:        ReadNPagesDown
   DESCRIPTION: 
   ---------------------------------------------------------------------- */

int
ReadNPagesDown (Log * lg, Page * pg, int n)
{
   Page *cp;
   int i;

   /* Go to place on file. */
   fseek (lg->fp, pg->lastchpos, SEEK_SET);

   cp = pg->next;
   /* Read n pages down.   */
   for (i = 0; i < n; i++)
   {
      ReadPageDown (lg, cp, FALSE /* exec_actions */);
      lg->lastpg = cp;
      lg->firstpg = lg->firstpg->next;
      if (cp->islastpage == TRUE)
	 return i+1;
      cp = cp->next;
   }

   return n;
}

/* ----------------------------------------------------------------------
   NAME:        ReadPageUp
   DESCRIPTION: Reads a page from the log file.
   ---------------------------------------------------------------------- */

int
ReadPageUp (Log * lg, Page * pg)
{
   LogLine *line;
   FILE *fp;
   char *c, ch, buffer[R_BUF_SIZE + 1];
   int ln;
   long int old_pos;

   fp = lg->fp;
   ln = LINES_P_PAGE - 1;

   pg->lastchpos = ftell (fp);
   pg->islastpage = FALSE;
   pg->isfirstpage = FALSE;
   pg->fl = 0;
   pg->ll = LINES_P_PAGE - 1;

   /* Tell if we are reading the last page */
   ch = fgetc (fp);
   if (ch == EOF)
     pg->islastpage = TRUE;
   ungetc (ch,fp);

   while (ln >= pg->fl)
   {
      c = buffer;
      if (fseek (fp, -1L, SEEK_CUR) < 0)
      {
	 pg->fl = ln;
	 pg->isfirstpage = TRUE;
	 break;
      }
      /* Go to end of previous line */
      ch = fgetc (fp);
      while (ch != '\n')
      {
	 if (fseek (fp, -2L, SEEK_CUR) < 0)
	 {
	    /*   ch = fgetc (fp); */
	    pg->isfirstpage = TRUE;
	    pg->fl = ln;
	    break;
	 }
	 ch = fgetc (fp);
      }
      ungetc (ch, fp);

      /* Read the line now. */
      old_pos = ftell (fp);
      if (pg->isfirstpage == FALSE)
	 fseek (fp, 1L, SEEK_CUR);
      fgets (buffer, R_BUF_SIZE, fp);

      /* Put cursor back where it was. */
      fseek (fp, old_pos, SEEK_SET);

      line = &pg->line[ln];
      ParseLine (buffer, line);
      ln--;
   }
   pg->firstchpos = ftell (fp);

   return pg->isfirstpage;
}

/* ----------------------------------------------------------------------
   NAME:        ReadPageDown
   DESCRIPTION: Reads a page from the log file.
   ---------------------------------------------------------------------- */

int
ReadPageDown (Log * lg, Page * pg, gboolean exec_actions)
{
   FILE *fp;
   LogLine *line;
   char *c, ch, buffer[R_BUF_SIZE + 1];
   int ln, len;

   g_return_val_if_fail (lg != NULL, FALSE);

   fp = lg->fp;
   ln = 0;

   pg->firstchpos = MAX (0, ftell (fp)-1);
   pg->islastpage = FALSE;
   pg->isfirstpage = FALSE;
   pg->fl = 0;
   pg->ll = 0;

   /* Check if its the first page */
   if ( ftell(fp) == 0)
     pg->isfirstpage = TRUE;

   while (ln < LINES_P_PAGE)
   {
      c = buffer;
      ch = fgetc (fp);
      if (ch == '\n')
	 ch = fgetc (fp);
      if (ch == EOF)
      {
	 if (ln == 0)
	    pg->prev->islastpage = TRUE;
	 else
	   pg->islastpage = TRUE;
	 ungetc (ch, fp);
	 break;
      }
      len = 0;
      while (ch != '\n')
      {
	 if (len < R_BUF_SIZE)
	 {
	    *c = ch;
	    c++;
	    len++;
	 }
	 ch = fgetc (fp);
	 if (ch == EOF)
	 {
	    pg->islastpage = TRUE;
	    ungetc (ch, fp);
	    break;
	 }
      }
      ungetc (ch, fp);
      *c = '\0';

      line = &pg->line[ln];
      ParseLine (buffer, line);

      if (exec_actions)
	      exec_action_in_db (lg, line, actions_db);

      ln++;
   }
   pg->ll = ln-1;
   pg->lastchpos = ftell (fp);

   return TRUE;
}


/* ----------------------------------------------------------------------
   NAME:        ParseLine
   DESCRIPTION: Extract date and other info from the line. If any field 
   seems to be missing fill the others with -1 and NULL.
   ---------------------------------------------------------------------- */

void
ParseLine (char *buff, LogLine * line)
{
   char *token;
   char scratch[1024];
   int i;

   token = strtok (buff, " ");
   if (token == NULL) return;
   /* This is not a good assumption I don't think, especially
    * if log is internationalized
    * -George
   if (strlen (token) != 3)
   {
      strncpy (line->message, buff, MAX_WIDTH);
      line->month = -1;
      line->date = -1;
      line->hour = -1;
      line->min = -1;
      line->sec = -1;
      strcpy (line->hostname, "");
      strcpy (line->process, "");
      return;
   }
   */
   i = get_month (token);

   if (i == 12)
   {
      strncpy (line->message, buff, MAX_WIDTH);
      line->message[MAX_WIDTH-1] = '\0';
      line->month = -1;
      line->date = -1;
      line->hour = -1;
      line->min = -1;
      line->sec = -1;
      strcpy (line->hostname, "");
      strcpy (line->process, "");
      return;
   }
   line->month = i;

   token = strtok (NULL, " ");
   if (token != NULL)
      line->date = (char) atoi (token);
   else
   {
      line->date = -1;
      line->hour = -1;
      line->min = -1;
      line->sec = -1;
      strcpy (line->hostname, "");
      strcpy (line->process, "");
      strcpy (line->message, "");
      return;
   }

   token = strtok (NULL, ":");
   if (token != NULL)
      line->hour = (char) atoi (token);
   else
   {
      line->hour = -1;
      line->min = -1;
      line->sec = -1;
      strcpy (line->hostname, "");
      strcpy (line->process, "");
      strcpy (line->message, "");
      return;
   }

   token = strtok (NULL, ":");
   if (token != NULL)
      line->min = (char) atoi (token);
   else
   {
      line->min = -1;
      line->sec = -1;
      strcpy (line->hostname, "");
      strcpy (line->process, "-");
      strcpy (line->message, "");
      return;
   }

   token = strtok (NULL, " ");
   if (token != NULL)
      line->sec = (char) atoi (token);
   else
   {
      line->sec = -1;
      strcpy (line->hostname, "");
      strcpy (line->process, "-");
      strcpy (line->message, "");
      return;
   }

   token = strtok (NULL, " ");
   if (token != NULL) {
      strncpy (line->hostname, token, MAX_HOSTNAME_WIDTH);
      line->hostname[MAX_HOSTNAME_WIDTH-1] = '\0';
   } else {
      line->sec = -1;
      strcpy (line->hostname, "");
      strcpy (line->process, "-");
      strcpy (line->message, "");
      return;
   }

   token = strtok (NULL, ":\n");
   if (token != NULL)
      strncpy (scratch, token, 254);
   else
      strncpy (scratch, " ", 254);
   scratch[254] = '\0';
   token = strtok (NULL, "\n");

   if (token == NULL)
   {
      strncpy (line->process, "", MAX_PROC_WIDTH);
      i = 0;
      while (scratch[i] == ' ')
	 i++;
      strncpy (line->message, &scratch[i], MAX_WIDTH);
      line->message [MAX_WIDTH-1] = '\0';
   } else
   {
      strncpy (line->process, scratch, MAX_PROC_WIDTH);
      line->process [MAX_PROC_WIDTH-1] = '\0';
      while (*token == ' ')
	 token++;
      strncpy (line->message, token, MAX_WIDTH);
      line->message [MAX_WIDTH-1] = '\0';
   }

   /* Search current data base for string and attach descritpion */
   /* -------------------------
   if (regexp_db == NULL)
     return;
   if (match_line_in_db (line, regexp_db))
     {
       sprintf (scratch, _("Expression /%s/ matched."),
		 line->description->regexp);
       line->description->description = g_strdup(scratch);
     }
     ---------------------------- */
}

/* ----------------------------------------------------------------------
   NAME:          ReadLogStats
   DESCRIPTION:   Read the log and get some statistics from it. Read
   all dates which have a log entry to create calendar.
   All dates are given with respect to the 1/1/1970
   and are then corrected to the correct year once we
   reach the end.
   ---------------------------------------------------------------------- */

void
ReadLogStats (Log * log)
{
   FILE *fp;
   char buffer[256];
   long filemark;
   int offsetyear, lastyear, thisyear;
   int nl;
   time_t curdate, newdate, correction;
   DateMark *curmark;
   struct stat filestat;
   struct tm *tmptm, footm;

   /* Clear struct.      */
   log->lstats.startdate = 0;
   log->lstats.enddate = 0;
   log->lstats.firstmark = NULL;
   log->lstats.mtime = 0;
   log->lstats.size = 0;

   fp = log->fp;
   if (fp == NULL)
      return;

   rewind (fp);
   stat (log->name, &filestat);
   log->lstats.mtime = filestat.st_mtime;
   log->lstats.size = filestat.st_size;

   /* Make sure we have at least a starting date. */
   nl = 0;
   curdate = -1;
   while (curdate < 0)
   {
      filemark = ftell (fp);
      if (fgets (buffer, 255, fp) == NULL)
	 return;
      curdate = GetDate (buffer);
   }
   log->lstats.startdate = curdate;
   nl++;

   /* Start building the list */
   offsetyear = 0;
   curmark = malloc (sizeof (DateMark));
   if (curmark == NULL)
   {
      ShowErrMessage (_("ReadLogStats: out of memory"));
      exit (0);
   }
   log->lstats.firstmark = curmark;
   curmark->time = curdate;
   curmark->year = offsetyear;
   curmark->next = NULL;
   curmark->prev = NULL;
   curmark->offset = filemark;
   curmark->ln = nl;

   /* Count lines up to end */
   filemark = ftell (fp);
   while (fgets (buffer, 255, fp) != NULL)
   {
     nl++;
     newdate = GetDate (buffer);
     if (newdate < 0)
       {
	 /*  lines continues */
	 nl--;
	 continue;
       }
     if (isSameDay (newdate, curdate) )
       {
	 filemark = ftell (fp);
	 continue;
       }
     curmark->next = malloc (sizeof (DateMark));
     if (curmark->next == NULL)
       {
	 ShowErrMessage (_("ReadLogStats: out of memory"));
	 exit (0);
       }
     curmark->next->prev = curmark;
     curmark = curmark->next;
     curmark->time = newdate;
     if (newdate < curdate)	/* The newdate is next year */
       offsetyear++;
     curmark->year = offsetyear;
     curmark->next = NULL;
     curmark->offset = filemark;
     curmark->ln = nl;
     curdate = newdate;
     filemark = ftell (fp);
   }
   
   log->lstats.enddate = curdate;

   /* Correct years now. We assume that the last date on the log
      is the date last accessed */
   curdate = filestat.st_mtime;
   tmptm = localtime (&curdate);
   thisyear = tmptm->tm_year;
   lastyear = offsetyear;
   footm.tm_sec = footm.tm_mon = 0;
   footm.tm_hour = footm.tm_min = 0;
   footm.tm_isdst = 0;
   footm.tm_mday = 1;
   curmark = log->lstats.firstmark;
   while (curmark != NULL)
   {
      footm.tm_year = (curmark->year - lastyear) + thisyear;
      correction = mktime (&footm);
/*       fprintf (stderr, "Antes de corregir: [%s]\n", ctime (&curmark->time)); */
      if (IsLeapYear (curmark->year - lastyear + thisyear))
	 curmark->time += 24 * 60 * 60;		/*  Add one day */

#if defined(__NetBSD__) || defined(__FreeBSD__)
      curmark->time += correction - tmptm->tm_gmtoff;
#else
      curmark->time += correction - timezone;
#endif

      memcpy (&curmark->fulldate, localtime (&curmark->time), sizeof (struct tm));

/*       fprintf (stderr, "%s\n", ctime (&curmark->time)); */
      if (curmark->next != NULL)
	 curmark = curmark->next;
      else
      {
	 log->lstats.lastmark = curmark;
	 break;
      }
   }

   log->curmark = log->lstats.lastmark;

   /* Correct start and end year in lstats. */
   footm.tm_year = thisyear;
   correction = mktime (&footm);
   if (IsLeapYear (thisyear))
      log->lstats.enddate += 24 * 60 * 60;	/*  Add one day */

#if defined(__NetBSD__) || defined(__FreeBSD__)
   log->lstats.enddate += correction - tmptm->tm_gmtoff;
#else
   log->lstats.enddate += correction - timezone;
#endif

   footm.tm_year = thisyear - lastyear;
   correction = mktime (&footm);
   if (IsLeapYear (thisyear - lastyear))
      log->lstats.startdate += 24 * 60 * 60;	/*  Add one day */

#if defined(__NetBSD__) || defined(__FreeBSD__)
   log->lstats.startdate += correction - tmptm->tm_gmtoff;
#else
   log->lstats.startdate += correction - timezone;
#endif
   log->lstats.numlines = nl;

   return;
}

/* ----------------------------------------------------------------------
   NAME:	UpdateLogStats
   DESCRIPTION:	
   ---------------------------------------------------------------------- */

void
UpdateLogStats( Log *log )
{
   FILE *fp;
   char buffer[256];
   long filemark;
   int thisyear, nl;
   time_t curdate, newdate;
   DateMark *curmark;
   struct tm *tmptm;

   if (log == NULL)
     return;

   fp = log->fp;
   if (fp == NULL)
      return;

   curmark = log->lstats.lastmark;
   curdate = log->lstats.enddate;
   tmptm = localtime (&curdate);
   thisyear = tmptm->tm_year;
   fseek( fp, curmark->offset, SEEK_SET);

   /* Read last line again and discard it */
   fgets (buffer, 255, fp);

   /* Add all new entries to the list */
   filemark = ftell (fp);
   nl=0;
   while (fgets (buffer, 255, fp) != NULL)
   {
     nl++;
     newdate = GetDate (buffer);
     if (newdate < 0 || isSameDay (newdate, curdate) )
       {
	 filemark = ftell (fp);
	 continue;
       }
     curmark->next = malloc (sizeof (DateMark));
     if (curmark->next == NULL)
       {
	 ShowErrMessage (_("ReadLogStats: out of memory"));
	 exit (0);
       }
     curmark->next->prev = curmark;
     curmark = curmark->next;
     curmark->time = newdate;
     if (newdate < curdate)	/* The newdate is next year */
       thisyear++;
     curmark->year = thisyear;
     curmark->next = NULL;
     curmark->offset = filemark;
     curdate = newdate;
     filemark = ftell (fp);
   }
   
   log->lstats.enddate = curdate;
   log->lstats.lastmark = curmark;
   log->lstats.numlines += nl;

   return;
}



/* ----------------------------------------------------------------------
   NAME:          MoveToMark
   DESCRIPTION:   From the current mark read NUM_PAGES/2 pages ahead and
                  NUM_PAGES/2 pages behind (if possible).
   ---------------------------------------------------------------------- */

void
MoveToMark (Log *log)
{
  DateMark *mark;
  FILE *fp;
  Page *middlepg;
  int pagesread, pagesdown, i;
  
  g_return_if_fail (log);
  
  mark = log->curmark;
  g_return_if_fail (mark);
  fp = log->fp;
  
  pagesdown = NUM_PAGES>>1;
  
  /* Read relative to middle page */
  middlepg = log->lastpg;
  for(i=0;i<pagesdown;i++)
    middlepg = middlepg->prev;
  
  /* Move file pointer to the current offset 
     into the file */
  if (mark->offset > 1)
    fseek(fp, mark->offset-1, SEEK_SET);
  else
    fseek(fp, mark->offset, SEEK_SET);
  ReadPageDown(log, middlepg, FALSE /* exec_actions */);
  if (middlepg->islastpage)
    {
      middlepg = log->lastpg;
      if (mark->offset > 1)
	fseek(fp, mark->offset-1, SEEK_SET);
      else
	fseek(fp, mark->offset, SEEK_SET);
      ReadPageDown(log, middlepg, FALSE /* exec_actions */);
      ReadNPagesUp(log, middlepg, NUM_PAGES-1);
      log->currentpg = middlepg;
      return;
    }
  
  if (middlepg->isfirstpage)
    {
      middlepg = log->firstpg;
      fseek(fp, mark->offset, SEEK_SET);
      ReadPageDown(log, middlepg, FALSE /* exec_actions */);
      ReadNPagesDown(log, middlepg, NUM_PAGES-1);
      log->currentpg = middlepg;
      return;
    }
  
  pagesread = ReadNPagesDown (log, middlepg, NUM_PAGES-1);
  ReadNPagesUp (log, middlepg, MAX (pagesdown, (NUM_PAGES-1-pagesread)));

  log->currentpg = middlepg;
   
}



/* ----------------------------------------------------------------------
   NAME:	UpdateLastPage
   DESCRIPTION:	Re-read the last log page.
   ---------------------------------------------------------------------- */

void
UpdateLastPage (Log *log)
{

  /* Check that the last page is in the list. */
  if (!log->lastpg->islastpage)
    return;


}


/* ----------------------------------------------------------------------
   NAME:          isSameDay
   DESCRIPTION:   Determine if the given times are the same.
   ---------------------------------------------------------------------- */

int
isSameDay (time_t day1, time_t day2)
{
   struct tm d1, d2, *foo;

   foo = localtime (&day1);
   memcpy (&d1, foo, sizeof (struct tm));

   foo = localtime (&day2);
   memcpy (&d2, foo, sizeof (struct tm));

   if (d1.tm_year != d2.tm_year)
      return FALSE;
   if (d1.tm_mon != d2.tm_mon)
      return FALSE;
   if (d1.tm_mday != d2.tm_mday)
      return FALSE;

   return TRUE;
}


/* ----------------------------------------------------------------------
   NAME:          GetDate
   DESCRIPTION:   Extract the date from the log line.
   ---------------------------------------------------------------------- */

time_t
GetDate (char *line)
{
   struct tm date;
   char *token;
   int i;

   token = strtok (line, " ");
   if (!token)
       return -1;
   
   i = get_month (token);

   if (i == 12)
      return -1;

   date.tm_mon = i;
   date.tm_year = 70;

   token = strtok (NULL, " ");
   if (token != NULL)
      date.tm_mday = atoi (token);
   else
      return -1;

   token = strtok (NULL, ":");
   if (token != NULL)
      date.tm_hour = atoi (token);
   else
      return -1;

   token = strtok (NULL, ":");
   if (token != NULL)
      date.tm_min = atoi (token);
   else
      return -1;

   token = strtok (NULL, " ");
   if (token != NULL)
      date.tm_sec = atoi (token);
   else
      return -1;

   date.tm_isdst = 0;

   return mktime (&date);

}

/* ----------------------------------------------------------------------
   NAME:          CloseLog
   DESCRIPTION:   Close log and free all memory used
   ---------------------------------------------------------------------- */

void
CloseLog (Log * log)
{
   Page *cur, *next;
   int i;

   /* Close file */
   if (log->fp != NULL)
      fclose (log->fp);

   /* Free all used memory */
   cur = log->currentpg;
   for (i = 0; i < NUM_PAGES; i++)
   {
      next = cur->next;
      free (cur);
      cur = next;
   }

   free (log);
   return;
}

/* ----------------------------------------------------------------------
   NAME:	WasModified
   DESCRIPTION:	Returns true if modified flag in log changed. It also
                changes the modified flag.
   ---------------------------------------------------------------------- */

int
WasModified (Log *log)
{
  struct stat filestatus;

  stat (log->name, &filestatus);
  if (filestatus.st_mtime != log->lstats.mtime)
    {
      log->lstats.mtime = filestatus.st_mtime;
      return TRUE;
    }
  else
    return FALSE;
}

/* ----------------------------------------------------------------------
   NAME:        reverse
   DESCRIPTION: Reverse the string.
   ---------------------------------------------------------------------- */

void
reverse (char *line)
{
   int i, j, len;
   char ch;

   len = strlen (line);
   j = len - 1;
   for (i = 0; i < (len >> 1); i++)
   {
      ch = line[i];
      line[i] = line[j];
      line[j] = ch;
      j--;
   }

   return;
}
