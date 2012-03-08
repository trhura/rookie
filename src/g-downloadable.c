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

#include "rookie-debug.h"
#include "g-downloadable.h"
#include "rookie-misc.h"
#include "g-downloadable-private.h"

#include "gio-download.h"  //FIXME: remove

enum {
	PROP_0,
	PROP_GDOWNLOADABLE_STATUS,
	PROP_GDOWNLOADABLE_SIZE,
};

enum {
	GDOWNLOADABLE_PROGRESSED,
	GDOWNLOADABLE_COMPLETED,
	GDOWNLOADABLE_STATUS_CHANGED,
	TOTAL_SIGNALS
};

static guint g_downloadable_signals [TOTAL_SIGNALS];

#define G_TYPE_DOWNLOADABLE_STATUS (g_downloadable_status_get_type())

G_DEFINE_TYPE (GDownloadable, g_downloadable, G_TYPE_OBJECT);

static GType g_downloadable_status_get_type ();

static void g_downloadable_set_property (GObject *, guint ,
										 const GValue *, GParamSpec *);
static void g_downloadable_get_property (GObject *, guint ,
										 GValue *, GParamSpec *);

static void on_download_completed (GDownloadable * download);
static void on_download_status_changed (GDownloadable * download);

static void on_download_progressed (GDownloadable *download,
									gssize downloaded);

static void
g_downloadable_init (GDownloadable *object)
{
	/* TODO: Add initialization code here */
	object->priv = G_DOWNLOADABLE_PRIVATE (object);
	object->priv->timer = g_timer_new ();
}

static void
g_downloadable_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */
	GDownloadable * download = G_DOWNLOADABLE (object);

	g_object_unref (download->priv->local_file);
	g_object_unref (download->priv->remote_file);
	g_object_unref (download->priv->log_file);
	g_free (download->priv->url);
	g_free (download->priv->icon_name);
	g_free (download->priv->local_path);
	g_free (download->priv->basename);
	g_free (download->priv->comment);
	g_timer_destroy (download->priv->timer);

	if (download->priv->iter)
		gtk_tree_iter_free (download->priv->iter);

	G_OBJECT_CLASS (g_downloadable_parent_class)->finalize (object);
}

