#ifndef _GTKMM_MARSHAL_H_
#define _GTKMM_MARSHAL_H_

// Much of this code is borrowed from gtk+

#include <gtk/gtksignal.h>

guint gtkmm_signal_set_marshal(guint,GtkSignalMarshaller);
bool gtkmm_clear_ignore();
void gtkmm_set_ignore();

/* We need the following combinations 
    gtkmm_marshal_BOOL__NONE
    gtkmm_marshal_BOOL__POINTER
    gtkmm_marshal_BOOL__POINTER_POINTER_INT_INT
    gtkmm_marshal_BOOL__POINTER_INT_INT
    gtkmm_marshal_BOOL__POINTER_INT_INT_INT
    gtkmm_marshal_BOOL__POINTER_POINTER_POINTER_POINTER
    gtkmm_marshal_INT__INT
    gtkmm_marshal_INT__POINTER
    gtkmm_marshal_INT__POINTER_CHAR_CHAR
*/

    
void gtkmm_marshal_BOOL__NONE (GtkObject * object,
                               GtkSignalFunc func,
                               gpointer func_data,
                               GtkArg * args);

void gtkmm_marshal_BOOL__POINTER (GtkObject * object,
                                  GtkSignalFunc func,
                                  gpointer func_data,
                                  GtkArg * args);

void gtkmm_marshal_BOOL__POINTER_POINTER_INT_INT (GtkObject * object,
                                                  GtkSignalFunc func,
                                                  gpointer func_data,
                                                  GtkArg * args);

void gtkmm_marshal_BOOL__POINTER_INT_INT (GtkObject * object,
                                          GtkSignalFunc func,
                                          gpointer func_data,
                                          GtkArg * args);
void gtkmm_marshal_BOOL__POINTER_INT_INT_INT (GtkObject * object,
                                              GtkSignalFunc func,
                                              gpointer func_data,
                                              GtkArg * args);
void gtkmm_marshal_BOOL__POINTER_POINTER_POINTER_POINTER (GtkObject * object,
                                                          GtkSignalFunc func,
                                                          gpointer func_data,
                                                          GtkArg * args);

void gtkmm_marshal_INT__INT (GtkObject * object,
                             GtkSignalFunc func,
                             gpointer func_data,
                             GtkArg * args);

void gtkmm_marshal_INT__POINTER (GtkObject * object,
                                 GtkSignalFunc func,
                                 gpointer func_data,
                                 GtkArg * args);

void gtkmm_marshal_INT__POINTER_CHAR_CHAR (GtkObject * object,
                                           GtkSignalFunc func,
                                           gpointer func_data,
                                           GtkArg * args);

#endif // _GTKMM_MARSHAL_H_
