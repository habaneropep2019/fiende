/* Stripchart -- the gnome-utils stripchart plotting utility
 * Copyright (C) 2000 John Kodis <kodis@jagunet.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "chart-app.h"
#include "chart.h"
#include "strip.h"

/*
 * strmatch -- case-blind match for tag at front of str.
 */
static int
strmatch(const char *tag, const char *str)
{
  return str && !strncasecmp(tag, str, strlen(tag));
}

/*
 * skipbl -- skips over leading whitespace in a string.
 */
static char *
skipbl(char *s)
{
  while (isspace(*s))
    s++;
  return s;
}

/*
 * Expr -- the info required to evaluate an expression.
 */
typedef struct
{
  char *s;
  double *t_diff;

  int vars;
  double *last, *now;
  jmp_buf err_jmp;

  double *filter;
  char *equation, *filename, *pattern;

#ifdef HAVE_LIBGTOP
  Gtop *gtop_now, *gtop_last;
#endif

  int pass;
  double val;
  char *error;
}
Expr;

/*
 * eval_error -- called to report an error in expression evaluation.
 *
 * Only pre-initialization errors are reported.  After initialization,
 * the expression evaluator just returns a result of zero, and keeps
 * on truckin'.
 */
static void
eval_error(Expr *expr, char *msg, ...)
{
  int len;
  va_list args;
  static char err_msg[1000];

  fflush(stdout);
  va_start(args, msg);
  len = sprintf(err_msg, "%s: ", prog_name);
  len += vsprintf(err_msg + len, msg, args);
  fprintf(stderr, "%s\n", err_msg);
  va_end(args);

  gnome_dialog_run(GNOME_DIALOG(gnome_error_dialog(err_msg)));

  expr->val = 0.0;
  expr->error = g_strdup(err_msg);
  longjmp(expr->err_jmp, 1);
}

/*
 * stripbl -- skips some chars, then strips any leading whitespace.
 */
static void
stripbl(Expr *expr, int skip)
{
  expr->s = skipbl(expr->s + skip);
}

#ifdef HAVE_LIBGTOP
typedef struct
{
  char *name;
  char type;	/* L=long, D=double */
  size_t off;
}
Gtop_data;

#define GTOP_OFF(el) offsetof(Gtop, el)

Gtop_data gtop_vars[] = 
{
  /* glibtop cpu stats */
  { "cpu_total",        'L', GTOP_OFF(cpu.total)             },
  { "cpu_user",         'L', GTOP_OFF(cpu.user)              },
  { "cpu_nice",         'L', GTOP_OFF(cpu.nice)              },
  { "cpu_sys",          'L', GTOP_OFF(cpu.sys)               },
  { "cpu_idle",         'L', GTOP_OFF(cpu.idle)              },
  { "cpu_freq",         'L', GTOP_OFF(cpu.frequency)         },

  /* glibtop memory stats */
  { "mem_total",        'L', GTOP_OFF(mem.total)             },
  { "mem_used",	        'L', GTOP_OFF(mem.used)              },
  { "mem_free",	        'L', GTOP_OFF(mem.free)              },
  { "mem_shared",       'L', GTOP_OFF(mem.shared)            },
  { "mem_buffer",       'L', GTOP_OFF(mem.buffer)            },
  { "mem_cached",       'L', GTOP_OFF(mem.cached)            },
  { "mem_user",	        'L', GTOP_OFF(mem.user)              },
  { "mem_locked",       'L', GTOP_OFF(mem.locked)            },

  /* glibtop swap stats */
  { "swap_total",       'L', GTOP_OFF(swap.total)            },
  { "swap_used",        'L', GTOP_OFF(swap.used)             },
  { "swap_free",        'L', GTOP_OFF(swap.free)             },
  { "swap_pagein",      'L', GTOP_OFF(swap.pageout)          },
  { "swap_pageout",     'L', GTOP_OFF(swap.pagein)           },

  /* glibtop uptime stats */
  { "uptime",           'D', GTOP_OFF(uptime.uptime)         },
  { "idletime",         'D', GTOP_OFF(uptime.idletime)       },

  /* glibtop loadavg stats */
  { "load_running",     'L', GTOP_OFF(loadavg.nr_running)    },
  { "load_tasks",       'L', GTOP_OFF(loadavg.nr_tasks)      },
  { "load_1m",          'D', GTOP_OFF(loadavg.loadavg[0])    },
  { "load_5m",          'D', GTOP_OFF(loadavg.loadavg[1])    },
  { "load_15m",         'D', GTOP_OFF(loadavg.loadavg[2])    },

  /* netload stats -- Linux-specific for the time being */
  { "net_pkts_in",      'L', GTOP_OFF(netload.packets_in)    },
  { "net_pkts_out",     'L', GTOP_OFF(netload.packets_out)   },
  { "net_pkts_total",   'L', GTOP_OFF(netload.packets_total) },
			     
  { "net_bytes_in",     'L', GTOP_OFF(netload.bytes_in)      },
  { "net_bytes_out",    'L', GTOP_OFF(netload.bytes_out)     },
  { "net_bytes_total",  'L', GTOP_OFF(netload.bytes_total)   },
			     
  { "net_errs_in",      'L', GTOP_OFF(netload.errors_in)     },
  { "net_errs_out",     'L', GTOP_OFF(netload.errors_out)    },
  { "net_errs_total",   'L', GTOP_OFF(netload.errors_total)  },

  /* end of array marker */
  { NULL,	         0,  0 }
};

