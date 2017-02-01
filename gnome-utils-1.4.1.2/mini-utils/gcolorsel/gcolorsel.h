#ifndef GCOLORSEL_H
#define GCOLORSEL_H

#include "gnome.h"

#include "mdi-color-generic.h"

extern GnomeMDI *mdi;
extern GtkWidget *event_widget;

typedef GtkWidget *(*views_new) (MDIColorGeneric *mcg);

typedef struct views_t {
  char *name;
  int scroll_h_policy;
  int scroll_v_policy;
  views_new new;
  GtkType (*type) (void);
  char *description;
} views_t;

typedef struct docs_t {
  char *name;
  GtkType (*type) (void);
  gboolean can_create;
  gboolean connect;
  char *description;
} docs_t;

extern views_t views_tab[];
extern docs_t  docs_tab[];

views_t *get_views_from_type (GtkType type);

typedef enum {
  ACTIONS_NOTHING = 0,
  ACTIONS_APPEND,
  ACTIONS_SEARCH,
  ACTIONS_EDIT
} actions_t;

typedef struct prefs_t {
  gboolean save_session;
  gboolean display_doc;

  int tab_pos;
  int mdi_mode;

  actions_t on_drop;
  actions_t on_grab;
  actions_t on_views;
  actions_t on_previews;

  actions_t on_drop2;
  actions_t on_grab2;
  actions_t on_views2;
  actions_t on_previews2;
} prefs_t;

extern prefs_t prefs;
void prefs_load (void);
void prefs_save (void);

void actions_drop (int r, int g, int b);
void actions_grab (int r, int g, int b);
void actions_previews (int r, int g, int b);

void actions_views (MDIColor *col);


#endif
