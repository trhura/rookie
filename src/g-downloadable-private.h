/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * g-downloable-private.h
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

#include "g-downloadable.h"
#include <gio/gio.h>
#include <gtk/gtk.h>

#define G_DOWNLOADABLE_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), G_TYPE_DOWNLOADABLE, GDownloadablePrivate))

struct _GDownloadablePrivate
{
	gint speed;
	guint64 size;
	guint64 downloaded_size; 
	GDownloadableStatus status;

	guint uid;
	gchar *url;
	gchar *basename;
	gchar *icon_name; 
	gchar *local_path;
	gchar *comment;
	GtkTreeIter *iter;
	gint finish_action;
	GTimer *timer;

	GDownloadableBackend *backend;
	Category *category;

	GFile *local_file;
	GFile *remote_file;
	GFile *log_file;
};

void g_downloadable_set_status (GDownloadable *download, GDownloadableStatus status); 