/*
 * gtop_value -- looks up a glibtop datum name, and returns its value.
 */
static int
gtop_value(
  char *str, int delta, 
  Gtop *gtop_now, Gtop *gtop_last, double *val)
{
  int i;

  for (i=0; gtop_vars[i].name; i++)
    {
      if (streq(str, gtop_vars[i].name))
	{
	  char *cp = ((char *)gtop_now) + gtop_vars[i].off;
	  if (gtop_vars[i].type == 'D')
	    {
	      double df = *((double *)cp);
	      if (delta)
		{
		  cp = ((char *)gtop_last) + gtop_vars[i].off;
		  df -= *((double *)cp);
		}
	      *val = df;
	    }
	  else /* if (gtop_vars[i].type == 'L') */
	    {
	      u_int64_t ul = *((u_int64_t *)cp);
	      if (delta)
		{
		  cp = ((char *)gtop_last) + gtop_vars[i].off;
		  ul -= *((unsigned long long *)cp);
		}
	      *val = ul;
	    }
	  return 1;
	}
    }

  return 0;
}
#endif

/*
 * add_op requires a forward prototype since it gets called from
 * num_op when recursing to evaluate a parenthesized expression.
 */
static double add_op(Expr *expr);

/*
 * num_op -- evaluates numeric constants, parenthesized expressions,
 * and named variables. 
 */
static double
num_op(Expr *expr)
{
  double val = 0;

  if (isdigit(*expr->s)
    || (*expr->s && strchr("+-.", *expr->s) && isdigit(expr->s[1])))
    {
      char *r;
      val = strtod(expr->s, &r);
      stripbl(expr, (int)(r - expr->s));
    }
  else if (*expr->s == '(')
    {
      stripbl(expr, 1);
      val = add_op(expr);
      if (*expr->s == ')')
	stripbl(expr, 1);
      else
	eval_error(expr, _("closing parenthesis expected"));
    }
  else if (*expr->s == '$' || *expr->s == '~')
    {
      int c, id_intro;
      char *idp, id[1000]; /* FIX THIS */

      id_intro = *expr->s++;
      for (idp = id; isalnum(c = (*idp++ = *expr->s++)) || c == '_'; )
	;
      idp[-1] = '\0';
      expr->s--;

      if (isdigit(*id))
	{
	  int id_num = atoi(id);
	  if (id_num > expr->vars)
	    eval_error(expr, _("no such field: %d"), id_num);
	  val = expr->now[id_num-1];
	  if (id_intro == '~')
	    val -= expr->last[id_num-1];
	}
      else if (streq(id, "t"))	/* time or delta time, in seconds */
	{
	  if (id_intro == '~')
	    val = *expr->t_diff;
	  else
	    {
	      struct timeval t;
	      static struct timeval t0;
	      if (t0.tv_sec == 0)
		gettimeofday(&t0, NULL);
	      gettimeofday(&t, NULL);
	      val = (t.tv_sec - t0.tv_sec) + (t.tv_usec - t0.tv_usec) / 1e6;
	    }
	}
#ifdef HAVE_LIBGTOP
      else if (gtop_value(id,
	id_intro=='~', expr->gtop_now, expr->gtop_last, &val))
	; /* gtop_value handles the assignment to val */
#endif
      else if (!*id)
	eval_error(expr, _("missing variable identifer"));
      else
	eval_error(expr, _("invalid variable identifer: %s"), id);
      stripbl(expr, 0);
    }
  else
    eval_error(expr, _("number expected"));

  return val;
}

