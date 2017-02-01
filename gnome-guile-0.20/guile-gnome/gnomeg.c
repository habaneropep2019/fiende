/* The two `main' functions are shamelessly ripped off from
 * guile-1.2/libguile/guile.c.
 *
 * Hopefully this will evolve into a nice shell.
 */

#include <config.h>

#include <libguile.h>
#include "../guile-gtk/guile-gtk.h"

#include "gnome.h"
#include "libgnome/gnome-history.h"
#include "gnome-ui-infos.h"

/* Debugger interface (don't change the order of the following lines) */
#define GDB_TYPE SCM
#include <libguile/gdb_interface.h>
GDB_INTERFACE;


extern void gnome_guile_client_init (void);

/***** Export Gnome functions to Scheme *****/

#if 0
/* FIXME!!!  All this stuff should be generated automatically.
 * XXX: Remember to export all useful GNOME functions to the Scheme part.
 */

static SCM
guile_gnome_libdir_file(SCM scm_filename)
{
	char *filename;
	char *libdir_file;
	SCM   ret;

	SCM_ASSERT(SCM_NIMP(scm_filename) && SCM_STRINGP(scm_filename), scm_filename, SCM_ARG1, "gnome-libdir-file");

	filename    = SCM_CHARS(scm_filename);
	libdir_file = gnome_libdir_file(filename);

	ret = scm_makfrom0str(libdir_file);
	g_free(libdir_file);

	return ret;
}


static SCM
guile_gnome_datadir_file(SCM scm_filename)
{
	char *filename;
	char *datadir_file;
	SCM   ret;

	SCM_ASSERT(SCM_NIMP(scm_filename) && SCM_STRINGP(scm_filename), scm_filename, SCM_ARG1, "gnome-datadir-file");
	
	filename    = SCM_CHARS(scm_filename);
	datadir_file = gnome_datadir_file(filename);

	ret = scm_makfrom0str(datadir_file);
	g_free(datadir_file);

	return ret;
}
	

static SCM
guile_gnome_pixmap_file(SCM scm_filename)
{
	char *filename;
	char *pixmap_file;
	SCM   ret;

	SCM_ASSERT(SCM_NIMP(scm_filename) && SCM_STRINGP(scm_filename), scm_filename, SCM_ARG1, "gnome-pixmap-file");
	
	filename    = SCM_CHARS(scm_filename);
	pixmap_file = gnome_pixmap_file(filename);

	ret = scm_makfrom0str(pixmap_file);
	g_free(pixmap_file);

	return ret;
}
	

static SCM
guile_gnome_unconditional_libdir_file(SCM scm_filename)
{
	char *filename;
	char *libdir_file;
	SCM   ret;

	SCM_ASSERT(SCM_NIMP(scm_filename) && SCM_STRINGP(scm_filename), scm_filename, SCM_ARG1,
		   "gnome-unconditional-libdir-file");

	filename    = SCM_CHARS(scm_filename);
	libdir_file = gnome_unconditional_libdir_file(filename);

	ret = scm_makfrom0str(libdir_file);
	g_free(libdir_file);

	return ret;
}


static SCM
guile_gnome_unconditional_datadir_file(SCM scm_filename)
{
	char *filename;
	char *datadir_file;
	SCM   ret;

	SCM_ASSERT(SCM_NIMP(scm_filename) && SCM_STRINGP(scm_filename), scm_filename, SCM_ARG1,
		   "gnome-unconditional-datadir-file");
	
	filename    = SCM_CHARS(scm_filename);
	datadir_file = gnome_unconditional_datadir_file(filename);

	ret = scm_makfrom0str(datadir_file);
	g_free(datadir_file);

	return ret;
}
	

static SCM
guile_gnome_unconditional_pixmap_file(SCM scm_filename)
{
	char *filename;
	char *pixmap_file;
	SCM   ret;

	SCM_ASSERT(SCM_NIMP(scm_filename) && SCM_STRINGP(scm_filename), scm_filename, SCM_ARG1,
		   "gnome-unconditional-pixmap-file");
	
	filename    = SCM_CHARS(scm_filename);
	pixmap_file = gnome_unconditional_pixmap_file(filename);

	ret = scm_makfrom0str(pixmap_file);
	g_free(pixmap_file);

	return ret;
}


static SCM
guile_gnome_config_get_string(SCM path)
{
	char *thepath;
	char *value;
	SCM   ret;
	
	SCM_ASSERT(SCM_NIMP(path) && SCM_STRINGP(path), path, SCM_ARG1, "gnome-config-get-string");

	thepath = SCM_CHARS(path);
	value = gnome_config_get_string(thepath);

	ret = scm_makfrom0str(value);
	g_free(value);

	return ret;
}

#endif

/* gnome config iterators - not defined in gnome.defs because map and
   foreach interface is more natural to Scheme and these are the only
   iterators in libgnome */

static char s_gnome_config_foreach[] = "gnome-config-foreach";

static SCM
sgtk_gnome_config_foreach (SCM p_path, SCM callback)
{
    char *path;
    void *iterator;
    char *key, *value;

    SCM_ASSERT ((SCM_NIMP(p_path) && SCM_STRINGP(p_path)), p_path, SCM_ARG1, s_gnome_config_foreach);
    path = SCM_CHARS (p_path);
    for (iterator = gnome_config_init_iterator(path); iterator;
	 iterator = gnome_config_iterator_next(iterator, &key, &value)) {
	scm_apply (callback, SCM_LIST2(scm_take0str(key), scm_take0str(value)), SCM_EOL);
    }
    return SCM_UNSPECIFIED;
}

static char s_gnome_config_map[] = "gnome-config-map";

static SCM
sgtk_gnome_config_map (SCM p_path, SCM callback)
{
    char *path;
    void *iterator;
    char *key, *value;
    SCM ret;

    SCM_ASSERT ((SCM_NIMP(p_path) && SCM_STRINGP(p_path)), p_path, SCM_ARG1, s_gnome_config_map);
    path = SCM_CHARS (p_path);
    ret = SCM_LIST0;
    for (iterator = gnome_config_init_iterator(path); iterator;
	 iterator = gnome_config_iterator_next(iterator, &key, &value)) {
	ret = scm_cons(scm_apply (callback, SCM_LIST2(scm_take0str(key), scm_take0str(value)), SCM_EOL), ret);
    }
    return scm_reverse(ret);
}

