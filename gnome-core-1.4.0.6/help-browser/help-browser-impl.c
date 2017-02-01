#include <stdio.h>
#include "help-browser.h"
#include "window.h"
#include <glib.h>

HelpWindow makeHelpWindow(gint x, gint y, gint w, gint h);

help_browser_simple_browser 
impl_help_browser_simple_browser__create(PortableServer_POA poa,
					 HelpWindow window, 
					 CORBA_Environment * ev);
void destroy_server(HelpWindow win);

static GHashTable* win_servant_hash = 0;


/*** App-specific servant structures ***/
typedef struct {
   POA_help_browser_simple_browser servant;
   PortableServer_POA              poa;
   HelpWindow                      window;
} impl_POA_help_browser_simple_browser;

/*** Implementation stub prototypes ***/
static void impl_help_browser_simple_browser__destroy(PortableServer_Servant servant,
						    CORBA_Environment * ev);
void
 impl_help_browser_simple_browser_fetch_url(PortableServer_Servant _servant,
					    const CORBA_char * URL,
					    CORBA_Environment * ev);

help_browser_simple_browser
impl_help_browser_simple_browser_show_url(PortableServer_Servant _servant,
					  const CORBA_char * URL,
					  CORBA_Environment * ev);

/*** epv structures ***/
static PortableServer_ServantBase__epv impl_help_browser_simple_browser_base_epv =
{
   NULL,			/* _private data */
   &impl_help_browser_simple_browser__destroy,	/* finalize routine */
   NULL,			/* default_POA routine */
};
static POA_help_browser_simple_browser__epv impl_help_browser_simple_browser_epv =
{
   NULL,			/* _private */
   & impl_help_browser_simple_browser_fetch_url,
   & impl_help_browser_simple_browser_show_url
};

/*** vepv structures ***/
static POA_help_browser_simple_browser__vepv impl_help_browser_simple_browser_vepv =
{
   &impl_help_browser_simple_browser_base_epv,
   &impl_help_browser_simple_browser_epv,
};

static guint
idhash(gconstpointer key)
{
  return GPOINTER_TO_UINT (key);
}

static gint
idcompare(gconstpointer a, gconstpointer b)
{
  return a == b;
}

/*** Stub implementations ***/
help_browser_simple_browser 
impl_help_browser_simple_browser__create(PortableServer_POA poa,
					 HelpWindow window, CORBA_Environment * ev)
{
   help_browser_simple_browser retval;
   impl_POA_help_browser_simple_browser *newservant;
   PortableServer_ObjectId *objid;

   newservant = g_new0(impl_POA_help_browser_simple_browser, 1);
   newservant->servant.vepv = &impl_help_browser_simple_browser_vepv;
   newservant->poa = poa;
   newservant->window = window;
   if (window)
     {
       if (!win_servant_hash)
	 {
	   win_servant_hash = g_hash_table_new(idhash, idcompare);
	 }
       g_hash_table_insert(win_servant_hash, (gpointer) window, newservant);
     }
   POA_help_browser_simple_browser__init((PortableServer_Servant) newservant, ev);
   objid = PortableServer_POA_activate_object(poa, newservant, ev);
   CORBA_free(objid);
   retval = PortableServer_POA_servant_to_reference(poa, newservant, ev);

   return retval;
}

/* You shouldn't call this routine directly without first deactivating the servant... */
void
impl_help_browser_simple_browser__destroy (PortableServer_Servant servant, CORBA_Environment * ev)
{

   POA_help_browser_simple_browser__fini (servant, ev);
   g_free (servant);
}

void
impl_help_browser_simple_browser_fetch_url(PortableServer_Servant _servant,
					   const CORBA_char * URL,
					   CORBA_Environment * ev)
{
  impl_POA_help_browser_simple_browser * servant =
	  (impl_POA_help_browser_simple_browser *) _servant;
  if (!servant->window)
    {
      servant->window = makeHelpWindow(0, 0, 0, 0);
      if (!win_servant_hash)
	{
	  win_servant_hash = g_hash_table_new(idhash, idcompare);
	}
      g_hash_table_insert(win_servant_hash, 
			  (gpointer) servant->window, servant);
    }
  helpWindowShowURL(servant->window, URL, TRUE, TRUE);
  gtk_widget_show (helpWindowGetAppWindow (servant->window));
}

help_browser_simple_browser
impl_help_browser_simple_browser_show_url(PortableServer_Servant _servant,
					  const CORBA_char * URL,
					  CORBA_Environment * ev)
{
   impl_POA_help_browser_simple_browser * servant =
	   (impl_POA_help_browser_simple_browser *) _servant;
   help_browser_simple_browser retval;
   HelpWindow                   window;

   window = makeHelpWindow(0,0,0,0);
   retval = impl_help_browser_simple_browser__create(servant->poa, window, ev);
   helpWindowShowURL(window, URL, TRUE, TRUE);
   return retval;
}

void
destroy_server(HelpWindow win)
{
  PortableServer_ObjectId*              oid;
  impl_POA_help_browser_simple_browser* servant;
  CORBA_Environment                     ev;
    
  CORBA_exception_init(&ev);
  servant = g_hash_table_lookup(win_servant_hash, win);
  if (servant) 
    {
      g_hash_table_remove(win_servant_hash, win);
      fprintf(stderr,"getting servant data %p from window %p\n", servant, win);
  
      oid     = PortableServer_POA_servant_to_id(servant->poa, servant, &ev);
    
      PortableServer_POA_deactivate_object(servant->poa, oid, &ev);
      impl_help_browser_simple_browser__destroy((PortableServer_Servant)servant, &ev);
    }
  CORBA_exception_free(&ev);
}
