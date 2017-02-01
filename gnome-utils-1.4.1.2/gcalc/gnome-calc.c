/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 1997, 1998, 1999, 2000 Free Software Foundation
 * All rights reserved.
 *
 * This file is part of the Gnome Library.
 *
 * The Gnome Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * The Gnome Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the Gnome Library; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
/*
  @NOTATION@
 */
/* GnomeCalc - double precision simple calculator widget
 *
 * Author: George Lebl <jirka@5z.com>
 */

#include <config.h>

/* needed for values of M_E and M_PI under 'gcc -ansi -pedantic'
 * on GNU/Linux */
#ifndef _BSD_SOURCE
#  define _BSD_SOURCE 1
#endif
#include <sys/types.h>

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <locale.h>
#include <errno.h> /* errno */
#include <signal.h> /* signal() */
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-i18n.h>
#include <libgnome/gnome-util.h>

#include "gnome-calc.h"
#include <gdk-pixbuf/gdk-pixbuf.h>

typedef void (*sighandler_t)(int);

#define FONT_WIDTH 20
#define FONT_HEIGHT 30
#define DISPLAY_LEN 13

struct _GnomeCalcPrivate {
	GtkWidget *display;

	GtkWidget *invert_button;
	GtkWidget *drg_button;

	GList *stack;
	GtkAccelGroup *accel;

	gdouble result;
	gdouble memory;

	gchar result_string[13];

	GnomeCalcMode mode : 2;

	guint add_digit : 1;	/*add a digit instead of starting a new
				  number*/
	guint error : 1;
	guint invert : 1;
};

typedef enum {
	CALCULATOR_NUMBER,
	CALCULATOR_FUNCTION,
	CALCULATOR_PARENTHESIS
} CalculatorActionType;

typedef gdouble (*MathFunction1) (gdouble);
typedef gdouble (*MathFunction2) (gdouble, gdouble);

typedef struct _CalculatorStack CalculatorStack;
struct _CalculatorStack {
	CalculatorActionType type;
	union {
		MathFunction2 func;
		gdouble number;
	} d;
};




static void gnome_calc_class_init	(GnomeCalcClass	*class);
static void gnome_calc_init	     (GnomeCalc	*gc);
static void gnome_calc_destroy	(GtkObject    	*object);
#if GCALC_GTK_2
static void gnome_calc_finalize	(GObject		*object);
#else
static GtkVBoxClass *parent_class;
#endif

/* The calculator font and our own reference count for it */
static GdkPixmap *calc_font;
static int calc_font_ref_count;

typedef struct _CalculatorButton CalculatorButton;
struct _CalculatorButton {
	char *name;
	GtkSignalFunc signal_func;
	gpointer data;
	gpointer invdata;
	gboolean convert_to_rad;
	guint keys[10]; /*key shortcuts, 0 terminated list,
			  make sure to increase the number if more is needed*/
};

typedef void (*GnomeCalcualtorResultChangedSignal) (GtkObject * object,
						    gdouble result,
						    gpointer data);


enum {
	RESULT_CHANGED_SIGNAL,
	LAST_SIGNAL
};

static gint gnome_calc_signals[LAST_SIGNAL] = {0};


#ifdef GCALC_GTK_2
GNOME_CLASS_BOILERPLATE (GnomeCalc, gnome_calculator,
			 GtkVBox, gtk_vbox)
#else
static void
gnome_calc_marshal_signal_result_changed (GtkObject * object,
					   GtkSignalFunc func,
					   gpointer func_data,
					   GtkArg * args)
{
	GnomeCalcualtorResultChangedSignal rfunc;

	rfunc = (GnomeCalcualtorResultChangedSignal) func;

	(*rfunc) (object, GTK_VALUE_DOUBLE (args[0]),
		  func_data);
}

guint
gnome_calc_get_type (void)
{
	static guint calculator_type = 0;

	if (!calculator_type) {
		GtkTypeInfo calculator_info = {
			"GnomeCalculator",
			sizeof (GnomeCalc),
			sizeof (GnomeCalcClass),
			(GtkClassInitFunc)  gnome_calc_class_init,
			(GtkObjectInitFunc) gnome_calc_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};

		calculator_type = gtk_type_unique (gtk_vbox_get_type (),
						   &calculator_info);
	}

	return calculator_type;
}

#endif

static void
gnome_calc_class_init (GnomeCalcClass *class)
{
	GtkObjectClass *object_class;
#ifdef GCALC_GTK_2
	GObjectClass *gobject_class;
#endif	
	
	object_class = (GtkObjectClass *) class;
#ifdef GCALC_GTK_2
	gobject_class = (GObjectClass *) class;
#else
	parent_class = gtk_type_class (gtk_vbox_get_type ());
#endif	
	object_class->destroy = gnome_calc_destroy;
#ifdef GCALC_GTK_2
	gobject_class->finalize = gnome_calc_finalize;
#endif	

	gnome_calc_signals[RESULT_CHANGED_SIGNAL] =
		gtk_signal_new("result_changed",
			       GTK_RUN_LAST,
#ifdef GCALC_GTK_2	
			       GTK_CLASS_TYPE(object_class),
#else
			       object_class->type,
#endif	
			       GTK_SIGNAL_OFFSET(GnomeCalcClass,
			       			 result_changed),
#ifdef GCALC_GTK_2	
			       gnome_marshal_VOID__DOUBLE,
#else
			       gnome_calc_marshal_signal_result_changed,
#endif	
			       GTK_TYPE_NONE,
			       1,
			       GTK_TYPE_DOUBLE);

	class->result_changed = NULL;
#ifdef GCALC_GTK_2
#else
	gtk_object_class_add_signals (object_class, gnome_calc_signals, LAST_SIGNAL);
#endif	
}

#if 0 /*only used for debugging*/
static void
dump_stack(GnomeCalc *gc)
{
	CalculatorStack *stack;
	GList *list;

	g_print ("STACK_DUMP start, we are in ");
	if (gc->_priv->add_digit)
		g_print ("add digit mode.\n");
	else
		g_print ("normal mode\n");
		
	for(list = gc->_priv->stack;list!=NULL;list = g_list_next(list)) {
		stack = list->data;
		if(stack == NULL)
			puts("NULL");
		else if(stack->type == CALCULATOR_PARENTHESIS)
			puts("(");
		else if(stack->type == CALCULATOR_NUMBER)
			printf("% .12g\n", stack->d.number);
		else if(stack->type == CALCULATOR_FUNCTION)
			puts("FUNCTION");
		else
			puts("UNKNOWN");
	}
	puts("STACK_DUMP end\n");
}
#endif