static char s_gnome_config_private_foreach[] = "gnome-config-private-foreach";

static SCM
sgtk_gnome_config_private_foreach (SCM p_path, SCM callback)
{
    char *path;
    void *iterator;
    char *key, *value;

    SCM_ASSERT ((SCM_NIMP(p_path) && SCM_STRINGP(p_path)), p_path, SCM_ARG1, s_gnome_config_private_foreach);
    path = SCM_CHARS (p_path);
    for (iterator = gnome_config_private_init_iterator(path); iterator;
	 iterator = gnome_config_iterator_next(iterator, &key, &value)) {
	scm_apply (callback, SCM_LIST2(scm_take0str(key), scm_take0str(value)), SCM_EOL);
    }
    return SCM_UNSPECIFIED;
}

static char s_gnome_config_private_map[] = "gnome-config-private-map";

static SCM
sgtk_gnome_config_private_map (SCM p_path, SCM callback)
{
    char *path;
    void *iterator;
    char *key, *value;
    SCM ret;

    SCM_ASSERT ((SCM_NIMP(p_path) && SCM_STRINGP(p_path)), p_path, SCM_ARG1, s_gnome_config_private_map);
    path = SCM_CHARS (p_path);
    ret = SCM_LIST0;
    for (iterator = gnome_config_private_init_iterator(path); iterator;
	 iterator = gnome_config_iterator_next(iterator, &key, &value)) {
	ret = scm_cons(scm_apply (callback, SCM_LIST2(scm_take0str(key), scm_take0str(value)), SCM_EOL), ret);
    }
    return scm_reverse(ret);
}

static char s_gnome_config_sections_foreach[] = "gnome-config-sections-foreach";

static SCM
sgtk_gnome_config_sections_foreach (SCM p_path, SCM callback)
{
    char *path;
    void *iterator;
    char *key, *value;

    SCM_ASSERT ((SCM_NIMP(p_path) && SCM_STRINGP(p_path)), p_path, SCM_ARG1, s_gnome_config_sections_foreach);
    path = SCM_CHARS (p_path);
    for (iterator = gnome_config_init_iterator(path); iterator;
	 iterator = gnome_config_iterator_next(iterator, &key, &value)) {
	scm_apply (callback, SCM_LIST2(scm_take0str(key), scm_take0str(value)), SCM_EOL);
    }
    return SCM_UNSPECIFIED;
}

static char s_gnome_config_sections_map[] = "gnome-config-sections-map";

static SCM
sgtk_gnome_config_sections_map (SCM p_path, SCM callback)
{
    char *path;
    void *iterator;
    char *key, *value;
    SCM ret;

    SCM_ASSERT ((SCM_NIMP(p_path) && SCM_STRINGP(p_path)), p_path, SCM_ARG1, s_gnome_config_sections_map);
    path = SCM_CHARS (p_path);
    ret = SCM_LIST0;
    for (iterator = gnome_config_init_iterator(path); iterator;
	 iterator = gnome_config_iterator_next(iterator, &key, &value)) {
	ret = scm_cons(scm_apply (callback, SCM_LIST2(scm_take0str(key), scm_take0str(value)), SCM_EOL), ret);
    }
    return scm_reverse(ret);
}

static char s_gnome_config_private_sections_foreach[] = "gnome-config-private-sections-foreach";

static SCM
sgtk_gnome_config_private_sections_foreach (SCM p_path, SCM callback)
{
    char *path;
    void *iterator;
    char *key, *value;

    SCM_ASSERT ((SCM_NIMP(p_path) && SCM_STRINGP(p_path)), p_path, SCM_ARG1, s_gnome_config_private_sections_foreach);
    path = SCM_CHARS (p_path);
    for (iterator = gnome_config_private_init_iterator(path); iterator;
	 iterator = gnome_config_iterator_next(iterator, &key, &value)) {
	scm_apply (callback, SCM_LIST2(scm_take0str(key), scm_take0str(value)), SCM_EOL);
    }
    return SCM_UNSPECIFIED;
}

static char s_gnome_config_private_sections_map[] = "gnome-config-private-sections-map";

static SCM
sgtk_gnome_config_private_sections_map (SCM p_path, SCM callback)
{
    char *path;
    void *iterator;
    char *key, *value;
    SCM ret;

    SCM_ASSERT ((SCM_NIMP(p_path) && SCM_STRINGP(p_path)), p_path, SCM_ARG1, s_gnome_config_private_sections_map);
    ret = SCM_LIST0;
    path = SCM_CHARS (p_path);
    for (iterator = gnome_config_private_init_iterator(path); iterator;
	 iterator = gnome_config_iterator_next(iterator, &key, &value)) {
	ret = scm_cons(scm_apply (callback, SCM_LIST2(scm_take0str(key), scm_take0str(value)), SCM_EOL), ret);
    }
    return scm_reverse(ret);
}


/* the coolest widget */

static char s_gtk_dialog_cauldron[] = "gtk-dialog-cauldron";

typedef struct {
    int type;
    union {
	gdouble d;
	gint i;
	gchar *s;
    } var;
} CauldronVarItem;

typedef struct {
    int num_data;
    CauldronVarItem *data_array;
    gchar *title;
    gchar *format;
    glong options;
    SCM args;
#ifdef CAULDRON_TAKES_PARENT
    GtkWidget *parent;
#endif
} CauldronInfo;

extern sgtk_enum_info sgtk_gtk_cauldron_options_info;

#define EXTRACT_SCM(args, val) SCM_ASSERT(SCM_NIMP(args) && SCM_CONSP(args) && SCM_NNULLP(args), args, SCM_WNA, s_gtk_dialog_cauldron); val = SCM_CAR(args); args = SCM_CDR(args);

static void
sgtk_cauldron_cleanup (void *data)
{
    CauldronInfo *info = (CauldronInfo*)data;
    int i;
    for (i = 0; i < info->num_data; i++)
	if (info->data_array[i].type == GTK_CAULDRON_TYPE_CHAR_P_P)
	    g_free(info->data_array[i].var.s);
    g_free(info->data_array);
}

