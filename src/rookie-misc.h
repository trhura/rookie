/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * rookie-misc.h
 * Copyright (C) Thura Hlaing 2010 <trhura@gmail.com>
 * 
 * rookie is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * rookie is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtk/gtk.h>

#ifndef _ROOKIE_MISC_H_
#define _ROOKIE_MISC_H_

typedef enum {
	G_DOWNLOADABLE_FINISH_ACTION_NOTIFY = 1 << 0,
	G_DOWNLOADABLE_FINISH_ACTION_OPEN   = 1 << 1,
	G_DOWNLOADABLE_FINISH_ACTION_OPEN_LOCATION = 1 << 2
} GDownloadableFinishAction;


gchar*			rookie_misc_get_state_file_path ();
gchar*			rookie_misc_get_category_file_path ();

GFile*			rookie_misc_get_log_file		(guint uid);
gchar*			rookie_misc_expand_path			(const gchar *path);
gchar*			rookie_misc_get_ui_path			(const gchar *name);

void			rookie_misc_quit ();
void			rookie_misc_add_download		(const gchar * url);
gboolean		rookie_misc_is_valid_url		(const gchar *url);

void			rookie_misc_show_error_dialog	(const gchar *title,
												 const gchar * message,
												 GtkWindow * modal);

void			rookie_misc_show_notification	(const gchar *summary,
												 const gchar *body,
												 const gchar *icon);

gboolean		rookie_misc_run_confirm_dialog	(const gchar *title,
												 const gchar *message,
												 const gchar *ok_button,
												 GtkWindow *modal);



#endif