static void
stack_pop(GList **stack)
{
	CalculatorStack *s;
	GList *p;

	g_return_if_fail(stack);

	if(*stack == NULL) {
		g_warning("Stack underflow!");
		return;
	}

	s = (*stack)->data;
	p = (*stack)->next;
	g_list_free_1(*stack);
	if(p) p->prev = NULL;
	*stack = p;
	g_free(s);
}

static void
do_error(GnomeCalc *gc)
{
	gc->_priv->error = TRUE;
	strcpy(gc->_priv->result_string,"e");
	gtk_widget_queue_draw (gc->_priv->display);
}

/*we handle sigfpe's so that we can find all the errors*/
static void
sigfpe_handler(int type)
{
	/*most likely, but we don't really care what the value is*/
	errno = ERANGE;
}

static void
reduce_stack(GnomeCalc *gc)
{
	CalculatorStack *stack;
	GList *list;
	MathFunction2 func;
	gdouble first;
	gdouble second;

	if(!gc->_priv->stack)
		return;

	stack = gc->_priv->stack->data;
	if(stack->type!=CALCULATOR_NUMBER)
		return;

	second = stack->d.number;

	list=g_list_next(gc->_priv->stack);
	if(!list)
		return;

	stack = list->data;
	if(stack->type==CALCULATOR_PARENTHESIS)
		return;
	if(stack->type!=CALCULATOR_FUNCTION) {
		g_warning("Corrupt GnomeCalc stack!");
		return;
	}
	func = stack->d.func;

	list=g_list_next(list);
	if(!list) {
		g_warning("Corrupt GnomeCalc stack!");
		return;
	}

	stack = list->data;
	if(stack->type!=CALCULATOR_NUMBER) {
		g_warning("Corrupt GnomeCalc stack!");
		return;
	}
	first = stack->d.number;

	stack_pop(&gc->_priv->stack);
	stack_pop(&gc->_priv->stack);

	errno = 0;

	{
		sighandler_t old = signal(SIGFPE,sigfpe_handler);
		stack->d.number = (*func)(first,second);
		signal(SIGFPE,old);
	}

	if(errno>0 ||
	   finite(stack->d.number)==0) {
		errno = 0;
		do_error(gc);
	}
}

/* Move these up for find_precedence(). */
static gdouble
c_add(gdouble arg1, gdouble arg2)
{
	return arg1+arg2;
}

static gdouble
c_sub(gdouble arg1, gdouble arg2)
{
	return arg1-arg2;
}

static gdouble
c_mul(gdouble arg1, gdouble arg2)
{
	return arg1*arg2;
}

static gdouble
c_div(gdouble arg1, gdouble arg2)
{
	if(arg2==0) {
		errno=ERANGE;
		return 0;
	}
	return arg1/arg2;
}

static int
find_precedence(MathFunction2 f)
{
        if ( f == NULL || f == c_add || f == c_sub)
                return 0;
        else if ( f == c_mul || f == c_div )
                return 1;
        else
                return 2;
}

static void
reduce_stack_prec(GnomeCalc *gc, MathFunction2 func)
{
        CalculatorStack *stack;
        GList *list;

        stack = gc->_priv->stack->data;
        if ( stack->type != CALCULATOR_NUMBER )
                return;

        list = g_list_next(gc->_priv->stack);
        if (!list)
                return;

        stack = list->data;
        if ( stack->type == CALCULATOR_PARENTHESIS )
                return;

        if ( find_precedence(func) <= find_precedence(stack->d.func) ) {
                reduce_stack(gc);
                reduce_stack_prec(gc,func);
        }

        return;
}

static void
push_input(GnomeCalc *gc)
{
	if(gc->_priv->add_digit) {
		CalculatorStack *stack;
		stack = g_new(CalculatorStack,1);
		stack->type = CALCULATOR_NUMBER;
		stack->d.number = gc->_priv->result;
		gc->_priv->stack = g_list_prepend(gc->_priv->stack,stack);
		gc->_priv->add_digit = FALSE;
	}
}

static void
set_result(GnomeCalc *gc)
{
	CalculatorStack *stack;
	gchar buf[80];
	gchar format[20];
	gint i;
#ifdef GCALC_GTK_2
#else
	char *old_locale;
#endif	

	g_return_if_fail(gc!=NULL);

	if(!gc->_priv->stack)
		return;

	stack = gc->_priv->stack->data;
	if(stack->type!=CALCULATOR_NUMBER)
		return;

	gc->_priv->result = stack->d.number;

        /* make sure put values in a consistent manner */
	/* XXX: perhaps we can make sure the calculator works on all locales,
	 * but currently it will lose precision if we don't do this */
#ifdef GCALC_GTK_2	
	gnome_i18n_push_c_numeric_locale ();
#else
	old_locale = g_strdup (setlocale (LC_NUMERIC, NULL));
	setlocale (LC_NUMERIC, "C");
#endif	
	for (i = 12; i > 0; i--) {
		g_snprintf (format, sizeof (format), "%c .%dg", '%', i);
		g_snprintf (buf, sizeof (buf), format, gc->_priv->result);
		if (strlen (buf) <= 12)
			break;
	}
#ifdef GCALC_GTK_2	
	gnome_i18n_pop_c_numeric_locale ();
#else
	setlocale (LC_NUMERIC, old_locale);
	g_free (old_locale);
#endif
	strncpy(gc->_priv->result_string,buf,12);
	gc->_priv->result_string[12]='\0';

	gtk_widget_queue_draw (gc->_priv->display);

	gtk_signal_emit(GTK_OBJECT(gc),
			gnome_calc_signals[RESULT_CHANGED_SIGNAL],
			gc->_priv->result);

}

static void
unselect_invert(GnomeCalc *gc)
{
	g_return_if_fail(gc != NULL);
	g_return_if_fail(GNOME_IS_CALC(gc));
	g_return_if_fail(gc->_priv->invert_button);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gc->_priv->invert_button),
				    FALSE);
	gc->_priv->invert=FALSE;
}

