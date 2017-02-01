
/* GConf
 * Copyright (C) 1999, 2000 Red Hat Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <config.h>
#include "gconf-internals.h"
#include "gconf-backend.h"
#include "gconf-schema.h"
#include "gconf.h"
#include <orb/orbit.h>
#include <liboaf/liboaf.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <locale.h>
#include <time.h>
#include <math.h>

#define g_utf8_skip gconf_g_utf8_skip

typedef void (* GSpawnChildSetupFunc) (gpointer user_data);

typedef enum
{
  G_SPAWN_LEAVE_DESCRIPTORS_OPEN = 1 << 0,
  G_SPAWN_DO_NOT_REAP_CHILD      = 1 << 1,
  /* look for argv[0] in the path i.e. use execvp() */
  G_SPAWN_SEARCH_PATH            = 1 << 2,
  /* Dump output to /dev/null */
  G_SPAWN_STDOUT_TO_DEV_NULL     = 1 << 3,
  G_SPAWN_STDERR_TO_DEV_NULL     = 1 << 4,
  G_SPAWN_CHILD_INHERITS_STDIN   = 1 << 5,
  G_SPAWN_FILE_AND_ARGV_ZERO     = 1 << 6
} GSpawnFlags;

static gboolean g_spawn_async (const gchar           *working_directory,
                               gchar                **argv,
                               gchar                **envp,
                               GSpawnFlags            flags,
                               GSpawnChildSetupFunc   child_setup,
                               gpointer               user_data,
                               gint                  *child_pid,
                               GError               **error);

gboolean gconf_log_debug_messages = FALSE;

static gboolean gconf_daemon_mode = FALSE;
static gchar* daemon_ior = NULL;

void
gconf_set_daemon_mode(gboolean setting)
{
  gconf_daemon_mode = setting;
}

gboolean
gconf_in_daemon_mode(void)
{
  return gconf_daemon_mode;
}

void
gconf_set_daemon_ior(const gchar* ior)
{
  if (daemon_ior != NULL)
    {
      g_free(daemon_ior);
      daemon_ior = NULL;
    }
      
  if (ior != NULL)
    daemon_ior = g_strdup(ior);
}

const gchar*
gconf_get_daemon_ior(void)
{
  return daemon_ior;
}

gchar*
gconf_key_directory  (const gchar* key)
{
  const gchar* end;
  gchar* retval;
  int len;

  end = strrchr(key, '/');

  if (end == NULL)
    {
      gconf_log(GCL_ERR, _("No '/' in key `%s'"), key);
      return NULL;
    }

  len = end-key+1;

  if (len == 1)
    {
      /* Root directory */
      retval = g_strdup("/");
    }
  else 
    {
      retval = g_malloc(len);
      
      strncpy(retval, key, len);
      
      retval[len-1] = '\0';
    }

  return retval;
}

const gchar*
gconf_key_key        (const gchar* key)
{
  const gchar* end;
  
  end = strrchr(key, '/');

  ++end;

  return end;
}

/*
 *  Random stuff 
 */

gboolean
gconf_file_exists (const gchar* filename)
{
  struct stat s;
  
  g_return_val_if_fail (filename != NULL,FALSE);
  
  return stat (filename, &s) == 0;
}

gboolean
gconf_file_test(const gchar* filename, int test)
{
  struct stat s;
  if(stat (filename, &s) != 0)
    return FALSE;
  if(!(test & GCONF_FILE_ISFILE) && S_ISREG(s.st_mode))
    return FALSE;
  if(!(test & GCONF_FILE_ISLINK) && S_ISLNK(s.st_mode))
    return FALSE;
  if(!(test & GCONF_FILE_ISDIR) && S_ISDIR(s.st_mode))
    return FALSE;
  return TRUE;
}

GConfValue* 
gconf_value_from_corba_value(const ConfigValue* value)
{
  GConfValue* gval;
  GConfValueType type = GCONF_VALUE_INVALID;
  
  switch (value->_d)
    {
    case InvalidVal:
      return NULL;
      break;
    case IntVal:
      type = GCONF_VALUE_INT;
      break;
    case StringVal:
      type = GCONF_VALUE_STRING;
      break;
    case FloatVal:
      type = GCONF_VALUE_FLOAT;
      break;
    case BoolVal:
      type = GCONF_VALUE_BOOL;
      break;
    case SchemaVal:
      type = GCONF_VALUE_SCHEMA;
      break;
    case ListVal:
      type = GCONF_VALUE_LIST;
      break;
    case PairVal:
      type = GCONF_VALUE_PAIR;
      break;
    default:
      gconf_log(GCL_DEBUG, "Invalid type in %s", G_GNUC_FUNCTION);
      return NULL;
    }

  g_assert(GCONF_VALUE_TYPE_VALID(type));
  
  gval = gconf_value_new(type);

  switch (gval->type)
    {
    case GCONF_VALUE_INT:
      gconf_value_set_int(gval, value->_u.int_value);
      break;
    case GCONF_VALUE_STRING:
      if (!g_utf8_validate (value->_u.string_value, -1, NULL))
        {
          gconf_log (GCL_ERR, _("Invalid UTF-8 in string value in '%s'"),
                     value->_u.string_value); 
        }
      else
        {
          gconf_value_set_string(gval, value->_u.string_value);
        }
      break;
    case GCONF_VALUE_FLOAT:
      gconf_value_set_float(gval, value->_u.float_value);
      break;
    case GCONF_VALUE_BOOL:
      gconf_value_set_bool(gval, value->_u.bool_value);
      break;
    case GCONF_VALUE_SCHEMA:
      gconf_value_set_schema_nocopy(gval, 
                                    gconf_schema_from_corba_schema(&(value->_u.schema_value)));
      break;
    case GCONF_VALUE_LIST:
      {
        GSList* list = NULL;
        guint i = 0;
        
        switch (value->_u.list_value.list_type)
          {
          case BIntVal:
            gconf_value_set_list_type(gval, GCONF_VALUE_INT);
            break;
          case BBoolVal:
            gconf_value_set_list_type(gval, GCONF_VALUE_BOOL);
            break;
          case BFloatVal:
            gconf_value_set_list_type(gval, GCONF_VALUE_FLOAT);
            break;
          case BStringVal:
            gconf_value_set_list_type(gval, GCONF_VALUE_STRING);
            break;
          case BInvalidVal:
            break;
          default:
            g_warning("Bizarre list type in %s", G_GNUC_FUNCTION);
            break;
          }

        if (gconf_value_get_list_type(gval) != GCONF_VALUE_INVALID)
          {
            i = 0;
            while (i < value->_u.list_value.seq._length)
              {
                GConfValue* val;
                
                /* This is a bit dubious; we cast a ConfigBasicValue to ConfigValue
                   because they have the same initial members, but by the time
                   the CORBA and C specs kick in, not sure we are guaranteed
                   to be able to do this.
                */
                val = gconf_value_from_corba_value((ConfigValue*)&value->_u.list_value.seq._buffer[i]);
                
                if (val == NULL)
                  gconf_log(GCL_ERR, _("Couldn't interpret CORBA value for list element"));
                else if (val->type != gconf_value_get_list_type(gval))
                  gconf_log(GCL_ERR, _("Incorrect type for list element in %s"), G_GNUC_FUNCTION);
                else
                  list = g_slist_prepend(list, val);
                
                ++i;
              }
        
            list = g_slist_reverse(list);
            
            gconf_value_set_list_nocopy(gval, list);
          }
        else
          {
            gconf_log(GCL_ERR, _("Received list from gconfd with a bad list type"));
          }
      }
      break;
    case GCONF_VALUE_PAIR:
      {
        g_return_val_if_fail(value->_u.pair_value._length == 2, gval);
        
        gconf_value_set_car_nocopy(gval,
                                   gconf_value_from_corba_value((ConfigValue*)&value->_u.list_value.seq._buffer[0]));

        gconf_value_set_cdr_nocopy(gval,
                                   gconf_value_from_corba_value((ConfigValue*)&value->_u.list_value.seq._buffer[1]));
      }
      break;
    default:
      g_assert_not_reached();
      break;
    }
  
  return gval;
}

void          
fill_corba_value_from_gconf_value(GConfValue* value, 
                                   ConfigValue* cv)
{
  if (value == NULL)
    {
      cv->_d = InvalidVal;
      return;
    }

  switch (value->type)
    {
    case GCONF_VALUE_INT:
      cv->_d = IntVal;
      cv->_u.int_value = gconf_value_get_int(value);
      break;
    case GCONF_VALUE_STRING:
      cv->_d = StringVal;
      cv->_u.string_value = CORBA_string_dup((char*)gconf_value_get_string(value));
      break;
    case GCONF_VALUE_FLOAT:
      cv->_d = FloatVal;
      cv->_u.float_value = gconf_value_get_float(value);
      break;
    case GCONF_VALUE_BOOL:
      cv->_d = BoolVal;
      cv->_u.bool_value = gconf_value_get_bool(value);
      break;
    case GCONF_VALUE_SCHEMA:
      cv->_d = SchemaVal;
      fill_corba_schema_from_gconf_schema(gconf_value_get_schema(value),
                                           &cv->_u.schema_value);
      break;
    case GCONF_VALUE_LIST:
      {
        guint n, i;
        GSList* list;
        
        cv->_d = ListVal;

        list = gconf_value_get_list(value);

        n = g_slist_length(list);

        cv->_u.list_value.seq._buffer =
          CORBA_sequence_ConfigBasicValue_allocbuf(n);
        cv->_u.list_value.seq._length = n;
        cv->_u.list_value.seq._maximum = n;
        CORBA_sequence_set_release(&cv->_u.list_value.seq, TRUE);
        
        switch (gconf_value_get_list_type(value))
          {
          case GCONF_VALUE_INT:
            cv->_u.list_value.list_type = BIntVal;
            break;

          case GCONF_VALUE_BOOL:
            cv->_u.list_value.list_type = BBoolVal;
            break;
            
          case GCONF_VALUE_STRING:
            cv->_u.list_value.list_type = BStringVal;
            break;

          case GCONF_VALUE_FLOAT:
            cv->_u.list_value.list_type = BFloatVal;
            break;

          case GCONF_VALUE_SCHEMA:
            cv->_u.list_value.list_type = BSchemaVal;
            break;
            
          default:
            cv->_u.list_value.list_type = BInvalidVal;
            gconf_log(GCL_DEBUG, "Invalid list type in %s", G_GNUC_FUNCTION);
            break;
          }
        
        i= 0;
        while (list != NULL)
          {
            /* That dubious ConfigBasicValue->ConfigValue cast again */
            fill_corba_value_from_gconf_value((GConfValue*)list->data,
                                               (ConfigValue*)&cv->_u.list_value.seq._buffer[i]);

            list = g_slist_next(list);
            ++i;
          }
      }
      break;
    case GCONF_VALUE_PAIR:
      {
        cv->_d = PairVal;

        cv->_u.pair_value._buffer =
          CORBA_sequence_ConfigBasicValue_allocbuf(2);
        cv->_u.pair_value._length = 2;
        cv->_u.pair_value._maximum = 2;
        CORBA_sequence_set_release(&cv->_u.pair_value, TRUE);
        
        /* dubious cast */
        fill_corba_value_from_gconf_value(gconf_value_get_car(value),
                                           (ConfigValue*)&cv->_u.pair_value._buffer[0]);
        fill_corba_value_from_gconf_value(gconf_value_get_cdr(value),
                                           (ConfigValue*)&cv->_u.pair_value._buffer[1]);
      }
      break;
      
    case GCONF_VALUE_INVALID:
      cv->_d = InvalidVal;
      break;
    default:
      cv->_d = InvalidVal;
      gconf_log(GCL_DEBUG, "Unknown type in %s", G_GNUC_FUNCTION);
      break;
    }
}

ConfigValue*  
corba_value_from_gconf_value(GConfValue* value)
{
  ConfigValue* cv;

  cv = ConfigValue__alloc();

  fill_corba_value_from_gconf_value(value, cv);

  return cv;
}

ConfigValue*  
invalid_corba_value()
{
  ConfigValue* cv;

  cv = ConfigValue__alloc();

  cv->_d = InvalidVal;

  return cv;
}

gchar*
gconf_object_to_string (CORBA_Object obj,
                        GError **err)
{
  CORBA_Environment ev;
  gchar *ior;
  gchar *retval;
  
  CORBA_exception_init (&ev);

  ior = CORBA_ORB_object_to_string (gconf_orb_get (), obj, &ev);

  if (ior == NULL)
    {
      gconf_set_error (err,
                       GCONF_ERROR_FAILED,
                       _("Failed to convert object to IOR"));

      return NULL;
    }

  retval = g_strdup (ior);

  CORBA_free (ior);

  return retval;
}

static ConfigValueType
corba_type_from_gconf_type(GConfValueType type)
{
  switch (type)
    {
    case GCONF_VALUE_INT:
      return IntVal;
    case GCONF_VALUE_BOOL:
      return BoolVal;
    case GCONF_VALUE_FLOAT:
      return FloatVal;
    case GCONF_VALUE_INVALID:
      return InvalidVal;
    case GCONF_VALUE_STRING:
      return StringVal;
    case GCONF_VALUE_SCHEMA:
      return SchemaVal;
    case GCONF_VALUE_LIST:
      return ListVal;
    case GCONF_VALUE_PAIR:
      return PairVal;
    default:
      g_assert_not_reached();
      return InvalidVal;
    }
}

static GConfValueType
gconf_type_from_corba_type(ConfigValueType type)
{
  switch (type)
    {
    case InvalidVal:
      return GCONF_VALUE_INVALID;
    case StringVal:
      return GCONF_VALUE_STRING;
    case IntVal:
      return GCONF_VALUE_INT;
    case FloatVal:
      return GCONF_VALUE_FLOAT;
    case SchemaVal:
      return GCONF_VALUE_SCHEMA;
    case BoolVal:
      return GCONF_VALUE_BOOL;
    case ListVal:
      return GCONF_VALUE_LIST;
    case PairVal:
      return GCONF_VALUE_PAIR;
    default:
      g_assert_not_reached();
      return GCONF_VALUE_INVALID;
    }
}