/*
 * mul_op -- evaluates multiplication, division, and remaindering.
 */
static double
mul_op(Expr *expr)
{
  double val1 = num_op(expr);

  while (*expr->s == '*' || *expr->s == '/' || *expr->s == '%')
    {
      char c = *expr->s;
      stripbl(expr, 1);
      if (c == '*')
	val1 *= num_op(expr);
      else
	{
	  double val2 = num_op(expr);
	  if (val2 == 0) /* FIX THIS: there's got to be a better way. */
	    val1 = 0;
	  else if (c == '/')
	    val1 /= val2;
	  else
	    val1 = fmod(val1, val2);
	}
    }

  return val1;
}

/*
 * add_op -- evaluates addition and subtraction,
 */
static double
add_op(Expr *expr)
{
  double val = mul_op(expr);

  while (*expr->s == '+' || *expr->s == '-')
    {
      char c = *expr->s;
      stripbl(expr, 1);
      if (c == '+')
	val += mul_op(expr);
      else
	val -= mul_op(expr);
    }

  return val;
}

/*
 * eval -- sets up an Expr, then calls add_op to evaluate the expression.
 */
static int
eval(Expr *expr)
{
  expr->pass++;
  expr->s = expr->equation;
  expr->val = 0;

  if (setjmp(expr->err_jmp))
    return -1;

  stripbl(expr, 0);
  expr->val = add_op(expr);
  if (*expr->s && *expr->s != ';')
    eval_error(expr, _("extra junk at end: \"%s\""), expr->s);

  return 0;
}

static int
stat_value(char *fn)
{
  struct stat stat_buf;
  int status = stat(fn, &stat_buf);
  if (status == -1)
    return -1;
  if (stat_buf.st_size == 0)
    return 0;
  if (stat_buf.st_mtime < stat_buf.st_atime)
    return 1;
  return 2;
}

static int
split_and_extract(char *str, int vars, double *var)
{
  int i = 0;
  char *t = strtok(str, " \t:");
  while (t && i < vars)
    {
      var[i] = atof(t);
      t = strtok(NULL, " \t:");
      i++;
    }
  return i;
}

static gdouble
evaluate_equation(Expr *expr)
{
  double last = expr->val;

  expr->val = 0;
  if (expr->error != NULL)
    return 0;

  if (expr->filename)
    {
      FILE *fd = NULL;

      if (*expr->filename == '?')
	expr->val = stat_value(skipbl(expr->filename + 1));
      else if (*expr->filename == '|')
	fd = popen(expr->filename + 1, "r");
      else
	fd = fopen(expr->filename, "r");

      if (expr->vars)
	{
	  char buf[1000];
	  *buf = '\0';
	  if (fd)
	    {
	      fgets(buf, sizeof(buf), fd);
	      if (expr->pattern && *expr->pattern)
		while (!ferror(fd) && !feof(fd) && !strstr(buf, expr->pattern))
		  fgets(buf, sizeof(buf), fd);
	    }

	  memcpy(expr->last, expr->now, expr->vars * sizeof(*expr->now));
	  split_and_extract(buf, expr->vars, expr->now);
	}

      if (fd)
	{
	  if (*expr->filename == '|')
	    pclose(fd);
	  else
	    fclose(fd);
	}
    }

  if (expr->equation && eval(expr) != 0)
    return 0;

  switch (expr->pass)
    {
    case 0:
    case 1:
      expr->val = 0;
      break;
    case 2:
      expr->val = expr->val;
      break;
    default:
      expr->val = last + *expr->filter * (expr->val - last);
      break;
    }

  return expr->val;
}