static void
setup_drg_label(GnomeCalc *gc)
{
	GtkWidget *label;

	g_return_if_fail(gc != NULL);
	g_return_if_fail(GNOME_IS_CALC(gc));
	g_return_if_fail(gc->_priv->drg_button);

	label = GTK_BUTTON(gc->_priv->drg_button)->child;

	if(gc->_priv->mode == GNOME_CALC_DEG)
		gtk_label_set_text(GTK_LABEL(label), _("DEG"));
	else if(gc->_priv->mode == GNOME_CALC_RAD)
		gtk_label_set_text(GTK_LABEL(label), _("RAD"));
	else
		gtk_label_set_text(GTK_LABEL(label), _("GRAD"));
}

static gdouble
convert_num(gdouble num, GnomeCalcMode from, GnomeCalcMode to)
{
	if(to==from)
		return num;
	else if(from==GNOME_CALC_DEG)
		if(to==GNOME_CALC_RAD)
			return (num*M_PI)/180;
		else /*GRAD*/
			return (num*200)/180;
	else if(from==GNOME_CALC_RAD)
		if(to==GNOME_CALC_DEG)
			return (num*180)/M_PI;
		else /*GRAD*/
			return (num*200)/M_PI;
	else /*GRAD*/
		if(to==GNOME_CALC_DEG)
			return (num*180)/200;
		else /*RAD*/
			return (num*M_PI)/200;
}

static void
no_func(GtkWidget *w, gpointer data)
{
	GnomeCalc *gc = gtk_object_get_user_data(GTK_OBJECT(w));

	g_return_if_fail(gc!=NULL);

	if(gc->_priv->error)
		return;

	push_input(gc);

	/* if no stack, nothing happens */
	if(!gc->_priv->stack) {
		unselect_invert(gc);
		return;
	}

	reduce_stack_prec(gc,NULL);
	if(gc->_priv->error) return;
	set_result(gc);

	unselect_invert(gc);
}

static void
simple_func(GtkWidget *w, gpointer data)
{
	GnomeCalc *gc = gtk_object_get_user_data(GTK_OBJECT(w));
	CalculatorStack *stack;
	CalculatorButton *but = data;
	MathFunction1 func = but->data;
	MathFunction1 invfunc = but->invdata;

	g_return_if_fail(func!=NULL);
	g_return_if_fail(gc!=NULL);

	if(gc->_priv->error)
		return;

	push_input(gc);

	if(!gc->_priv->stack) {
		unselect_invert(gc);
		return;
	}

	stack = gc->_priv->stack->data;
	if(stack->type!=CALCULATOR_NUMBER) {
		unselect_invert(gc);
		return;
	}

	/*only convert non inverting functions*/
	if(!gc->_priv->invert && but->convert_to_rad)
		stack->d.number = convert_num(stack->d.number,
					      gc->_priv->mode,
					      GNOME_CALC_RAD);

	errno = 0;
	{
		sighandler_t old = signal(SIGFPE,sigfpe_handler);
		if(!gc->_priv->invert || invfunc==NULL)
			stack->d.number = (*func)(stack->d.number);
		else
			stack->d.number = (*invfunc)(stack->d.number);
		signal(SIGFPE,old);
	}

	if(errno>0 ||
	   finite(stack->d.number)==0) {
		errno = 0;
		do_error(gc);
		return;
	}

	/*we are converting back from rad to mode*/
	if(gc->_priv->invert && but->convert_to_rad)
		stack->d.number = convert_num(stack->d.number,
					      GNOME_CALC_RAD,
					      gc->_priv->mode);

	set_result(gc);

	unselect_invert(gc);
}

static void
math_func(GtkWidget *w, gpointer data)
{
	GnomeCalc *gc = gtk_object_get_user_data(GTK_OBJECT(w));
	CalculatorStack *stack;
	CalculatorButton *but = data;
	MathFunction2 func = but->data;
	MathFunction2 invfunc = but->invdata;

	g_return_if_fail(func!=NULL);
	g_return_if_fail(gc!=NULL);

	if(gc->_priv->error)
		return;

	push_input(gc);

	if(!gc->_priv->stack) {
		unselect_invert(gc);
		return;
	}

	reduce_stack_prec(gc,func);
	if(gc->_priv->error) return;
	set_result(gc);

	stack = gc->_priv->stack->data;
	if(stack->type!=CALCULATOR_NUMBER) {
		unselect_invert(gc);
		return;
	}

	stack = g_new(CalculatorStack,1);
	stack->type = CALCULATOR_FUNCTION;
	if(!gc->_priv->invert || invfunc==NULL)
		stack->d.func = func;
	else
		stack->d.func = invfunc;

	gc->_priv->stack = g_list_prepend(gc->_priv->stack,stack);

	unselect_invert(gc);
}

static void
reset_calc(GtkWidget *w, gpointer data)
{
	GnomeCalc *gc;
	if(w)
		gc = gtk_object_get_user_data(GTK_OBJECT(w));
	else
		gc = data;

	g_return_if_fail(gc!=NULL);

	while(gc->_priv->stack)
		stack_pop(&gc->_priv->stack);

	gc->_priv->result = 0;
	strcpy(gc->_priv->result_string, " 0");
	gc->_priv->memory = 0;
	gc->_priv->mode = GNOME_CALC_DEG;
	gc->_priv->invert = FALSE;
	gc->_priv->error = FALSE;

	gc->_priv->add_digit = TRUE;
	push_input(gc);
	set_result(gc);

	unselect_invert(gc);
	setup_drg_label(gc);
}

static void
clear_calc(GtkWidget *w, gpointer data)
{
	GnomeCalc *gc = gtk_object_get_user_data(GTK_OBJECT(w));

	g_return_if_fail(gc!=NULL);

	/* if in add digit mode, just clear the number, otherwise clear
	 * state as well */
	if(!gc->_priv->add_digit) {
		while(gc->_priv->stack)
			stack_pop(&gc->_priv->stack);
	}

	gc->_priv->result = 0;
	strcpy(gc->_priv->result_string, " 0");
	gc->_priv->error = FALSE;
	gc->_priv->invert = FALSE;

	gc->_priv->add_digit = TRUE;
	push_input(gc);
	set_result(gc);

	unselect_invert(gc);
}


/**
 * gnome_calc_clear
 * @gc: Pointer to GNOME calculator widget.
 * @reset: %FALSE to zero, %TRUE to reset calculator completely
 *
 * Description:
 * Resets the calculator back to zero.  If @reset is %TRUE, results
 * stored in memory and the calculator mode are cleared also.
 **/