void          
fill_corba_schema_from_gconf_schema(GConfSchema* sc, 
                                     ConfigSchema* cs)
{
  cs->value_type = corba_type_from_gconf_type(sc->type);
  cs->value_list_type = corba_type_from_gconf_type(sc->list_type);
  cs->value_car_type = corba_type_from_gconf_type(sc->car_type);
  cs->value_cdr_type = corba_type_from_gconf_type(sc->cdr_type);

  cs->locale = CORBA_string_dup(sc->locale ? sc->locale : "");
  cs->short_desc = CORBA_string_dup(sc->short_desc ? sc->short_desc : "");
  cs->long_desc = CORBA_string_dup(sc->long_desc ? sc->long_desc : "");
  cs->owner = CORBA_string_dup(sc->owner ? sc->owner : "");

  {
    gchar* encoded;
    GConfValue* default_val;

    default_val = gconf_schema_get_default_value(sc);

    if (default_val)
      {
        encoded = gconf_value_encode(default_val);

        g_assert(encoded != NULL);

        cs->encoded_default_value = CORBA_string_dup(encoded);

        g_free(encoded);
      }
    else
      cs->encoded_default_value = CORBA_string_dup("");
  }
}

ConfigSchema* 
corba_schema_from_gconf_schema(GConfSchema* sc)
{
  ConfigSchema* cs;

  cs = ConfigSchema__alloc();

  fill_corba_schema_from_gconf_schema(sc, cs);

  return cs;
}

GConfSchema*  
gconf_schema_from_corba_schema(const ConfigSchema* cs)
{
  GConfSchema* sc;
  GConfValueType type = GCONF_VALUE_INVALID;
  GConfValueType list_type = GCONF_VALUE_INVALID;
  GConfValueType car_type = GCONF_VALUE_INVALID;
  GConfValueType cdr_type = GCONF_VALUE_INVALID;

  type = gconf_type_from_corba_type(cs->value_type);
  list_type = gconf_type_from_corba_type(cs->value_list_type);
  car_type = gconf_type_from_corba_type(cs->value_car_type);
  cdr_type = gconf_type_from_corba_type(cs->value_cdr_type);

  sc = gconf_schema_new();

  gconf_schema_set_type(sc, type);
  gconf_schema_set_list_type(sc, list_type);
  gconf_schema_set_car_type(sc, car_type);
  gconf_schema_set_cdr_type(sc, cdr_type);

  if (*cs->locale != '\0')
    {
      if (!g_utf8_validate (cs->locale, -1, NULL))
        gconf_log (GCL_ERR, _("Invalid UTF-8 in locale for schema"));
      else
        gconf_schema_set_locale(sc, cs->locale);
    }

  if (*cs->short_desc != '\0')
    {
      if (!g_utf8_validate (cs->short_desc, -1, NULL))
        gconf_log (GCL_ERR, _("Invalid UTF-8 in short description for schema"));
      else
        gconf_schema_set_short_desc(sc, cs->short_desc);
    }

  if (*cs->long_desc != '\0')
    {
      if (!g_utf8_validate (cs->long_desc, -1, NULL))
        gconf_log (GCL_ERR, _("Invalid UTF-8 in long description for schema"));
      else
        gconf_schema_set_long_desc(sc, cs->long_desc);
    }

  if (*cs->owner != '\0')
    {
      if (!g_utf8_validate (cs->owner, -1, NULL))
        gconf_log (GCL_ERR, _("Invalid UTF-8 in owner for schema"));
      else
        gconf_schema_set_owner(sc, cs->owner);
    }
      
  {
    GConfValue* val;

    val = gconf_value_decode(cs->encoded_default_value);

    if (val)
      gconf_schema_set_default_value_nocopy(sc, val);
  }
  
  return sc;
}

const gchar* 
gconf_value_type_to_string(GConfValueType type)
{
  switch (type)
    {
    case GCONF_VALUE_INT:
      return "int";
      break;
    case GCONF_VALUE_STRING:
      return "string";
      break;
    case GCONF_VALUE_FLOAT:
      return "float";
      break;
    case GCONF_VALUE_BOOL:
      return "bool";
      break;
    case GCONF_VALUE_SCHEMA:
      return "schema";
      break;
    case GCONF_VALUE_LIST:
      return "list";
      break;
    case GCONF_VALUE_PAIR:
      return "pair";
      break;
    case GCONF_VALUE_INVALID:
      return "*invalid*";
      break;
    default:
      g_assert_not_reached();
      return NULL; /* for warnings */
      break;
    }
}

GConfValueType 
gconf_value_type_from_string(const gchar* type_str)
{
  if (strcmp(type_str, "int") == 0)
    return GCONF_VALUE_INT;
  else if (strcmp(type_str, "float") == 0)
    return GCONF_VALUE_FLOAT;
  else if (strcmp(type_str, "string") == 0)
    return GCONF_VALUE_STRING;
  else if (strcmp(type_str, "bool") == 0)
    return GCONF_VALUE_BOOL;
  else if (strcmp(type_str, "schema") == 0)
    return GCONF_VALUE_SCHEMA;
  else if (strcmp(type_str, "list") == 0)
    return GCONF_VALUE_LIST;
  else if (strcmp(type_str, "pair") == 0)
    return GCONF_VALUE_PAIR;
  else
    return GCONF_VALUE_INVALID;
}

/*
 * Config files (yikes! we can't store our config in GConf!)
 */

static gchar*
unquote_string(gchar* s)
{
  gchar* end;

  /* Strip whitespace and first quote from front of string */
  while (*s && (isspace(*s) || (*s == '"')))
    ++s;

  end = s;
  while (*end)
    ++end;

  --end; /* one back from '\0' */

  /* Strip whitespace and last quote from end of string */
  while ((end > s) && (isspace(*end) || (*end == '"')))
    {
      *end = '\0';
      --end;
    }

  return s;
}

static const gchar*
get_variable(const gchar* varname)
{
  /* These first two DO NOT use environment variables, which
     makes things a bit more "secure" in some sense
  */
  if (strcmp(varname, "HOME") == 0)
    {
      return g_get_home_dir();
    }
  else if (strcmp(varname, "USER") == 0)
    {
      return g_get_user_name();
    }
  else if (varname[0] == 'E' &&
           varname[1] == 'N' &&
           varname[2] == 'V' &&
           varname[3] == '_')
    {
      /* This is magic: if a variable called ENV_FOO is used,
         then the environment variable FOO is checked */
      gchar* envvar = getenv(&varname[4]);

      if (envvar)
        return envvar;
      else
        return "";
    }
  else
    return "";
}

static gchar*
subst_variables(const gchar* src)
{
  const gchar* iter;
  gchar* retval;
  guint retval_len;
  guint pos;
  
  g_return_val_if_fail(src != NULL, NULL);

  retval_len = strlen(src) + 1;
  pos = 0;
  
  retval = g_malloc0(retval_len+3); /* add 3 just to avoid off-by-one
                                       segvs - yeah I know it bugs
                                       you, but C sucks */
  
  iter = src;

  while (*iter)
    {
      gboolean performed_subst = FALSE;
      
      if (pos >= retval_len)
        {
          retval_len *= 2;
          retval = g_realloc(retval, retval_len+3); /* add 3 for luck */
        }
      
      if (*iter == '$' && *(iter+1) == '(')
        {
          const gchar* varstart = iter + 2;
          const gchar* varend   = strchr(varstart, ')');

          if (varend != NULL)
            {
              char* varname;
              const gchar* varval;
              guint varval_len;

              performed_subst = TRUE;

              varname = g_strndup(varstart, varend - varstart);
              
              varval = get_variable(varname);
              g_free(varname);

              varval_len = strlen(varval);

              if ((retval_len - pos) < varval_len)
                {
                  retval_len *= 2;
                  retval = g_realloc(retval, retval_len+3);
                }
              
              strcpy(&retval[pos], varval);
              pos += varval_len;

              iter = varend + 1;
            }
        }

      if (!performed_subst)
        {
          retval[pos] = *iter;
          ++pos;
          ++iter;
        }
    }
  retval[pos] = '\0';

  return retval;
}

gchar**       
gconf_load_source_path(const gchar* filename, GError** err)
{
  FILE* f;
  GSList* l = NULL;
  gchar** addresses;
  gchar buf[512];
  GSList* tmp;
  guint n;

  f = fopen(filename, "r");

  if (f == NULL)
    {
      if (err)
        *err = gconf_error_new(GCONF_ERROR_FAILED,
                               _("Couldn't open path file `%s': %s\n"), 
                               filename, 
                               strerror(errno));
      return NULL;
    }

  while (fgets(buf, 512, f) != NULL)
    {
      gchar* s = buf;
      
      while (*s && isspace(*s))
        ++s;

      if (*s == '#')
        {
          /* Allow comments, why not */
        }
      else if (*s == '\0')
        {
          /* Blank line */
        }
      else if (strncmp("include", s, 7) == 0)
        {
          gchar* unq;
          gchar** included;
          gchar *varsubst;
          
          s += 7;

          unq = unquote_string(s);

          varsubst = subst_variables (unq);
          
          included = gconf_load_source_path (varsubst, NULL);

          g_free (varsubst);
          
          if (included != NULL)
            {
              gchar** iter = included;

              while (*iter)
                {
                  l = g_slist_prepend(l, *iter); /* Note that we won't free *included */
                  ++iter;
                }

              g_free(included); /* Only the array, not the contained strings */
            }
        }
      else 
        {
          gchar* unq;
          gchar* varsubst;
          
          unq = unquote_string(buf);
          varsubst = subst_variables(unq);
          
          if (*varsubst != '\0') /* Drop lines with just two quote marks or something */
            {
              gconf_log(GCL_DEBUG, _("Adding source `%s'\n"), varsubst);
              l = g_slist_prepend(l, g_strdup(varsubst));
            }
          g_free(varsubst);
        }
    }

  if (ferror(f))
    {
      /* This should basically never happen */
      if (err)
        *err = gconf_error_new(GCONF_ERROR_FAILED,
                               _("Read error on file `%s': %s\n"), 
                               filename,
                               strerror(errno));
      /* don't return, we want to go ahead and return any 
         addresses we already loaded. */
    }

  fclose(f);  

  /* This will make sense if you realize that we reversed the list 
     as we loaded it, and are now reversing it to be correct again. 
  */

  if (l == NULL)
    return NULL;

  n = g_slist_length(l);

  g_assert(n > 0);
  
  addresses = g_malloc0(sizeof(gchar*) * (n+1));

  addresses[n] = NULL;

  --n;
  tmp = l;

  while (tmp != NULL)
    {
      addresses[n] = tmp->data;

      tmp = g_slist_next(tmp);
      --n;
    }
  
  g_assert(addresses[0] != NULL); /* since we used malloc0 this detects bad logic */

  return addresses;
}

/* This should also support concatting filesystem dirs and keys, 
   or dir and subdir.
*/
gchar*        
gconf_concat_dir_and_key(const gchar* dir, const gchar* key)
{
  guint dirlen;
  guint keylen;
  gchar* retval;

  g_return_val_if_fail(dir != NULL, NULL);
  g_return_val_if_fail(key != NULL, NULL);
  g_return_val_if_fail(*dir == '/', NULL);

  dirlen = strlen(dir);
  keylen = strlen(key);

  retval = g_malloc0(dirlen+keylen+3); /* auto-null-terminate */

  strcpy(retval, dir);

  if (dir[dirlen-1] == '/')
    {
      /* dir ends in slash, strip key slash if needed */
      if (*key == '/')
        ++key;

      strcpy((retval+dirlen), key);
    }
  else 
    {
      /* Dir doesn't end in slash, add slash if key lacks one. */
      gchar* dest = retval + dirlen;

      if (*key != '/')
        {
          *dest = '/';
          ++dest;
        }
      
      strcpy(dest, key);
    }
  
  return retval;
}

gulong
gconf_string_to_gulong(const gchar* str)
{
  gulong retval;
  gchar *end;
  errno = 0;
  retval = strtoul(str, &end, 10);
  if (end == str || errno != 0)
    retval = 0;

  return retval;
}

gboolean
gconf_string_to_double(const gchar* str, gdouble* retloc)
{
  int res;
  char *old_locale;

  /* make sure we write values to files in a consistent manner */
  old_locale = g_strdup (setlocale (LC_NUMERIC, NULL));
  setlocale (LC_NUMERIC, "C");

  *retloc = 0.0;
  res = sscanf (str, "%lf", retloc);

  setlocale (LC_NUMERIC, old_locale);
  g_free (old_locale);

  if (res == 1)
    return TRUE;
  else
    return FALSE;
}

gchar*
gconf_double_to_string(gdouble val)
{
  char str[101 + DBL_DIG];
  char *old_locale;

  /* make sure we write values to files in a consistent manner */
  old_locale = g_strdup (setlocale (LC_NUMERIC, NULL));
  setlocale (LC_NUMERIC, "C");
  
  if (fabs (val) < 1e9 && fabs (val) > 1e-5)
    g_snprintf (str, 100 + DBL_DIG, "%.*g", DBL_DIG, val);
  else
    g_snprintf (str, 100 + DBL_DIG, "%f", val);

  setlocale (LC_NUMERIC, old_locale);
  g_free (old_locale);
  
  return g_strdup(str);
}

const gchar*
gconf_current_locale(void)
{
#ifdef HAVE_LC_MESSAGES
  return setlocale(LC_MESSAGES, NULL);
#else
  return setlocale(LC_CTYPE, NULL);
#endif
}

/*
 * Log
 */

gchar*
gconf_quote_percents(const gchar* src)
{
  gchar* dest;
  const gchar* s;
  gchar* d;

  g_return_val_if_fail(src != NULL, NULL);
  
  /* waste memory! woo-hoo! */
  dest = g_malloc0(strlen(src)*2+4);
  
  d = dest;
  
  s = src;
  while (*s)
    {
      switch (*s)
        {
        case '%':
          {
            *d = '%';
            ++d;
            *d = '%';
            ++d;
          }
          break;
          
        default:
          {
            *d = *s;
            ++d;
          }
          break;
        }
      ++s;
    }

  /* End with NULL */
  *d = '\0';
  
  return dest;
}