static void
g_downloadable_class_init (GDownloadableClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (GDownloadablePrivate));

	object_class->set_property = g_downloadable_set_property;
	object_class->get_property = g_downloadable_get_property;

	klass->download_completed  = on_download_completed;
	klass->download_progressed = on_download_progressed;
	klass->status_changed	   = on_download_status_changed;

	g_object_class_install_property (object_class,	PROP_GDOWNLOADABLE_STATUS,
									 g_param_spec_enum ("status", "Status",
														"Status of download",
														G_TYPE_DOWNLOADABLE_STATUS,
														G_DOWNLOADABLE_PAUSED,
														G_PARAM_READWRITE));

	g_object_class_install_property (object_class,	PROP_GDOWNLOADABLE_SIZE,
									 g_param_spec_uint64 ("size", "Size",
														  "Size of download",
														  0, G_MAXUINT64, 0,
														  G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	g_downloadable_signals[GDOWNLOADABLE_PROGRESSED] =
		g_signal_new ("download-progressed",
					  G_TYPE_DOWNLOADABLE,
					  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
					  G_STRUCT_OFFSET (GDownloadableClass, download_progressed),
					  NULL, NULL,
					  g_cclosure_marshal_VOID__LONG,
					  G_TYPE_NONE, 1, G_TYPE_LONG);

	g_downloadable_signals[GDOWNLOADABLE_COMPLETED] =
		g_signal_new ("download-completed",
					  G_TYPE_DOWNLOADABLE,
					  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
					  G_STRUCT_OFFSET (GDownloadableClass, download_completed),
					  NULL, NULL,
					  g_cclosure_marshal_VOID__VOID,
					  G_TYPE_NONE, 0, NULL);


	g_downloadable_signals[	GDOWNLOADABLE_STATUS_CHANGED] =
		g_signal_new ("status-changed",
					  G_TYPE_DOWNLOADABLE,
					  G_SIGNAL_RUN_CLEANUP | G_SIGNAL_NO_RECURSE,
					  G_STRUCT_OFFSET (GDownloadableClass, status_changed),
					  NULL, NULL,
					  g_cclosure_marshal_VOID__VOID,
					  G_TYPE_NONE, 0, NULL);

	object_class->finalize = g_downloadable_finalize;
}


static GType g_downloadable_status_get_type ()
{
	static GType status_type = 0;

	static GEnumValue status[] = {
		{  G_DOWNLOADABLE_CONNECTING,		"connecting",	"Connecting"	},
		{  G_DOWNLOADABLE_DOWNLOADING,      "downloading",	"Downloading"	},
		{  G_DOWNLOADABLE_PAUSED,			"paused",       "Paused"		},
		{  G_DOWNLOADABLE_COMPLETED,		"completed",	"Completed"		},
		{  G_DOWNLOADABLE_NETWORK_ERROR,	"Inactive",		"Network Error"	},
		{ -1, NULL, NULL}
	};

	if (!status_type)
		status_type = g_enum_register_static("GstSbcMode", status);

	return status_type;
}

static void
g_downloadable_set_property (GObject      *object,
							 guint         property_id,
							 const GValue *value,
							 GParamSpec   *pspec)
{
	GDownloadable * self = G_DOWNLOADABLE (object);

	switch (property_id)
		{
		case PROP_GDOWNLOADABLE_STATUS:
			self->priv->status = g_value_get_enum (value);
			break;

		case PROP_GDOWNLOADABLE_SIZE:
			self->priv->size = g_value_get_uint64 (value);
			break;

		default:
			/* We don't have any other property... */
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
			break;
		}
}

static void
g_downloadable_get_property (GObject    *object,
							 guint       property_id,
							 GValue     *value,
							 GParamSpec *pspec)
{
	GDownloadable * self = G_DOWNLOADABLE (object);

	switch (property_id)
		{
		case PROP_GDOWNLOADABLE_STATUS:
			g_value_set_enum (value, self->priv->status);
			break;

		case PROP_GDOWNLOADABLE_SIZE:
			g_value_set_uint64 (value, self->priv->size);
			break;

		default:
			/* We don't have any other property... */
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
			break;
		}
}

gboolean g_downloadable_can_start (GDownloadable * download)
{
	g_return_val_if_fail (G_IS_DOWNLOADABLE(download), FALSE);

	GDownloadableStatus status = download->priv->status;
	return status == G_DOWNLOADABLE_PAUSED || status == G_DOWNLOADABLE_NETWORK_ERROR;
}

gboolean g_downloadable_can_pause (GDownloadable * download)
{
	g_return_val_if_fail (G_IS_DOWNLOADABLE(download), FALSE);

	GDownloadableStatus status = download->priv->status;
	return status == G_DOWNLOADABLE_CONNECTING || status == G_DOWNLOADABLE_DOWNLOADING;
}

gboolean g_downloadable_is_completed (GDownloadable * download)
{
	g_return_val_if_fail (G_IS_DOWNLOADABLE(download), FALSE);

	return download->priv->status == G_DOWNLOADABLE_COMPLETED;
}

gboolean g_downloadable_is_downloading (GDownloadable * download)
{
	g_return_val_if_fail (G_IS_DOWNLOADABLE(download), FALSE);

	GDownloadableStatus status = download->priv->status;
	return status == G_DOWNLOADABLE_DOWNLOADING || status == G_DOWNLOADABLE_CONNECTING;
}

GDownloadable * g_downloadable_deserialize (GKeyFile * keys,
											const gchar * group,
											GDownloadableBackend * backend)
{
	g_assert (keys != NULL);
	g_assert (backend != NULL);

	GDownloadable *download = g_object_new (backend->type,
											"status", G_DOWNLOADABLE_PAUSED,
											NULL);

	download->priv->url = g_key_file_get_string (keys, group, "url", NULL);
	download->priv->size = g_key_file_get_integer (keys, group, "size", NULL);
	download->priv->comment = g_key_file_get_string (keys, group, "comment", NULL);
	download->priv->local_path = g_key_file_get_string (keys, group, "local-path", NULL);
	download->priv->downloaded_size = g_key_file_get_integer (keys, group, "downloaded-size", NULL);
	download->priv->finish_action   = g_key_file_get_integer (keys, group, "finish-action", NULL);

	gchar *category = g_key_file_get_string (keys, group, "category", NULL);
	download->priv->category = categories_get_category_by_name (category);
	g_free (category);

	download->priv->remote_file = g_file_new_for_uri  (download->priv->url);
	download->priv->local_file	= g_file_new_for_path (download->priv->local_path);
	download->priv->log_file	= g_file_new_for_path (rookie_misc_get_log_file(download->priv->uid));
	download->priv->basename	= g_file_get_basename (download->priv->local_file);

	GDownloadableStatus status = g_key_file_get_integer (keys, group, "status", NULL);
	download->priv->status = (status == G_DOWNLOADABLE_COMPLETED) ? status : G_DOWNLOADABLE_PAUSED; // TODO:

	gchar * content_type = g_content_type_guess (g_file_get_basename (download->priv->remote_file), NULL, 0, NULL);
	GIcon * icon = g_content_type_get_icon (content_type);
	download->priv->icon_name = g_icon_to_string (icon);

	g_free (content_type);
	g_object_unref (icon);

	G_DOWNLOADABLE_GET_CLASS (download)->deserialize (download, keys, group);

	return download;
}

static void on_download_progressed (GDownloadable *download, gssize downloaded)
{
	g_timer_stop (download->priv->timer);

	gdouble elapsed = g_timer_elapsed (download->priv->timer, NULL);
	g_timer_start (download->priv->timer);

	download->priv->speed = (gint) ((gdouble) downloaded / elapsed );
	download->priv->downloaded_size += downloaded;
}

static void on_download_status_changed (GDownloadable * download)
{
	g_timer_stop (download->priv->timer);
}

static void on_download_completed (GDownloadable * download)
{
	if (download->priv->finish_action & G_DOWNLOADABLE_FINISH_ACTION_NOTIFY) {
		gchar *body = g_strdup_printf ("Successfully downloaded %s.",
									   g_downloadable_get_basename (download));
		rookie_misc_show_notification ("Download Completed",
									   body,
									   GTK_STOCK_DIALOG_INFO);
		g_free (body);

	}

	if (download->priv->finish_action & G_DOWNLOADABLE_FINISH_ACTION_OPEN) {
		g_downloadable_open (download);
	}

	if (download->priv->finish_action & G_DOWNLOADABLE_FINISH_ACTION_OPEN_LOCATION) {
		g_downloadable_open_location (download);
	}
}

void g_downloadable_start (GDownloadable * object,
						   gboolean resume)
{
	g_return_if_fail (G_IS_DOWNLOADABLE (object));

	if (!object->priv->local_file)
		object->priv->local_file = g_file_new_for_path (object->priv->local_path);

	if (!object->priv->remote_file)
		object->priv->remote_file = g_file_new_for_uri (object->priv->url);

	if (!object->priv->log_file)
		object->priv->log_file = g_file_new_for_path (rookie_misc_get_log_file(download->priv->uid));

	(G_DOWNLOADABLE_GET_CLASS (object))->start (object, resume);
}

void g_downloadable_pause (GDownloadable * object)
{
	g_return_if_fail (G_IS_DOWNLOADABLE (object));
	(G_DOWNLOADABLE_GET_CLASS (object))->pause (object);
}

void g_downloadable_serialize (GDownloadable * download,
							   GKeyFile * keys,
							   const gchar * group)
{
	g_return_if_fail (G_IS_DOWNLOADABLE (download));

	(G_DOWNLOADABLE_GET_CLASS (download))->serialize (download, keys, group);

	g_key_file_set_string (keys, group,  "url", download->priv->url);
	g_key_file_set_integer (keys, group, "size", download->priv->size);
	g_key_file_set_integer (keys, group, "status", download->priv->status);
	g_key_file_set_string (keys, group,  "local-path", download->priv->local_path);
	g_key_file_set_string (keys, group,  "comment", g_downloadable_get_comment (download));
	g_key_file_set_integer (keys, group, "downloaded-size", download->priv->downloaded_size);
	g_key_file_set_integer (keys, group, "finish-action", download->priv->finish_action);

	if (download->priv->category != NULL)
		g_key_file_set_string (keys, group, "category", category_get_name (download->priv->category));
}

const gchar *
g_downloadable_get_url (GDownloadable * download)
{
	return download->priv->url;
}

gchar *
g_downloadable_get_save_path (GDownloadable * download)
{
	GFile *file = g_file_get_parent (download->priv->local_file);
	gchar *path = g_file_get_path (file);

	g_object_unref (file);
	return path;
}

const gchar *
g_downloadable_get_icon_name (GDownloadable * download)
{
	return download->priv->icon_name;
}

guint g_downloadable_get_start_time (GDownloadable * download)
{
	return download->priv->uid;
}

const gchar *
g_downloadable_get_basename (GDownloadable * download)
{
	return download->priv->basename;
}

const gchar *
g_downloadable_get_comment (GDownloadable * download)
{
	if (download->priv->comment == NULL)
		/* TODO: some clever quote */
		return "Marlin's Beard";

	return download->priv->comment;
}

void
g_downloadable_set_comment (GDownloadable * download, const gchar *comment)
{
	g_free (download->priv->comment);
	download->priv->comment = g_strdup (comment);
}

guint64
g_downloadable_get_size (GDownloadable * download)
{
	return download->priv->size;
}


guint64
g_downloadable_get_downloaded_size (GDownloadable * download)
{
	return download->priv->downloaded_size;
}


gint
g_downloadable_get_speed (GDownloadable * download)
{
	return download->priv->speed;
}


GDownloadableStatus
g_downloadable_get_status (GDownloadable * download)
{
	return download->priv->status;
}

GtkTreeIter *
g_downloadable_get_view_iter (GDownloadable * download)
{
	return download->priv->iter;
}

void
g_downloadable_set_view_iter (GDownloadable * download, GtkTreeIter * iter)
{
	if (download->priv->iter)
		gtk_tree_iter_free (download->priv->iter);

	download->priv->iter = iter;
}

GDownloadable*  g_downloadable_create_download (gchar * url, gchar * save_path, GDownloadableBackend * backend)
{
	g_return_val_if_fail (backend != NULL, NULL);

	GDownloadable * download = g_object_new (backend->type, NULL);
	time_t now = time (NULL);

	download->priv->url		= url;
	download->priv->backend = backend;
	download->priv->uid		= (guint) now;
	download->priv->local_path = save_path;
	download->priv->status	= G_DOWNLOADABLE_PAUSED;
	//	download->priv->log_file   = rookie_misc_get_log_file (download->priv->uid);
	download->priv->local_file = g_file_new_for_path (download->priv->local_path);
	download->priv->basename   = g_file_get_basename (download->priv->local_file);
	download->priv->remote_file = g_file_new_for_uri (download->priv->url);

	gchar *content_type = g_content_type_guess (g_file_get_basename (download->priv->remote_file), NULL, 0, NULL);
	GIcon *icon = g_content_type_get_icon (content_type);
	download->priv->icon_name = g_icon_to_string (icon);
	g_free (content_type);
	g_object_unref (icon);

	return download;
}

void g_downloadable_log (const gchar *message)
{
	/* gchar time_str[128]; */
	/* time_t time = time (NULL); */
	/* struct tm *started_time = localtime (&time); */
	/* strftime (time_str, 128, "%a %d %b %R %p", started_time); */

	/* static GFileIOStream *log_stream; */

	/* if (l */

}

void g_downloadable_set_category (GDownloadable *download, Category *category)
{
	download->priv->category = category;
}

void g_downloadable_set_finish_action (GDownloadable *download,
									   GDownloadableFinishAction action)
{
	download->priv->finish_action = action;
}

void g_downloadable_add_finish_action (GDownloadable *download,
									   GDownloadableFinishAction action)
{
	download->priv->finish_action |= action;
}

void g_downloadable_remove_finish_action (GDownloadable *download,
										  GDownloadableFinishAction action)
{
	download->priv->finish_action &= ~action;
}

GDownloadableFinishAction g_downloadable_get_finish_action (GDownloadable *download)
{
	return download->priv->finish_action;
}

Category* g_downloadable_get_category (GDownloadable *download)
{
	if (download->priv->category != NULL)
		return download->priv->category;
	else
		return categories_get_category_by_url (download->priv->url);
}

void g_downloadable_delete (GDownloadable * download)
{
	g_file_delete (download->priv->local_file, NULL, NULL);
}

void g_downloadable_open (GDownloadable * download)
{
	gchar * uri = g_file_get_uri (download->priv->local_file);
	gtk_show_uri (NULL, uri, GDK_CURRENT_TIME, NULL);
	g_free (uri);
}

void g_downloadable_open_location (GDownloadable * download)
{
	GFile * parent = g_file_get_parent (download->priv->local_file);
	gchar * uri = g_file_get_uri (parent);
	gtk_show_uri (NULL, uri, GDK_CURRENT_TIME, NULL);
	g_object_unref (parent);
	g_free (uri);
}

void g_downloadable_set_status (GDownloadable *download, GDownloadableStatus status)
{
	download->priv->status = status;
	download->priv->speed  = 0;
	g_signal_emit_by_name (download, "status-changed");
}

