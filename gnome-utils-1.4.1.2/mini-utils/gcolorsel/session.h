#ifndef SESSION_H
#define SESSION_H

#include "gnome.h"

void     session_save      (GnomeMDI *mdi);
gboolean session_load      (GnomeMDI *mdi);
void     session_load_data (GnomeMDI *mdi);
void     session_create    (GnomeMDI *mdi, gboolean init_actions);

#endif