static void
register_gtop_groups(Param_group *pg, char *eqn)
{
#ifdef HAVE_LIBGTOP
  while (eqn && *(eqn += strcspn(eqn, "$~")))
    if (strmatch(     "cpu_", ++eqn))   pg->gtop_cpu++;
    else if (strmatch("mem_", eqn))     pg->gtop_mem++;
    else if (strmatch("swap_", eqn))    pg->gtop_swap++;
    else if (strmatch("uptime", eqn))   pg->gtop_uptime++;
    else if (strmatch("idletime", eqn)) pg->gtop_uptime++;
    else if (strmatch("load_", eqn))    pg->gtop_load++;
    else if (strmatch("net_", eqn))     pg->gtop_net++;
#endif
}

static char *
expand_env(char *src)
{
  if (src)
    {
      char *exp, *dst, *key, *val, *tmp;
      exp = dst = g_malloc(strlen(src) + 1);
      while (1)
	switch (*src)
	  {
	  default:
	    *dst++ = *src++;
	    break;
	  case '\0':
	    *dst = '\0';
	    return exp;
	  case '$':
	    key = val = g_strdup(src);
	    for (tmp = dst, *tmp++ = *val++; isalnum(*tmp++ = *val++); )
	      ;
	    *--val = '\0';
	    if ((val = getenv(key + 1)) == NULL)
	      dst = tmp;
	    else
	      {
		*dst = '\0';
		tmp = g_malloc(strlen(exp) + strlen(val) + strlen(src) + 1);
		strcpy(tmp, exp);
		strcpy(tmp + strlen(tmp), val);
		g_free(exp);
		exp = tmp;
		dst = exp + strlen(exp);
	      }
	    src += strlen(key);
	    free(key);
	    break;
	  }
    }
  return NULL;
}

static void
free_expr(Expr *expr)
{
  if (expr->equation) g_free(expr->equation);
  if (expr->pattern) g_free(expr->pattern);
  if (expr->filename) g_free(expr->filename);
  if (expr->last) g_free(expr->last);
  if (expr->now) g_free(expr->now);
  if (expr) g_free(expr);
}

ChartDatum *
chart_equation_add(Chart *chart,
  Param_group *group, Param_desc *desc, GtkAdjustment *adj,
  int pageno, int rescale)
{
  char *s;
  ChartDatum *datum;
  Expr *expr = g_malloc(sizeof(*expr));

  expr->val = 0;
  expr->pass = -1;
  expr->error = NULL;

  expr->t_diff = &group->t_diff;
  expr->filter = &group->filter;

  expr->gtop_now  = &group->gtop_now;
  expr->gtop_last = &group->gtop_last;

  expr->equation = desc && desc->eqn ? g_strdup(desc->eqn) : NULL;
  expr->filename = desc && desc->fn ? expand_env(desc->fn) : NULL;
  expr->pattern  = desc && desc->pattern ? g_strdup(desc->pattern) : NULL;

  register_gtop_groups(group, expr->equation);
  expr->vars = 0;
  if (expr->equation)
    for (s = expr->equation; *(s += strcspn(s, "$~")); )
      if (isdigit(*++s))
	if (expr->vars < atoi(s))
	  expr->vars = atoi(s);
  if (expr->vars)
    {
      expr->last = g_malloc(expr->vars * sizeof(*expr->last));
      expr->now  = g_malloc(expr->vars * sizeof(*expr->now));
    }

  evaluate_equation(expr);
  if (expr->error != NULL)
    {
      free_expr(expr);
      return NULL;
    }

  datum = chart_parameter_add(chart,
    evaluate_equation, expr, desc->color_names, adj, pageno,
    str_to_gdouble(desc->bot_min, -G_MAXDOUBLE),
    str_to_gdouble(desc->bot_max, +G_MAXDOUBLE),
    str_to_gdouble(desc->top_min, -G_MAXDOUBLE),
    str_to_gdouble(desc->top_max, +G_MAXDOUBLE));

  if (rescale)
    chart_set_autorange(datum, chart_rescale_by_decade, NULL);

  chart_set_scale_style(datum,
    desc ? str_to_scale_style(desc->scale) : chart_scale_linear);
  chart_set_plot_style(datum,
    desc ? str_to_plot_style(desc->plot) : chart_plot_line);

  return datum;
}