#include <syslog.h>

void
gconf_log(GConfLogPriority pri, const gchar* fmt, ...)
{
  gchar* msg;
  va_list args;
  int syslog_pri = LOG_DEBUG;

  if (!gconf_log_debug_messages && 
      pri == GCL_DEBUG)
    return;
  
  va_start (args, fmt);
  msg = g_strdup_vprintf(fmt, args);
  va_end (args);

  if (gconf_daemon_mode)
    {
      switch (pri)
        {
        case GCL_EMERG:
          syslog_pri = LOG_EMERG;
          break;
      
        case GCL_ALERT:
          syslog_pri = LOG_ALERT;
          break;
      
        case GCL_CRIT:
          syslog_pri = LOG_CRIT;
          break;
      
        case GCL_ERR:
          syslog_pri = LOG_ERR;
          break;
      
        case GCL_WARNING:
          syslog_pri = LOG_WARNING;
          break;
      
        case GCL_NOTICE:
          syslog_pri = LOG_NOTICE;
          break;
      
        case GCL_INFO:
          syslog_pri = LOG_INFO;
          break;
      
        case GCL_DEBUG:
          syslog_pri = LOG_DEBUG;
          break;

        default:
          g_assert_not_reached();
          break;
        }

      syslog(syslog_pri, "%s", msg);
    }
  else
    {
      switch (pri)
        {
        case GCL_EMERG:
        case GCL_ALERT:
        case GCL_CRIT:
        case GCL_ERR:
        case GCL_WARNING:
          fprintf(stderr, "%s\n", msg);
          break;
      
        case GCL_NOTICE:
        case GCL_INFO:
        case GCL_DEBUG:
          printf("%s\n", msg);
          break;

        default:
          g_assert_not_reached();
          break;
        }
    }
  
  g_free(msg);
}

/*
 * List/pair conversion
 */

GConfValue*
gconf_value_list_from_primitive_list(GConfValueType list_type, GSList* list,
                                     GError **err)
{
  GSList* value_list;
  GSList* tmp;

  g_return_val_if_fail(list_type != GCONF_VALUE_INVALID, NULL);
  g_return_val_if_fail(list_type != GCONF_VALUE_LIST, NULL);
  g_return_val_if_fail(list_type != GCONF_VALUE_PAIR, NULL);
  
  value_list = NULL;

  tmp = list;

  while (tmp != NULL)
    {
      GConfValue* val;

      val = gconf_value_new(list_type);

      switch (list_type)
        {
        case GCONF_VALUE_INT:
          gconf_value_set_int(val, GPOINTER_TO_INT(tmp->data));
          break;

        case GCONF_VALUE_BOOL:
          gconf_value_set_bool(val, GPOINTER_TO_INT(tmp->data));
          break;

        case GCONF_VALUE_FLOAT:
          gconf_value_set_float(val, *((gdouble*)tmp->data));
          break;

        case GCONF_VALUE_STRING:
          if (!g_utf8_validate (tmp->data, -1, NULL))
            {
              g_set_error (err, GCONF_ERROR,
                           GCONF_ERROR_FAILED,
                           _("Text contains invalid UTF-8"));
              goto error;
            }
                     
          gconf_value_set_string(val, tmp->data);
          break;

        case GCONF_VALUE_SCHEMA:
          if (!gconf_schema_validate (tmp->data, err))
            goto error;
          gconf_value_set_schema(val, tmp->data);
          break;
          
        default:
          g_assert_not_reached();
          break;
        }

      value_list = g_slist_prepend(value_list, val);

      tmp = g_slist_next(tmp);
    }

  /* Get it in the right order. */
  value_list = g_slist_reverse(value_list);

  {
    GConfValue* value_with_list;
    
    value_with_list = gconf_value_new(GCONF_VALUE_LIST);
    gconf_value_set_list_type(value_with_list, list_type);
    gconf_value_set_list_nocopy(value_with_list, value_list);

    return value_with_list;
  }

 error:
  g_slist_foreach (value_list, (GFunc)gconf_value_free, NULL);
  g_slist_free (value_list);
  return NULL;
}


static GConfValue*
from_primitive(GConfValueType type, gconstpointer address,
               GError **err)
{
  GConfValue* val;

  val = gconf_value_new(type);

  switch (type)
    {
    case GCONF_VALUE_INT:
      gconf_value_set_int(val, *((const gint*)address));
      break;

    case GCONF_VALUE_BOOL:
      gconf_value_set_bool(val, *((const gboolean*)address));
      break;

    case GCONF_VALUE_STRING:
      if (!g_utf8_validate (*((const gchar**)address), -1, NULL))
        {
          g_set_error (err, GCONF_ERROR,
                       GCONF_ERROR_FAILED,
                       _("Text contains invalid UTF-8"));
          gconf_value_free (val);
          return NULL;
        }

      gconf_value_set_string(val, *((const gchar**)address));
      break;

    case GCONF_VALUE_FLOAT:
      gconf_value_set_float(val, *((const gdouble*)address));
      break;

    case GCONF_VALUE_SCHEMA:
      if (!gconf_schema_validate (*((GConfSchema**)address), err))
        {
          gconf_value_free (val);
          return NULL;
        }
      
      gconf_value_set_schema(val, *((GConfSchema**)address));
      break;
      
    default:
      g_assert_not_reached();
      break;
    }

  return val;
}

GConfValue*
gconf_value_pair_from_primitive_pair(GConfValueType car_type,
                                     GConfValueType cdr_type,
                                     gconstpointer address_of_car,
                                     gconstpointer address_of_cdr,
                                     GError       **err)
{
  GConfValue* pair;
  GConfValue* car;
  GConfValue* cdr;
  
  g_return_val_if_fail(car_type != GCONF_VALUE_INVALID, NULL);
  g_return_val_if_fail(car_type != GCONF_VALUE_LIST, NULL);
  g_return_val_if_fail(car_type != GCONF_VALUE_PAIR, NULL);
  g_return_val_if_fail(cdr_type != GCONF_VALUE_INVALID, NULL);
  g_return_val_if_fail(cdr_type != GCONF_VALUE_LIST, NULL);
  g_return_val_if_fail(cdr_type != GCONF_VALUE_PAIR, NULL);
  g_return_val_if_fail(address_of_car != NULL, NULL);
  g_return_val_if_fail(address_of_cdr != NULL, NULL);
  
  car = from_primitive(car_type, address_of_car, err);
  if (car == NULL)
    return NULL;
  cdr = from_primitive(cdr_type, address_of_cdr, err);
  if (cdr == NULL)
    {
      gconf_value_free (car);
      return NULL;
    }
  
  pair = gconf_value_new(GCONF_VALUE_PAIR);
  gconf_value_set_car_nocopy(pair, car);
  gconf_value_set_cdr_nocopy(pair, cdr);

  return pair;
}


GSList*
gconf_value_list_to_primitive_list_destructive(GConfValue* val,
                                               GConfValueType list_type,
                                               GError** err)
{
  GSList* retval;

  g_return_val_if_fail(val != NULL, NULL);
  g_return_val_if_fail(list_type != GCONF_VALUE_INVALID, NULL);
  g_return_val_if_fail(list_type != GCONF_VALUE_LIST, NULL);
  g_return_val_if_fail(list_type != GCONF_VALUE_PAIR, NULL);
  g_return_val_if_fail(err == NULL || *err == NULL, NULL);
  
  if (val->type != GCONF_VALUE_LIST)
    {
      if (err)
        *err = gconf_error_new(GCONF_ERROR_TYPE_MISMATCH,
                               _("Expected list, got %s"),
                               gconf_value_type_to_string(val->type));
      gconf_value_free(val);
      return NULL;
    }

  if (gconf_value_get_list_type(val) != list_type)
    {
      if (err)
        *err = gconf_error_new(GCONF_ERROR_TYPE_MISMATCH,
                               _("Expected list of %s, got list of %s"),
                               gconf_value_type_to_string(list_type),
                               gconf_value_type_to_string(val->type));
      gconf_value_free(val);
      return NULL;
    }

  g_assert(gconf_value_get_list_type(val) == list_type);
      
  retval = gconf_value_get_list(val);

  /* Cheating the API to avoid a list copy; set this to NULL to
         avoid destroying the list */
  val->d.list_data.list = NULL;
      
  gconf_value_free(val);
  val = NULL;
      
  {
    /* map (typeChange, retval) */
    GSList* tmp;

    tmp = retval;

    while (tmp != NULL)
      {
        GConfValue* elem = tmp->data;

        g_assert(elem != NULL);
        g_assert(elem->type == list_type);
            
        switch (list_type)
          {
          case GCONF_VALUE_INT:
          case GCONF_VALUE_BOOL:
            tmp->data = GINT_TO_POINTER(gconf_value_get_int(elem));
            break;
                
          case GCONF_VALUE_FLOAT:
            {
              gdouble* d = g_new(gdouble, 1);
              *d = gconf_value_get_float(elem);
              tmp->data = d;
            }
            break;

          case GCONF_VALUE_STRING:
            {
              /* Cheat again, and steal the string from the value */
              tmp->data = elem->d.string_data;
              elem->d.string_data = NULL;
            }
            break;

          case GCONF_VALUE_SCHEMA:
            {
              /* and also steal the schema... */
              tmp->data = elem->d.schema_data;
              elem->d.schema_data = NULL;
            }
            break;
                
          default:
            g_assert_not_reached();
            break;
          }

        /* Clean up the value */
        gconf_value_free(elem);
            
        tmp = g_slist_next(tmp);
      }
  } /* list conversion block */
      
  return retval;
}


static void
primitive_value(gpointer retloc, GConfValue* val)
{
  switch (val->type)
    {
    case GCONF_VALUE_INT:
      *((gint*)retloc) = gconf_value_get_int(val);
      break;

    case GCONF_VALUE_FLOAT:
      *((gdouble*)retloc) = gconf_value_get_float(val);
      break;

    case GCONF_VALUE_STRING:
      {
        *((gchar**)retloc) = val->d.string_data;
        /* cheat and steal the string to avoid a copy */
        val->d.string_data = NULL;
      }
      break;

    case GCONF_VALUE_BOOL:
      *((gboolean*)retloc) = gconf_value_get_bool(val);
      break;

    case GCONF_VALUE_SCHEMA:
      *((GConfSchema**)retloc) = gconf_value_get_schema(val);
      break;
      
    default:
      g_assert_not_reached();
      break;
    }
}

gboolean
gconf_value_pair_to_primitive_pair_destructive(GConfValue* val,
                                               GConfValueType car_type,
                                               GConfValueType cdr_type,
                                               gpointer car_retloc,
                                               gpointer cdr_retloc,
                                               GError** err)
{
  GConfValue* car;
  GConfValue* cdr;

  g_return_val_if_fail(val != NULL, FALSE);
  g_return_val_if_fail(car_type != GCONF_VALUE_INVALID, FALSE);
  g_return_val_if_fail(car_type != GCONF_VALUE_LIST, FALSE);
  g_return_val_if_fail(car_type != GCONF_VALUE_PAIR, FALSE);
  g_return_val_if_fail(cdr_type != GCONF_VALUE_INVALID, FALSE);
  g_return_val_if_fail(cdr_type != GCONF_VALUE_LIST, FALSE);
  g_return_val_if_fail(cdr_type != GCONF_VALUE_PAIR, FALSE);
  g_return_val_if_fail(car_retloc != NULL, FALSE);
  g_return_val_if_fail(cdr_retloc != NULL, FALSE);
  g_return_val_if_fail(err == NULL || *err == NULL, FALSE);  
      
  if (val->type != GCONF_VALUE_PAIR)
    {
      if (err)
        *err = gconf_error_new(GCONF_ERROR_TYPE_MISMATCH,
                               _("Expected pair, got %s"),
                               gconf_value_type_to_string(val->type));
      gconf_value_free(val);
      return FALSE;
    }

  car = gconf_value_get_car(val);
  cdr = gconf_value_get_cdr(val);
      
  if (car == NULL ||
      cdr == NULL)
    {
      if (err)
        *err = gconf_error_new(GCONF_ERROR_TYPE_MISMATCH, 
                               _("Expected (%s,%s) pair, got a pair with one or both values missing"),
                               gconf_value_type_to_string(car_type),
                               gconf_value_type_to_string(cdr_type));

      gconf_value_free(val);
      return FALSE;
    }

  g_assert(car != NULL);
  g_assert(cdr != NULL);
      
  if (car->type != car_type ||
      cdr->type != cdr_type)
    {
      if (err)
        *err = gconf_error_new(GCONF_ERROR_TYPE_MISMATCH,
                               _("Expected pair of type (%s,%s) got type (%s,%s)"),
                               gconf_value_type_to_string(car_type),
                               gconf_value_type_to_string(cdr_type),
                               gconf_value_type_to_string(car->type),
                               gconf_value_type_to_string(cdr->type));
      gconf_value_free(val);
      return FALSE;
    }

  primitive_value(car_retloc, car);
  primitive_value(cdr_retloc, cdr);

  gconf_value_free(val);

  return TRUE;
}



/*
 * Encode/decode
 */

gchar*
gconf_quote_string   (const gchar* src)
{
  gchar* dest;
  const gchar* s;
  gchar* d;

  g_return_val_if_fail(src != NULL, NULL);
  
  /* waste memory! woo-hoo! */
  dest = g_malloc0(strlen(src)*2+4);
  
  d = dest;

  *d = '"';
  ++d;
  
  s = src;
  while (*s)
    {
      switch (*s)
        {
        case '"':
          {
            *d = '\\';
            ++d;
            *d = '"';
            ++d;
          }
          break;
          
        case '\\':
          {
            *d = '\\';
            ++d;
            *d = '\\';
            ++d;
          }
          break;
          
        default:
          {
            *d = *s;
            ++d;
          }
          break;
        }
      ++s;
    }

  /* End with quote mark and NULL */
  *d = '"';
  ++d;
  *d = '\0';
  
  return dest;
}