void
gnome_calc_clear(GnomeCalc *gc, const gboolean reset)
{
	if (reset)
		reset_calc(NULL, gc);
	else
		clear_calc(NULL, gc);
}

static void
add_digit (GtkWidget *w, gpointer data)
{
	GnomeCalc *gc = gtk_object_get_user_data (GTK_OBJECT (w));
	CalculatorButton *but = data;
	gchar *digit = but->name;

	if(gc->_priv->error)
		return;

	/* For "EE" where name is not what we want to add */
	if (but->data != NULL)
		digit = but->data;

	g_return_if_fail(gc!=NULL);
	g_return_if_fail(digit!=NULL);

	if(!gc->_priv->add_digit) {
		if(gc->_priv->stack) {
			CalculatorStack *stack=gc->_priv->stack->data;
			if(stack->type==CALCULATOR_NUMBER)
				stack_pop(&gc->_priv->stack);
		}
		gc->_priv->add_digit = TRUE;
		gc->_priv->result_string[0] = '\0';
	}

	unselect_invert(gc);

	if(digit[0]=='e') {
		if(strchr(gc->_priv->result_string,'e'))
			return;
		else if(strlen(gc->_priv->result_string)>9)
			return;
		else if(gc->_priv->result_string[0]=='\0')
			strcpy(gc->_priv->result_string," 1");
	} else if(digit[0]=='.') {
		if(strchr(gc->_priv->result_string,'.'))
			return;
		else if(strlen(gc->_priv->result_string)>10)
			return;
		else if(gc->_priv->result_string[0]=='\0')
			strcpy(gc->_priv->result_string," 0");
	} else { /*numeric*/
		if(strlen(gc->_priv->result_string)>11)
			return;
		else if (strcmp (gc->_priv->result_string, " 0") == 0 ||
			 gc->_priv->result_string[0]=='\0')
			strcpy(gc->_priv->result_string," ");
	}

	strcat(gc->_priv->result_string,digit);

	gtk_widget_queue_draw (gc->_priv->display);

#ifdef GCALC_GTK_2	
        /* make sure get values in a consistent manner */
	gnome_i18n_push_c_numeric_locale ();
	sscanf(gc->_priv->result_string, "%lf", &gc->_priv->result);
	gnome_i18n_pop_c_numeric_locale ();
#else
	{
		gchar *old_locale;
		/* make sure get values in a consistent manner */
		old_locale = g_strdup (setlocale (LC_NUMERIC, NULL));
		setlocale (LC_NUMERIC, "C");
		sscanf(gc->_priv->result_string, "%lf", &gc->_priv->result);
		setlocale (LC_NUMERIC, old_locale);
		g_free (old_locale);
	}
#endif	
	
}

static gdouble
c_neg(gdouble arg)
{
	return -arg;
}


static void
negate_val(GtkWidget *w, gpointer data)
{
	GnomeCalc *gc = gtk_object_get_user_data(GTK_OBJECT(w));
	char *p;

	g_return_if_fail(gc!=NULL);

	if(gc->_priv->error)
		return;

	unselect_invert(gc);

	if(!gc->_priv->add_digit) {
		simple_func(w,data);
		return;
	}

	if((p=strchr(gc->_priv->result_string,'e'))!=NULL) {
		p++;
		if(*p=='-')
			*p='+';
		else
			*p='-';
	} else {
		if(gc->_priv->result_string[0]=='-')
			gc->_priv->result_string[0]=' ';
		else
			gc->_priv->result_string[0]='-';
	}

        /* make sure get values in a consistent manner */
#ifdef GCALC_GTK_2	
	gnome_i18n_pop_c_numeric_locale ();
	sscanf(gc->_priv->result_string, "%lf", &gc->_priv->result);
	gnome_i18n_push_c_numeric_locale ();
#else	
	{
		gchar *old_locale;
		/* make sure get values in a consistent manner */
		old_locale = g_strdup (setlocale (LC_NUMERIC, NULL));
		setlocale (LC_NUMERIC, "C");
		sscanf(gc->_priv->result_string, "%lf", &gc->_priv->result);
		setlocale (LC_NUMERIC, old_locale);
		g_free (old_locale);
	}
#endif	
	
	gtk_widget_queue_draw (gc->_priv->display);
}

static gdouble
c_inv(gdouble arg1)
{
	if(arg1==0) {
		errno=ERANGE;
		return 0;
	}
	return 1/arg1;
}

static gdouble
c_pow2(gdouble arg1)
{
	return pow(arg1,2);
}

static gdouble
c_pow10(gdouble arg1)
{
	return pow(10,arg1);
}

static gdouble
c_powe(gdouble arg1)
{
	return pow(M_E,arg1);
}

static gdouble
c_fact(gdouble arg1)
{
	int i;
	gdouble r;
	if(arg1<0) {
		errno=ERANGE;
		return 0;
	}
	i = (int)(arg1+0.5);
	if((fabs(arg1-i))>1e-9) {
		errno=ERANGE;
		return 0;
	}
	for(r=1;i>0;i--)
		r*=i;

	return r;
}

static gdouble
set_result_to(GnomeCalc *gc, gdouble result)
{
	gdouble old;

	if(gc->_priv->stack==NULL ||
	   ((CalculatorStack *)gc->_priv->stack->data)->type!=CALCULATOR_NUMBER) {
		gc->_priv->add_digit = TRUE;
		old = gc->_priv->result;
		gc->_priv->result = result;
		push_input(gc);
	} else {
		old = ((CalculatorStack *)gc->_priv->stack->data)->d.number;
		((CalculatorStack *)gc->_priv->stack->data)->d.number = result;
	}

	set_result(gc);

	return old;
}


/**
 * gnome_calc_set
 * @gc: Pointer to GNOME calculator widget.
 * @result: New value of calculator buffer.
 *
 * Description:  Sets the value stored in the calculator's result buffer to the
 * given @result.
 **/

void
gnome_calc_set(GnomeCalc *gc, gdouble result)
{
	set_result_to(gc,result);
}

static void
store_m(GtkWidget *w, gpointer data)
{
	GnomeCalc *gc = gtk_object_get_user_data(GTK_OBJECT(w));

	g_return_if_fail(gc!=NULL);

	if(gc->_priv->error)
		return;

	push_input(gc);

	gc->_priv->memory = gc->_priv->result;

	gtk_widget_queue_draw (gc->_priv->display);

	unselect_invert(gc);
}

