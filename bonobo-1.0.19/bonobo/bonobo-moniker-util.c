/*
 * bonobo-moniker-util.c
 *
 * Copyright (C) 2000  Helix Code, Inc.
 *
 * Authors:
 *	Michael Meeks    (michael@helixcode.com)
 *	Ettore Perazzoli (ettore@helixcode.com)
 */
/* #include <syslog.h> */

#include <bonobo/bonobo-moniker-util.h>
#include <bonobo/bonobo-exception.h>
#include <liboaf/liboaf.h>
#include <liboaf/oaf-async.h>
#include <ORBitservices/CosNaming.h>

struct {
	char *prefix;
	char *oafiid;
} fast_prefix [] = {
	{ "file:",   "OAFIID:Bonobo_Moniker_File"  },
	{ "query:(", "OAFIID:Bonobo_Moniker_Query" },
	{ "!",       "OAFIID:Bonobo_Moniker_Item"  },
	{ "OAFIID:", "OAFIID:Bonobo_Moniker_Oaf"   },
	{ "OAFAID:", "OAFIID:Bonobo_Moniker_Oaf"   },
	{ "new:",    "OAFIID:Bonobo_Moniker_New"   },
	{ "http:",   "OAFIID:Bonobo_Moniker_http"  },
/*
	{ "queue:", "" },
*/
	{ NULL, NULL }
};

static char *
moniker_id_from_nickname (const CORBA_char *name)
{
	int i;

	for (i = 0; fast_prefix [i].prefix; i++) {
		int len = strlen (fast_prefix [i].prefix);

		if (!g_strncasecmp (fast_prefix [i].prefix, name, len)) {

			return fast_prefix [i].oafiid;
		}
	}

	return NULL;
}

/*
 * get_full_interface_name:
 * @ifname: original name: can be in form Bonobo/Control
 *
 * Return value: full name eg. IDL:Bonobo/Control:1.0
 */
static gchar *
get_full_interface_name (const char *ifname)
{
	int len, had_ver;
	const char *a;
	char *retval, *b;

	g_return_val_if_fail (ifname != NULL, NULL);

	len = strlen (ifname);
	retval = g_new (char, len + 4 + 4 + 1);

	strcpy (retval, "IDL:");
	a = ifname;
	b = retval + 4;

	if (ifname [0] == 'I' &&
	    ifname [1] == 'D' &&
	    ifname [2] == 'L' &&
	    ifname [3] == ':')
		a += 4;

	for (had_ver = 0; (*b = *a); a++, b++) {
		if (*a == ':')
			had_ver = 1;
	}

	if (!had_ver)
		strcpy (b, ":1.0");

	return retval;
}

static gchar *
query_from_name (const char *name)
{
	char *prefix, *query;
	int   len;

	for (len = 0; name [len]; len++) {
		if (name [len] == ':') {
			len++;
			break;
		}
	}


	prefix = g_strndup (name, len);
		
	query = g_strdup_printf (
		"repo_ids.has ('IDL:Bonobo/Moniker:1.0') AND "
		"bonobo:moniker.has ('%s')", prefix);
	g_free (prefix);

	return query;
}

/**
 * bonobo_moniker_util_new_from_name_full:
 * @parent: A parent moniker to chain to or CORBA_OBJECT_NIL
 * @name: the display name
 * @ev: corba environment
 * 
 *  This routine is used to continue building up the chain
 * that forms a multi-part moniker. The parent is referenced
 * as the parent and passed onto the next stage of parsing
 * the 'name'. We eventually return a moniker handle which
 * represents the end of a linked list of monikers each
 * pointing to their parent:
 *
 * file:/tmp/a.tar.gz <-- gzip: <-- tar: <-- [ this is returned ]
 * 
 * Return value: The end node of a list of monikers representing @name
 **/