#ifdef HAVE_LIBGTOP
/*
 * glibtop_get_swap_occasionally -- like glibtop_get_swap, except that
 * glibtop_get_swap only gets called at most once every 15 seconds.
 * This ugly hack was put in because on Linux, reading /proc/meminfo
 * to get swap informetion takes ~ 10ms, thus chewing up much CPU time.
 */
static void
glibtop_get_swap_occasionally(glibtop_swap *swap)
{
  time_t time_now = time(NULL);
  static time_t time_last;
  static glibtop_swap swap_last;

  if ((time_now - time_last) > 15)
    {
      glibtop_get_swap(&swap_last);
      time_last = time_now;
    }
  *swap = swap_last;
}

/*
 * get_all_netload -- a Linux-specific routine to get net load
 * information for all interfaces.  This assumes that u_int64_t is
 * defined and that 64-bit ints are available, but since this is quite
 * linux-specific anyway, I think that this should be okay. 
 */
static void
get_all_netload(glibtop_netload *netload)
{
  FILE *fd;
  memset(netload, 0, sizeof(*netload));
#if linux
  if ((fd = fopen("/proc/net/dev", "r")) != NULL)
    {
      u_int64_t bytes_in, pkts_in, errs_in, bytes_out, pkts_out, errs_out;
      const char *format;
      fscanf(fd, "%*[^\n]\n%*[^\n]\n");
      /* get the correct format according to arch */
      if (sizeof (u_int64_t) == sizeof (long))
	      format = "%*[^:]:%ld%ld%ld%*d%*d%*d%*d%*d%ld%ld%ld%*d%*d%*d%*d%*d";
      else
	      format = "%*[^:]:%lld%lld%lld%*d%*d%*d%*d%*d%lld%lld%lld%*d%*d%*d%*d%*d";
      while (fscanf(fd, format,
	&bytes_in, &pkts_in, &errs_in, &bytes_out, &pkts_out, &errs_out) == 6)
	{
	  netload->packets_in    += pkts_in;
	  netload->packets_out   += pkts_out;
	  netload->packets_total += pkts_in + pkts_out;
	  netload->bytes_in      += bytes_in;
	  netload->bytes_out     += bytes_out;
	  netload->bytes_total   += bytes_in + bytes_out;
	  netload->errors_in     += errs_in;
	  netload->errors_out    += errs_out;
	  netload->errors_total  += errs_in + errs_out;
	}
      fclose(fd);
    }
#endif
}
#endif

void
chart_start(GtkWidget *chart, Param_group *pg)
{
  pg->t_last = pg->t_now;
  gettimeofday(&pg->t_now, NULL);
  pg->t_diff = (pg->t_now.tv_sec - pg->t_last.tv_sec) +
    (pg->t_now.tv_usec - pg->t_last.tv_usec) / 1e6;
#ifdef HAVE_LIBGTOP
  pg->gtop_last = pg->gtop_now;
  if (pg->gtop_cpu)    glibtop_get_cpu(&pg->gtop_now.cpu);
  if (pg->gtop_mem)    glibtop_get_mem(&pg->gtop_now.mem);
  if (pg->gtop_swap)   glibtop_get_swap_occasionally(&pg->gtop_now.swap);
  if (pg->gtop_uptime) glibtop_get_uptime(&pg->gtop_now.uptime);
  if (pg->gtop_load)   glibtop_get_loadavg(&pg->gtop_now.loadavg);
  if (pg->gtop_net)    get_all_netload(&pg->gtop_now.netload);
#endif
}