static void
recall_m(GtkWidget *w, gpointer data)
{
	GnomeCalc *gc = gtk_object_get_user_data(GTK_OBJECT(w));

	g_return_if_fail(gc!=NULL);

	if(gc->_priv->error)
		return;

	set_result_to(gc,gc->_priv->memory);

	unselect_invert(gc);
}

static void
sum_m(GtkWidget *w, gpointer data)
{
	GnomeCalc *gc = gtk_object_get_user_data(GTK_OBJECT(w));

	g_return_if_fail(gc!=NULL);

	if(gc->_priv->error)
		return;

	push_input(gc);

	gc->_priv->memory += gc->_priv->result;

	gtk_widget_queue_draw (gc->_priv->display);

	unselect_invert(gc);
}

static void
exchange_m(GtkWidget *w, gpointer data)
{
	GnomeCalc *gc = gtk_object_get_user_data(GTK_OBJECT(w));

	g_return_if_fail(gc!=NULL);

	if(gc->_priv->error)
		return;

	gc->_priv->memory = set_result_to(gc,gc->_priv->memory);

	unselect_invert(gc);
}

static void
invert_toggle(GtkWidget *w, gpointer data)
{
	GnomeCalc *gc = gtk_object_get_user_data(GTK_OBJECT(w));

	g_return_if_fail(gc!=NULL);

	if(gc->_priv->error)
		return;

	if(GTK_TOGGLE_BUTTON(w)->active)
		gc->_priv->invert=TRUE;
	else
		gc->_priv->invert=FALSE;
}

static void
drg_toggle(GtkWidget *w, gpointer data)
{
	GnomeCalc *gc = gtk_object_get_user_data(GTK_OBJECT(w));
	GnomeCalcMode oldmode;

	g_return_if_fail(gc!=NULL);

	if(gc->_priv->error)
		return;

	oldmode = gc->_priv->mode;

	if(gc->_priv->mode==GNOME_CALC_DEG)
		gc->_priv->mode=GNOME_CALC_RAD;
	else if(gc->_priv->mode==GNOME_CALC_RAD)
		gc->_priv->mode=GNOME_CALC_GRAD;
	else
		gc->_priv->mode=GNOME_CALC_DEG;

	setup_drg_label(gc);

	/*convert if invert is on*/
	if(gc->_priv->invert) {
		CalculatorStack *stack;
		push_input(gc);
		stack = gc->_priv->stack->data;
		stack->d.number = convert_num(stack->d.number,
					      oldmode,gc->_priv->mode);
		set_result(gc);
	}

	unselect_invert(gc);
}

static void
set_pi(GtkWidget *w, gpointer data)
{
	GnomeCalc *gc = gtk_object_get_user_data(GTK_OBJECT(w));

	g_return_if_fail(gc!=NULL);

	if(gc->_priv->error)
		return;

	set_result_to(gc,M_PI);

	unselect_invert(gc);
}

static void
set_e(GtkWidget *w, gpointer data)
{
	GnomeCalc *gc = gtk_object_get_user_data(GTK_OBJECT(w));

	g_return_if_fail(gc!=NULL);

	if(gc->_priv->error)
		return;

	set_result_to(gc,M_E);

	unselect_invert(gc);
}

static void
add_parenth(GtkWidget *w, gpointer data)
{
	GnomeCalc *gc = gtk_object_get_user_data(GTK_OBJECT(w));

	g_return_if_fail(gc!=NULL);

	if(gc->_priv->error)
		return;

	if(gc->_priv->stack &&
	   ((CalculatorStack *)gc->_priv->stack->data)->type==CALCULATOR_NUMBER)
		((CalculatorStack *)gc->_priv->stack->data)->type =
			CALCULATOR_PARENTHESIS;
	else {
		CalculatorStack *stack;
		stack = g_new(CalculatorStack,1);
		stack->type = CALCULATOR_PARENTHESIS;
		gc->_priv->stack = g_list_prepend(gc->_priv->stack,stack);
	}

	unselect_invert(gc);
}

static void
sub_parenth(GtkWidget *w, gpointer data)
{
	GnomeCalc *gc = gtk_object_get_user_data(GTK_OBJECT(w));
	g_return_if_fail(gc!=NULL);

	if(gc->_priv->error)
		return;

	push_input(gc);
	reduce_stack_prec(gc,NULL);
	if(gc->_priv->error) return;
	set_result(gc);

	if(gc->_priv->stack) {
		CalculatorStack *stack = gc->_priv->stack->data;
		if(stack->type==CALCULATOR_PARENTHESIS)
			stack_pop(&gc->_priv->stack);
		else if(g_list_next(gc->_priv->stack)) {
			stack = g_list_next(gc->_priv->stack)->data;
			if(stack->type==CALCULATOR_PARENTHESIS) {
				GList *n = g_list_next(gc->_priv->stack);
				gc->_priv->stack = g_list_remove_link(gc->_priv->stack,n);
				g_list_free_1(n);
			}
		}
	}

	unselect_invert(gc);
}