gchar*
gconf_unquote_string (const gchar* str, const gchar** end, GError** err)
{
  gchar* unq;
  gchar* unq_end = NULL;

  g_return_val_if_fail(end != NULL, NULL);
  g_return_val_if_fail(err == NULL || *err == NULL, NULL);
  g_return_val_if_fail(str != NULL, NULL);
  
  unq = g_strdup(str);

  gconf_unquote_string_inplace(unq, &unq_end, err);

  *end = (str + (unq_end - unq));

  return unq;
}

void
gconf_unquote_string_inplace (gchar* str, gchar** end, GError** err)
{
  gchar* dest;
  gchar* s;

  g_return_if_fail(end != NULL);
  g_return_if_fail(err == NULL || *err == NULL);
  g_return_if_fail(str != NULL);
  
  dest = s = str;

  if (*s != '"')
    {
      if (err)
        *err = gconf_error_new(GCONF_ERROR_PARSE_ERROR,
                               _("Quoted string doesn't begin with a quotation mark"));
      *end = str;
      return;
    }

  /* Skip the initial quote mark */
  ++s;
  
  while (*s)
    {
      g_assert(s > dest); /* loop invariant */
      
      switch (*s)
        {
        case '"':
          /* End of the string, return now */
          *dest = '\0';
          ++s;
          *end = s;
          return;
          break;

        case '\\':
          /* Possible escaped quote or \ */
          ++s;
          if (*s == '"')
            {
              *dest = *s;
              ++s;
              ++dest;
            }
          else if (*s == '\\')
            {
              *dest = *s;
              ++s;
              ++dest;
            }
          else
            {
              /* not an escaped char */
              *dest = '\\';
              ++dest;
              /* ++s already done. */
            }
          break;

        default:
          *dest = *s;
          ++dest;
          ++s;
          break;
        }

      g_assert(s > dest); /* loop invariant */
    }
  
  /* If we reach here this means the close quote was never encountered */

  *dest = '\0';
  
  if (err)
    *err = gconf_error_new(GCONF_ERROR_PARSE_ERROR,
                           _("Quoted string doesn't end with a quotation mark"));
  *end = s;
  return;
}

/* The encoding format

   The first byte of the encoded string is the type of the value:

    i  int
    b  bool
    f  float
    s  string
    c  schema
    p  pair
    l  list
    v  invalid

    For int, the rest of the encoded value is the integer to be parsed with atoi()
    For bool, the rest is 't' or 'f'
    For float, the rest is a float to parse with g_strtod()
    For string, the rest is the string (not quoted)
    For schema, the encoding is complicated; see below.
    For pair, the rest is two primitive encodings (ibcfs), quoted, separated by a comma,
              car before cdr
    For list, first character is type, the rest is primitive encodings, quoted,
              separated by commas

    Schema:

    After the 'c' indicating schema, the second character is a byte indicating
    the type the schema expects. Then a comma, and the quoted locale, or "" for none.
    comma, and quoted short description; comma, quoted long description; comma, default
    value in the encoded format given above, quoted.
*/

static gchar type_byte(GConfValueType type)
{
  switch (type)
    {
    case GCONF_VALUE_INT:
      return 'i';
      break;
        
    case GCONF_VALUE_BOOL:
      return 'b';
      break;

    case GCONF_VALUE_FLOAT:
      return 'f';
      break;

    case GCONF_VALUE_STRING:
      return 's';
      break;

    case GCONF_VALUE_SCHEMA:
      return 'c';
      break;

    case GCONF_VALUE_LIST:
      return 'l';
      break;

    case GCONF_VALUE_PAIR:
      return 'p';
      break;

    case GCONF_VALUE_INVALID:
      return 'v';
      break;
      
    default:
      g_assert_not_reached();
      return '\0';
      break;
    }
}

GConfValueType
byte_type(gchar byte)
{
  switch (byte)
    {
    case 'i':
      return GCONF_VALUE_INT;
      break;

    case 'b':
      return GCONF_VALUE_BOOL;
      break;

    case 's':
      return GCONF_VALUE_STRING;
      break;

    case 'c':
      return GCONF_VALUE_SCHEMA;
      break;

    case 'f':
      return GCONF_VALUE_FLOAT;
      break;

    case 'l':
      return GCONF_VALUE_LIST;
      break;

    case 'p':
      return GCONF_VALUE_PAIR;
      break;
      
    case 'v':
      return GCONF_VALUE_INVALID;
      break;

    default:
      return GCONF_VALUE_INVALID;
      break;
    }
}

GConfValue*
gconf_value_decode (const gchar* encoded)
{
  GConfValueType type;
  GConfValue* val;
  const gchar* s;
  
  type = byte_type(*encoded);

  if (type == GCONF_VALUE_INVALID)
    return NULL;

  if (!g_utf8_validate (encoded, -1, NULL))
    {
      gconf_log (GCL_ERR, _("Encoded value is not valid UTF-8"));
      return NULL;
    }
  
  val = gconf_value_new(type);

  s = encoded + 1;
  
  switch (val->type)
    {
    case GCONF_VALUE_INT:
      gconf_value_set_int(val, atoi(s));
      break;
        
    case GCONF_VALUE_BOOL:
      gconf_value_set_bool(val, *s == 't' ? TRUE : FALSE);
      break;

    case GCONF_VALUE_FLOAT:
      {
        double d;
        gchar* endptr = NULL;
        
        d = g_strtod(s, &endptr);
        if (endptr == s)
          g_warning("Failure converting string to double in %s", G_GNUC_FUNCTION);
        gconf_value_set_float(val, d);
      }
      break;

    case GCONF_VALUE_STRING:
      {
        gconf_value_set_string(val, s);
      }
      break;

    case GCONF_VALUE_SCHEMA:
      {
        GConfSchema* sc = gconf_schema_new();
        const gchar* end = NULL;
        gchar* unquoted;
        
        gconf_value_set_schema(val, sc);

        gconf_schema_set_type(sc, byte_type(*s));
        ++s;
        gconf_schema_set_list_type(sc, byte_type(*s));
        ++s;
        gconf_schema_set_car_type(sc, byte_type(*s));
        ++s;
        gconf_schema_set_cdr_type(sc, byte_type(*s));
        ++s;

        /* locale */
        unquoted = gconf_unquote_string(s, &end, NULL);

        gconf_schema_set_locale(sc, unquoted);

        g_free(unquoted);
        
        if (*end != ',')
          g_warning("no comma after locale in schema");

        ++end;
        s = end;

        /* short */
        unquoted = gconf_unquote_string(s, &end, NULL);

        gconf_schema_set_short_desc(sc, unquoted);

        g_free(unquoted);
        
        if (*end != ',')
          g_warning("no comma after short desc in schema");

        ++end;
        s = end;


        /* long */
        unquoted = gconf_unquote_string(s, &end, NULL);

        gconf_schema_set_long_desc(sc, unquoted);

        g_free(unquoted);
        
        if (*end != ',')
          g_warning("no comma after long desc in schema");

        ++end;
        s = end;
        
        
        /* default value */
        unquoted = gconf_unquote_string(s, &end, NULL);

        gconf_schema_set_default_value_nocopy(sc, gconf_value_decode(unquoted));

        g_free(unquoted);
        
        if (*end != '\0')
          g_warning("trailing junk after encoded schema");
      }
      break;

    case GCONF_VALUE_LIST:
      {
        GSList* value_list = NULL;

        gconf_value_set_list_type(val, byte_type(*s));
	++s;

        while (*s)
          {
            gchar* unquoted;
            const gchar* end;
            
            GConfValue* elem;
            
            unquoted = gconf_unquote_string(s, &end, NULL);            

            elem = gconf_value_decode(unquoted);

            g_free(unquoted);
            
            if (elem)
              value_list = g_slist_prepend(value_list, elem);
            
            s = end;
            if (*s == ',')
              ++s;
            else if (*s != '\0')
              {
                g_warning("weird character in encoded list");
                break; /* error */
              }
          }

        value_list = g_slist_reverse(value_list);

        gconf_value_set_list_nocopy(val, value_list);
      }
      break;

    case GCONF_VALUE_PAIR:
      {
        gchar* unquoted;
        const gchar* end;
        
        GConfValue* car;
        GConfValue* cdr;
        
        unquoted = gconf_unquote_string(s, &end, NULL);            
        
        car = gconf_value_decode(unquoted);

        g_free(unquoted);
        
        s = end;
        if (*s == ',')
          ++s;
        else
          {
            g_warning("weird character in encoded pair");
          }
        
        unquoted = gconf_unquote_string(s, &end, NULL);
        
        cdr = gconf_value_decode(unquoted);
        g_free(unquoted);


        gconf_value_set_car_nocopy(val, car);
        gconf_value_set_cdr_nocopy(val, cdr);
      }
      break;

    default:
      g_assert_not_reached();
      break;
    }

  return val;
}

gchar*
gconf_value_encode (GConfValue* val)
{
  gchar* retval = NULL;
  
  g_return_val_if_fail(val != NULL, NULL);

  switch (val->type)
    {
    case GCONF_VALUE_INT:
      retval = g_strdup_printf("i%d", gconf_value_get_int(val));
      break;
        
    case GCONF_VALUE_BOOL:
      retval = g_strdup_printf("b%c", gconf_value_get_bool(val) ? 't' : 'f');
      break;

    case GCONF_VALUE_FLOAT:
      retval = g_strdup_printf("f%g", gconf_value_get_float(val));
      break;

    case GCONF_VALUE_STRING:
      retval = g_strdup_printf("s%s", gconf_value_get_string(val));
      break;

    case GCONF_VALUE_SCHEMA:
      {
        gchar* tmp;
        gchar* retval;
        gchar* quoted;
        gchar* encoded;
        GConfSchema* sc;

        sc = gconf_value_get_schema(val);
        
        tmp = g_strdup_printf("c%c%c%c%c,",
			      type_byte(gconf_schema_get_type(sc)),
			      type_byte(gconf_schema_get_list_type(sc)),
			      type_byte(gconf_schema_get_car_type(sc)),
			      type_byte(gconf_schema_get_cdr_type(sc)));

        quoted = gconf_quote_string(gconf_schema_get_locale(sc) ?
                                    gconf_schema_get_locale(sc) : "");
        retval = g_strconcat(tmp, quoted, ",", NULL);

        g_free(tmp);
        g_free(quoted);

        tmp = retval;
        quoted = gconf_quote_string(gconf_schema_get_short_desc(sc) ?
                                    gconf_schema_get_short_desc(sc) : "");

        retval = g_strconcat(tmp, quoted, ",", NULL);

        g_free(tmp);
        g_free(quoted);


        tmp = retval;
        quoted = gconf_quote_string(gconf_schema_get_long_desc(sc) ?
                                    gconf_schema_get_long_desc(sc) : "");

        retval = g_strconcat(tmp, quoted, ",", NULL);

        g_free(tmp);
        g_free(quoted);
        

        if (gconf_schema_get_default_value(sc) != NULL)
          encoded = gconf_value_encode(gconf_schema_get_default_value(sc));
        else
          encoded = g_strdup("");

        tmp = retval;
          
        quoted = gconf_quote_string(encoded);

        retval = g_strconcat(tmp, quoted, NULL);

        g_free(tmp);
        g_free(quoted);
        g_free(encoded);
      }
      break;

    case GCONF_VALUE_LIST:
      {
        GSList* tmp;

        retval = g_strdup_printf("l%c", type_byte(gconf_value_get_list_type(val)));
        
        tmp = gconf_value_get_list(val);

        while (tmp != NULL)
          {
            GConfValue* elem = tmp->data;
            gchar* encoded;
            gchar* quoted;
            
            g_assert(elem != NULL);

            encoded = gconf_value_encode(elem);

            quoted = gconf_quote_string(encoded);

            g_free(encoded);

            {
              gchar* free_me;
              free_me = retval;
              
              retval = g_strconcat(retval, ",", quoted, NULL);
              
              g_free(quoted);
              g_free(free_me);
            }
            
            tmp = g_slist_next(tmp);
          }
      }
      break;

    case GCONF_VALUE_PAIR:
      {
        gchar* car_encoded;
        gchar* cdr_encoded;
        gchar* car_quoted;
        gchar* cdr_quoted;

        car_encoded = gconf_value_encode(gconf_value_get_car(val));
        cdr_encoded = gconf_value_encode(gconf_value_get_cdr(val));

        car_quoted = gconf_quote_string(car_encoded);
        cdr_quoted = gconf_quote_string(cdr_encoded);

        retval = g_strconcat("p", car_quoted, ",", cdr_quoted, NULL);

        g_free(car_encoded);
        g_free(cdr_encoded);
        g_free(car_quoted);
        g_free(cdr_quoted);
      }
      break;

    default:
      g_assert_not_reached();
      break;
      
    }

  return retval;
}

gboolean
gconf_handle_oaf_exception(CORBA_Environment* ev, GError** err)
{
  switch (ev->_major)
    {
    case CORBA_NO_EXCEPTION:
      CORBA_exception_free(ev);
      return FALSE;
      break;
    case CORBA_SYSTEM_EXCEPTION:
      if (err)
        *err = gconf_error_new (GCONF_ERROR_NO_SERVER, _("CORBA error: %s"),
                                CORBA_exception_id (ev));
      CORBA_exception_free (ev);
      return TRUE;
      break;

    case CORBA_USER_EXCEPTION:
      {
        const gchar* id = CORBA_exception_id(ev);

        if (strcmp(id, "IDL:OAF/GeneralError:1.0") == 0)
          {
            OAF_GeneralError* ge = CORBA_exception_value(ev);

            if (err)
              *err = gconf_error_new (GCONF_ERROR_OAF_ERROR,
                                      _("OAF problem description: '%s'"),
                                      ge->description);
          }
        else if (strcmp (id,"IDL:OAF/ActivationContext/NotListed:1.0" ) == 0)
          {
            if (err)
              *err = gconf_error_new(GCONF_ERROR_OAF_ERROR, _("attempt to remove not-listed OAF object directory"));
          }
        else if (strcmp (id,"IDL:OAF/ActivationContext/AlreadyListed:1.0" ) == 0)
          {
            if (err)
              *err = gconf_error_new(GCONF_ERROR_OAF_ERROR, _("attempt to add already-listed OAF directory")); 
          }
        else if (strcmp (id,"IDL:OAF/ActivationContext/ParseFailed:1.0") == 0)
          {
            OAF_ActivationContext_ParseFailed* pe = CORBA_exception_value(ev);
            
            if (err)
              *err = gconf_error_new(GCONF_ERROR_OAF_ERROR, _("OAF parse error: %s"), pe->description);
          }
        else
          {
            if (err)
              *err = gconf_error_new(GCONF_ERROR_OAF_ERROR, _("Unknown OAF error"));
          }
        
        CORBA_exception_free(ev);
        return TRUE;
      }
      break;
    default:
      g_assert_not_reached();
      return TRUE;
      break;
    }
}