Bonobo_Moniker
bonobo_moniker_util_new_from_name_full (Bonobo_Moniker     parent,
					const CORBA_char  *name,
					CORBA_Environment *ev)
{
	Bonobo_Unknown   object;
	Bonobo_Moniker   toplevel, moniker;
	const char       *iid;

	g_return_val_if_fail (ev != NULL, NULL);
	g_return_val_if_fail (name != NULL, NULL);

	if (!name [0])
		return bonobo_object_dup_ref (parent, ev);

	if (name [0] == '#')
		name++;

	if (!(iid = moniker_id_from_nickname (name))) {
		char *query;

		query = query_from_name (name);

		object = oaf_activate (query, NULL, 0, NULL, ev);

		g_free (query);
		
		if (BONOBO_EX (ev))
			return CORBA_OBJECT_NIL;

		if (object == CORBA_OBJECT_NIL) {
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
					     ex_Bonobo_Moniker_UnknownPrefix, NULL);
			return CORBA_OBJECT_NIL;
		}
	} else {
		object = oaf_activate_from_id ((gchar *) iid, 0, NULL, ev);

		if (BONOBO_EX (ev))
			return CORBA_OBJECT_NIL;
		
		if (object == CORBA_OBJECT_NIL) {
			g_warning ("Activating '%s' returned nothing", iid);
			return CORBA_OBJECT_NIL;
		}
	}

	toplevel = Bonobo_Unknown_queryInterface (
		object, "IDL:Bonobo/Moniker:1.0", ev);

	if (BONOBO_EX (ev)) {
		bonobo_object_release_unref (object, ev);
		return CORBA_OBJECT_NIL;
	}

	bonobo_object_release_unref (object, ev);

	if (toplevel == CORBA_OBJECT_NIL) {
		g_warning ("Moniker object '%s' doesn't implement "
			   "the Moniker interface", iid);
		return CORBA_OBJECT_NIL;
	}

	moniker = Bonobo_Moniker_parseDisplayName (toplevel, parent,
						   name, ev);
	if (BONOBO_EX (ev))
		return CORBA_OBJECT_NIL;

	bonobo_object_release_unref (toplevel, ev);

	if (BONOBO_EX (ev))
		return CORBA_OBJECT_NIL;

	return moniker;
}

/**
 * bonobo_moniker_util_get_parent_name:
 * @moniker: the moniker
 * @ev: a corba exception environment
 * 
 *  This gets the display name of the parent moniker ( recursively
 * all of the parents of this moniker ).
 * 
 * Return value: the display name; use CORBA_free to release it.
 **/
CORBA_char *
bonobo_moniker_util_get_parent_name (Bonobo_Moniker     moniker,
				     CORBA_Environment *ev)
{
	Bonobo_Moniker parent;
	CORBA_char    *name;

	g_return_val_if_fail (ev != NULL, NULL);
	g_return_val_if_fail (moniker != CORBA_OBJECT_NIL, NULL);

	parent = Bonobo_Moniker__get_parent (moniker, ev);

	if (BONOBO_EX (ev) ||
	    parent == CORBA_OBJECT_NIL)
		return NULL;
	
	name = Bonobo_Moniker_getDisplayName (parent, ev);

	if (BONOBO_EX (ev))
		name = NULL;

	bonobo_object_release_unref (parent, ev);

	return name;
}

/**
 * bonobo_moniker_util_qi_return:
 * @object: the unknown to query
 * @requested_interface: the desired interface
 * @ev: a corba exception environment 
 * 
 *  A helper function to share code from the end of a resolve
 * implementation; this ensures that the returned object is of
 * the correct interface by doing a queryInterface on the object.
 * 
 * Return value: an handle to the requested interface
 **/
Bonobo_Unknown
bonobo_moniker_util_qi_return (Bonobo_Unknown     object,
			       const CORBA_char  *requested_interface,
			       CORBA_Environment *ev)
{
	Bonobo_Unknown retval = CORBA_OBJECT_NIL;

	if (BONOBO_EX (ev))
		return CORBA_OBJECT_NIL;
	
	if (object == CORBA_OBJECT_NIL) {
		g_warning ("Object is NIL");
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Moniker_InterfaceNotFound, NULL);
		return CORBA_OBJECT_NIL;
	}

	retval = Bonobo_Unknown_queryInterface (
		object, requested_interface, ev);

	if (BONOBO_EX (ev))
		goto release_unref_object;
	
	if (retval == CORBA_OBJECT_NIL) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Moniker_InterfaceNotFound, NULL);
		goto release_unref_object;
	}

 release_unref_object:	
	bonobo_object_release_unref (object, ev);

	if (retval != CORBA_OBJECT_NIL)
		return CORBA_Object_duplicate (retval, ev);
	else
		return CORBA_OBJECT_NIL;
}

