#include "marshal.h"

// Most of this code is taken from gtk+

// These must match gtk+ system declaration
struct _GtkSignal
{
  guint               signal_id;
  GtkType             object_type;
  gchar              *name;
  guint               function_offset;
  GtkSignalMarshaller marshaller;
  GtkType             return_val;
  guint               signal_flags : 16;
  guint               nparams : 16;
  GtkType            *params;
  GHookList          *hook_list;
};


typedef _GtkSignal GtkSignal;

extern GtkSignal                *_gtk_private_signals;
extern guint                     _gtk_private_n_signals;


#define LOOKUP_SIGNAL_ID(signal_id)     ( \
  signal_id > 0 && signal_id < _gtk_private_n_signals ? \
    (GtkSignal*) _gtk_private_signals + signal_id : \
    (GtkSignal*) 0 \
)


/* internal for gtk+ wrappers */
guint
gtkmm_signal_set_marshal (guint signal_id,GtkSignalMarshaller  marshaller)
{
  GtkSignal *signal;
  
  g_return_val_if_fail (signal_id >= 1, 0);

  signal = LOOKUP_SIGNAL_ID (signal_id);
  signal->marshaller = marshaller;

  return 1;
}

bool ignore_=0;

void gtkmm_set_ignore()
 { ignore_=true; }

bool gtkmm_clear_ignore()
 { bool i=ignore_; ignore_=false; return i; }

/******************************************************************/

typedef gboolean (*GtkSignal_BOOL__NONE) (GtkObject * object,
					  gpointer user_data);
void 
gtkmm_marshal_BOOL__NONE (GtkObject * object,
			GtkSignalFunc func,
			gpointer func_data,
			GtkArg * args)
{
  GtkSignal_BOOL__NONE rfunc;
  gboolean *return_val,rc;
  return_val = GTK_RETLOC_BOOL (args[0]);
  rfunc = (GtkSignal_BOOL__NONE) func;
  ignore_=false;
  rc = (*rfunc) (object,
			  func_data);
  if (!ignore_) *return_val=rc;
  ignore_=false;
}

typedef gboolean (*GtkSignal_BOOL__POINTER) (GtkObject * object,
					     gpointer arg1,
					     gpointer user_data);
void 
gtkmm_marshal_BOOL__POINTER (GtkObject * object,
			   GtkSignalFunc func,
			   gpointer func_data,
			   GtkArg * args)
{
  GtkSignal_BOOL__POINTER rfunc;
  gboolean *return_val,rc;
  return_val = GTK_RETLOC_BOOL (args[1]);
  rfunc = (GtkSignal_BOOL__POINTER) func;
  ignore_=false;
  rc = (*rfunc) (object,
			  GTK_VALUE_POINTER (args[0]),
			  func_data);
  if (!ignore_) *return_val=rc;
  ignore_=false;
}

typedef gboolean (*GtkSignal_BOOL__POINTER_POINTER_INT_INT) (GtkObject * object,
							     gpointer arg1,
							     gpointer arg2,
							     gint arg3,
							     gint arg4,
							gpointer user_data);
void 
gtkmm_marshal_BOOL__POINTER_POINTER_INT_INT (GtkObject * object,
					   GtkSignalFunc func,
					   gpointer func_data,
					   GtkArg * args)
{
  GtkSignal_BOOL__POINTER_POINTER_INT_INT rfunc;
  gboolean *return_val,rc;
  return_val = GTK_RETLOC_BOOL (args[4]);
  rfunc = (GtkSignal_BOOL__POINTER_POINTER_INT_INT) func;
  ignore_=false;
  rc = (*rfunc) (object,
			  GTK_VALUE_POINTER (args[0]),
			  GTK_VALUE_POINTER (args[1]),
			  GTK_VALUE_INT (args[2]),
			  GTK_VALUE_INT (args[3]),
			  func_data);
  if (!ignore_) *return_val=rc;
  ignore_=false;
}


typedef gboolean (*GtkSignal_BOOL__POINTER_INT_INT) (GtkObject * object,
						     gpointer arg1,
						     gint arg2,
						     gint arg3,
						     gpointer user_data);
void 
gtkmm_marshal_BOOL__POINTER_INT_INT (GtkObject * object,
				   GtkSignalFunc func,
				   gpointer func_data,
				   GtkArg * args)
{
  GtkSignal_BOOL__POINTER_INT_INT rfunc;
  gboolean *return_val,rc;
  return_val = GTK_RETLOC_BOOL (args[3]);
  rfunc = (GtkSignal_BOOL__POINTER_INT_INT) func;
  ignore_=false;
  rc = (*rfunc) (object,
			  GTK_VALUE_POINTER (args[0]),
			  GTK_VALUE_INT (args[1]),
			  GTK_VALUE_INT (args[2]),
			  func_data);
  if (!ignore_) *return_val=rc;
  ignore_=false;
}