/*
 * Locks
 */

/*
 * Locks works as follows. We have a lock directory to hold the locking
 * mess, and we have an IOR file inside the lock directory with the
 * gconfd IOR, and we have an fcntl() lock on the IOR file. The IOR
 * file is created atomically using a temporary file, then link()
 */

struct _GConfLock {
  gchar *lock_directory;
  gchar *iorfile;
  int    lock_fd;
};

static void
gconf_lock_destroy (GConfLock* lock)
{
  if (lock->lock_fd >= 0)
    close (lock->lock_fd);
  g_free (lock->iorfile);
  g_free (lock->lock_directory);
  g_free (lock);
}

static void
set_close_on_exec (int fd)
{
  int val;

  val = fcntl (fd, F_GETFD, 0);
  if (val < 0)
    {
      gconf_log (GCL_DEBUG, "couldn't F_GETFD: %s\n", g_strerror (errno));
      return;
    }

  val |= FD_CLOEXEC;

  if (fcntl (fd, F_SETFD, val) < 0)
    gconf_log (GCL_DEBUG, "couldn't F_SETFD: %s\n", g_strerror (errno));
}

/* Your basic Stevens cut-and-paste */
static int
lock_reg (int fd, int cmd, int type, off_t offset, int whence, off_t len)
{
  struct flock lock;

  lock.l_type = type; /* F_RDLCK, F_WRLCK, F_UNLCK */
  lock.l_start = offset; /* byte offset relative to whence */
  lock.l_whence = whence; /* SEEK_SET, SEEK_CUR, SEEK_END */
  lock.l_len = len; /* #bytes, 0 for eof */

  return fcntl (fd, cmd, &lock);
}

#define lock_entire_file(fd) \
  lock_reg ((fd), F_SETLK, F_WRLCK, 0, SEEK_SET, 0)
#define unlock_entire_file(fd) \
  lock_reg ((fd), F_SETLK, F_UNLCK, 0, SEEK_SET, 0)

static gboolean
file_locked_by_someone_else (int fd)
{
  struct flock lock;

  lock.l_type = F_WRLCK;
  lock.l_start = 0;
  lock.l_whence = SEEK_SET;
  lock.l_len = 0;

  if (fcntl (fd, F_GETLK, &lock) < 0)
    return TRUE; /* pretend it's locked */

  if (lock.l_type == F_UNLCK)
    return FALSE; /* we have the lock */
  else
    return TRUE; /* someone else has it */
}

static char*
unique_filename (const char *directory)
{
  char *guid;
  char *uniquefile;
  
  guid = gconf_unique_key ();
  uniquefile = g_strconcat (directory, "/", guid, NULL);
  g_free (guid);

  return uniquefile;
}

static int
create_new_locked_file (const gchar *directory,
                        const gchar *filename,
                        GError     **err)
{
  int fd;
  char *uniquefile;
  gboolean got_lock;
  
  got_lock = FALSE;
  
  uniquefile = unique_filename (directory);

  fd = open (uniquefile, O_WRONLY | O_CREAT, 0700);

  /* Lock our temporary file, lock hopefully applies to the
   * inode and so also counts once we link it to the new name
   */
  if (lock_entire_file (fd) < 0)
    {
      g_set_error (err,
                   GCONF_ERROR,
                   GCONF_ERROR_LOCK_FAILED,
                   _("Could not lock temporary file '%s': %s"),
                   uniquefile, g_strerror (errno));
      goto out;
    }
  
  /* Create lockfile as a link to unique file */
  if (link (uniquefile, filename) == 0)
    {
      /* filename didn't exist before, and open succeeded, and we have the lock */
      got_lock = TRUE;
      goto out;
    }
  else
    {
      /* see if the link really succeeded */
      struct stat sb;
      if (stat (uniquefile, &sb) == 0 &&
          sb.st_nlink == 2)
        {
          got_lock = TRUE;
          goto out;
        }
      else
        {
          g_set_error (err,
                       GCONF_ERROR,
                       GCONF_ERROR_LOCK_FAILED,
                       _("Could not create file '%s', probably because it already exists"),
                       filename);
          goto out;
        }
    }
  
 out:
  if (got_lock)
    set_close_on_exec (fd);
  
  unlink (uniquefile);
  g_free (uniquefile);

  if (!got_lock)
    {
      if (fd >= 0)
        close (fd);
      fd = -1;
    }
  
  return fd;
}

static int
open_empty_locked_file (const gchar *directory,
                        const gchar *filename,
                        GError     **err)
{
  int fd;

  fd = create_new_locked_file (directory, filename, NULL);

  if (fd >= 0)
    return fd;
  
  /* We failed to create the file, most likely because it already
   * existed; try to get the lock on the existing file, and if we can
   * get that lock, delete it, then start over.
   */
  fd = open (filename, O_RDWR, 0700);
  if (fd < 0)
    {
      /* File has gone away? */
      g_set_error (err,
                   GCONF_ERROR,
                   GCONF_ERROR_LOCK_FAILED,
                   _("Failed to create or open '%s'"),
                   filename);
      return -1;
    }

  if (lock_entire_file (fd) < 0)
    {
      g_set_error (err,
                   GCONF_ERROR,
                   GCONF_ERROR_LOCK_FAILED,
                   _("Failed to lock '%s': another process has the lock (%s)"),
                   filename, strerror (errno));
      close (fd);
      return -1;
    }

  /* We have the lock on filename, so delete it */
  unlink (filename);
  close (fd);
  fd = -1;

  /* Now retry creating our file */
  fd = create_new_locked_file (directory, filename, err);
  
  return fd;
}

static ConfigServer
read_current_server (const gchar *iorfile,
                     gboolean     warn_if_fail)
{
  FILE *fp;
  
  fp = fopen (iorfile, "r");
          
  if (fp == NULL)
    {
      if (warn_if_fail)
        gconf_log (GCL_WARNING, _("IOR file '%s' not opened successfully, no gconfd located: %s"),
                   iorfile, g_strerror (errno));

      return CORBA_OBJECT_NIL;
    }
  else /* successfully opened IOR file */
    {
      char buf[2048] = { '\0' };
      const char *str = NULL;

      fgets (buf, sizeof (buf) - 2, fp);
      fclose (fp);

      /* The lockfile format is <pid>:<ior> for gconfd
       * or <pid>:none for gconftool
       */
      str = buf;
      while (isdigit(*str))
        ++str;

      if (*str == ':')
        ++str;
          
      if (str[0] == 'n' &&
          str[1] == 'o' &&
          str[2] == 'n' &&
          str[3] == 'e')
        {
          if (warn_if_fail)
            gconf_log (GCL_WARNING,
                       _("gconftool or other non-gconfd process has the lock file '%s'"),
                       iorfile);          
        }
      else /* file contains daemon IOR */
        {
          CORBA_ORB orb;
          CORBA_Environment ev;
          ConfigServer server;
          
          CORBA_exception_init (&ev);
                  
          orb = gconf_orb_get ();

          if (orb == NULL)
            {
              if (warn_if_fail)
                gconf_log (GCL_WARNING,
                           _("couldn't contact ORB to resolve existing gconfd object reference"));
              return CORBA_OBJECT_NIL;
            }
                  
          server = CORBA_ORB_string_to_object (orb, (char*) str, &ev);
          CORBA_exception_free (&ev);

          return server;
        }

      return CORBA_OBJECT_NIL;
    }
}

GConfLock*
gconf_get_lock_or_current_holder (const gchar  *lock_directory,
                                  ConfigServer *current_server,
                                  GError      **err)
{
  ConfigServer server;
  GConfLock* lock;
  
  g_return_val_if_fail(lock_directory != NULL, NULL);

  if (current_server)
    *current_server = CORBA_OBJECT_NIL;
  
  if (mkdir (lock_directory, 0700) < 0 &&
      errno != EEXIST)
    {
      gconf_set_error (err,
                       GCONF_ERROR_LOCK_FAILED,
                       _("couldn't create directory `%s': %s"),
                       lock_directory, g_strerror (errno));

      return NULL;
    }

  server = CORBA_OBJECT_NIL;
    
  lock = g_new0 (GConfLock, 1);

  lock->lock_directory = g_strdup (lock_directory);

  lock->iorfile = g_strconcat (lock->lock_directory, "/ior", NULL);

  /* Check the current IOR file and ping its daemon */
  
  lock->lock_fd = open_empty_locked_file (lock->lock_directory,
                                          lock->iorfile,
                                          err);
  
  if (lock->lock_fd < 0)
    {
      /* We didn't get the lock. Read the old server, and provide
       * it to the caller. Error is already set.
       */
      if (current_server)
        *current_server = read_current_server (lock->iorfile, TRUE);

      gconf_lock_destroy (lock);
      
      return NULL;
    }
  else
    {
      /* Write IOR to lockfile */
      const gchar* ior;
      int retval;
      gchar* s;
      
      s = g_strdup_printf ("%u:", (guint) getpid ());
        
      retval = write (lock->lock_fd, s, strlen (s));

      g_free (s);
        
      if (retval >= 0)
        {
          ior = gconf_get_daemon_ior();
            
          if (ior == NULL)
            retval = write (lock->lock_fd, "none", 4);
          else
            retval = write (lock->lock_fd, ior, strlen (ior));
        }

      if (retval < 0)
        {
          gconf_set_error (err,
                           GCONF_ERROR_LOCK_FAILED,
                           _("Can't write to file `%s': %s"),
                           lock->iorfile, g_strerror (errno));

          unlink (lock->iorfile);
          gconf_lock_destroy (lock);

          return NULL;
        }
    }

  return lock;
}

GConfLock*
gconf_get_lock (const gchar *lock_directory,
                GError     **err)
{
  return gconf_get_lock_or_current_holder (lock_directory, NULL, err);
}

gboolean
gconf_release_lock (GConfLock *lock,
                    GError   **err)
{
  gboolean retval;
  char *uniquefile;
  
  retval = FALSE;
  uniquefile = NULL;
  
  /* A paranoia check to avoid disaster if e.g.
   * some random client code opened and closed the
   * lockfile (maybe Nautilus checking its MIME type or
   * something)
   */
  if (lock->lock_fd < 0 ||
      file_locked_by_someone_else (lock->lock_fd))
    {
      g_set_error (err,
                   GCONF_ERROR,
                   GCONF_ERROR_FAILED,
                   _("We didn't have the lock on file `%s', but we should have"),
                   lock->iorfile);
      goto out;
    }

  /* To avoid annoying .nfs3435314513453145 files on unlink, which keep us
   * from removing the lock directory, we don't want to hold the
   * lockfile open after removing all links to it. But we can't
   * close it then unlink, because then we would be unlinking without
   * holding the lock. So, we create a unique filename and link it too
   * the locked file, then unlink the locked file, then drop our locks
   * and close file descriptors, then unlink the unique filename
   */
  
  uniquefile = unique_filename (lock->lock_directory);

  if (link (lock->iorfile, uniquefile) < 0)
    {
      g_set_error (err,
                   GCONF_ERROR,
                   GCONF_ERROR_FAILED,
                   _("Failed to link '%s' to '%s': %s"),
                   uniquefile, lock->iorfile, g_strerror (errno));

      goto out;
    }
  
  /* Note that we unlink while still holding the lock to avoid races */
  if (unlink (lock->iorfile) < 0)
    {
      g_set_error (err,
                   GCONF_ERROR,
                   GCONF_ERROR_FAILED,
                   _("Failed to remove lock file `%s': %s"),
                   lock->iorfile,
                   g_strerror (errno));
      goto out;
    }

  /* Now drop our lock */
  if (lock->lock_fd >= 0)
    {
      close (lock->lock_fd);
      lock->lock_fd = -1;
    }

  /* Now remove the temporary link we used to avoid .nfs351453 garbage */
  if (unlink (uniquefile) < 0)
    {
      g_set_error (err,
                   GCONF_ERROR,
                   GCONF_ERROR_FAILED,
                   _("Failed to clean up file '%s': %s"),
                   uniquefile, g_strerror (errno));

      goto out;
    }

  /* And finally clean up the directory - this would have failed if
   * we had .nfs323423423 junk
   */
  if (rmdir (lock->lock_directory) < 0)
    {
      g_set_error (err,
                   GCONF_ERROR,
                   GCONF_ERROR_FAILED,
                   _("Failed to remove lock directory `%s': %s"),
                   lock->lock_directory,
                   g_strerror (errno));
      goto out;
    }

  retval = TRUE;
  
 out:

  g_free (uniquefile);
  gconf_lock_destroy (lock);
  return retval;
}

/* This function doesn't try to see if the lock is valid or anything
 * of the sort; it just reads it. It does do the object_to_string
 */
ConfigServer
gconf_get_current_lock_holder  (const gchar *lock_directory)
{
  char *iorfile;
  ConfigServer server;

  iorfile = g_strconcat (lock_directory, "/ior", NULL);
  server = read_current_server (iorfile, FALSE);
  g_free (iorfile);
  return server;
}

/* Copied from OAF */
#ifndef ORBIT_USES_GLIB_MAIN_LOOP

static gboolean
orb_handle_connection (GIOChannel * source, GIOCondition cond,
		       GIOPConnection * cnx)
{
	/* The best way to know about an fd exception is if select()/poll()
	 * tells you about it, so we just relay that information on to ORBit
	 * if possible
	 */

	if (cond & (G_IO_HUP | G_IO_NVAL | G_IO_ERR))
		giop_main_handle_connection_exception (cnx);
	else
		giop_main_handle_connection (cnx);

	return TRUE;
}

