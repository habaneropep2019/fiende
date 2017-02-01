/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *   guname: System information dialog. 
 *
 *   Copyright (C) 1998 Havoc Pennington <hp@pobox.com> except marquee code.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include <config.h>
#include <gnome.h>
#include "info.h"
#include <unistd.h>
#include <string.h> /* memset */
#include <sys/utsname.h>
#include <errno.h>

#define DEBIAN_STRING "Debian GNU/Linux"
#define MANDRAKE_STRING "Mandrake-Linux"
#define REDHAT_STRING "Red Hat Linux"
#define SUSE_STRING "SuSE Linux"

/************************************
  Globals
  ***********************************/

const gchar * descriptions[] = {
  N_("Distribution:"), 
  N_("Operating System:"),
  N_("Distribution Version:"),
  N_("Operating System Version:"),
  N_("Operating System Release:"),
  N_("Processor Type:"),
  N_("Processor Speed:"),
  N_("Host Name:"),
  N_("Domain Name:"),
  N_("User Name:"),
  N_("X Display Name:"),
  N_("System Status:"),
  N_("Real Memory:"),
  N_("Swap Space (\"virtual memory\"):"),
  N_("Total Memory:"),
  N_("Free Memory:")
};

#if 0
const gchar * disk_descriptions[] = {
  N_("Device"),
  N_("Size"),
  N_("Used space"),
  N_("Free space"),
  N_("Percent free"),
  N_("Mount point")
};
#endif

const gchar * info[end_system_info];
#if 0
GList * disks = NULL;
#endif

/****************
  Prototypes
  *******************/

static void get_portable_info();
static void get_uptime();
static void get_linux_info();



/*****************************
  Code
  *****************************/

void load_system_info()
{
  static gboolean first_time = TRUE;

  if (first_time) {
    /* Set all the pointers in the array to NULL */
    memset(&info, '\0', sizeof(info));
    first_time = FALSE;
  }
  else {
    /* Clear out the array. */
    int i = 0;
    while ( i < end_system_info ) {
      if (info[i] != NULL) g_free((gpointer)info[i]);
      info[i] = NULL;
      ++i;
    }
  }

  get_portable_info();

  get_linux_info();
}

static void get_portable_info()
{
  struct utsname buf;

  if ( uname(&buf) == -1 ) {
    g_error("uname() failed, %s\n", g_unix_error_string(errno));
  }

  info[si_OS] = g_strdup(buf.sysname);
  info[si_release] = g_strdup(buf.release);
  info[si_OS_version] = g_strdup(buf.version);
  info[si_CPU_type] = g_strdup(buf.machine);
  info[si_host] = g_strdup(buf.nodename);
  /*  info[si_domain] = g_strdup(buf.domainname); */ /* man page wrong? */

  info[si_user] = g_strdup(g_get_user_name());

  info[si_display] = g_strdup(gdk_get_display());

  get_uptime();
  
  /* Add more later */

}

static void get_linux_info()
{
  /* Identify distribution (really this could be compiled in) */
  if (g_file_exists("/etc/debian_version")) {
    FILE * f;
    gchar buf[10];

    info[si_distribution] = g_strdup(DEBIAN_STRING);
    f = fopen("/etc/debian_version", "r");
    if (f) { 
      fscanf(f, "%8s", buf);
      info[si_distribution_version] = g_strdup(buf);
      fclose(f);
    }
  } else if (g_file_exists("/etc/mandrake-release")) {  
    FILE *f;
    gchar buf[80];

    info[si_distribution] = g_strdup(MANDRAKE_STRING);
    f = fopen("/etc/mandrake-release", "r");
    if (f) {
      fgets(buf, 79, f);
      info[si_distribution_version] = g_strdup(buf);
      fclose(f);
    }
  } else if (g_file_exists("/etc/SuSE-release")) {
    FILE *f;
    gchar buf[80];

    info[si_distribution] = g_strdup(SUSE_STRING);
    f = fopen("/etc/SuSE-release", "r");
    if (f) { 
      fgets(buf, 79, f);
      info[si_distribution_version] = g_strdup(buf);
      fclose(f);
    }

  } else if (g_file_exists("/etc/redhat-release")) {
    FILE *f;
    gchar buf[80];

    info[si_distribution] = g_strdup(REDHAT_STRING);
    f = fopen("/etc/redhat-release", "r");
    if (f) {
      fgets(buf, 79, f);
      info[si_distribution_version] = g_strdup(buf);
      fclose(f);
    }
  } else if (g_file_exists("/etc/SuSE-release")) {
    FILE *f;
	gchar buf[80];

	info[si_distribution] = g_strdup(SUSE_STRING);
	f = fopen("/etc/SuSE-release", "r");
	if (f) { 
	  fgets(buf, 79, f);
	  info[si_distribution_version] = g_strdup(buf);
	  fclose(f);
	}
  }		
  /* OK, people using other dists will need to add theirs, I have
     no idea how to identify a Red Hat system. */
  else {
    /* This is about the only case where we'll put Unknown instead
       of simply not showing the info. */
    info[si_distribution_version] = g_strdup(_("Unknown"));
  }

  /* More to come. */

}

static void get_uptime()
{
  FILE * f;
  static const gint bufsize = 80;
  gchar buffer[81];
  gint chars_read;
  int i = 0;

  memset(buffer, '\0', bufsize + 1);

  f = popen("uptime", "r");

  if (f) {
    chars_read = fread(buffer, sizeof(char), bufsize, f);

    if ( chars_read > 0 ) {

      /* Strip leading whitespace from uptime results, looks nicer */
      while ( (buffer[i] == ' ') && (buffer[i] != '\0') ) {
        ++i;
      }

      info[si_uptime] = g_strdup(&buffer[i]);
    }

    pclose(f);
  }
}





