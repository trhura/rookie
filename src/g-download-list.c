/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * g-download-list.c
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
#include "rookie-misc.h"
#include "gio-download.h"
#include "g-download-list.h"
#include "g-downloadable-private.h"
#include "g-downloadable-backends.h"

static void g_download_list_save_state (GDownloadList *);
static void g_download_list_load_state (GDownloadList *);

#define G_DOWNLOAD_LIST_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE		\
									 ((o),								\
									  G_TYPE_DOWNLOAD_LIST,				\
									  GDownloadListPrivate))

G_DEFINE_TYPE (GDownloadList, g_download_list, G_TYPE_OBJECT);

/* This is the format used as group in keyfile, uid; backend
 * backend is necessary so we know which backend is
 * responsible for deserialization */
#define GROUP_FORMAT "%u ; %s"

struct _GDownloadListPrivate
{
	GHashTable * table;
};

enum {
	DOWNLOAD_ADDED,
	DOWNLOAD_REMOVED,
	CHANGED, /* covenience signal for the above two */
	TOTAL_SIGNALS
};

static guint g_download_list_signals[TOTAL_SIGNALS];

static void
g_download_list_init (GDownloadList *object)
{
	object->priv = G_DOWNLOAD_LIST_PRIVATE (object);
	object->priv->table = g_hash_table_new (g_direct_hash,
											g_direct_equal);
	g_download_list_load_state (object);
}

static void
g_download_list_finalize (GObject *object)
{
	GDownloadList * download_list = G_DOWNLOAD_LIST (object);
	g_download_list_save_state (download_list);
	g_hash_table_destroy (download_list->priv->table);

	G_OBJECT_CLASS (g_download_list_parent_class)->finalize (object);
}

static void
g_download_list_class_init (GDownloadListClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (GDownloadListPrivate));

	g_download_list_signals[DOWNLOAD_ADDED] =
		g_signal_new ("download-added",
					  G_TYPE_DOWNLOAD_LIST,
					  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
					  0, //G_STRUCT_OFFSET (GDownloadableClass, download_progressed),
					  NULL, NULL,
					  g_cclosure_marshal_VOID__OBJECT,
					  G_TYPE_NONE, 1, G_TYPE_DOWNLOADABLE);

	g_download_list_signals[DOWNLOAD_REMOVED] =
		g_signal_new ("download-removed",
					  G_TYPE_DOWNLOAD_LIST,
					  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
					  0, //G_STRUCT_OFFSET (GDownloadableClass, download_progressed),
					  NULL, NULL,
					  g_cclosure_marshal_VOID__OBJECT,
					  G_TYPE_NONE, 1, G_TYPE_DOWNLOADABLE);

	g_download_list_signals[CHANGED] =
		g_signal_new ("changed",
					  G_TYPE_DOWNLOAD_LIST,
					  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
					  0, //G_STRUCT_OFFSET (GDownloadableClass, download_list_changed),
					  NULL, NULL,
					  g_cclosure_marshal_VOID__VOID,
					  G_TYPE_NONE, 0, NULL);

	object_class->finalize = g_download_list_finalize;
}

static void
g_download_list_load_state (GDownloadList * object)
{
	GKeyFile *keys = g_key_file_new ();
	gchar	 *path = rookie_misc_get_state_file_path ();
	GError	 *error = NULL;

	g_key_file_load_from_file (keys, path, G_KEY_FILE_NONE, &error);
	handle_error (error);

	guint uid;
	gsize i, length;
	gchar backend_name[128];
	gchar **groups = g_key_file_get_groups (keys, &length);

	for (i = 0; i < length; i++) {
		sscanf (groups[i], GROUP_FORMAT, &uid, backend_name);

		GDownloadableBackend * backend =
			g_downloadable_backends_by_name (backend_name);
		GDownloadable * download =
			g_downloadable_deserialize (keys, groups[i], backend);

		download->priv->uid = uid;
		download->priv->backend = backend;

		g_hash_table_insert (object->priv->table,
							 GUINT_TO_POINTER(download->priv->uid),
							 download);
		g_signal_emit_by_name (object, "download-added", download);
		g_signal_emit_by_name (object, "changed");
	}

	g_free (path);
	g_strfreev (groups);
	g_key_file_free (keys);
}

static void
g_download_list_save_state (GDownloadList * object)
{
	GKeyFile * keys = g_key_file_new ();
	GHashTableIter iter;
	gpointer key, value;

	g_hash_table_iter_init (&iter, object->priv->table);

	while (g_hash_table_iter_next (&iter, &key, &value)) {
		GDownloadable *download = G_DOWNLOADABLE (value);
		gchar * group = g_strdup_printf (GROUP_FORMAT, GPOINTER_TO_UINT(key),
										 download->priv->backend->name);
		g_downloadable_serialize (download, keys, group);
		//g_object_unref (download);
		g_free (group);
	}

	gsize length;
	gchar *data = g_key_file_to_data (keys, &length, NULL);

	gchar *path = rookie_misc_get_state_file_path ();
	GFile *file = g_file_new_for_path (path);
	GError *error = NULL;

	if (!g_file_replace_contents (file, data, length, NULL,FALSE,
								  G_FILE_CREATE_NONE, NULL, NULL, &error))
		handle_error (error);

	g_key_file_free (keys);
}

GDownloadList * g_download_list_get ()
{
	static GDownloadList * download_list = NULL;

	if (download_list == NULL)
		download_list = g_object_new (G_TYPE_DOWNLOAD_LIST, NULL);

	return download_list;
}

void g_download_list_add (GDownloadable * download)
{
	GDownloadList *object = g_download_list_get ();

	g_hash_table_insert (object->priv->table,
						 GUINT_TO_POINTER(download->priv->uid),
						 download);
	g_signal_emit_by_name (object, "download-added", download);
	g_signal_emit_by_name (object, "changed");
}

void g_download_list_foreach (GFunc func, gpointer data)
{
	GDownloadList *object = g_download_list_get ();
	GHashTableIter iter;
	gpointer key, value;

	g_hash_table_iter_init (&iter, object->priv->table);

	while (g_hash_table_iter_next (&iter, &key, &value)) {
		func (value, data);
	}
}

void g_download_list_remove (GDownloadable * download)
{
	GDownloadList *object = g_download_list_get ();

	g_hash_table_remove (object->priv->table,
						 GUINT_TO_POINTER(download->priv->uid));
	g_signal_emit_by_name (object, "download-removed", download);
	g_signal_emit_by_name (object, "changed");
	g_object_unref (download); /* Is it supposed to? */
}