/**
 * bonobo_moniker_util_seek_std_separator:
 * @str: the string to scan
 * @min_idx: the minimum offset at which a separator can be found.
 * 
 *  This looks for a standard separator in a moniker's
 * display name string. Most monikers will want to use
 * standard separators.
 *
 *  See also bonobo_moniker_util_escape
 * 
 * Return value: the position of the standard separator, or a
 * pointer to the end of the string.
 **/
int
bonobo_moniker_util_seek_std_separator (const CORBA_char *str,
					int               min_idx)
{
	int i;

	g_return_val_if_fail (str != NULL, 0);
	g_return_val_if_fail (min_idx >= 0, 0);

	for (i = 0; i < min_idx; i++) {
		if (!str [i]) {
			g_warning ("Serious separator error, seeking in '%s' < %d",
				   str, min_idx);
			return i;
		}
	}

	for (; str [i]; i++) {

		if (str [i] == '\\' && str [i + 1])
			i++;
		else if (str [i] == '!' ||
			 str [i] == '#')
			break;
	}
	
	return i;
}

/**
 * bonobo_moniker_util_escape:
 * @string: an unescaped string
 * @offset: an offset of characters to ignore
 * 
 *  Escapes possible separator characters inside a moniker
 * these include '!' and '#', the '\' escaping character is
 * used.
 * 
 * Return value: an escaped sub-string.
 **/
char *
bonobo_moniker_util_escape (const char *string, int offset)
{
	gchar *escaped, *p;
	guint  backslashes = 0;
	int    i, len;

	g_return_val_if_fail (string != NULL, NULL);

	len = strlen (string);
	g_return_val_if_fail (offset < len, NULL);

	for (i = offset; i < len; i++) {
		if (string [i] == '\0')
			break;
		else if (string [i] == '\\' ||
			 string [i] == '#'  ||
			 string [i] == '!')
			backslashes ++;
	}
	
	if (!backslashes)
		return g_strdup (&string [offset]);

	p = escaped = g_new (gchar, len - offset + backslashes + 1);

	for (i = offset; i < len; i++) {
		if (string [i] == '\\' ||
		    string [i] == '#'  ||
		    string [i] == '!')
			*p++ = '\\';
		*p++ = string [i];
	}
	*p = '\0';

	return escaped;
}

/**
 * bonobo_moniker_util_unescape:
 * @string: a string
 * @num_chars: the number of chars to process.
 * 
 *  This routine strips @num_chars: from the start of
 * @string, discards the rest, and proceeds to un-escape
 * characters escaped with '\'.
 * 
 * Return value: the unescaped sub string.
 **/
char *
bonobo_moniker_util_unescape (const char *string, int num_chars)
{
	gchar *escaped, *p;
	guint  backslashes = 0;
	int    i;

	g_return_val_if_fail (string != NULL, NULL);

	for (i = 0; i < num_chars; i++) {
		if (string [i] == '\0')
			break;
		else if (string [i] == '\\') {
			if (string [i + 1] == '\\')
				i++;
			backslashes ++;
		}
	}

	if (!backslashes)
		return g_strndup (string, num_chars);

	p = escaped = g_new (gchar, strlen (string) - backslashes + 1);

	for (i = 0; i < num_chars; i++) {
		if (string [i] == '\\') {
			if (!string [++i])
				break;
			*p++ = string [i];
		} else
			*p++ = string [i];
	}
	*p = '\0';

	return escaped;
}

/**
 * bonobo_moniker_client_new_from_name:
 * @name: the display name of a moniker
 * @ev: a corba exception environment 
 * 
 *  This routine tries to parse a Moniker in string form
 *
 * eg. file:/tmp/a.tar.gz#gzip:#tar:
 *
 * into a CORBA_Object representation of this that can
 * later be resolved against an interface.
 * 
 * Return value: a new Moniker handle
 **/
Bonobo_Moniker
bonobo_moniker_client_new_from_name (const CORBA_char  *name,
				     CORBA_Environment *ev)
{
	return bonobo_moniker_util_new_from_name_full (
		CORBA_OBJECT_NIL, name, ev);
}

/**
 * bonobo_moniker_client_get_name:
 * @moniker: a moniker handle
 * @ev: a corba exception environment 
 * 
 * Return value: the display name of the moniker.
 **/
