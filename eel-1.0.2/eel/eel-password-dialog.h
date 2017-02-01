/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-password-dialog.h - A use password prompting dialog widget.

   Copyright (C) 1999, 2000 Eazel, Inc.

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Authors: Ramiro Estrugo <ramiro@eazel.com>
*/

#ifndef EEL_PASSWORD_DIALOG_H
#define EEL_PASSWORD_DIALOG_H

#include <libgnomeui/gnome-dialog.h>

BEGIN_GNOME_DECLS

#define GTK_TYPE_AUTH_DIALOG            (eel_password_dialog_get_type ())
#define EEL_PASSWORD_DIALOG(obj)            (GTK_CHECK_CAST ((obj), GTK_TYPE_AUTH_DIALOG, EelPasswordDialog))
#define EEL_PASSWORD_DIALOG_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_AUTH_DIALOG, EelPasswordDialogClass))
#define EEL_IS_PASSWORD_DIALOG(obj)         (GTK_CHECK_TYPE ((obj), GTK_TYPE_AUTH_DIALOG))
#define EEL_IS_PASSWORD_DIALOG_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_AUTH_DIALOG))

typedef struct EelPasswordDialog        EelPasswordDialog;
typedef struct EelPasswordDialogClass   EelPasswordDialogClass;
typedef struct EelPasswordDialogDetails EelPasswordDialogDetails;

struct EelPasswordDialog
{
	GnomeDialog gnome_dialog;

	EelPasswordDialogDetails *details;
};

struct EelPasswordDialogClass
{
	GnomeDialogClass parent_class;
};

GtkType    eel_password_dialog_get_type                (void);
GtkWidget* eel_password_dialog_new                     (const char        *dialog_title,
							const char        *message,
							const char        *username,
							const char        *password,
							gboolean           readonly_username);
gboolean   eel_password_dialog_run_and_block           (EelPasswordDialog *password_dialog);

/* Attribute mutators */
void       eel_password_dialog_set_username            (EelPasswordDialog *password_dialog,
							const char        *username);
void       eel_password_dialog_set_password            (EelPasswordDialog *password_dialog,
							const char        *password);
void       eel_password_dialog_set_readonly_username   (EelPasswordDialog *password_dialog,
							gboolean           readonly);
void       eel_password_dialog_set_remember            (EelPasswordDialog *password_dialog,
							gboolean           remember);
void       eel_password_dialog_set_remember_label_text (EelPasswordDialog *password_dialog,
							const char        *remember_label_text);

/* Attribute accessors */
char *     eel_password_dialog_get_username            (EelPasswordDialog *password_dialog);
char *     eel_password_dialog_get_password            (EelPasswordDialog *password_dialog);
gboolean   eel_password_dialog_get_remember            (EelPasswordDialog *password_dialog);

END_GNOME_DECLS

#endif /* EEL_PASSWORD_DIALOG_H */
