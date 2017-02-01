/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-ui-node.h: Code to manipulate BonoboUINode objects
 *
 * Author:
 *	Havoc Pennington <hp@redhat.com>
 *
 * Copyright 2000 Red Hat, Inc.
 */
#ifndef _BONOBO_UI_NODE_H_
#define _BONOBO_UI_NODE_H_

#include <glib.h>

typedef struct _BonoboUINode BonoboUINode;

BonoboUINode *bonobo_ui_node_new         (const char   *name);
BonoboUINode *bonobo_ui_node_new_child   (BonoboUINode *parent,
                                          const char   *name);
BonoboUINode *bonobo_ui_node_copy        (BonoboUINode *node,
					  gboolean      recursive);
void          bonobo_ui_node_free        (BonoboUINode *node);
void          bonobo_ui_node_set_data    (BonoboUINode *node,
                                          gpointer      data);
gpointer      bonobo_ui_node_get_data    (BonoboUINode *node);
void          bonobo_ui_node_set_attr    (BonoboUINode *node,
                                          const char   *name,
                                          const char   *value);
char *        bonobo_ui_node_get_attr    (BonoboUINode *node,
                                          const char   *name);
gboolean      bonobo_ui_node_has_attr    (BonoboUINode *node,
                                          const char   *name);
void          bonobo_ui_node_remove_attr (BonoboUINode *node,
                                          const char   *name);
void          bonobo_ui_node_add_child   (BonoboUINode *parent,
                                          BonoboUINode *child);
void          bonobo_ui_node_insert_before (BonoboUINode *sibling,
                                            BonoboUINode *prev_sibling);
void          bonobo_ui_node_unlink      (BonoboUINode *node);
void          bonobo_ui_node_replace     (BonoboUINode *old_node,
					  BonoboUINode *new_node);
void          bonobo_ui_node_set_content (BonoboUINode *node,
                                          const char   *content);
char         *bonobo_ui_node_get_content (BonoboUINode *node);
BonoboUINode *bonobo_ui_node_next        (BonoboUINode *node);
BonoboUINode *bonobo_ui_node_prev        (BonoboUINode *node);
BonoboUINode *bonobo_ui_node_children    (BonoboUINode *node);
BonoboUINode *bonobo_ui_node_parent      (BonoboUINode *node);
const char   *bonobo_ui_node_get_name    (BonoboUINode *node);
gboolean      bonobo_ui_node_has_name    (BonoboUINode *node,
					  const char   *name);
gboolean      bonobo_ui_node_transparent (BonoboUINode *node);
void          bonobo_ui_node_copy_attrs  (BonoboUINode *src,
					  BonoboUINode *dest);

/* This blows. libxml2 fixes it I guess. */
void          bonobo_ui_node_free_string (char *str);
void          bonobo_ui_node_strip       (BonoboUINode **node);

char *        bonobo_ui_node_to_string   (BonoboUINode *node,
					  gboolean      recurse);
BonoboUINode* bonobo_ui_node_from_string (const char *str);
BonoboUINode* bonobo_ui_node_from_file   (const char *filename);

#endif /* _BONOBO_UI_NODE_H_ */