static GtkWidget*
sgtk_cauldron_standard_callback (GtkWidget *widget, gpointer data)
{
    SCM proc = (SCM) data;
    SCM ret;

    ret = scm_apply(proc, SCM_LIST1(sgtk_wrap_gtkobj ((GtkObject*)widget)),
		    SCM_EOL);
    SCM_ASSERT(sgtk_is_a_gtkobj (gtk_widget_get_type (), ret),
	       ret, "Widget expected", s_gtk_dialog_cauldron);
    return (GtkWidget*)sgtk_get_gtkobj (ret);
}

static void
sgtk_cauldron_args_callback (gint type, gpointer data, void *result)
{
    CauldronInfo *info = (CauldronInfo*)data;
    switch (type) {
    case GTK_CAULDRON_TYPE_CHAR_P: {
	gchar **x = (gchar**)result;
	SCM arg;
	EXTRACT_SCM(info->args, arg);
	SCM_ASSERT (SCM_NIMP(arg) && SCM_STRINGP(arg), arg, "string expected", s_gtk_dialog_cauldron);
	*x = (gchar*)SCM_CHARS (arg);
	break;
    }
    case GTK_CAULDRON_TYPE_CHAR_P_P: {
	gchar ***x = (gchar***)result;
	SCM arg;
	EXTRACT_SCM(info->args, arg);
	SCM_ASSERT (arg == SCM_BOOL_F || (SCM_NIMP(arg) && SCM_STRINGP(arg)), arg, "string or #f expected", s_gtk_dialog_cauldron);
	*x = &info->data_array[info->num_data].var.s;
	info->num_data++;
	info->data_array = g_renew(CauldronVarItem, info->data_array, info->num_data);
	info->data_array[info->num_data - 1].type = type;
	**x = (arg != SCM_BOOL_F) ? SCM_CHARS (arg) : NULL;
	break;
    }
    case GTK_CAULDRON_TYPE_INT: {
	gint *x = (gint*)result;
	SCM arg;
	EXTRACT_SCM(info->args, arg);
	SCM_ASSERT (SCM_INUMP(arg), arg, "int expected", s_gtk_dialog_cauldron);
	*x = SCM_INUM(arg);
	break;
    }
    case GTK_CAULDRON_TYPE_INT_P: {
	gint **x = (gint**)result;
	SCM arg;
	EXTRACT_SCM(info->args, arg);
	SCM_ASSERT (SCM_INUMP(arg), arg, "int expected", s_gtk_dialog_cauldron);
	*x = &info->data_array[info->num_data].var.i;
	info->num_data++;
	info->data_array = g_renew(CauldronVarItem, info->data_array, info->num_data);
	info->data_array[info->num_data - 1].type = type;
	**x = SCM_INUM(arg);
	break;
    }
    case GTK_CAULDRON_TYPE_USERDATA_P: {
	gpointer *x = (gpointer)result;
	SCM arg;
	EXTRACT_SCM(info->args, arg);
	SCM_ASSERT (scm_procedure_p(arg), arg, "procedure expected", s_gtk_dialog_cauldron);
	*x = (gpointer) arg;
	break;
    }
    case GTK_CAULDRON_TYPE_DOUBLE: {
	gdouble *x = (gdouble*)result;
	SCM arg;
	EXTRACT_SCM(info->args, arg);
	SCM_ASSERT (SCM_NIMP(arg) && SCM_REALP(arg), arg, "inexact expected", s_gtk_dialog_cauldron);
	*x = SCM_REALPART(arg);
	break;
    }
    case GTK_CAULDRON_TYPE_DOUBLE_P: {
	gdouble **x = (gdouble**)result;
	SCM arg;
	EXTRACT_SCM(info->args, arg);
	SCM_ASSERT (SCM_NIMP(arg) && SCM_REALP(arg), arg, "inexact expected", s_gtk_dialog_cauldron);
	*x = &info->data_array[info->num_data].var.d;
	info->num_data++;
	info->data_array = g_renew(CauldronVarItem, info->data_array, info->num_data);
	info->data_array[info->num_data - 1].type = type;
	**x = SCM_REALPART(arg);
	break;
    }
    case GTK_CAULDRON_TYPE_CALLBACK: {
	GtkCauldronCustomCallback *x = (GtkCauldronCustomCallback *) result;
	*x = sgtk_cauldron_standard_callback;
	break;
    
    }
    }
}

static void
sgtk_cauldron_do_nothing (void *data)
{
}

static SCM
sgtk_cauldron_body (void *data)
{
    CauldronInfo *info = (CauldronInfo*)data;
    int i;
    SCM ret;
    gchar *ret_status;
#ifdef CAULDRON_TAKES_PARENT
    ret_status = 
	gtk_dialog_cauldron_parse  (info->title, info->options, info->format,
				    sgtk_cauldron_args_callback, data,
				    info->parent);
#else
    ret_status = 
	gtk_dialog_cauldron_parse  (info->title, info->options, info->format,
				    sgtk_cauldron_args_callback, data);
#endif
    if (ret_status == NULL)
	ret = SCM_BOOL_F;
    else
	ret = scm_take0str(ret_status);
    ret = SCM_LIST1(ret);
    for (i = 0; i < info->num_data; i++) {
	SCM tmp;
	switch (info->data_array[i].type) {
	case GTK_CAULDRON_TYPE_CHAR_P_P:
	    tmp = scm_take0str(info->data_array[i].var.s);
	    break;
	case GTK_CAULDRON_TYPE_INT_P:
	    tmp = SCM_MAKINUM(info->data_array[i].var.i);
	    break;
	case GTK_CAULDRON_TYPE_DOUBLE_P:
	    tmp = scm_makdbl(info->data_array[i].var.d, 0.0);
	    break;
	}
	ret = scm_cons(tmp, ret);
    }
    return scm_reverse(ret);
}

