#include <config.h>
#include <stdio.h>
#include <ctype.h>
#include <libgnorba/gnorba.h>
#include "help-browser.h"

static void Exception( CORBA_Environment* ev )
{
  switch( ev->_major )
    {
    case CORBA_SYSTEM_EXCEPTION:
      fprintf( stderr, "CORBA system exception %s.\n",
	       CORBA_exception_id(ev));
      exit ( 1 );
    case CORBA_USER_EXCEPTION:
      fprintf( stderr, "CORBA user exception: %s.\n",
	       CORBA_exception_id( ev ) );
      exit ( 1 );
    default:
      break;
    }
}

int
main(int argc, char* argv[])
{
  CORBA_ORB orb;
  CORBA_Environment ev;
  CORBA_Object browser;
  gchar  URL[1024];
  gchar* ptr;
  
  CORBA_exception_init(&ev);
  
  orb = gnome_CORBA_init("help-caller", VERSION, &argc, argv, 0, &ev);
  Exception(&ev);

  browser = goad_server_activate_with_repo_id(0, "IDL:help_browser/simple_browser:1.0", GOAD_ACTIVATE_REMOTE, NULL);

  if (browser == CORBA_OBJECT_NIL)
    {
      fprintf(stderr,"Cannot activate browser\n");
      exit(1);
    }
  fprintf(stderr,"gnome-help-caller gets browser at IOR='%s'\n",
	  CORBA_ORB_object_to_string(orb, browser, &ev));
  do
    {
      printf("Enter URL: ");
      fflush(stdout);
      if ((ptr = fgets(URL, sizeof(URL), stdin)))
	{
	  char* ptr_end = ptr + strlen(ptr) - 1;
	  while (isspace(*ptr_end))
	    ptr_end--;
	  *++ptr_end = '\0';
	  while (*ptr && isspace(*ptr))
	    ptr++;
	  if (*ptr == '#')
	    {
	      ptr++;
	      if (!*ptr)
		ptr = "toc:";
	      fprintf(stderr,"Displaying URL '%s' in same window\n", ptr);
	      help_browser_simple_browser_fetch_url(browser, ptr, &ev);
	    }
	  else
	    {
	      if (!*ptr)
		ptr = "toc:";
	      fprintf(stderr,"Displaying URL '%s' in new window\n", ptr);
	      browser = help_browser_simple_browser_show_url(browser, ptr, &ev);
	    }
	  Exception(&ev);
	}
    }while (ptr);

  return 0;
}

					      
  