static const CalculatorButton buttons[8][5] = {
	{
		{N_("1/x"),  (GtkSignalFunc)simple_func, c_inv,  NULL,   FALSE, {0} },
		{N_("x^2"),  (GtkSignalFunc)simple_func, c_pow2, sqrt,   FALSE, {0} },
		{N_("SQRT"), (GtkSignalFunc)simple_func, sqrt,   c_pow2, FALSE, {'r','R',0} },
		{N_("CE/C"), (GtkSignalFunc)clear_calc,  NULL,   NULL,   FALSE, {GDK_Clear,GDK_Delete,0} },
		{N_("AC"),   (GtkSignalFunc)reset_calc,  NULL,   NULL,   FALSE, {'a','A', GDK_Escape, 0} }
	},{
		{N_("INV"),  NULL,                       NULL,   NULL,   FALSE, {'i','I',0} }, /*inverse button*/
		{N_("sin"),  (GtkSignalFunc)simple_func, sin,    asin,   TRUE,  {'s','S',0} },
		{N_("cos"),  (GtkSignalFunc)simple_func, cos,    acos,   TRUE,  {'c','C',0} },
		{N_("tan"),  (GtkSignalFunc)simple_func, tan,    atan,   TRUE,  {'t','T',0} },
		{N_("DEG"),  (GtkSignalFunc)drg_toggle,  NULL,   NULL,   FALSE, {'d','D',0} }
	},{
		{N_("e"),    (GtkSignalFunc)set_e,       NULL,   NULL,   FALSE, {'e','E',0} },
		{N_("EE"),   (GtkSignalFunc)add_digit,   "e+",   NULL,   FALSE, {0} },
		{N_("log"),  (GtkSignalFunc)simple_func, log10,  c_pow10,FALSE, {0} },
		{N_("ln"),   (GtkSignalFunc)simple_func, log,    c_powe, FALSE, {'l','L',0} },
		{N_("x^y"),  (GtkSignalFunc)math_func,   pow,    NULL,   FALSE, {'^',0} }
	},{
		{N_("PI"),   (GtkSignalFunc)set_pi,      NULL,   NULL,   FALSE, {'p','P',0}},
		{N_("x!"),   (GtkSignalFunc)simple_func, c_fact, NULL,   FALSE, {'!',0} },
		{N_("("),    (GtkSignalFunc)add_parenth, NULL,   NULL,   FALSE, {'(',0} },
		{N_(")"),    (GtkSignalFunc)sub_parenth, NULL,   NULL,   FALSE, {')',0} },
		{N_("/"),    (GtkSignalFunc)math_func,   c_div,  NULL,   FALSE, {'/',GDK_KP_Divide,0} }
	},{
		{N_("STO"),  (GtkSignalFunc)store_m,     NULL,   NULL,   FALSE, {0} },
		{N_("7"),    (GtkSignalFunc)add_digit,   NULL,   NULL,   FALSE, {'7',GDK_KP_7,GDK_KP_Home,0} },
		{N_("8"),    (GtkSignalFunc)add_digit,   NULL,   NULL,   FALSE, {'8',GDK_KP_8,GDK_KP_Up,0} },
		{N_("9"),    (GtkSignalFunc)add_digit,   NULL,   NULL,   FALSE, {'9',GDK_KP_9,GDK_KP_Page_Up,0} },
		{N_("*"),    (GtkSignalFunc)math_func,   c_mul,  NULL,   FALSE, {'*',GDK_KP_Multiply,0} }
	},{
		{N_("RCL"),  (GtkSignalFunc)recall_m,    NULL,   NULL,   FALSE, {0} },
		{N_("4"),    (GtkSignalFunc)add_digit,   NULL,   NULL,   FALSE, {'4',GDK_KP_4,GDK_KP_Left,0} },
		{N_("5"),    (GtkSignalFunc)add_digit,   NULL,   NULL,   FALSE, {'5',GDK_KP_5,GDK_KP_Begin,0} },
		{N_("6"),    (GtkSignalFunc)add_digit,   NULL,   NULL,   FALSE, {'6',GDK_KP_6,GDK_KP_Right,0} },
		{N_("-"),    (GtkSignalFunc)math_func,   c_sub,  NULL,   FALSE, {'-',GDK_KP_Subtract,0} }
	},{
		{N_("SUM"),  (GtkSignalFunc)sum_m,       NULL,   NULL,   FALSE, {0} },
		{N_("1"),    (GtkSignalFunc)add_digit,   NULL,   NULL,   FALSE, {'1',GDK_KP_1,GDK_KP_End,0} },
		{N_("2"),    (GtkSignalFunc)add_digit,   NULL,   NULL,   FALSE, {'2',GDK_KP_2,GDK_KP_Down,0} },
		{N_("3"),    (GtkSignalFunc)add_digit,   NULL,   NULL,   FALSE, {'3',GDK_KP_3,GDK_KP_Page_Down,0} },
		{N_("+"),    (GtkSignalFunc)math_func,   c_add,  NULL,   FALSE, {'+',GDK_KP_Add,0} }
	},{
		{N_("EXC"),  (GtkSignalFunc)exchange_m,  NULL,   NULL,   FALSE, {0} },
		{N_("0"),    (GtkSignalFunc)add_digit,   NULL,   NULL,   FALSE, {'0',GDK_KP_0,GDK_KP_Insert,0} },
		{N_("."),    (GtkSignalFunc)add_digit,   NULL,   NULL,   FALSE, {'.',GDK_KP_Decimal,',',GDK_KP_Delete,0} },
		{N_("+/-"),  (GtkSignalFunc)negate_val,  c_neg,  NULL,   FALSE, {0} },
		{N_("="),    (GtkSignalFunc)no_func,     NULL,   NULL,   FALSE, {'=',GDK_KP_Enter,0} }
	}
};

/* Loads the font for the calculator if necessary, or adds a reference count to it */
static void
ref_font (void)
{
	char *filename;
	GdkPixbuf *pb;
#ifdef GCALC_GTK_2	
	GError *error;
#endif	

	if (calc_font) {
		g_assert (calc_font_ref_count > 0);

		calc_font_ref_count++;
		return;
	}

	g_assert (calc_font_ref_count == 0);

	/* Translators:  This allows you to create translated font pngs,
	 * though probably won't be necessary */
	filename = gnome_pixmap_file (_("calculator-font.png"));
	if (filename == NULL)
		filename = gnome_pixmap_file ("calculator-font.png");
	if (filename == NULL) {
		g_message ("ref_font(): could not find calculator-font.png");
		return;
	}

#ifdef GCALC_GTK_2	
	error = NULL;
#endif	
	pb = gdk_pixbuf_new_from_file(filename /*, &error */);
	if (!pb) {
#ifdef GCALC_GTK_2
		g_message (G_STRLOC ": could not load %s: %s", filename,
			   error->message);
#else
		g_warning ("Could not load %s\n", filename);
#endif	
		g_free (filename);
#ifdef GCALC_GTK_2	
		g_error_free (error);
#endif
		return;
	}
	g_free (filename);

	gdk_pixbuf_render_pixmap_and_mask(pb, &calc_font, NULL, 128);
	gdk_pixbuf_unref (pb);

	if (!calc_font) {
		g_message ("ref_font(): could not render the calculator font");
		return;
	}

	calc_font_ref_count = 1;
}

/* Unrefs the calculator font pixmap and destroys it if necessary */
static void
unref_font (void)
{
	g_assert (calc_font_ref_count > 0);
	g_assert (calc_font != NULL);

	if (calc_font_ref_count > 1) {
		calc_font_ref_count--;
		return;
	}

	calc_font_ref_count = 0;
	gdk_pixmap_unref (calc_font);
	calc_font = NULL;
}