static void
orb_add_connection (GIOPConnection * cnx)
{
	int tag;
	GIOChannel *channel;

	channel = g_io_channel_unix_new (GIOP_CONNECTION_GET_FD (cnx));
	tag = g_io_add_watch_full (channel, G_PRIORITY_LOW,
				   G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL,
				   (GIOFunc) orb_handle_connection,
				   cnx, NULL);
	g_io_channel_unref (channel);

	cnx->user_data = GUINT_TO_POINTER (tag);
}

static void
orb_remove_connection (GIOPConnection * cnx)
{
	g_source_remove (GPOINTER_TO_UINT (cnx->user_data));
	cnx->user_data = GINT_TO_POINTER (-1);
}

#endif /* !ORBIT_USES_GLIB_MAIN_LOOP */


#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
const char *
get_hostname (void)
{
	static char *hostname = NULL;
	char hn_tmp[65], ha_tmp[4];
	struct hostent *hent;

	if (!hostname) {
		gethostname (hn_tmp, sizeof (hn_tmp) - 1);

		hent = gethostbyname (hn_tmp);
		if (hent) {
			memcpy (ha_tmp, hent->h_addr, 4);
			hent = gethostbyaddr (ha_tmp, 4, AF_INET);
			if (hent)
				hostname = g_strdup (hent->h_name);
			else
				hostname =
					g_strdup (inet_ntoa
						  (*
						   ((struct in_addr *)
						    ha_tmp)));
		} else
			hostname = g_strdup (hn_tmp);
	}

	return hostname;
}

CORBA_ORB
gconf_orb_get (void)
{
  if (gconf_in_daemon_mode ())
    {
      static CORBA_ORB gconf_orb = CORBA_OBJECT_NIL;      

      if (gconf_orb == CORBA_OBJECT_NIL)
        {
          CORBA_Environment ev;
          int argc = 1;
          char *argv[] = { "gconf", NULL };
          CORBA_Context context;
          const char *hostname;
          
#ifndef ORBIT_USES_GLIB_MAIN_LOOP
          IIOPAddConnectionHandler = orb_add_connection;
          IIOPRemoveConnectionHandler = orb_remove_connection;
#endif /* !ORBIT_USES_GLIB_MAIN_LOOP */
      
          CORBA_exception_init (&ev);
      
          gconf_orb = CORBA_ORB_init (&argc, argv, "orbit-local-orb", &ev);
          g_assert (ev._major == CORBA_NO_EXCEPTION);

          /* Set values in default context */
          CORBA_ORB_get_default_context (gconf_orb, &context, &ev);
          g_assert (ev._major == CORBA_NO_EXCEPTION);
          
          hostname = get_hostname ();
          CORBA_Context_set_one_value (context, "hostname",
                                       (char *) hostname, &ev);
          CORBA_Context_set_one_value (context, "domain", "user", &ev);
          CORBA_Context_set_one_value (context, "username",
                                       g_get_user_name (), &ev);
          
          CORBA_exception_free (&ev);
        }
      
      return gconf_orb;
    }
  else
    {
      return oaf_orb_get ();
    }
}

char*
gconf_get_daemon_dir (void)
{
  return g_strconcat (g_get_home_dir (), "/.gconfd", NULL);
}

char*
gconf_get_lock_dir (void)
{
  char *gconfd_dir;
  char *lock_dir;
  
  gconfd_dir = gconf_get_daemon_dir ();
  lock_dir = g_strconcat (gconfd_dir, "/lock", NULL);

  g_free (gconfd_dir);
  return lock_dir;
}


static void set_cloexec (gint fd);

static void
close_fd_func (gpointer data)
{
  int *pipes = data;
  
  gint open_max;
  gint i;
  
  open_max = sysconf (_SC_OPEN_MAX);
  for (i = 3; i < open_max; i++)
    {
      /* don't close our write pipe */
      if (i != pipes[1])
        set_cloexec (i);
    }
}

ConfigServer
gconf_activate_server (gboolean  start_if_not_found,
                       GError  **error)
{
  ConfigServer server;
  int p[2] = { -1, -1 };
  char buf[1];
  GError *tmp_err;
  char *argv[3];
  char *gconfd_dir;
  char *lock_dir;
  CORBA_Environment ev;
  
  gconfd_dir = gconf_get_daemon_dir ();
  
  if (mkdir (gconfd_dir, 0700) < 0 && errno != EEXIST)
    gconf_log (GCL_WARNING, _("Failed to create %s: %s"),
               gconfd_dir, g_strerror (errno));

  g_free (gconfd_dir);
  
  lock_dir = gconf_get_lock_dir ();
  
  server = gconf_get_current_lock_holder (lock_dir);

  /* Confirm server exists */
  CORBA_exception_init (&ev);

  if (!CORBA_Object_is_nil (server, &ev))
    {
      ConfigServer_ping (server, &ev);
      
      if (ev._major != CORBA_NO_EXCEPTION)
        server = CORBA_OBJECT_NIL;
    }

  CORBA_exception_free (&ev);  

  if (server != CORBA_OBJECT_NIL)
    return server;
  
  if (start_if_not_found)
    {
      /* Spawn server */
      if (pipe (p) < 0)
        {
          g_set_error (error,
                       GCONF_ERROR,
                       GCONF_ERROR_NO_SERVER,
                       _("Failed to create pipe for communicating with spawned gconf daemon: %s\n"),
                       g_strerror (errno));
          goto out;
        }

      argv[0] = g_strconcat (GCONF_BINDIR, "/" GCONFD, NULL);
      argv[1] = g_strdup_printf ("%d", p[1]);
      argv[2] = NULL;
  
      tmp_err = NULL;
      if (!g_spawn_async (NULL,
                          argv,
                          NULL,
                          G_SPAWN_LEAVE_DESCRIPTORS_OPEN,
                          close_fd_func,
                          p,
                          NULL,
                          &tmp_err))
        {
          g_free (argv[0]);
          g_free (argv[1]);
          g_set_error (error,
                       GCONF_ERROR,
                       GCONF_ERROR_NO_SERVER,
                       _("Failed to launch configuration server: %s\n"),
                       tmp_err->message);
          g_error_free (tmp_err);
          goto out;
        }
      
      g_free (argv[0]);
      g_free (argv[1]);
  
      /* Block until server starts up */
      read (p[0], buf, 1);

      server = gconf_get_current_lock_holder (lock_dir);
    }
  
 out:
  if (server == CORBA_OBJECT_NIL &&
      error &&
      *error == NULL)
    g_set_error (error,
                 GCONF_ERROR,
                 GCONF_ERROR_NO_SERVER,
                 _("Failed to contact configuration server (a likely cause of this is that you have an existing configuration server (gconfd) running, but it isn't reachable from here - if you're logged in from two machines at once, you may need to enable TCP networking for ORBit)\n"));

  close (p[0]);
  close (p[1]);
  
  g_free (lock_dir);
  return server;
}

/* g_spawn cut-and-paste, changed to make all symbols static */

/* gspawn.h - Process launching
 *
 *  Copyright 2000 Red Hat, Inc.
 *
 * GLib is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * GLib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GLib; see the file COPYING.LIB.  If not, write
 * to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __G_SPAWN_H__
#define __G_SPAWN_H__

/* I'm not sure I remember our proposed naming convention here. */
#define G_SPAWN_ERROR g_spawn_error_quark ()

typedef enum
{
  G_SPAWN_ERROR_FORK,   /* fork failed due to lack of memory */
  G_SPAWN_ERROR_READ,   /* read or select on pipes failed */
  G_SPAWN_ERROR_CHDIR,  /* changing to working dir failed */
  G_SPAWN_ERROR_ACCES,  /* execv() returned EACCES */
  G_SPAWN_ERROR_PERM,   /* execv() returned EPERM */
  G_SPAWN_ERROR_2BIG,   /* execv() returned E2BIG */
  G_SPAWN_ERROR_NOEXEC, /* execv() returned ENOEXEC */
  G_SPAWN_ERROR_NAMETOOLONG, /* ""  "" ENAMETOOLONG */
  G_SPAWN_ERROR_NOENT,       /* ""  "" ENOENT */
  G_SPAWN_ERROR_NOMEM,       /* ""  "" ENOMEM */
  G_SPAWN_ERROR_NOTDIR,      /* ""  "" ENOTDIR */
  G_SPAWN_ERROR_LOOP,        /* ""  "" ELOOP   */
  G_SPAWN_ERROR_TXTBUSY,     /* ""  "" ETXTBUSY */
  G_SPAWN_ERROR_IO,          /* ""  "" EIO */
  G_SPAWN_ERROR_NFILE,       /* ""  "" ENFILE */
  G_SPAWN_ERROR_MFILE,       /* ""  "" EMFLE */
  G_SPAWN_ERROR_INVAL,       /* ""  "" EINVAL */
  G_SPAWN_ERROR_ISDIR,       /* ""  "" EISDIR */
  G_SPAWN_ERROR_LIBBAD,      /* ""  "" ELIBBAD */
  G_SPAWN_ERROR_FAILED       /* other fatal failure, error->message
                              * should explain
                              */
} GSpawnError;

static GQuark g_spawn_error_quark (void);

static gboolean g_spawn_async (const gchar           *working_directory,
                               gchar                **argv,
                               gchar                **envp,
                               GSpawnFlags            flags,
                               GSpawnChildSetupFunc   child_setup,
                               gpointer               user_data,
                               gint                  *child_pid,
                               GError               **error);


/* Opens pipes for non-NULL standard_output, standard_input, standard_error,
 * and returns the parent's end of the pipes.
 */
static gboolean g_spawn_async_with_pipes (const gchar          *working_directory,
                                          gchar               **argv,
                                          gchar               **envp,
                                          GSpawnFlags           flags,
                                          GSpawnChildSetupFunc  child_setup,
                                          gpointer              user_data,
                                          gint                 *child_pid,
                                          gint                 *standard_input,
                                          gint                 *standard_output,
                                          gint                 *standard_error,
                                          GError              **error);


#endif /* __G_SPAWN_H__ */




/* gspawn.c - Process launching
 *
 *  Copyright 2000 Red Hat, Inc.
 *  g_execvpe implementation based on GNU libc execvp:
 *   Copyright 1991, 92, 95, 96, 97, 98, 99 Free Software Foundation, Inc.
 *
 * GLib is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * GLib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GLib; see the file COPYING.LIB.  If not, write
 * to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "glib.h"
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif /* HAVE_SYS_SELECT_H */

static gint g_execute (const gchar  *file,
                       gchar **argv,
                       gchar **envp,
                       gboolean search_path);

static gboolean make_pipe            (gint                  p[2],
                                      GError              **error);
static gboolean fork_exec_with_pipes (gboolean              intermediate_child,
                                      const gchar          *working_directory,
                                      gchar               **argv,
                                      gchar               **envp,
                                      gboolean              close_descriptors,
                                      gboolean              search_path,
                                      gboolean              stdout_to_null,
                                      gboolean              stderr_to_null,
                                      gboolean              child_inherits_stdin,
                                      gboolean              file_and_argv_zero,
                                      GSpawnChildSetupFunc  child_setup,
                                      gpointer              user_data,
                                      gint                 *child_pid,
                                      gint                 *standard_input,
                                      gint                 *standard_output,
                                      gint                 *standard_error,
                                      GError              **error);

GQuark
g_spawn_error_quark (void)
{
  static GQuark quark = 0;
  if (quark == 0)
    quark = g_quark_from_static_string ("g-exec-error-quark");
  return quark;
}

/**
 * g_spawn_async:
 * @working_directory: child's current working directory, or NULL to inherit parent's
 * @argv: child's argument vector
 * @envp: child's environment, or NULL to inherit parent's
 * @flags: flags from #GSpawnFlags
 * @child_setup: function to run in the child just before exec()
 * @user_data: user data for @child_setup
 * @child_pid: return location for child process ID, or NULL
 * @error: return location for error
 * 
 * See g_spawn_async_with_pipes() for a full description; this function
 * simply calls the g_spawn_async_with_pipes() without any pipes.
 * 
 * Return value: TRUE on success, FALSE if error is set
 **/
static gboolean
g_spawn_async (const gchar          *working_directory,
               gchar               **argv,
               gchar               **envp,
               GSpawnFlags           flags,
               GSpawnChildSetupFunc  child_setup,
               gpointer              user_data,
               gint                 *child_pid,
               GError              **error)
{
  g_return_val_if_fail (argv != NULL, FALSE);
  
  return g_spawn_async_with_pipes (working_directory,
                                   argv, envp,
                                   flags,
                                   child_setup,
                                   user_data,
                                   child_pid,
                                   NULL, NULL, NULL,
                                   error);
}

/* Avoids a danger in threaded situations (calling close()
 * on a file descriptor twice, and another thread has
 * re-opened it since the first close)
 */
static gint
close_and_invalidate (gint *fd)
{
  gint ret;

  ret = close (*fd);
  *fd = -1;

  return ret;
}

typedef enum
{
  READ_FAILED = 0, /* FALSE */
  READ_OK,
  READ_EOF
} ReadResult;