CORBA_char *
bonobo_moniker_client_get_name (Bonobo_Moniker     moniker,
				CORBA_Environment *ev)
{
	CORBA_char *name;

	g_return_val_if_fail (ev != NULL, NULL);
	g_return_val_if_fail (moniker != CORBA_OBJECT_NIL, NULL);

	name = Bonobo_Moniker_getDisplayName (moniker, ev);

	if (BONOBO_EX (ev))
		return NULL;

	return name;
}

static void
init_default_resolve_options (Bonobo_ResolveOptions *options)
{
	options->flags = 0;
	options->timeout = -1;
}

/**
 * bonobo_moniker_client_resolve_default:
 * @moniker: a moniker
 * @interface_name: the name of the interface we want returned as the result 
 * @ev: a corba exception environment 
 * 
 *  This resolves the moniker object against the given interface,
 * with a default set of resolve options.
 * 
 * Return value: the interfaces resolved to or CORBA_OBJECT_NIL
 **/
Bonobo_Unknown
bonobo_moniker_client_resolve_default (Bonobo_Moniker     moniker,
				       const char        *interface_name,
				       CORBA_Environment *ev)
{
	Bonobo_ResolveOptions options;
	Bonobo_Unknown        retval;
	char                 *real_if;
	
	g_return_val_if_fail (interface_name != NULL, CORBA_OBJECT_NIL);
	g_return_val_if_fail (moniker != CORBA_OBJECT_NIL, CORBA_OBJECT_NIL);

	real_if = get_full_interface_name (interface_name);

	init_default_resolve_options (&options);

	retval = Bonobo_Moniker_resolve (moniker, &options, real_if, ev);

	g_free (real_if);

	return retval;
}

/**
 * bonobo_moniker_client_resolve_client_default:
 * @moniker: the moniker
 * @interface_name: the name of the interface we want returned as the result 
 * @ev: a corba exception environment 
 * 
 * See: bonobo_moniker_client_resolve_default; however this version returns
 * a BonoboObjectClient wrapped reference.
 * 
 * Return value: a BonoboObjectClient style reference.
 **/
BonoboObjectClient *
bonobo_moniker_client_resolve_client_default (Bonobo_Moniker     moniker,
					      const char        *interface_name,
					      CORBA_Environment *ev)
{
	Bonobo_Unknown unknown;

	g_return_val_if_fail (ev != NULL, NULL);

	unknown = bonobo_moniker_client_resolve_default (
		moniker, interface_name, ev);

	if (BONOBO_EX (ev))
		return NULL;

	if (unknown == CORBA_OBJECT_NIL)
		return NULL;

	return bonobo_object_client_from_corba (unknown);
}

/**
 * bonobo_get_object:
 * @name: the display name of a moniker
 * @interface_name: the name of the interface we want returned as the result 
 * @ev: a corba exception environment 
 * 
 *  This encapsulates both the parse stage and resolve process of using
 * a moniker, providing a simple VisualBasic like mechanism for using the
 * object name space.
 * 
 * Return value: the requested interface or CORBA_OBJECT_NIL
 **/
Bonobo_Unknown
bonobo_get_object (const CORBA_char *name,
		   const char        *interface_name,
		   CORBA_Environment *ev)
{
	Bonobo_Moniker moniker;
	Bonobo_Unknown retval = CORBA_OBJECT_NIL;

	moniker = bonobo_moniker_client_new_from_name (name, ev);

	if (BONOBO_EX (ev)) 
		retval = CORBA_OBJECT_NIL;
	else {
		retval = bonobo_moniker_client_resolve_default (
			moniker, interface_name, ev);

		bonobo_object_release_unref (moniker, ev);
		
		if (BONOBO_EX (ev))
			retval = CORBA_OBJECT_NIL;
	}

/*	syslog (LOG_WARNING, "Bonobo: get_object '%s' '%s' returns %p ex '%s'",
		name ? name : "<null>", interface_name ? interface_name : "<null>",
		retval, ev->_major == CORBA_NO_EXCEPTION ? "<no exception>" : 
		bonobo_exception_get_text (ev)); */

	return retval;
}

typedef struct {
	char                *name;
	BonoboMonikerAsyncFn cb;
	gpointer             user_data;
	guint                timeout_msec;
	Bonobo_Unknown       moniker;
} parse_async_ctx_t;

static void
parse_async_ctx_free (parse_async_ctx_t *ctx)
{
	if (ctx) {
		g_free (ctx->name);
		g_free (ctx);
	}
}