static SCM
sgtk_gtk_dialog_cauldron (SCM title, SCM options, SCM format, 
#ifdef CAULDRON_TAKES_PARENT
			  SCM parent
#endif
			  , SCM rest)
{
    CauldronInfo info;
    SCM_ASSERT(SCM_NIMP(title) && SCM_STRINGP(title), title, SCM_ARG1, s_gtk_dialog_cauldron);
    SCM_ASSERT(SCM_NIMP(format) && SCM_STRINGP(format), format, SCM_ARG3, s_gtk_dialog_cauldron);
#ifdef CAULDRON_TAKES_PARENT
    SCM_ASSERT(sgtk_is_a_gtkobj (gtk_widget_get_type (), parent),
	       parent, SCM_ARG4, s_gtk_dialog_cauldron);
    info.parent = (GtkWidget*)sgtk_get_gtkobj(parent);
#endif
    info.options = sgtk_scm2flags (options, &sgtk_gtk_cauldron_options_info, SCM_ARG2, s_gtk_dialog_cauldron);
    info.num_data = 0;
    info.data_array = g_new (CauldronVarItem, 1);
    info.title = SCM_CHARS (title);
    info.format = SCM_CHARS (format);
    info.args = rest;
    return scm_internal_dynamic_wind (sgtk_cauldron_do_nothing,
				      sgtk_cauldron_body,
				      sgtk_cauldron_cleanup,
				      &info,
				      &info);
}

static SCM
guile_gnome_about (SCM title, SCM version, SCM copyright,
		   SCM comments, SCM logo, SCM author_list)
{
  gchar **authors;
  int i, count;
  SCM l;
  GtkWidget *c_ret;

  SCM_ASSERT (SCM_NIMP (title) && SCM_STRINGP (title), title,
	      SCM_ARG1, "gnome-about");
  SCM_ASSERT (SCM_NIMP (version) && SCM_STRINGP (version), version,
	      SCM_ARG2, "gnome-about");
  SCM_ASSERT (SCM_NIMP (copyright) && SCM_STRINGP (copyright), copyright,
	      SCM_ARG3, "gnome-about");
  SCM_ASSERT (SCM_NIMP (comments) && SCM_STRINGP (comments), comments,
	      SCM_ARG4, "gnome-about");
  SCM_ASSERT (logo == SCM_BOOL_F
	      || (SCM_NIMP (logo) && SCM_STRINGP (logo)), logo,
	      SCM_ARG5, "gnome-about");

  SCM_COERCE_SUBSTR (title);
  SCM_COERCE_SUBSTR (version);
  SCM_COERCE_SUBSTR (copyright);
  SCM_COERCE_SUBSTR (comments);
  if (logo != SCM_BOOL_F)
    SCM_COERCE_SUBSTR (logo);

  /* Start at one for trailing NULL.  */
  count = 1;
  for (l = author_list; SCM_NNULLP (l); l = SCM_CDR (l))
    {
      SCM item;
      SCM_ASSERT (SCM_NIMP (l) && SCM_CONSP (l), l,
		  SCM_ARG6, "gnome-about");
      item = SCM_CAR (l);
      SCM_ASSERT (SCM_NIMP (item) && SCM_STRINGP (item), item,
		  SCM_ARG6, "gnome-about");
      ++count;
    }

  SCM_DEFER_INTS;
  authors = malloc (count * sizeof (char *));

  i = 0;
  for (l = author_list; ! SCM_NULLP (l); l = SCM_CDR (l))
    {
      SCM item = SCM_CAR (l);
      SCM_COERCE_SUBSTR (item);
      authors[i] = SCM_CHARS (item);
      ++i;
    }
  authors[i] = NULL;

  c_ret = gnome_about_new (SCM_CHARS (title), SCM_CHARS (version),
			   SCM_CHARS (copyright), (const char **)authors,
			   SCM_CHARS (comments),
			   logo == SCM_BOOL_F? NULL : SCM_CHARS (logo));
  free (authors);

  SCM_ALLOW_INTS;

  return sgtk_wrap_gtkobj ((GtkObject *) c_ret);
}



/* FIXME: this is taken from guile-gtk.c.  */
static void
make_argv (SCM list, int *argc, char ***argv)
{
  static char *argv_storage[1] = { "guile-gtk" };

  int c = scm_ilength (list), i;
  char **v;

  *argv = argv_storage;
  *argc = 1;

  if (c < 0)
    return;

  v = (char **)scm_must_malloc ((c+1) * sizeof(char**), "make-argv");
  for (i = 0; i < c; i++, list = SCM_CDR (list))
    {
      if (SCM_IMP (SCM_CAR (list)) || SCM_NSTRINGP (SCM_CAR (list)))
	{
	  scm_must_free ((char *)v);
	  return;
	}
      v[i] = strdup (SCM_CHARS (SCM_CAR (list)));
    }
  v[c] = NULL;
  
  *argv = v;
  *argc = c;
}


static void
guile_gnome_parser (poptContext ctx,
		    enum poptCallbackReason reason,
		    const struct poptOption *opt,
		    const char *arg, void *data)
{
  SCM args = SCM_EOL, ans;

  if(opt->argInfo == POPT_ARG_STRING)
    args = scm_cons(scm_makfrom0str(arg), args);
  else
    args = scm_cons(SCM_BOOL_F, args);

  args = scm_cons(scm_makfrom0str(opt->longName), args);

  ans = scm_apply((SCM)data, args, SCM_EOL);

  if(SCM_FALSEP(ans)) {
    poptPrintHelp(ctx, stdout, 0);
    exit(0);
  }
}

/** notice copied from the earlier, argp version of this routine:

   A hacky wrapper for gnome_init().  We give the function a funny
   name so that a closer approximation can later be named
   `gnome-init'.  This function handles argument parsing.  But it only
   lets long options through.  ARGS is a list of triples.  Each triple
   has the form (LONG-NAME DOC NAME); NAME is #f if the option has no
   argument.  See?  I told you it was hacky.  When an argument is
   found, PARSE_FUNC is called with the option name and the option
   argument (or #f if no argument) as arguments.  If an argument
   (e.g., file name) is seen, then PARSE_FUNC is called with the first
   arg #f and the second arg the argument.
*/

static char sgnome_init_hack[] = "gnome-init-hack";