/**
 * g_spawn_async_with_pipes:
 * @working_directory: child's current working directory, or NULL to inherit parent's
 * @argv: child's argument vector
 * @envp: child's environment, or NULL to inherit parent's
 * @flags: flags from #GSpawnFlags
 * @child_setup: function to run in the child just before exec()
 * @user_data: user data for @child_setup
 * @child_pid: return location for child process ID, or NULL
 * @standard_input: return location for file descriptor to write to child's stdin, or NULL
 * @standard_output: return location for file descriptor to read child's stdout, or NULL
 * @standard_error: return location for file descriptor to read child's stderr, or NULL
 * @error: return location for error
 *
 * Executes a child program asynchronously (your program will not
 * block waiting for the child to exit). The child program is
 * specified by the only argument that must be provided, @argv. @argv
 * should be a NULL-terminated array of strings, to be passed as the
 * argument vector for the child. The first string in @argv is of
 * course the name of the program to execute. By default, the name of
 * the program must be a full path; the PATH shell variable will only
 * be searched if you pass the %G_SPAWN_SEARCH_PATH flag.
 *
 * @envp is a NULL-terminated array of strings, where each string
 * has the form <literal>KEY=VALUE</literal>. This will become
 * the child's environment. If @envp is NULL, the child inherits its
 * parent's environment.
 *
 * @flags should be the bitwise OR of any flags you want to affect the
 * function's behavior. The %G_SPAWN_DO_NOT_REAP_CHILD means that the
 * child will not be automatically reaped; you must call waitpid() or
 * handle SIGCHLD yourself, or the child will become a zombie.
 * %G_SPAWN_LEAVE_DESCRIPTORS_OPEN means that the parent's open file
 * descriptors will be inherited by the child; otherwise all
 * descriptors except stdin/stdout/stderr will be closed before
 * calling exec() in the child. %G_SPAWN_SEARCH_PATH means that
 * <literal>argv[0]</literal> need not be an absolute path, it
 * will be looked for in the user's PATH. %G_SPAWN_STDOUT_TO_DEV_NULL
 * means that the child's standad output will be discarded, instead
 * of going to the same location as the parent's standard output.
 * %G_SPAWN_STDERR_TO_DEV_NULL means that the child's standard error
 * will be discarded. %G_SPAWN_CHILD_INHERITS_STDIN means that
 * the child will inherit the parent's standard input (by default,
 * the child's standard input is attached to /dev/null).
 * %G_SPAWN_FILE_AND_ARGV_ZERO means that the first element of @argv is
 * the file to execute, while the remaining elements are the
 * actual argument vector to pass to the file. Normally
 * g_spawn_async_with_pipes() uses @argv[0] as the file to execute, and
 * passes all of @argv to the child.
 *
 * @child_setup and @user_data are a function and user data to be
 * called in the child after GLib has performed all the setup it plans
 * to perform (including creating pipes, closing file descriptors,
 * etc.) but before calling exec(). That is, @child_setup is called
 * just before calling exec() in the child. Obviously actions taken in
 * this function will only affect the child, not the parent. 
 *
 * If non-NULL, @child_pid will be filled with the child's process
 * ID. You can use the process ID to send signals to the child, or
 * to waitpid() if you specified the %G_SPAWN_DO_NOT_REAP_CHILD flag.
 *
 * If non-NULL, the @standard_input, @standard_output, @standard_error
 * locations will be filled with file descriptors for writing to the child's
 * standard input or reading from its standard output or standard error.
 * The caller of g_spawn_async_with_pipes() must close these file descriptors
 * when they are no longer in use. If these parameters are NULL, the
 * corresponding pipe won't be created.
 *
 * @error can be NULL to ignore errors, or non-NULL to report errors.
 * If an error is set, the function returns FALSE. Errors
 * are reported even if they occur in the child (for example if the
 * executable in <literal>argv[0]</literal> is not found). Typically
 * the <literal>message</literal> field of returned errors should be displayed
 * to users. Possible errors are those from the #G_SPAWN_ERROR domain.
 *
 * If an error occurs, @child_pid, @standard_input, @standard_output,
 * and @standard_error will not be filled with valid values.
 * 
 * Return value: TRUE on success, FALSE if an error was set
 **/
static gboolean
g_spawn_async_with_pipes (const gchar          *working_directory,
                          gchar               **argv,
                          gchar               **envp,
                          GSpawnFlags           flags,
                          GSpawnChildSetupFunc  child_setup,
                          gpointer              user_data,
                          gint                 *child_pid,
                          gint                 *standard_input,
                          gint                 *standard_output,
                          gint                 *standard_error,
                          GError              **error)
{
  g_return_val_if_fail (argv != NULL, FALSE);
  g_return_val_if_fail (standard_output == NULL ||
                        !(flags & G_SPAWN_STDOUT_TO_DEV_NULL), FALSE);
  g_return_val_if_fail (standard_error == NULL ||
                        !(flags & G_SPAWN_STDERR_TO_DEV_NULL), FALSE);
  /* can't inherit stdin if we have an input pipe. */
  g_return_val_if_fail (standard_input == NULL ||
                        !(flags & G_SPAWN_CHILD_INHERITS_STDIN), FALSE);
  
  return fork_exec_with_pipes (!(flags & G_SPAWN_DO_NOT_REAP_CHILD),
                               working_directory,
                               argv,
                               envp,
                               !(flags & G_SPAWN_LEAVE_DESCRIPTORS_OPEN),
                               (flags & G_SPAWN_SEARCH_PATH) != 0,
                               (flags & G_SPAWN_STDOUT_TO_DEV_NULL) != 0,
                               (flags & G_SPAWN_STDERR_TO_DEV_NULL) != 0,
                               (flags & G_SPAWN_CHILD_INHERITS_STDIN) != 0,
                               (flags & G_SPAWN_FILE_AND_ARGV_ZERO) != 0,
                               child_setup,
                               user_data,
                               child_pid,
                               standard_input,
                               standard_output,
                               standard_error,
                               error);
}

static gint
exec_err_to_g_error (gint en)
{
  switch (en)
    {
#ifdef EACCES
    case EACCES:
      return G_SPAWN_ERROR_ACCES;
      break;
#endif

#ifdef EPERM
    case EPERM:
      return G_SPAWN_ERROR_PERM;
      break;
#endif

#ifdef E2BIG
    case E2BIG:
      return G_SPAWN_ERROR_2BIG;
      break;
#endif

#ifdef ENOEXEC
    case ENOEXEC:
      return G_SPAWN_ERROR_NOEXEC;
      break;
#endif

#ifdef ENAMETOOLONG
    case ENAMETOOLONG:
      return G_SPAWN_ERROR_NAMETOOLONG;
      break;
#endif

#ifdef ENOENT
    case ENOENT:
      return G_SPAWN_ERROR_NOENT;
      break;
#endif

#ifdef ENOMEM
    case ENOMEM:
      return G_SPAWN_ERROR_NOMEM;
      break;
#endif

#ifdef ENOTDIR
    case ENOTDIR:
      return G_SPAWN_ERROR_NOTDIR;
      break;
#endif

#ifdef ELOOP
    case ELOOP:
      return G_SPAWN_ERROR_LOOP;
      break;
#endif
      
#ifdef ETXTBUSY
    case ETXTBUSY:
      return G_SPAWN_ERROR_TXTBUSY;
      break;
#endif

#ifdef EIO
    case EIO:
      return G_SPAWN_ERROR_IO;
      break;
#endif

#ifdef ENFILE
    case ENFILE:
      return G_SPAWN_ERROR_NFILE;
      break;
#endif

#ifdef EMFILE
    case EMFILE:
      return G_SPAWN_ERROR_MFILE;
      break;
#endif

#ifdef EINVAL
    case EINVAL:
      return G_SPAWN_ERROR_INVAL;
      break;
#endif

#ifdef EISDIR
    case EISDIR:
      return G_SPAWN_ERROR_ISDIR;
      break;
#endif

#ifdef ELIBBAD
    case ELIBBAD:
      return G_SPAWN_ERROR_LIBBAD;
      break;
#endif
      
    default:
      return G_SPAWN_ERROR_FAILED;
      break;
    }
}

static void
write_err_and_exit (gint fd, gint msg)
{
  gint en = errno;
  
  write (fd, &msg, sizeof(msg));
  write (fd, &en, sizeof(en));
  
  _exit (1);
}

static void
set_cloexec (gint fd)
{
  fcntl (fd, F_SETFD, FD_CLOEXEC);
}

static gint
sane_dup2 (gint fd1, gint fd2)
{
  gint ret;

 retry:
  ret = dup2 (fd1, fd2);
  if (ret < 0 && errno == EINTR)
    goto retry;

  return ret;
}

enum
{
  CHILD_CHDIR_FAILED,
  CHILD_EXEC_FAILED,
  CHILD_DUP2_FAILED,
  CHILD_FORK_FAILED
};

static void
do_exec (gint                  child_err_report_fd,
         gint                  stdin_fd,
         gint                  stdout_fd,
         gint                  stderr_fd,
         const gchar          *working_directory,
         gchar               **argv,
         gchar               **envp,
         gboolean              close_descriptors,
         gboolean              search_path,
         gboolean              stdout_to_null,
         gboolean              stderr_to_null,
         gboolean              child_inherits_stdin,
         gboolean              file_and_argv_zero,
         GSpawnChildSetupFunc  child_setup,
         gpointer              user_data)
{
  if (working_directory && chdir (working_directory) < 0)
    write_err_and_exit (child_err_report_fd,
                        CHILD_CHDIR_FAILED);

  /* Close all file descriptors but stdin stdout and stderr as
   * soon as we exec. Note that this includes
   * child_err_report_fd, which keeps the parent from blocking
   * forever on the other end of that pipe.
   */
  if (close_descriptors)
    {
      gint open_max;
      gint i;
      
      open_max = sysconf (_SC_OPEN_MAX);
      for (i = 3; i < open_max; i++)
        set_cloexec (i);
    }
  else
    {
      /* We need to do child_err_report_fd anyway */
      set_cloexec (child_err_report_fd);
    }
  
  /* Redirect pipes as required */
  
  if (stdin_fd >= 0)
    {
      /* dup2 can't actually fail here I don't think */
          
      if (sane_dup2 (stdin_fd, 0) < 0)
        write_err_and_exit (child_err_report_fd,
                            CHILD_DUP2_FAILED);

      /* ignore this if it doesn't work */
      close_and_invalidate (&stdin_fd);
    }
  else if (!child_inherits_stdin)
    {
      /* Keep process from blocking on a read of stdin */
      gint read_null = open ("/dev/null", O_RDONLY);
      sane_dup2 (read_null, 0);
      close_and_invalidate (&read_null);
    }

  if (stdout_fd >= 0)
    {
      /* dup2 can't actually fail here I don't think */
          
      if (sane_dup2 (stdout_fd, 1) < 0)
        write_err_and_exit (child_err_report_fd,
                            CHILD_DUP2_FAILED);

      /* ignore this if it doesn't work */
      close_and_invalidate (&stdout_fd);
    }
  else if (stdout_to_null)
    {
      gint write_null = open ("/dev/null", O_WRONLY);
      sane_dup2 (write_null, 1);
      close_and_invalidate (&write_null);
    }

  if (stderr_fd >= 0)
    {
      /* dup2 can't actually fail here I don't think */
          
      if (sane_dup2 (stderr_fd, 2) < 0)
        write_err_and_exit (child_err_report_fd,
                            CHILD_DUP2_FAILED);

      /* ignore this if it doesn't work */
      close_and_invalidate (&stderr_fd);
    }
  else if (stderr_to_null)
    {
      gint write_null = open ("/dev/null", O_WRONLY);
      sane_dup2 (write_null, 2);
      close_and_invalidate (&write_null);
    }
  
  /* Call user function just before we exec */
  if (child_setup)
    {
      (* child_setup) (user_data);
    }

  g_execute (argv[0],
             file_and_argv_zero ? argv + 1 : argv,
             envp, search_path);

  /* Exec failed */
  write_err_and_exit (child_err_report_fd,
                      CHILD_EXEC_FAILED);
}

static gboolean
read_ints (int      fd,
           gint*    buf,
           gint     n_ints_in_buf,    
           gint    *n_ints_read,      
           GError **error)
{
  gsize bytes = 0;    
  
  while (TRUE)
    {
      gssize chunk;    

      if (bytes >= sizeof(gint)*2)
        break; /* give up, who knows what happened, should not be
                * possible.
                */
          
    again:
      chunk = read (fd,
                    ((gchar*)buf) + bytes,
                    sizeof(gint) * n_ints_in_buf - bytes);
      if (chunk < 0 && errno == EINTR)
        goto again;
          
      if (chunk < 0)
        {
          /* Some weird shit happened, bail out */
              
          g_set_error (error,
                       G_SPAWN_ERROR,
                       G_SPAWN_ERROR_FAILED,
                       _("Failed to read from child pipe (%s)"),
                       g_strerror (errno));

          return FALSE;
        }
      else if (chunk == 0)
        break; /* EOF */
      else /* chunk > 0 */
	bytes += chunk;
    }

  *n_ints_read = (gint)(bytes / sizeof(gint));

  return TRUE;
}