static void
async_parse_cb (BonoboAsyncReply  *reply,
		CORBA_Environment *ev,
		gpointer           user_data)
{
	parse_async_ctx_t *ctx = user_data;

	if (BONOBO_EX (ev))
		ctx->cb (CORBA_OBJECT_NIL, ev, ctx->user_data);
	else {
		Bonobo_Moniker retval;

		bonobo_async_demarshal (reply, &retval, NULL);

		ctx->cb (retval, ev, ctx->user_data);
	}

	bonobo_object_release_unref (ctx->moniker, ev);
	parse_async_ctx_free (ctx);
}

static void
async_activation_cb (CORBA_Object activated_object, 
		     const char  *error_reason, 
		     gpointer     user_data)
{
	parse_async_ctx_t *ctx = user_data;
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	if (error_reason) { /* badly designed oaf interface */

		CORBA_exception_set (&ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Moniker_UnknownPrefix, NULL);

		ctx->cb (CORBA_OBJECT_NIL, &ev, ctx->user_data);
		parse_async_ctx_free (ctx);
	} else {
		ctx->moniker = Bonobo_Unknown_queryInterface (
			activated_object, "IDL:Bonobo/Moniker:1.0", &ev);

		if (BONOBO_EX (&ev)) {
			ctx->cb (CORBA_OBJECT_NIL, &ev, ctx->user_data);
			parse_async_ctx_free (ctx);
		
		} else if (ctx->moniker == CORBA_OBJECT_NIL) {
			CORBA_exception_set (&ev, CORBA_USER_EXCEPTION,
					     ex_Bonobo_Moniker_InterfaceNotFound, NULL);
			ctx->cb (CORBA_OBJECT_NIL, &ev, ctx->user_data);
			parse_async_ctx_free (ctx);
		} else {
			static const BonoboAsyncArg arguments [] = {
				{ TC_Object, BONOBO_ASYNC_IN },
				{ TC_string, BONOBO_ASYNC_IN },
				{ NULL }
			};
			static const CORBA_TypeCode exceptions [] = {
				TC_Bonobo_Moniker_InvalidSyntax,
				TC_Bonobo_Moniker_UnknownPrefix,
				NULL
			};
			static const BonoboAsyncMethod method = {
				"parseDisplayName", 
				TC_Object, 
				arguments,
				exceptions
			};
			CORBA_Object obj = CORBA_OBJECT_NIL;
			gpointer arg_values [2] = { &obj, &ctx->name };
	
			bonobo_async_invoke (&method, async_parse_cb, ctx,
					     ctx->timeout_msec,
					     ctx->moniker, arg_values, &ev);
			
			if (BONOBO_EX (&ev)) {
				ctx->cb (CORBA_OBJECT_NIL, &ev, ctx->user_data);
				parse_async_ctx_free (ctx);
			}

			bonobo_object_release_unref (activated_object, &ev);
		}
	}

	CORBA_exception_free (&ev);
}

/**
 * bonobo_moniker_client_new_from_name_async:
 * @name: the name
 * @ev: a corba exception environment 
 * @timeout_msec: the timeout in milliseconds 
 * @cb: the async callback that gets the response
 * @user_data: user context data to pass to that callback
 * 
 * An asynchronous version of new_from_name
 **/
void
bonobo_moniker_client_new_from_name_async (const CORBA_char    *name,
					   CORBA_Environment   *ev,
					   guint                timeout_msec,
					   BonoboMonikerAsyncFn cb,
					   gpointer             user_data)
{
	parse_async_ctx_t *ctx;
	const char        *iid;

	g_return_if_fail (ev != NULL);
	g_return_if_fail (cb != NULL);
	g_return_if_fail (name != NULL);

	if (!name [0]) {
		cb (CORBA_OBJECT_NIL, ev, user_data);
		return;
	}

	if (name [0] == '#')
		name++;

	ctx = g_new0 (parse_async_ctx_t, 1);
	ctx->name         = g_strdup (name);
	ctx->cb           = cb;
	ctx->user_data    = user_data;
	ctx->timeout_msec = timeout_msec;
	ctx->moniker      = CORBA_OBJECT_NIL;

	if (!(iid = moniker_id_from_nickname (name))) {
		char *query;

		query = query_from_name (name);

		oaf_activate_async (query, NULL, 0,
				    async_activation_cb, ctx, ev);

		g_free (query);
	} else
		oaf_activate_from_id_async ((gchar *) iid, 0,
					    async_activation_cb, ctx, ev);
}