/* Expose handler for the calculator display drawing area */
static gint
display_expose (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	GnomeCalc *calc;
	GdkWindow *window;
	GdkGC *gc;
	char *text;
	int x, i;

	int win_width, win_height;

	calc = GNOME_CALC (data);

	window = calc->_priv->display->window;
	gc = calc->_priv->display->style->black_gc;

	gdk_window_get_size (window, &win_width, &win_height);

	gdk_draw_rectangle (window, gc, TRUE, 0, 0, -1, -1);

	/* If the font could not be loaded, just bail out */
	if (!calc_font)
		return TRUE;

	if (calc->_priv->memory != 0)
		gdk_draw_pixmap (window, gc, calc_font,
				 13 * FONT_WIDTH, 0, win_width, 0, FONT_WIDTH, FONT_HEIGHT);

	text = calc->_priv->result_string;
	i = strlen (text) - 1;
	for (x = 1; i >= 0; x++, i--) {
		if (text[i] >= '0' && text[i] <= '9') {
			gdk_draw_pixmap (window, gc, calc_font,
					 (text[i] - '0') * FONT_WIDTH, 0,
					 win_width - x * FONT_WIDTH, 0,
					 FONT_WIDTH, FONT_HEIGHT);
		}
		else if (text[i] == '.') {
			gdk_draw_pixmap (window, gc, calc_font,
					 10 * FONT_WIDTH, 0,
					 win_width - x * FONT_WIDTH, 0,
					 FONT_WIDTH, FONT_HEIGHT);
		}
		else if (text[i] == '+') {
			gdk_draw_pixmap (window, gc, calc_font,
					 11 * FONT_WIDTH, 0,
					 win_width - x * FONT_WIDTH, 0,
					 FONT_WIDTH, FONT_HEIGHT);
		}
		else if (text[i] == '-') {
			gdk_draw_pixmap (window, gc, calc_font,
					 12 * FONT_WIDTH, 0,
					 win_width - x * FONT_WIDTH, 0,
					 FONT_WIDTH, FONT_HEIGHT);
		}
		else if (text[i] == 'e') {
			gdk_draw_pixmap (window, gc, calc_font,
					 14 * FONT_WIDTH, 0,
					 win_width - x * FONT_WIDTH, 0,
					 FONT_WIDTH, FONT_HEIGHT);
		}
	}

	return TRUE;
}

static void
create_button(GnomeCalc *gc, GtkWidget *table, int x, int y)
{
	const CalculatorButton *but = &buttons[y][x];
	GtkWidget *w;
	int i;

	if(!but->name)
		return;

	if (strcmp (but->name, "INV") == 0) {
		w = gtk_toggle_button_new_with_label(_("INV"));
		gc->_priv->invert_button = w;
		gtk_signal_connect (GTK_OBJECT (w), "toggled",
				    GTK_SIGNAL_FUNC(invert_toggle), gc);
	} else {
		w = gtk_button_new_with_label(_(but->name));
		gtk_signal_connect(GTK_OBJECT(w), "clicked",
				   but->signal_func,
				   (gpointer) but);
	}

	for(i=0;but->keys[i]!=0;i++) {
		gtk_widget_add_accelerator(w, "clicked",
					   gc->_priv->accel,
					   but->keys[i], 0,
					   GTK_ACCEL_VISIBLE);
		gtk_widget_add_accelerator(w, "clicked",
					   gc->_priv->accel,
					   but->keys[i],
					   GDK_SHIFT_MASK,
					   GTK_ACCEL_VISIBLE);
		gtk_widget_add_accelerator(w, "clicked",
					   gc->_priv->accel,
					   but->keys[i],
					   GDK_LOCK_MASK,
					   GTK_ACCEL_VISIBLE);
	}
	gtk_object_set_user_data(GTK_OBJECT(w),gc);
	gtk_widget_show(w);
	gtk_table_attach(GTK_TABLE(table), w,
			 x, x+1, y, y+1,
			 GTK_FILL | GTK_EXPAND |
			 GTK_SHRINK,
			 GTK_FILL | GTK_EXPAND | GTK_SHRINK, 2, 2);

	/* if this is the DRG button, remember it's pointer */
	if(but->signal_func == GTK_SIGNAL_FUNC(drg_toggle))
		gc->_priv->drg_button = w;
}

static void
gnome_calc_init (GnomeCalc *gc)
{
	gint x,y;
	GtkWidget *table;

	gc->_priv = g_new0(GnomeCalcPrivate, 1);

	ref_font ();

	gtk_widget_push_colormap (gdk_rgb_get_cmap ());

	gc->_priv->display = gtk_drawing_area_new ();
	gtk_drawing_area_size (GTK_DRAWING_AREA (gc->_priv->display), DISPLAY_LEN * FONT_WIDTH, FONT_HEIGHT);

	gtk_widget_pop_colormap ();

	gtk_signal_connect (GTK_OBJECT (gc->_priv->display), "expose_event",
			    GTK_SIGNAL_FUNC (display_expose),
			    gc);

	gtk_box_pack_start (GTK_BOX (gc), gc->_priv->display, FALSE, FALSE, 0);
	gtk_widget_show (gc->_priv->display);

	gc->_priv->stack = NULL;
	gc->_priv->result = 0;
	strcpy(gc->_priv->result_string," 0");
	gc->_priv->memory = 0;
	gc->_priv->mode = GNOME_CALC_DEG;
	gc->_priv->invert = FALSE;
	gc->_priv->add_digit = TRUE;
	gc->_priv->accel = gtk_accel_group_new();

	table = gtk_table_new(8,5,TRUE);
	gtk_widget_show(table);

	gtk_box_pack_end(GTK_BOX(gc),table,TRUE,TRUE,0);

	for(x=0;x<5;x++) {
		for(y=0;y<8;y++) {
			create_button(gc, table, x, y);
		}
	}
}


/**
 * gnome_calc_new:
 *
 * Description: Creates a calculator widget, a window with all the common
 * buttons and functions found on a standard pocket calculator.
 *
 * Returns: Pointer to newly-created calculator widget.
 **/

GtkWidget *
gnome_calc_new (void)
{
	GnomeCalc *gcalculator;

	gcalculator = gtk_type_new (gnome_calc_get_type ());

	return GTK_WIDGET (gcalculator);
}