typedef gboolean (*GtkSignal_BOOL__POINTER_INT_INT_INT) (GtkObject * object,
							 gpointer arg1,
							 gint arg2,
							 gint arg3,
							 gint arg4,
							 gpointer user_data);
void 
gtkmm_marshal_BOOL__POINTER_INT_INT_INT (GtkObject * object,
				       GtkSignalFunc func,
				       gpointer func_data,
				       GtkArg * args)
{
  GtkSignal_BOOL__POINTER_INT_INT_INT rfunc;
  gboolean *return_val,rc;
  return_val = GTK_RETLOC_BOOL (args[4]);
  rfunc = (GtkSignal_BOOL__POINTER_INT_INT_INT) func;
  ignore_=false;
  rc = (*rfunc) (object,
			  GTK_VALUE_POINTER (args[0]),
			  GTK_VALUE_INT (args[1]),
			  GTK_VALUE_INT (args[2]),
			  GTK_VALUE_INT (args[3]),
			  func_data);
  if (!ignore_) *return_val=rc;
  ignore_=false;
}

typedef gboolean (*GtkSignal_BOOL__POINTER_POINTER_POINTER_POINTER) (GtkObject * object,
							      gpointer arg1,
							      gpointer arg2,
							      gpointer arg3,
							      gpointer arg4,
							gpointer user_data);
void 
gtkmm_marshal_BOOL__POINTER_POINTER_POINTER_POINTER (GtkObject * object,
						   GtkSignalFunc func,
						   gpointer func_data,
						   GtkArg * args)
{
  GtkSignal_BOOL__POINTER_POINTER_POINTER_POINTER rfunc;
  gboolean *return_val,rc;
  return_val = GTK_RETLOC_BOOL (args[4]);
  rfunc = (GtkSignal_BOOL__POINTER_POINTER_POINTER_POINTER) func;
  ignore_=false;
  rc = (*rfunc) (object,
			  GTK_VALUE_POINTER (args[0]),
			  GTK_VALUE_POINTER (args[1]),
			  GTK_VALUE_POINTER (args[2]),
			  GTK_VALUE_POINTER (args[3]),
			  func_data);
  if (!ignore_) *return_val=rc;
  ignore_=false;
}

typedef gint (*GtkSignal_INT__INT) (GtkObject * object,
				    gint arg1,
				    gpointer user_data);
void 
gtkmm_marshal_INT__INT (GtkObject * object,
		      GtkSignalFunc func,
		      gpointer func_data,
		      GtkArg * args)
{
  GtkSignal_INT__INT rfunc;
  gint *return_val,rc;
  return_val = GTK_RETLOC_INT (args[1]);
  rfunc = (GtkSignal_INT__INT) func;
  ignore_=false;
  rc = (*rfunc) (object,
			  GTK_VALUE_INT (args[0]),
			  func_data);
  if (!ignore_) *return_val=rc;
  ignore_=false;
}

typedef gint (*GtkSignal_INT__POINTER) (GtkObject * object,
					gpointer arg1,
					gpointer user_data);
void 
gtkmm_marshal_INT__POINTER (GtkObject * object,
			  GtkSignalFunc func,
			  gpointer func_data,
			  GtkArg * args)
{
  GtkSignal_INT__POINTER rfunc;
  gint *return_val,rc;
  return_val = GTK_RETLOC_INT (args[1]);
  rfunc = (GtkSignal_INT__POINTER) func;
  ignore_=false;
  rc = (*rfunc) (object,
			  GTK_VALUE_POINTER (args[0]),
			  func_data);
  if (!ignore_) *return_val=rc;
  ignore_=false;
}

typedef gint (*GtkSignal_INT__POINTER_CHAR_CHAR) (GtkObject * object,
						  gpointer arg1,
						  char arg2,
						  char arg3,
						  gpointer user_data);
void 
gtkmm_marshal_INT__POINTER_CHAR_CHAR (GtkObject * object,
				    GtkSignalFunc func,
				    gpointer func_data,
				    GtkArg * args)
{
  GtkSignal_INT__POINTER_CHAR_CHAR rfunc;
  gint *return_val,rc;
  return_val = GTK_RETLOC_INT (args[3]);
  rfunc = (GtkSignal_INT__POINTER_CHAR_CHAR) func;
  ignore_=false;
  rc = (*rfunc) (object,
			  GTK_VALUE_POINTER (args[0]),
			  GTK_VALUE_CHAR (args[1]),
			  GTK_VALUE_CHAR (args[2]),
			  func_data);
  if (!ignore_) *return_val=rc;
  ignore_=false;
}