typedef struct {
	Bonobo_Moniker       moniker;
	BonoboMonikerAsyncFn cb;
	gpointer             user_data;
} resolve_async_ctx_t;

static void
resolve_async_cb (BonoboAsyncReply  *handle,
		  CORBA_Environment *ev,
		  gpointer           user_data)
{
	resolve_async_ctx_t *ctx = user_data;

	if (BONOBO_EX (ev))
		ctx->cb (CORBA_OBJECT_NIL, ev, ctx->user_data);
	else {
		Bonobo_Unknown object;
		bonobo_async_demarshal (handle, &object, NULL);
		ctx->cb (object, ev, ctx->user_data);
	}

	bonobo_object_release_unref (ctx->moniker, ev);
	g_free (ctx);
}

/**
 * bonobo_moniker_resolve_async:
 * @moniker: the moniker to resolve
 * @options: resolve options
 * @interface_name: the name of the interface we want returned as the result 
 * @ev: a corba exception environment 
 * @timeout_msec: the timeout in milliseconds 
 * @cb: the async callback that gets the response 
 * @user_data: user context data to pass to that callback 
 * 
 * An async version of bonobo_moniker_client_resolve
 **/
void
bonobo_moniker_resolve_async (Bonobo_Moniker         moniker,
			      Bonobo_ResolveOptions *options,
			      const char            *interface_name,
			      CORBA_Environment     *ev,
			      guint                  timeout_msec,
			      BonoboMonikerAsyncFn   cb,
			      gpointer               user_data)
{
	static const BonoboAsyncArg arguments [] = {
		{ TC_Bonobo_ResolveOptions, BONOBO_ASYNC_IN },
		{ TC_string,                BONOBO_ASYNC_IN },
		{ NULL }
	};
	static const CORBA_TypeCode exceptions [] = {
		TC_Bonobo_Moniker_InterfaceNotFound,
		TC_Bonobo_Moniker_UnknownPrefix,
		NULL
	};
	static const BonoboAsyncMethod method = {
		"resolve", 
		TC_Object, 
		arguments,
		exceptions
	};
	gpointer arg_values [2] = { &options, &interface_name };
	resolve_async_ctx_t *ctx;
	
	g_return_if_fail (ev != NULL);
	g_return_if_fail (cb != NULL);
	g_return_if_fail (moniker != CORBA_OBJECT_NIL);
	g_return_if_fail (options != CORBA_OBJECT_NIL);
	g_return_if_fail (interface_name != CORBA_OBJECT_NIL);

	ctx = g_new0 (resolve_async_ctx_t, 1);
	ctx->cb = cb;
	ctx->user_data = user_data;
	ctx->moniker = bonobo_object_dup_ref (moniker, ev);

	bonobo_async_invoke (&method, resolve_async_cb, ctx,
			     timeout_msec, ctx->moniker, arg_values, ev);
}

/**
 * bonobo_moniker_resolve_async_default:
 * @moniker: 
 * @interface_name: the name of the interface we want returned as the result 
 * @ev: a corba exception environment 
 * @timeout_msec: the timeout in milliseconds 
 * @cb: the async callback that gets the response 
 * @user_data: user context data to pass to that callback 
 * 
 * An async version of bonobo_moniker_client_resolve_default
 **/
void
bonobo_moniker_resolve_async_default (Bonobo_Moniker       moniker,
				      const char          *interface_name,
				      CORBA_Environment   *ev,
				      guint                timeout_msec,
				      BonoboMonikerAsyncFn cb,
				      gpointer             user_data)
{
	Bonobo_ResolveOptions options;

	g_return_if_fail (ev != NULL);
	g_return_if_fail (cb != NULL);
	g_return_if_fail (moniker != CORBA_OBJECT_NIL);
	g_return_if_fail (interface_name != CORBA_OBJECT_NIL);

	init_default_resolve_options (&options);

	bonobo_moniker_resolve_async (moniker, &options, interface_name,
				      ev, timeout_msec, cb, user_data);
}


typedef struct {
	guint                timeout_msec;
	char                *interface_name;
	BonoboMonikerAsyncFn cb;
	gpointer             user_data;
} get_object_async_ctx_t;

static void
get_object_async_ctx_free (get_object_async_ctx_t *ctx)
{
	if (ctx) {
		g_free (ctx->interface_name);
		g_free (ctx);
	}
}

