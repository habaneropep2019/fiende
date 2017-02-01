#ifndef _GNOME_HELP_TOC_H_
#define _GNOME_HELP_TOC_H_

#define TOC_MAN_TYPE   0
#define TOC_INFO_TYPE  1
#define TOC_GHELP_TYPE 2

extern struct _toc_config {
    char *path;
    int type;
} toc_config[];

GtkWidget *createToc(GtkSignalFunc selectCallback);

void setWatch(GtkWidget *widget);
void unsetWatch(GtkWidget *widget);
void showToc(GtkWidget *window);
void hideToc(GtkWidget *window);

#endif