static SCM
guile_gnome_init_hack (SCM app_id, SCM parse_func, SCM args)
{
  struct poptOption *my_options;
  SCM l;
  int i, len;
  int argc;
  char **argv;

  SCM_ASSERT(SCM_NIMP(app_id) && SCM_STRINGP(app_id), app_id, SCM_ARG1, sgnome_init_hack);

  for(len = 2, /* callback ent + trailing ent */
	l = args;
      SCM_NNULLP(l); l = SCM_CDR(l)) {
    SCM sub;
    int n;


    SCM_ASSERT (SCM_NIMP (l) && SCM_CONSP (l), l,
		SCM_ARG3, sgnome_init_hack);

    n = 0;

    for (sub = SCM_CAR (l); SCM_NNULLP (sub) && n < 3; sub = SCM_CDR (sub)) {
      SCM item;

      SCM_ASSERT (SCM_NIMP (sub) && SCM_CONSP (sub), sub,
		  SCM_ARG3, sgnome_init_hack);

      item = SCM_CAR (sub);
      /* Last item in triple can be #f.  */
      if (n == 3 && SCM_FALSEP (item))
	break;
      SCM_ASSERT (SCM_NIMP (item) && SCM_STRINGP (item), item,
		  SCM_ARG3, sgnome_init_hack);
      SCM_COERCE_SUBSTR (item);

      ++n;
    }

    ++len;
  }

  my_options = g_new0(struct poptOption, len);

  my_options[0].argInfo = POPT_ARG_CALLBACK;
  my_options[0].arg = guile_gnome_parser;
  my_options[0].descrip = (char *)parse_func;

  for(i = 1, l = args; SCM_NNULLP(l); l = SCM_CDR(l)) {
    SCM sub, item;

    sub = SCM_CAR(l);
    my_options[i].longName = SCM_CHARS(SCM_CAR(sub));
    my_options[i].val = - i;
    my_options[i].descrip = _(SCM_CHARS(SCM_CAR(SCM_CDR(sub))));
    item = SCM_CAR(SCM_CDR(SCM_CDR(sub)));
    if(SCM_FALSEP(item))
      my_options[i].argInfo = POPT_ARG_NONE;
    else {
      my_options[i].argInfo = POPT_ARG_STRING;
      my_options[i].argDescrip = SCM_CHARS(item);
    }

    i++;
  }

  make_argv(scm_program_arguments(), &argc, &argv);

  gnome_init_with_popt_table(SCM_CHARS(app_id), VERSION,
			     argc, argv, my_options, 0, NULL);

  g_free(my_options);

  return 0;
}

/* Gnome Canvas */

#ifndef HAVE_GNOME_CANVAS_POINTS_REF
GnomeCanvasPoints*
gnome_canvas_points_ref (GnomeCanvasPoints *pts)
{
  /* Hope for the best, die like the rest. */
  return pts;
}
#endif

GnomeCanvasPoints *
gnome_canvas_points_from_coords (int n_coords, double *coords)
{
  int n_points = n_coords/2, i;
  GnomeCanvasPoints *points = gnome_canvas_points_new (n_points);
  for (i = 0; i < 2*n_points; i++)
    points->coords[i] = coords[i];
  return points;
}

extern sgtk_boxed_info sgtk_gnome_canvas_points_info;
extern SCM sgtk_gnome_canvas_points_from_vector (SCM);

SCM
sgnome_canvas_points_conversion (SCM points)
{
  if (sgtk_valid_boxed (points, &sgtk_gnome_canvas_points_info))
    return points;

  return sgtk_gnome_canvas_points_from_coords (points);
}

GnomeCanvasPoints *
gnome_canvas_points_intern (GnomeCanvasPoints *points)
{
  return points;
}

static char s_gnome_canvas_item_new[] = "gnome-canvas-item-new";

SCM
sgtk_gnome_canvas_item_new (SCM p_group, SCM type_sym, SCM scm_args)
{
  GnomeCanvasItem* cr_ret;
  GnomeCanvasGroup* c_group;

  int n_args;
  sgtk_object_info *info;
  GtkArg *args;

  SCM_ASSERT (sgtk_is_a_gtkobj (gnome_canvas_group_get_type (), p_group),
	      p_group, SCM_ARG1, s_gnome_canvas_item_new);
  SCM_ASSERT (SCM_NIMP(type_sym) && SCM_SYMBOLP(type_sym), type_sym,
	      SCM_ARG2, s_gnome_canvas_item_new);
  n_args = scm_ilength (scm_args);
  SCM_ASSERT (n_args >= 0 && (n_args%2) == 0, scm_args,
	      SCM_ARG3, s_gnome_canvas_item_new);
  n_args = n_args/2;

  info = sgtk_find_object_info (SCM_CHARS(type_sym));
  SCM_ASSERT (info != NULL, type_sym, SCM_ARG2, s_gnome_canvas_item_new);

  SCM_DEFER_INTS;
  c_group = (GnomeCanvasGroup*)sgtk_get_gtkobj (p_group);
  args = sgtk_build_args (info, &n_args, scm_args, SCM_BOOL_F, 
			  s_gnome_canvas_item_new);
  cr_ret = gnome_canvas_item_newv (c_group, info->header.type, n_args, args);
  g_free (args);
  SCM_ALLOW_INTS;

  return sgtk_wrap_gtkobj ((GtkObject*)cr_ret);
}

static char s_gnome_canvas_item_set[] = "gnome-canvas-item-set";

SCM
sgtk_gnome_canvas_item_set (SCM p_item, SCM scm_args)
{
  GnomeCanvasItem* c_item;
  int n_args;
  sgtk_object_info *info;
  GtkArg *args;

  SCM_ASSERT (sgtk_is_a_gtkobj (gnome_canvas_item_get_type (), p_item), p_item, SCM_ARG1, s_gnome_canvas_item_set);
  n_args = scm_ilength (scm_args);
  SCM_ASSERT (n_args >= 0 && (n_args%2) == 0, scm_args,
	      SCM_ARG2, s_gnome_canvas_item_set);
  n_args = n_args/2;

  c_item = (GnomeCanvasItem*)sgtk_get_gtkobj (p_item);
  info = sgtk_find_object_info_from_type (GTK_OBJECT_TYPE(c_item));
  SCM_ASSERT (info != NULL, p_item, SCM_ARG1, s_gnome_canvas_item_set);
  
  SCM_DEFER_INTS;
  args = sgtk_build_args (info, &n_args, scm_args, p_item,
			  s_gnome_canvas_item_set);
  gnome_canvas_item_setv (c_item, n_args, args);
  g_free (args);
  SCM_ALLOW_INTS;

  return SCM_UNSPECIFIED;
}



/*  The Gnome App Helper */

static void gnome_uiinfos_class_init     (GnomeUIInfosClass   *klass);
static void gnome_uiinfos_init           (GnomeUIInfos        *uiinfos);
static void gnome_uiinfos_finalize       (GtkObject          *uiinfos);

