/* hello.cc
 *
 * Copyright (C) 1999 Havoc Pennington, The Gtk-- Development Team
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


#include <gnome.h>
#include <gnome--/client.h>
#include <gnome--/main.h>

#include "application.h"


/*** gnomehello-popttable */
static int greet_mode = FALSE;
static char* message  = "";
static char* geometry = "";

struct poptOption options[] = {
  {
    "greet",
    'g',
    POPT_ARG_NONE,
    &greet_mode,
    0,
    N_("Say hello to specific people listed on the command line"),
    NULL
  },
  { 
    "message",
    'm',
    POPT_ARG_STRING,
    &message,
    0,
    N_("Specify a message other than \"Hello, World!\""),
    N_("MESSAGE")
  },
  { 
    "geometry",
    '\0',
    POPT_ARG_STRING,
    &geometry,
    0,
    N_("Specify the geometry of the main window"),
    N_("GEOMETRY")
  },
  {
    NULL,
    '\0',
    0,
    NULL,
    0,
    NULL,
    NULL
  }
};
/* gnomehello-popttable ***/

class Gnome_Hello : public SigC::Object
// : public Gnome_Client - you wish, but you can't, because you need
// to get the master client, that is you need to specifically
// initialize yourself to gnome_master_client().
{
public:

  Gnome_Hello();

protected:
  // sig handlers

  void session_die(gpointer client_data);

  gint save_session(gint phase, 
		    GnomeSaveStyle save_style,
		    gint is_shutdown, GnomeInteractStyle interact_style,
		    gint is_fast, gpointer client_data);

protected:
  Gnome::Client *client_;
  
};

Gnome_Hello::Gnome_Hello()
  : client_(Gnome::Client::master_client())
{
//   client_->save_yourself.connect(SigC::slot(this, &Gnome_Hello::save_session));
//   client_->die.connect(SigC::slot(this, &Gnome_Hello::session_die));
}

int 
main(int argc, char* argv[])
{
  // gnomehello-parsing */

//   bindtextdomain("GnomeHello", "");  // GNOMELOCALEDIR
//   textdomain("GnomeHello");

  poptContext pctx;

  Gnome::Main gnomeMain("GnomeHello", "0.1", argc, argv, 
		       options, 0, &pctx);  

  // Argument parsing
  //
  const char** args;

  vector<string> greet;

  args = poptGetArgs(pctx);

  if (greet_mode && args)
    {
      int i = 0;
      while (args[i] != NULL) 
        {
          greet.push_back(args[i]);
          ++i;
        }
    }
  else if (greet_mode && args == NULL)
    {
      g_error(_("You must specify someone to greet."));
    }
  else if (args != NULL)
    {
      g_error(_("Command line arguments are only allowed with --greet."));
    }
  else
    { 
      g_assert(!greet_mode && args == NULL);
    }

  poptFreeContext(pctx);


  // gnomehello-client
  //
  Gnome_Hello client;

  
  // Main app
  //
  geometry = "";
  Hello_App* myApp=manage(new Hello_App(message, geometry, greet));

//   g_slist_free(greet);

  gnomeMain.run();

  return 0;
  /* gnomehello-main ***/
}

// gnomehello-save-session
gint // should be bool
Gnome_Hello::save_session(gint phase, GnomeSaveStyle save_style,
			  gint is_shutdown, GnomeInteractStyle interact_style,
			  gint is_fast, gpointer client_data)
{
  cout << "save_session" <<endl;
  vector<string> argv(4);

  argv.push_back(static_cast<char*>(client_data)); // yuck

  if (message)
    {
      argv.push_back("--message");
      argv.push_back(message);
    }
  
  client_->set_clone_command (argv);
  client_->set_restart_command (argv);

  return true;
}


// gnomehello-session-die */
void
Gnome_Hello::session_die(gpointer client_data)
{
  cout << "session_die" <<endl;
}