static void
gnome_calc_destroy (GtkObject *object)
{
	GnomeCalc *gc;

	/* remember, destroy can be run multiple times! */

	g_return_if_fail (object != NULL);
	g_return_if_fail (GNOME_IS_CALC (object));

	gc = GNOME_CALC (object);

	while(gc->_priv->stack)
		stack_pop(&gc->_priv->stack);

#ifdef GCALC_GTK_2	
	GNOME_CALL_PARENT_HANDLER (GTK_OBJECT_CLASS, destroy, (object));
#else
	unref_font ();
	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
#endif	
}

#ifdef GCALC_GTK_2	
static void
gnome_calc_finalize (GObject *object)
{
	GnomeCalc *gc;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GNOME_IS_CALC (object));

	gc = GNOME_CALC (object);

	if(gc->_priv) {
		unref_font ();

		g_free(gc->_priv);
		gc->_priv = NULL;
	}

	GNOME_CALL_PARENT_HANDLER (G_OBJECT_CLASS, finalize, (object));
}
#endif

/**
 * gnome_calc_get_result
 * @gc: Pointer to GNOME calculator widget
 *
 * Description:  Gets the value of the result buffer as a double.
 * This should read off whatever is currently on the display of the
 * calculator.
 *
 * Returns:  A double precision result.
 **/

gdouble
gnome_calc_get_result (GnomeCalc *gc)
{
	g_return_val_if_fail (gc, 0.0);
	g_return_val_if_fail (GNOME_IS_CALC (gc), 0.0);

	return gc->_priv->result;
}

/**
 * gnome_calc_get_accel_group
 * @gc: Pointer to GNOME calculator widget
 *
 * Description:  Gets the accelerator group which you can add to 
 * the toplevel window or wherever you want to catch the keys for this
 * widget.
 *
 * Returns:  The accelerator group
 **/

GtkAccelGroup   *
gnome_calc_get_accel_group(GnomeCalc *gc)
{
	g_return_val_if_fail (gc, NULL);
	g_return_val_if_fail (GNOME_IS_CALC (gc), NULL);

	return gc->_priv->accel;
}
/**
 * gnome_calc_get_result_string:
 * @gc: Pointer to GNOME calculator widget
 *
 * Description:  Gets the internal string representation of the result.
 *
 * Returns:  Internal string pointer, do not free
 **/
const char *
gnome_calc_get_result_string(GnomeCalc *gc)
{
	g_return_val_if_fail (gc, NULL);
	g_return_val_if_fail (GNOME_IS_CALC (gc), NULL);

	return gc->_priv->result_string;
}




/**
 * delete_digit:
 * @w: 
 * @data: 
 * 
 * Handle the backspace event
 **/
static void
backspace_cb (GtkWidget *w, gpointer data)
{
	GnomeCalc *gc = gtk_object_get_user_data(GTK_OBJECT(w));
	char *old_locale;
	gint length;

	g_return_if_fail (GNOME_IS_CALC (gc));

	if(gc->_priv->error)
		return;

	unselect_invert(gc);

	length = strlen (gc->_priv->result_string);

	/* The behaivor that I expect from the calculator when the backspace key
	 * is pressed is :
	 * - If i was typing a number, i expect to delete the last char
	 * - If i have a result already, i expect to clear the calculator
	 */
	if (!gc->_priv->add_digit) {
		clear_calc (w, data);
		return;
	}

	if (atof (gc->_priv->result_string) == 0.0)
		return;
	
	gc->_priv->result_string [length - 1] = 0;
	if (atof (gc->_priv->result_string) == 0.0) {
		gc->_priv->result_string [0] = ' ';
		gc->_priv->result_string [1] = '0';
		gc->_priv->result_string [2] = 0;
	}

	gc->_priv->result = atof (gc->_priv->result_string);
	if (gc->_priv->stack) {
		CalculatorStack *stack = gc->_priv->stack->data;
		if (stack->type == CALCULATOR_NUMBER)
			stack->d.number = gc->_priv->result;
	}
	
	gtk_widget_queue_draw (gc->_priv->display);

	/* make sure get values in a consistent manner */
	old_locale = g_strdup (setlocale (LC_NUMERIC, NULL));
	setlocale (LC_NUMERIC, "C");
	sscanf(gc->_priv->result_string, "%lf", &gc->_priv->result);
	setlocale (LC_NUMERIC, old_locale);
	g_free (old_locale);
}

/*
 * This code adds the ability to bound other keys to certain functions
 * usefull for keys that can't be bound as an accelerator or functions
 * that do not have a button in the calculator. Chema
 */
typedef struct _GnomeCalcExtraKeys GnomeCalcExtraKeys;

struct _GnomeCalcExtraKeys {
	gint key;
	GtkSignalFunc signal_func;
	gpointer data;
};

static const GnomeCalcExtraKeys extra_keys [] = {
	{GDK_BackSpace, backspace_cb, NULL},
	{GDK_Return,    no_func,      NULL},
	{GDK_KP_Enter,  no_func,      NULL},
	{GDK_Delete,    clear_calc,   NULL},
	{GDK_KP_Delete, add_digit,    "."},
	{GDK_KP_Left,   add_digit,    "4"},
	{GDK_KP_Right,  add_digit,    "6"},
	{GDK_KP_Up,     add_digit,    "8"},
	{GDK_KP_Down,   add_digit,    "2"}
};
	
static gboolean
event_cb (GtkWidget *widget, GdkEvent *event, gpointer calc)
{
	GnomeCalc *gc;
	gint i, num;
	GdkEventKey *kevent;

	if (event->type != GDK_KEY_PRESS)
		return FALSE;

	kevent = (GdkEventKey *)event;

	gc = (GnomeCalc *) calc;
	g_return_val_if_fail (GNOME_IS_CALC (gc), FALSE);

	num = sizeof (extra_keys) / sizeof (GnomeCalcExtraKeys);

	for (i = 0; i < num; i++) {
		if (kevent->keyval == extra_keys[i].key) {
			CalculatorButton but = {NULL};
			GtkSignalFunc func = extra_keys [i].signal_func;
			but.data = extra_keys[i].data;
			(* func) (widget, &but);
			return TRUE;
		}
	}

	return FALSE;
}


void
gnome_calc_bind_extra_keys (GnomeCalc *gc,
			    GtkWidget *widget)
{
	g_return_if_fail (GNOME_IS_CALC (gc));
	g_return_if_fail (GTK_IS_WIDGET (widget));

	gtk_object_set_user_data (GTK_OBJECT (widget), gc);
	
	gtk_signal_connect (GTK_OBJECT (widget), "event",
			    GTK_SIGNAL_FUNC (event_cb), gc);

}