static GtkDataClass *uiinfos_parent_class;

GtkType
gnome_uiinfos_get_type (void)
{
  static GtkType uiinfos_type = 0;

  if (!uiinfos_type)
    {
      static const GtkTypeInfo uiinfos_info =
      {
	"GnomeUIInfos",
	sizeof (GnomeUIInfos),
	sizeof (GnomeUIInfosClass),
	(GtkClassInitFunc) gnome_uiinfos_class_init,
	(GtkObjectInitFunc) gnome_uiinfos_init,
        /* reserved_1 */ NULL,
	/* reserved_2 */ NULL,
	(GtkClassInitFunc) NULL,
      };

      uiinfos_type = gtk_type_unique (GTK_TYPE_DATA, &uiinfos_info);
    }

  return uiinfos_type;
}

static void
gnome_uiinfos_class_init (GnomeUIInfosClass *klass)
{
  GtkObjectClass *object_class;
 
  object_class = (GtkObjectClass*) klass;
  uiinfos_parent_class = gtk_type_class (GTK_TYPE_DATA);
  object_class->finalize = gnome_uiinfos_finalize;
}

static void
gnome_uiinfos_init (GnomeUIInfos *uiinfos)
{
  uiinfos->infos = NULL;
}

static void
free_uiinfos (GnomeUIInfo *infos)
{
  GnomeUIInfo *info;

  for (info = infos; info->type != GNOME_APP_UI_ENDOFINFO; info++)
    {
      if (info->moreinfo)
	{
	  switch (info->type)
	    {
	    case GNOME_APP_UI_SUBTREE:
	    case GNOME_APP_UI_RADIOITEMS:
	      free_uiinfos ((GnomeUIInfo*)info->moreinfo);
	      break;
	    case GNOME_APP_UI_HELP:
	    case GNOME_APP_UI_BUILDER_DATA:
	      g_free (info->moreinfo);
	      break;
	    default:
	      break;
	    }
	}
    }

  g_free (infos);
}

static void
gnome_uiinfos_finalize (GtkObject *object)
{
  GnomeUIInfos *uiinfos = GNOME_UIINFOS(object);
  
  if (uiinfos->infos)
    free_uiinfos (uiinfos->infos);
  GTK_OBJECT_CLASS(uiinfos_parent_class)->finalize (object);
}

GnomeUIInfos *
gnome_uiinfos_new ()
{
  return GNOME_UIINFOS(gtk_type_new (GTK_TYPE_GNOME_UIINFOS));
}

static void convert_uiinfos (GnomeUIInfo *infos, SCM list, int len,
			     SCM protector);

static void
uiinfo_error (char *msg, SCM val)
{
  scm_misc_error ("GnomeUIInfo", "%s: %S",
		  scm_cons2 (scm_makfrom0str (msg), val, SCM_EOL));
}

static void
get_string (gchar **ptr, SCM val, SCM protector)
{
  gchar *str, *dup;

  if (!(SCM_NIMP(val) && SCM_STRINGP(val)))
    uiinfo_error ("not a string", val);
  SCM_COERCE_SUBSTR (val);
  sgtk_protect (protector, val);
  *ptr = SCM_CHARS(val);
}

static void
sgnome_connect_func (GnomeUIInfo *info, gchar *signal_name,
		     GnomeUIBuilderData *data)
{
  if (info->moreinfo)
    {
      GtkObject *object = GTK_OBJECT(info->widget);
      SCM proxy = sgtk_wrap_gtkobj (object);
      
      sgtk_protect (proxy, (SCM)info->moreinfo);
      gtk_signal_connect_full (object, signal_name,
			       NULL, sgtk_callback_marshal, info->moreinfo,
			       sgtk_callback_destroy, 0, 0);
    }
}

static GnomeUIInfo *
create_empty_uiinfos (int n_infos)
{
  GnomeUIBuilderData *uidata;
  GnomeUIInfo *uiinfo;

  uidata = g_new0 (GnomeUIBuilderData, 1);
  uiinfo = g_new0 (GnomeUIInfo, n_infos+2);

  uiinfo[0].type = GNOME_APP_UI_BUILDER_DATA;
  uiinfo[0].moreinfo = uidata;
  uidata->connect_func = sgnome_connect_func;
  uidata->data = NULL;
  uidata->is_interp = 1;
  uidata->relay_func = sgtk_callback_marshal;
  uidata->destroy_func = sgtk_callback_destroy;

  return uiinfo;
}
  
extern sgtk_enum_info sgtk_gnome_uiinfo_type_info;
extern sgtk_enum_info sgtk_gnome_uiinfo_configurable_types_info;
extern sgtk_enum_info sgtk_gdk_modifier_type_info;

static SCM uiinfo_macro_translator;

static char s_gnome_uiinfo_set_translator[] = "gnome-uiinfo-set-translator";

SCM
sgtk_gnome_uiinfo_set_translator (SCM trans)
{
  SCM_SETCAR (uiinfo_macro_translator, trans);
  return SCM_UNSPECIFIED;
}

