/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-ui-xml.h: A module for merging, overlaying and de-merging XML 
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2000 Helix Code, Inc.
 */
#ifndef _BONOBO_UI_XML_H_
#define _BONOBO_UI_XML_H_

#include <gtk/gtkobject.h>
#include <bonobo/bonobo-ui-node.h>
#include <bonobo/bonobo-ui-engine.h>

#define BONOBO_UI_XML_TYPE        (bonobo_ui_xml_get_type ())
#define BONOBO_UI_XML(o)          (GTK_CHECK_CAST ((o), BONOBO_UI_XML_TYPE, BonoboUIXml))
#define BONOBO_UI_XML_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_UI_XML_TYPE, BonoboUIXmlClass))
#define BONOBO_IS_UI_XML(o)       (GTK_CHECK_TYPE ((o), BONOBO_UI_XML_TYPE))
#define BONOBO_IS_UI_XML_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_UI_XML_TYPE))

typedef struct _BonoboUIXml BonoboUIXml;

typedef struct {
	gpointer id;
	gboolean dirty;
	GSList  *overridden;
} BonoboUIXmlData;

typedef gboolean         (*BonoboUIXmlCompareFn)   (gpointer         id_a,
						    gpointer         id_b);
typedef BonoboUIXmlData *(*BonoboUIXmlDataNewFn)   (void);
typedef void             (*BonoboUIXmlDataFreeFn)  (BonoboUIXmlData *data);
typedef void             (*BonoboUIXmlDumpFn)      (BonoboUIXml      *tree,
						    BonoboUINode     *node);
typedef void             (*BonoboUIXmlAddNode)     (BonoboUINode     *parent,
						    BonoboUINode     *child,
						    gpointer          user_data);
typedef void             (*BonoboUIXmlWatchFn)     (BonoboUIXml      *xml,
						    const char       *path,
						    BonoboUINode     *opt_node,
						    gpointer          user_data);

struct _BonoboUIXml {
	GtkObject              object;

	BonoboUIXmlCompareFn   compare;
	BonoboUIXmlDataNewFn   data_new;
	BonoboUIXmlDataFreeFn  data_free;
	BonoboUIXmlDumpFn      dump;
	BonoboUIXmlAddNode     add_node;
	BonoboUIXmlWatchFn     watch;
	gpointer               user_data;

	BonoboUINode          *root;

	GSList                *watches;
};

typedef struct {
	GtkObjectClass         object_klass;

	void                 (*override)          (BonoboUINode *new_node,
						   BonoboUINode *old_node);
	void                 (*replace_override)  (BonoboUINode *new_node,
						   BonoboUINode *old_node);
	void                 (*reinstate)         (BonoboUINode *node);
	void                 (*rename)            (BonoboUINode *node);
	void                 (*remove)            (BonoboUINode *node);

	gpointer               dummy;
} BonoboUIXmlClass;

GtkType          bonobo_ui_xml_get_type          (void);

BonoboUIXml     *bonobo_ui_xml_new               (BonoboUIXmlCompareFn  compare,
						  BonoboUIXmlDataNewFn  data_new,
						  BonoboUIXmlDataFreeFn data_free,
						  BonoboUIXmlDumpFn     dump,
						  BonoboUIXmlAddNode    add_node,
						  gpointer              user_data);

/* Nominaly BonoboUIXmlData * */
gpointer         bonobo_ui_xml_get_data          (BonoboUIXml  *tree,
						  BonoboUINode *node);

void             bonobo_ui_xml_set_dirty         (BonoboUIXml  *tree,
						  BonoboUINode *node);

void             bonobo_ui_xml_clean             (BonoboUIXml  *tree,
						  BonoboUINode *node);

BonoboUINode    *bonobo_ui_xml_get_path          (BonoboUIXml  *tree,
						  const char   *path);
BonoboUINode    *bonobo_ui_xml_get_path_wildcard (BonoboUIXml  *tree,
						  const char   *path,
						  gboolean     *wildcard);

char            *bonobo_ui_xml_make_path         (BonoboUINode *node);
char            *bonobo_ui_xml_get_parent_path   (const char   *path);

BonoboUIError    bonobo_ui_xml_merge             (BonoboUIXml  *tree,
						  const char   *path,
						  BonoboUINode *nodes,
						  gpointer      id);

BonoboUIError    bonobo_ui_xml_rm                (BonoboUIXml  *tree,
						  const char   *path,
						  gpointer      id);

void             bonobo_ui_xml_dump              (BonoboUIXml  *tree,
						  BonoboUINode *node,
						  const char   *msg);

void             bonobo_ui_xml_set_watch_fn      (BonoboUIXml  *tree,
						  BonoboUIXmlWatchFn watch);

void             bonobo_ui_xml_add_watch         (BonoboUIXml  *tree,
						  const char   *path,
						  gpointer      user_data);

void             bonobo_ui_xml_remove_watch_by_data (BonoboUIXml  *tree,
						     gpointer      user_data);

#endif /* _BONOBO_UI_XML_H_ */
