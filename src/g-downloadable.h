/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * rookie
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

#ifndef _G_DOWNLOADABLE_H_
#define _G_DOWNLOADABLE_H_

#include <gtk/gtk.h>
#include <glib-object.h>

#include "g-downloadable-backends.h"
#include "rookie-misc.h"
#include "categories.h"

G_BEGIN_DECLS

#define G_TYPE_DOWNLOADABLE				(g_downloadable_get_type ())
#define G_DOWNLOADABLE(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), G_TYPE_DOWNLOADABLE, GDownloadable))
#define G_DOWNLOADABLE_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), G_TYPE_DOWNLOADABLE, GDownloadableClass))
#define G_IS_DOWNLOADABLE(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), G_TYPE_DOWNLOADABLE))
#define G_IS_DOWNLOADABLE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), G_TYPE_DOWNLOADABLE))
#define G_DOWNLOADABLE_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), G_TYPE_DOWNLOADABLE, GDownloadableClass))

typedef struct _GDownloadablePrivate GDownloadablePrivate;
typedef struct _GDownloadableClass GDownloadableClass;
typedef struct _GDownloadable GDownloadable;

typedef enum  {
	G_DOWNLOADABLE_CONNECTING,
	G_DOWNLOADABLE_DOWNLOADING,
	G_DOWNLOADABLE_PAUSED,
	G_DOWNLOADABLE_COMPLETED,
	G_DOWNLOADABLE_NETWORK_ERROR,

	G_DOWNLOADABLE_TOTAL_STATUS
} GDownloadableStatus;

struct _GDownloadableClass
{
	GObjectClass parent_class;

	void (*pause) (GDownloadable *download);
	void (*start) (GDownloadable *download, gboolean resume);
	void (*serialize) (GDownloadable *download, GKeyFile *file, const gchar *group);
	void (*deserialize) (GDownloadable *download, GKeyFile *file, const gchar *group);

	void (*download_progressed) (GDownloadable *download, gssize downloaded);
	void (*download_completed) (GDownloadable *download);
	void (*status_changed) (GDownloadable *download);
};

struct _GDownloadable
{
	GObject parent_instance;
	GDownloadablePrivate * priv;
};

GType g_downloadable_get_type (void) G_GNUC_CONST;

gboolean g_downloadable_can_start (GDownloadable * download);
gboolean g_downloadable_can_pause (GDownloadable * download);
gboolean g_downloadable_is_completed (GDownloadable * download);
gboolean g_downloadable_is_downloading (GDownloadable * download);

guint g_downloadable_get_start_time (GDownloadable * download);
gint g_downloadable_get_speed (GDownloadable * download);
guint64 g_downloadable_get_size (GDownloadable * download);
guint64 g_downloadable_get_downloaded_size (GDownloadable * download);
const gchar * g_downloadable_get_url (GDownloadable * download);
const gchar * g_downloadable_get_icon_name (GDownloadable * download);
const gchar * g_downloadable_get_basename (GDownloadable * download);
const gchar * g_downloadable_get_comment (GDownloadable *download);
gchar * g_downloadable_get_save_path (GDownloadable * download);
GDownloadableStatus g_downloadable_get_status (GDownloadable * download);
GDownloadableFinishAction g_downloadable_get_finish_action (GDownloadable *download) ;
void g_downloadable_set_finish_action (GDownloadable *download, GDownloadableFinishAction action);
void g_downloadable_add_finish_action (GDownloadable *download, GDownloadableFinishAction action);
void g_downloadable_remove_finish_action (GDownloadable *download, GDownloadableFinishAction action);
void g_downloadable_set_view_iter (GDownloadable * download, GtkTreeIter * iter);
GtkTreeIter *  g_downloadable_get_view_iter (GDownloadable * download);

Category *g_downloadable_get_category (GDownloadable *download);
void	  g_downloadable_set_category (GDownloadable *download, Category *category);

void	g_downloadable_set_comment (GDownloadable *download, const gchar *comment);
void	g_downloadable_log (const gchar *message);

void	g_downloadable_start (GDownloadable * object, gboolean resume);
void	g_downloadable_pause (GDownloadable * object);
void	g_downloadable_open (GDownloadable * object);
void	g_downloadable_open_location (GDownloadable * object);
void	g_downloadable_delete (GDownloadable * object);

void	g_downloadable_serialize (GDownloadable * download, GKeyFile * keys, const gchar * group);
GDownloadable *g_downloadable_create_download (gchar * url, gchar * save_path, GDownloadableBackend * backend);
GDownloadable *g_downloadable_deserialize (GKeyFile * keys, const gchar * group, GDownloadableBackend * backend);

G_END_DECLS

#endif /* _G_DOWNLOADABLE_H_ */