static void
convert_uiinfo (GnomeUIInfo *info, SCM orig_item, SCM protector)
{
  int len, first;
  SCM item = orig_item;

  len = scm_ilength (item);
  if (len < 1)
    uiinfo_error ("invalid item", item);

  /* The type.  When we don't recognize it, we call back to Scheme.
   */
  if (!sgtk_valid_enum (SCM_CAR(item), &sgtk_gnome_uiinfo_type_info))
    {
      if (SCM_NIMP(SCM_CAR(item)) && SCM_SYMBOLP(SCM_CAR(item))
	  && scm_procedure_p (SCM_CAR(uiinfo_macro_translator)) == SCM_BOOL_T)
	{
	  item = scm_apply (SCM_CAR(uiinfo_macro_translator),
			    scm_cons (item, SCM_EOL), SCM_EOL);
	  if (SCM_CONSP(item) 
	      && sgtk_valid_enum (SCM_CAR(item), &sgtk_gnome_uiinfo_type_info))
	    goto valid;
	}
      uiinfo_error ("unrecognized item type",
		    SCM_CONSP(item)? SCM_CAR(item):item);
    }

 valid:
  info->type = sgtk_scm2enum (SCM_CAR(item), &sgtk_gnome_uiinfo_type_info, 0, "" /* checked already */);
  item = SCM_CDR(item);

  /* default settings. */

  info->label = info->hint = info->pixmap_info = NULL;
  info->user_data = info->unused_data = info->moreinfo = NULL;
  info->accelerator_key = '\0';
  info->ac_mods = 0;

  /* keyword value pairs 
   */
  first = 1;
  while (item != SCM_EOL)
    {
      SCM key = SCM_CAR(item);
      SCM val;
      char *keystr;

      if (SCM_CDR(item) != SCM_EOL && SCM_NIMP(key) && SCM_SYMBOLP(key))
	{
	  keystr = SCM_CHARS(key);
	  if (keystr[0] == ':')
	    keystr++;
	  val = SCM_CADR(item);
	  first = 0;
	}
      else if (SCM_CDR(item) != SCM_EOL && SCM_NIMP(key) && SCM_KEYWORDP(key))
	{
	  keystr = SCM_CHARS(SCM_KEYWORDSYM(key))+1;
	  val = SCM_CADR(item);
	  first = 0;
	}
      else if (first)
	{
	  val = key;
	  if (info->type == GNOME_APP_UI_RADIOITEMS)
	    keystr = "items";
	  else if (info->type == GNOME_APP_UI_ITEM_CONFIGURABLE)
	    keystr = "subtype";
	  else
	    keystr = "label";
	}
      else
	break;

      if (!strcmp (keystr, "label"))
	get_string (&info->label, val, protector);
      else if (!strcmp (keystr, "hint"))
	get_string (&info->hint, val, protector);
      else if (!strcmp (keystr, "callback"))
	{
	  if (info->type != GNOME_APP_UI_ITEM
	      && info->type != GNOME_APP_UI_TOGGLEITEM
	      && info->type != GNOME_APP_UI_ITEM_CONFIGURABLE)
	    uiinfo_error ("callback keyword not valid here",
			  orig_item);
	  if (info->moreinfo != NULL)
	    sgtk_unprotect ((SCM)info->moreinfo);
	  sgtk_protect (protector, val);
	  info->moreinfo = (void*)val;
	}
      else if (!strcmp (keystr, "items"))
	{
	  int sublen;
	  GnomeUIInfo *subinfos;

	  if (info->type != GNOME_APP_UI_SUBTREE
	      && info->type != GNOME_APP_UI_SUBTREE_STOCK
	      && info->type != GNOME_APP_UI_RADIOITEMS)
	    uiinfo_error ("items keyword not valid here", orig_item);
	  sublen = scm_ilength (val);
	  if (sublen < 0)
	    uiinfo_error ("not a list", val);
	  subinfos = create_empty_uiinfos (sublen);
	  if (info->moreinfo)
	    free_uiinfos ((GnomeUIInfo*)info->moreinfo);
	  info->moreinfo = (void*)subinfos;
	  convert_uiinfos (subinfos+1, val, sublen, protector);
	}
      else if (info->type != GNOME_APP_UI_ITEM_CONFIGURABLE)
	{
	  if (!strcmp (keystr, "node"))
	    {
	      if (info->type != GNOME_APP_UI_HELP)
		uiinfo_error ("node keyword not valid here", orig_item);
	      get_string ((char **)&info->moreinfo, val, protector);
	    }
	  else if (!strcmp (keystr, "stock-pixmap"))
	    {
	      info->pixmap_type = GNOME_APP_PIXMAP_STOCK;
	      get_string ((char **)&info->pixmap_info, val, protector);
	    }
	  else if (!strcmp (keystr, "file-pixmap"))
	    {
	      info->pixmap_type = GNOME_APP_PIXMAP_FILENAME;
	      get_string ((char **)&info->pixmap_info, val, protector);
	    }
	  else if (!strcmp (keystr, "data-pixmap"))
	    {
	      info->pixmap_type = GNOME_APP_PIXMAP_DATA;
	      get_string ((char **)&info->pixmap_info, val, protector);
	    }
	  else if (!strcmp (keystr, "accel-key"))
	    {
	      if (!SCM_ICHRP(val))
		uiinfo_error ("not a character", val);
	      info->accelerator_key = SCM_ICHR(val);
	    }
	  else if (!strcmp (keystr, "accel-mods"))
	    {
	      if (!sgtk_valid_flags (val, &sgtk_gdk_modifier_type_info))
		uiinfo_error ("not a GdkModifierType", val);
	      info->ac_mods = sgtk_scm2flags (val,
					      &sgtk_gdk_modifier_type_info, 0, "");
	    }
	  else
	    uiinfo_error ("unrecognized keyword", key);
	}
      else
	{
	  if (!strcmp (keystr, "subtype"))
	    {
	      if (!sgtk_valid_enum (val, &sgtk_gnome_uiinfo_configurable_types_info))
		uiinfo_error ("not a GnomeUIInfoConfigurableType", val);
	      info->accelerator_key = sgtk_scm2enum (val, &sgtk_gnome_uiinfo_configurable_types_info, 0, "");
	    }
	  else
	    uiinfo_error ("unrecognized keyword", key);
	}

      if (first)
	{
	  item = SCM_CDR(item);
	  first = 0;
	}
      else
	item = SCM_CDDR(item);
    }

  if (item != SCM_EOL)
    uiinfo_error ("junk at end of item", item);

  if (!(info->type == GNOME_APP_UI_RADIOITEMS
	|| info->type == GNOME_APP_UI_SEPARATOR
	|| (info->type == GNOME_APP_UI_ITEM_CONFIGURABLE
	    && info->accelerator_key != GNOME_APP_CONFIGURABLE_ITEM_NEW)))
    {
      if (info->label == NULL)
	uiinfo_error ("must set label", orig_item);
    }
  else
    {
      if (info->label || info->hint)
	uiinfo_error ("can't set label or hint", orig_item);
    }
}

static void
convert_uiinfos (GnomeUIInfo *infos, SCM list, int len, SCM protector)
{
  while (list != SCM_EOL)
    {
      convert_uiinfo (infos, SCM_CAR(list), protector);
      list = SCM_CDR(list);
      infos += 1;
    }
}