static gboolean
fork_exec_with_pipes (gboolean              intermediate_child,
                      const gchar          *working_directory,
                      gchar               **argv,
                      gchar               **envp,
                      gboolean              close_descriptors,
                      gboolean              search_path,
                      gboolean              stdout_to_null,
                      gboolean              stderr_to_null,
                      gboolean              child_inherits_stdin,
                      gboolean              file_and_argv_zero,
                      GSpawnChildSetupFunc  child_setup,
                      gpointer              user_data,
                      gint                 *child_pid,
                      gint                 *standard_input,
                      gint                 *standard_output,
                      gint                 *standard_error,
                      GError              **error)     
{
  gint pid;
  gint stdin_pipe[2] = { -1, -1 };
  gint stdout_pipe[2] = { -1, -1 };
  gint stderr_pipe[2] = { -1, -1 };
  gint child_err_report_pipe[2] = { -1, -1 };
  gint child_pid_report_pipe[2] = { -1, -1 };
  gint status;
  
  if (!make_pipe (child_err_report_pipe, error))
    return FALSE;

  if (intermediate_child && !make_pipe (child_pid_report_pipe, error))
    goto cleanup_and_fail;
  
  if (standard_input && !make_pipe (stdin_pipe, error))
    goto cleanup_and_fail;
  
  if (standard_output && !make_pipe (stdout_pipe, error))
    goto cleanup_and_fail;

  if (standard_error && !make_pipe (stderr_pipe, error))
    goto cleanup_and_fail;

  pid = fork ();

  if (pid < 0)
    {      
      g_set_error (error,
                   G_SPAWN_ERROR,
                   G_SPAWN_ERROR_FORK,
                   _("Failed to fork (%s)"),
                   g_strerror (errno));

      goto cleanup_and_fail;
    }
  else if (pid == 0)
    {
      /* Immediate child. This may or may not be the child that
       * actually execs the new process.
       */
      
      /* Be sure we crash if the parent exits
       * and we write to the err_report_pipe
       */
      signal (SIGPIPE, SIG_DFL);

      /* Close the parent's end of the pipes;
       * not needed in the close_descriptors case,
       * though
       */
      close_and_invalidate (&child_err_report_pipe[0]);
      close_and_invalidate (&child_pid_report_pipe[0]);
      close_and_invalidate (&stdin_pipe[1]);
      close_and_invalidate (&stdout_pipe[0]);
      close_and_invalidate (&stderr_pipe[0]);
      
      if (intermediate_child)
        {
          /* We need to fork an intermediate child that launches the
           * final child. The purpose of the intermediate child
           * is to exit, so we can waitpid() it immediately.
           * Then the grandchild will not become a zombie.
           */
          gint grandchild_pid;

          grandchild_pid = fork ();

          if (grandchild_pid < 0)
            {
              /* report -1 as child PID */
              write (child_pid_report_pipe[1], &grandchild_pid,
                     sizeof(grandchild_pid));
              
              write_err_and_exit (child_err_report_pipe[1],
                                  CHILD_FORK_FAILED);              
            }
          else if (grandchild_pid == 0)
            {
              do_exec (child_err_report_pipe[1],
                       stdin_pipe[0],
                       stdout_pipe[1],
                       stderr_pipe[1],
                       working_directory,
                       argv,
                       envp,
                       close_descriptors,
                       search_path,
                       stdout_to_null,
                       stderr_to_null,
                       child_inherits_stdin,
                       file_and_argv_zero,
                       child_setup,
                       user_data);
            }
          else
            {
              write (child_pid_report_pipe[1], &grandchild_pid, sizeof(grandchild_pid));
              close_and_invalidate (&child_pid_report_pipe[1]);
              
              _exit (0);
            }
        }
      else
        {
          /* Just run the child.
           */

          do_exec (child_err_report_pipe[1],
                   stdin_pipe[0],
                   stdout_pipe[1],
                   stderr_pipe[1],
                   working_directory,
                   argv,
                   envp,
                   close_descriptors,
                   search_path,
                   stdout_to_null,
                   stderr_to_null,
                   child_inherits_stdin,
                   file_and_argv_zero,
                   child_setup,
                   user_data);
        }
    }
  else
    {
      /* Parent */
      
      gint buf[2];
      gint n_ints = 0;    

      /* Close the uncared-about ends of the pipes */
      close_and_invalidate (&child_err_report_pipe[1]);
      close_and_invalidate (&child_pid_report_pipe[1]);
      close_and_invalidate (&stdin_pipe[0]);
      close_and_invalidate (&stdout_pipe[1]);
      close_and_invalidate (&stderr_pipe[1]);

      /* If we had an intermediate child, reap it */
      if (intermediate_child)
        {
        wait_again:
          if (waitpid (pid, &status, 0) < 0)
            {
              if (errno == EINTR)
                goto wait_again;
              else if (errno == ECHILD)
                ; /* do nothing, child already reaped */
              else
                g_warning ("waitpid() should not fail in "
			   "'fork_exec_with_pipes'");
            }
        }
      

      if (!read_ints (child_err_report_pipe[0],
                      buf, 2, &n_ints,
                      error))
        goto cleanup_and_fail;
        
      if (n_ints >= 2)
        {
          /* Error from the child. */

          switch (buf[0])
            {
            case CHILD_CHDIR_FAILED:
              g_set_error (error,
                           G_SPAWN_ERROR,
                           G_SPAWN_ERROR_CHDIR,
                           _("Failed to change to directory '%s' (%s)"),
                           working_directory,
                           g_strerror (buf[1]));

              break;
              
            case CHILD_EXEC_FAILED:
              g_set_error (error,
                           G_SPAWN_ERROR,
                           exec_err_to_g_error (buf[1]),
                           _("Failed to execute child process (%s)"),
                           g_strerror (buf[1]));

              break;
              
            case CHILD_DUP2_FAILED:
              g_set_error (error,
                           G_SPAWN_ERROR,
                           G_SPAWN_ERROR_FAILED,
                           _("Failed to redirect output or input of child process (%s)"),
                           g_strerror (buf[1]));

              break;

            case CHILD_FORK_FAILED:
              g_set_error (error,
                           G_SPAWN_ERROR,
                           G_SPAWN_ERROR_FORK,
                           _("Failed to fork child process (%s)"),
                           g_strerror (buf[1]));
              break;
              
            default:
              g_set_error (error,
                           G_SPAWN_ERROR,
                           G_SPAWN_ERROR_FAILED,
                           _("Unknown error executing child process"));
              break;
            }

          goto cleanup_and_fail;
        }

      /* Get child pid from intermediate child pipe. */
      if (intermediate_child)
        {
          n_ints = 0;
          
          if (!read_ints (child_pid_report_pipe[0],
                          buf, 1, &n_ints, error))
            goto cleanup_and_fail;

          if (n_ints < 1)
            {
              g_set_error (error,
                           G_SPAWN_ERROR,
                           G_SPAWN_ERROR_FAILED,
                           _("Failed to read enough data from child pid pipe (%s)"),
                           g_strerror (errno));
              goto cleanup_and_fail;
            }
          else
            {
              /* we have the child pid */
              pid = buf[0];
            }
        }
      
      /* Success against all odds! return the information */
      
      if (child_pid)
        *child_pid = pid;

      if (standard_input)
        *standard_input = stdin_pipe[1];
      if (standard_output)
        *standard_output = stdout_pipe[0];
      if (standard_error)
        *standard_error = stderr_pipe[0];
      
      return TRUE;
    }

 cleanup_and_fail:
  close_and_invalidate (&child_err_report_pipe[0]);
  close_and_invalidate (&child_err_report_pipe[1]);
  close_and_invalidate (&child_pid_report_pipe[0]);
  close_and_invalidate (&child_pid_report_pipe[1]);
  close_and_invalidate (&stdin_pipe[0]);
  close_and_invalidate (&stdin_pipe[1]);
  close_and_invalidate (&stdout_pipe[0]);
  close_and_invalidate (&stdout_pipe[1]);
  close_and_invalidate (&stderr_pipe[0]);
  close_and_invalidate (&stderr_pipe[1]);

  return FALSE;
}

static gboolean
make_pipe (gint     p[2],
           GError **error)
{
  if (pipe (p) < 0)
    {
      g_set_error (error,
                   G_SPAWN_ERROR,
                   G_SPAWN_ERROR_FAILED,
                   _("Failed to create pipe for communicating with child process (%s)"),
                   g_strerror (errno));
      return FALSE;
    }
  else
    return TRUE;
}

/* Based on execvp from GNU C Library */

static void
script_execute (const gchar *file,
                gchar      **argv,
                gchar      **envp,
                gboolean     search_path)
{
  /* Count the arguments.  */
  int argc = 0;
  while (argv[argc])
    ++argc;
  
  /* Construct an argument list for the shell.  */
  {
    gchar **new_argv;

    new_argv = g_new0 (gchar*, argc + 1);
    
    new_argv[0] = (char *) "/bin/sh";
    new_argv[1] = (char *) file;
    while (argc > 1)
      {
	new_argv[argc] = argv[argc - 1];
	--argc;
      }

    /* Execute the shell. */
    if (envp)
      execve (new_argv[0], new_argv, envp);
    else
      execv (new_argv[0], new_argv);
    
    g_free (new_argv);
  }
}

static gchar*
my_strchrnul (const gchar *str, gchar c)
{
  gchar *p = (gchar*) str;
  while (*p && (*p != c))
    ++p;

  return p;
}

static gint
g_execute (const gchar *file,
           gchar      **argv,
           gchar      **envp,
           gboolean     search_path)
{
  if (*file == '\0')
    {
      /* We check the simple case first. */
      errno = ENOENT;
      return -1;
    }

  if (!search_path || strchr (file, '/') != NULL)
    {
      /* Don't search when it contains a slash. */
      if (envp)
        execve (file, argv, envp);
      else
        execv (file, argv);
      
      if (errno == ENOEXEC)
	script_execute (file, argv, envp, FALSE);
    }
  else
    {
      gboolean got_eacces = 0;
      const gchar *path, *p;
      gchar *name, *freeme;
      size_t len;
      size_t pathlen;

      path = g_getenv ("PATH");
      if (path == NULL)
	{
	  /* There is no `PATH' in the environment.  The default
	   * search path in libc is the current directory followed by
	   * the path `confstr' returns for `_CS_PATH'.
           */

          /* In GLib we put . last, for security, and don't use the
           * unportable confstr(); UNIX98 does not actually specify
           * what to search if PATH is unset. POSIX may, dunno.
           */
          
          path = "/bin:/usr/bin:.";
	}

      len = strlen (file) + 1;
      pathlen = strlen (path);
      freeme = name = g_malloc (pathlen + len + 1);
      
      /* Copy the file name at the top, including '\0'  */
      memcpy (name + pathlen + 1, file, len);
      name = name + pathlen;
      /* And add the slash before the filename  */
      *name = '/';

      p = path;
      do
	{
	  char *startp;

	  path = p;
	  p = my_strchrnul (path, ':');

	  if (p == path)
	    /* Two adjacent colons, or a colon at the beginning or the end
             * of `PATH' means to search the current directory.
             */
	    startp = name + 1;
	  else
	    startp = memcpy (name - (p - path), path, p - path);

	  /* Try to execute this name.  If it works, execv will not return.  */
          if (envp)
            execve (startp, argv, envp);
          else
            execv (startp, argv);
          
	  if (errno == ENOEXEC)
	    script_execute (startp, argv, envp, search_path);

	  switch (errno)
	    {
	    case EACCES:
	      /* Record the we got a `Permission denied' error.  If we end
               * up finding no executable we can use, we want to diagnose
               * that we did find one but were denied access.
               */
	      got_eacces = TRUE;

              /* FALL THRU */
              
	    case ENOENT:
#ifdef ESTALE
	    case ESTALE:
#endif
#ifdef ENOTDIR
	    case ENOTDIR:
#endif
	      /* Those errors indicate the file is missing or not executable
               * by us, in which case we want to just try the next path
               * directory.
               */
	      break;

	    default:
	      /* Some other error means we found an executable file, but
               * something went wrong executing it; return the error to our
               * caller.
               */
              g_free (freeme);
	      return -1;
	    }
	}
      while (*p++ != '\0');

      /* We tried every element and none of them worked.  */
      if (got_eacces)
	/* At least one failure was due to permissions, so report that
         * error.
         */
        errno = EACCES;

      g_free (freeme);
    }

  /* Return the error from the last attempt (probably ENOENT).  */
  return -1;
}



#define UTF8_COMPUTE(Char, Mask, Len)					      \
  if (Char < 128)							      \
    {									      \
      Len = 1;								      \
      Mask = 0x7f;							      \
    }									      \
  else if ((Char & 0xe0) == 0xc0)					      \
    {									      \
      Len = 2;								      \
      Mask = 0x1f;							      \
    }									      \
  else if ((Char & 0xf0) == 0xe0)					      \
    {									      \
      Len = 3;								      \
      Mask = 0x0f;							      \
    }									      \
  else if ((Char & 0xf8) == 0xf0)					      \
    {									      \
      Len = 4;								      \
      Mask = 0x07;							      \
    }									      \
  else if ((Char & 0xfc) == 0xf8)					      \
    {									      \
      Len = 5;								      \
      Mask = 0x03;							      \
    }									      \
  else if ((Char & 0xfe) == 0xfc)					      \
    {									      \
      Len = 6;								      \
      Mask = 0x01;							      \
    }									      \
  else									      \
    Len = -1;

#define UTF8_LENGTH(Char)              \
  ((Char) < 0x80 ? 1 :                 \
   ((Char) < 0x800 ? 2 :               \
    ((Char) < 0x10000 ? 3 :            \
     ((Char) < 0x200000 ? 4 :          \
      ((Char) < 0x4000000 ? 5 : 6)))))
   

#define UTF8_GET(Result, Chars, Count, Mask, Len)			      \
  (Result) = (Chars)[0] & (Mask);					      \
  for ((Count) = 1; (Count) < (Len); ++(Count))				      \
    {									      \
      if (((Chars)[(Count)] & 0xc0) != 0x80)				      \
	{								      \
	  (Result) = -1;						      \
	  break;							      \
	}								      \
      (Result) <<= 6;							      \
      (Result) |= ((Chars)[(Count)] & 0x3f);				      \
    }

#define UNICODE_VALID(Char)                   \
    ((Char) < 0x110000 &&                     \
     ((Char) < 0xD800 || (Char) >= 0xE000) && \
     (Char) != 0xFFFE && (Char) != 0xFFFF)
   
     
static const gchar utf8_skip_data[256] = {
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,6,6,0,0
};

static const gchar * const gconf_g_utf8_skip = utf8_skip_data;

typedef guint32 gunichar;

gboolean
gconf_g_utf8_validate (const gchar  *str,
                       gssize        max_len,    
                       const gchar **end)
{

  const gchar *p;

  g_return_val_if_fail (str != NULL, FALSE);
  
  if (end)
    *end = str;
  
  p = str;
  
  while ((max_len < 0 || (p - str) < max_len) && *p)
    {
      int i, mask = 0, len;
      gunichar result;
      unsigned char c = (unsigned char) *p;
      
      UTF8_COMPUTE (c, mask, len);

      if (len == -1)
        break;

      /* check that the expected number of bytes exists in str */
      if (max_len >= 0 &&
          ((max_len - (p - str)) < len))
        break;
        
      UTF8_GET (result, p, i, mask, len);

      if (UTF8_LENGTH (result) != len) /* Check for overlong UTF-8 */
	break;

      if (result == (gunichar)-1)
        break;

      if (!UNICODE_VALID (result))
	break;
      
      p += len;
    }

  if (end)
    *end = p;

  /* See that we covered the entire length if a length was
   * passed in, or that we ended on a nul if not
   */
  if (max_len >= 0 &&
      p != (str + max_len))
    return FALSE;
  else if (max_len < 0 &&
           *p != '\0')
    return FALSE;
  else
    return TRUE;
}