static void
get_async2_cb (Bonobo_Unknown     object,
	       CORBA_Environment *ev,
	       gpointer           user_data)
{
	get_object_async_ctx_t *ctx = user_data;

	ctx->cb (object, ev, ctx->user_data);

	get_object_async_ctx_free (ctx);
}	

static void
get_async1_cb (Bonobo_Unknown     object,
	       CORBA_Environment *ev,
	       gpointer           user_data)
{
	get_object_async_ctx_t *ctx = user_data;

	if (BONOBO_EX (ev)) {
		ctx->cb (CORBA_OBJECT_NIL, ev, ctx->user_data);
		get_object_async_ctx_free (ctx);
	} else {
                bonobo_moniker_resolve_async_default (
			object, ctx->interface_name, ev,
			ctx->timeout_msec, get_async2_cb, ctx);

		if (BONOBO_EX (ev)) {
			ctx->cb (CORBA_OBJECT_NIL, ev, ctx->user_data);
			get_object_async_ctx_free (ctx);
		}
	}
}	

/**
 * bonobo_get_object_async:
 * @name: 
 * @interface_name: the name of the interface we want returned as the result 
 * @ev: a corba exception environment 
 * @timeout_msec: the timeout in milliseconds 
 * @cb: the async callback that gets the response 
 * @user_data: user context data to pass to that callback 
 * 
 * An async version of bonobo_get_object
 **/
void
bonobo_get_object_async (const CORBA_char    *name,
			 const char          *interface_name,
			 CORBA_Environment   *ev,
			 guint                timeout_msec,
			 BonoboMonikerAsyncFn cb,
			 gpointer             user_data)
{
	get_object_async_ctx_t *ctx;

	g_return_if_fail (ev != NULL);
	g_return_if_fail (cb != NULL);
	g_return_if_fail (name != NULL);
	g_return_if_fail (interface_name != NULL);

	ctx = g_new0 (get_object_async_ctx_t, 1);
	ctx->cb = cb;
	ctx->user_data = user_data;
	ctx->interface_name = get_full_interface_name (interface_name);
	ctx->timeout_msec = timeout_msec;

	bonobo_moniker_client_new_from_name_async (
		name, ev, timeout_msec, get_async1_cb, ctx);
}

/**
 * bonobo_moniker_client_equal:
 * @moniker: The moniker
 * @name: a display name eg. file:/demo/a.jpeg
 * @opt_ev: optional CORBA_Environment or NULL.
 * 
 * Compare a full @moniker with the given @name
 * 
 * Return value: TRUE if they are the same
 **/
gboolean
bonobo_moniker_client_equal (Bonobo_Moniker     moniker,
			     const CORBA_char  *name,
			     CORBA_Environment *opt_ev)
{
	CORBA_long l;
	CORBA_Environment *real_ev, tmp_ev;
	
	g_return_val_if_fail (name != NULL, FALSE);
	g_return_val_if_fail (moniker != CORBA_OBJECT_NIL, FALSE);

	if (opt_ev)
		real_ev = opt_ev;
	else {
		CORBA_exception_init (&tmp_ev);
		real_ev = &tmp_ev;
	}

	l = Bonobo_Moniker_equal (moniker, name, real_ev);

	if (BONOBO_EX (real_ev))
		l = 0;

	if (!opt_ev)
		CORBA_exception_free (&tmp_ev);

	return l != 0;
}

static CosNaming_NamingContext
lookup_naming_context (GList *path,
		       CORBA_Environment *ev)
{
	CosNaming_NamingContext ns, ctx, new_ctx;
	CosNaming_Name *cn;
	GList          *l;

	g_return_val_if_fail (path != NULL, CORBA_OBJECT_NIL);
	g_return_val_if_fail (path->data != NULL, CORBA_OBJECT_NIL);
	g_return_val_if_fail (ev != NULL, CORBA_OBJECT_NIL);

	ns =  oaf_name_service_get (ev);
	if (BONOBO_EX (ev) || ns == CORBA_OBJECT_NIL)
		return CORBA_OBJECT_NIL;

	ctx = ns;

	for (l = path; l != NULL; l = l->next) {

		cn =  ORBit_string_to_CosNaming_Name (l->data, ev);
		if (BONOBO_EX (ev) || !cn)
			break;

		new_ctx = CosNaming_NamingContext_resolve (ctx, cn, ev);
		if (BONOBO_USER_EX (ev, ex_CosNaming_NamingContext_NotFound)) {
			CORBA_exception_init (ev);
			new_ctx = CosNaming_NamingContext_bind_new_context (
				ctx, cn, ev);
			if (BONOBO_EX (ev) || new_ctx == CORBA_OBJECT_NIL) {
				CORBA_free (cn);
				break;
			}
		}

		CORBA_free (cn);

		if (BONOBO_EX (ev))
			new_ctx = CORBA_OBJECT_NIL;
		
		CORBA_Object_release (ctx, ev);

		ctx = new_ctx;

		if (!ctx)
			break;
	}

	return ctx;
}