SCM
sgnome_uiinfos_conversion (SCM val)
{
  GnomeUIInfos *uiinfos;
  GnomeUIInfo *uiinfo;
  SCM scm_uiinfos;

  int n_infos;

  n_infos = scm_ilength (val);
  if (n_infos < 0)
    return val;

  uiinfos = gnome_uiinfos_new ();
  uiinfo = create_empty_uiinfos (n_infos);

  /* All types of UIINFO are GNOME_APP_UI_ENDOFINFO (expcept the first
     one, which is GNOME_APP_UI_BUILDERDATA).  We create the final SCM
     object right now and stuff our empty structures into it.  When we
     want to freak out during the actual conversion, we can just throw
     an error and the GC will clean up for us.  */

  scm_uiinfos = sgtk_wrap_gtkobj (GTK_OBJECT(uiinfos));
  uiinfos->infos = uiinfo;
  convert_uiinfos (uiinfo+1, val, n_infos, scm_uiinfos);

  return scm_uiinfos;
}

/* All the real work is done in the wrapper. */

GnomeUIInfos *
gnome_uiinfos_intern (GnomeUIInfos *uiinfos)
{
  return uiinfos;
}

void
gnome_app_create_menus_from_uiinfos (GnomeApp *app, GnomeUIInfos *uiinfos)
{
  gnome_app_create_menus (app, uiinfos->infos);
}

void
gnome_help_goto_file (gchar *file)
{
  gnome_help_goto (NULL, file);
}



void
gnome_init_guile_support (void)
{
	/* utilities */

#if 0
	scm_make_gsubr("gnome-libdir-file", 1, 0, 0, guile_gnome_libdir_file);
	scm_make_gsubr("gnome-datadir-file", 1, 0, 0, guile_gnome_datadir_file);
	scm_make_gsubr("gnome-pixmap-file", 1, 0, 0, guile_gnome_pixmap_file);
	scm_make_gsubr("gnome-unconditional-libdir-file", 1, 0, 0, guile_gnome_unconditional_libdir_file);
	scm_make_gsubr("gnome-unconditional-datadir-file", 1, 0, 0, guile_gnome_unconditional_datadir_file);
	scm_make_gsubr("gnome-unconditional-pixmap-file", 1, 0, 0, guile_gnome_unconditional_pixmap_file);

	scm_make_gsubr("gnome-config-get-string", 1, 0, 0, guile_gnome_config_get_string);
#endif

	/* FIXME: this should really be done via `gnome.defs'.  But
	   currently the .defs file doesn't have the syntax to
	   describe this interface.  Note that I changed the order of
	   arguments here.  I named the function "gnome-about" and not
	   "gnome-about-new" so that the latter can be used by the
	   .defs file without changing anything.  */
	scm_make_gsubr("gnome-about", 5, 0, 1, guile_gnome_about);

	scm_make_gsubr(sgnome_init_hack, 3, 0, 0, guile_gnome_init_hack);
	scm_make_gsubr (s_gnome_canvas_item_set, 1, 0, 1, sgtk_gnome_canvas_item_set);
	scm_make_gsubr (s_gnome_canvas_item_new, 2, 0, 1, sgtk_gnome_canvas_item_new);

	/* I don't think this should use gnome.defs - it's one of a
           kind interface, and iterators are not Schemish */

	scm_make_gsubr (s_gnome_config_foreach, 1, 0, 0, sgtk_gnome_config_foreach);
	scm_make_gsubr (s_gnome_config_map, 1, 0, 0, sgtk_gnome_config_map);
	scm_make_gsubr (s_gnome_config_private_foreach, 1, 0, 0, sgtk_gnome_config_private_foreach);
	scm_make_gsubr (s_gnome_config_private_map, 1, 0, 0, sgtk_gnome_config_private_map);
	scm_make_gsubr (s_gnome_config_sections_foreach, 1, 0, 0, sgtk_gnome_config_sections_foreach);
	scm_make_gsubr (s_gnome_config_sections_map, 1, 0, 0, sgtk_gnome_config_sections_map);
	scm_make_gsubr (s_gnome_config_private_sections_foreach, 1, 0, 0, sgtk_gnome_config_private_sections_foreach);
	scm_make_gsubr (s_gnome_config_private_sections_map, 1, 0, 0, sgtk_gnome_config_private_sections_map);
#ifdef CAULDRON_TAKES_PARENT
	scm_make_gsubr (s_gtk_dialog_cauldron, 4, 0, 1, sgtk_gtk_dialog_cauldron);
#else
	scm_make_gsubr (s_gtk_dialog_cauldron, 3, 0, 1, sgtk_gtk_dialog_cauldron);
#endif
	uiinfo_macro_translator =
	  scm_permanent_object (scm_cons (SCM_BOOL_F, SCM_EOL));
	scm_make_gsubr (s_gnome_uiinfo_set_translator, 1, 0, 0,
			sgtk_gnome_uiinfo_set_translator);

}

#if 0
static void
locate_error (char *s)
{
	fprintf (stderr,
		 "I could not locate the %s file\n"
		 "please check that you have either installed the software,\n"
		 "Or you have set the GNOMEDIR variable to point to an appropiate\n"
		 "location\n", s);
	exit (1);
}

static void
inner_main (void *closure, int argc, char **argv)
{
	char *shared_guile_scm = gnome_datadir_file ("gnome-guile/gnome.scm");
	char *cmd;
	char **argv_copy;
	int  i;

	/* We load the glue... */

	if (!shared_guile_scm){
		locate_error ("gnome.scm");
		exit (1);
	}

	/* ... initialize Guile-Gtk... */

	sgtk_init_with_args (&argc, &argv);

	/* ... load the Scheme part of Gnome for apps to use... */

	init_guile_gnome_defs();
	sgtk_init_gnome_defs ();
	gnome_guile_client_init ();
	scm_init_toolkits_gtkstubs_module ();
	scm_primitive_load_path (scm_makfrom0str ("ice-9/boot-9.scm"));
	{
		extern int scm_ice_9_already_loaded;
		
		scm_ice_9_already_loaded = 1;
	}
	scm_primitive_load (scm_makfrom0str (shared_guile_scm));

	scm_shell (argc, argv);

	g_free (shared_guile_scm);
}

int
main (int argc, char **argv)
{
	scm_boot_guile (argc, argv, inner_main, 0);
	return 0; /* never reached */
}
#endif