static CosNaming_Name *
url_to_name (char *url, char *opt_kind)
{
	LName ln;
	LNameComponent lnc;
	CosNaming_Name *retval;
	CORBA_Environment ev;

	g_return_val_if_fail (url != NULL, NULL);

	CORBA_exception_init (&ev);

	lnc = create_lname_component ();
	LNameComponent_set_id (lnc, url, &ev);
	
	if (opt_kind)
		LNameComponent_set_kind (lnc, opt_kind, &ev);

	ln = create_lname ();
	LName_insert_component (ln, 1, lnc, &ev);

	retval = LName_to_idl_form (ln, &ev);

	LName_destroy (ln, &ev);

	CORBA_exception_free (&ev);

	return retval;
}

static CosNaming_NamingContext
get_url_context (char *oafiid,
		 CORBA_Environment *ev)
{
	CosNaming_NamingContext  ctx = NULL;
	GList                   *path = NULL;

	path = g_list_append (path, "GNOME");
	path = g_list_append (path, "URL");
	path = g_list_append (path, oafiid);

	ctx = lookup_naming_context (path, ev);

	g_list_free (path);
		
	return ctx;
}

void
bonobo_url_register (char              *oafiid, 
		     char              *url, 
		     char              *mime_type,
		     Bonobo_Unknown     object,
		     CORBA_Environment *ev)
{
	CosNaming_NamingContext  ctx = NULL;
	CosNaming_Name          *cn;

	bonobo_return_if_fail (oafiid != NULL, ev);
	bonobo_return_if_fail (url != NULL, ev);
	bonobo_return_if_fail (object != CORBA_OBJECT_NIL, ev);
	
	ctx = get_url_context (oafiid, ev);
		
	if (BONOBO_EX (ev) || ctx == CORBA_OBJECT_NIL)
		return;
	
	cn = url_to_name (url, mime_type);

	CosNaming_NamingContext_bind (ctx, cn, object, ev);

	CORBA_free (cn);

	CORBA_Object_release (ctx, NULL);
}

void
bonobo_url_unregister (char              *oafiid, 
		       char              *url,
		       CORBA_Environment *ev)
{
	CosNaming_NamingContext  ctx = NULL;
	CosNaming_Name          *cn;

	bonobo_return_if_fail (oafiid != NULL, ev);
	bonobo_return_if_fail (url != NULL, ev);

	ctx = get_url_context (oafiid, ev);
		
	if (BONOBO_EX (ev) || ctx == CORBA_OBJECT_NIL)
		return;
	
	cn = url_to_name (url, NULL);

	CosNaming_NamingContext_unbind (ctx, cn, ev);

	CORBA_free (cn);

	CORBA_Object_release (ctx, NULL);
}

Bonobo_Unknown
bonobo_url_lookup (char              *oafiid, 
		   char              *url,
		   CORBA_Environment *ev)
{
	CosNaming_NamingContext  ctx = NULL;
	CosNaming_Name          *cn;
	Bonobo_Unknown           retval;

	bonobo_return_val_if_fail (oafiid != NULL, CORBA_OBJECT_NIL, ev);
	bonobo_return_val_if_fail (url != NULL, CORBA_OBJECT_NIL, ev);

	ctx = get_url_context (oafiid, ev);
		
	if (BONOBO_EX (ev) || ctx == CORBA_OBJECT_NIL)
		return CORBA_OBJECT_NIL;
	
	cn = url_to_name (url, NULL);

	retval = CosNaming_NamingContext_resolve (ctx, cn, ev);

	CORBA_free (cn);

	CORBA_Object_release (ctx, NULL);

	return retval;
}

