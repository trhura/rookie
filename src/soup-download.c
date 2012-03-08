/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * soup-download.c
 * Copyright (C) Thura Hlaing 2010 <trhura@gmail.com>
 * 
 * soup-download.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * soup-download.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <libsoup/soup.h>
#include "rookie-debug.h"
#include "soup-download.h"
#include "g-downloadable-private.h"

struct _SoupDownloadPrivate
{
	SoupSession *session;
	SoupMessage *message;
	SoupURI		*proxy;

	gboolean	paused;
	gboolean	need_to_write;
	gint64		prev_downloaded;
};

#define SOUP_DOWNLOAD_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), SOUP_TYPE_DOWNLOAD, SoupDownloadPrivate))

G_DEFINE_TYPE (SoupDownload, soup_download, G_TYPE_DOWNLOADABLE);

static void soup_download_pause (GDownloadable  *download);
static void soup_download_start (GDownloadable  *download, gboolean resume);
static void soup_download_serialize (GDownloadable *download, GKeyFile *keys, const gchar *group);
static void	soup_download_deserialize (GDownloadable * download, GKeyFile * keys, const gchar * group);

static void on_download_progressed (SoupMessage *message, SoupBuffer *buffer, gpointer user_data);
static void on_got_headers (SoupMessage *message, gpointer user_data);
static void on_download_finished (SoupMessage *message, gpointer user_data); 
static void write_output_file (gpointer data);
	
static void
soup_download_init (SoupDownload *object)
{
	object->priv = SOUP_DOWNLOAD_PRIVATE (object);
	object->priv->session = soup_session_async_new ();
	object->priv->need_to_write  = FALSE; 
}

static void
soup_download_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */
	SoupDownload *download = SOUP_DOWNLOAD (object);

	if (download->priv->need_to_write) {
		rookie_debug ("writing file ...");
		write_output_file (download);
	}
	
	g_object_unref (SOUP_DOWNLOAD (object)->priv->session);
	G_OBJECT_CLASS (soup_download_parent_class)->finalize (object);
}

static void
soup_download_class_init (SoupDownloadClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	GDownloadableClass* parent_class = G_DOWNLOADABLE_CLASS (klass);

	g_type_class_add_private (klass, sizeof (SoupDownloadPrivate));

	parent_class->start = soup_download_start;
	parent_class->pause = soup_download_pause;
	parent_class->serialize = soup_download_serialize;
	parent_class->deserialize = soup_download_deserialize;
	object_class->finalize = soup_download_finalize; 
} 

static void soup_message_connect_signals (SoupMessage *message, SoupDownload *download)
{
	g_signal_connect (message, "got-chunk", G_CALLBACK(on_download_progressed), download); 
	g_signal_connect (message, "got-body",  G_CALLBACK(on_download_finished), download);
	g_signal_connect (message, "got-headers", G_CALLBACK(on_got_headers), download);
}

static void soup_download_deserialize (GDownloadable * download, GKeyFile * keys, const gchar * group)
{
	SoupDownload *soup_download = SOUP_DOWNLOAD (download);

	if (!g_downloadable_is_completed (download)) {

		soup_download->priv->message  = soup_message_new ("GET", download->priv->url);
		soup_message_connect_signals   (soup_download->priv->message, soup_download);
		soup_message_headers_set_range (soup_download->priv->message->request_headers,
										download->priv->downloaded_size, -1);
		soup_download->priv->prev_downloaded = download->priv->downloaded_size;
	}
}

static void soup_download_serialize (GDownloadable *download, GKeyFile *keys, const gchar *group)
{
		/* Nothing needs to be done */ 
}

static void soup_download_pause (GDownloadable  *download)
{
	SoupDownload *soup_download = SOUP_DOWNLOAD (download);
	soup_download->priv->paused = TRUE;

	soup_session_pause_message (soup_download->priv->session, soup_download->priv->message);
	g_downloadable_set_status (download, G_DOWNLOADABLE_PAUSED);
}

static void soup_download_start (GDownloadable  *download, gboolean resume)
{
	SoupDownload *soup_download = SOUP_DOWNLOAD (download);

	if (soup_download->priv->paused) {
		soup_session_unpause_message (soup_download->priv->session, soup_download->priv->message);
	} else {
		if (soup_download->priv->message == NULL) {
			/* New Download */
			soup_download->priv->message  = soup_message_new ("GET", download->priv->url);
			soup_message_connect_signals  (soup_download->priv->message, soup_download);
		}
		soup_session_queue_message	(soup_download->priv->session, soup_download->priv->message, NULL, NULL); 
	}
	g_downloadable_set_status (G_DOWNLOADABLE(download), G_DOWNLOADABLE_DOWNLOADING);
}

/* static void foreach_header (const gchar *name,const gchar *value,gpointer user_data) */
/* { */
/* 	g_printf ("%s = %s\n", name, value); */
/* } */

static void on_got_headers (SoupMessage *message, gpointer user_data)
{
	GDownloadable *download = G_DOWNLOADABLE (user_data);
	g_assert (download != NULL);

	if (message->status_code != SOUP_STATUS_OK) {
		g_printf ("status code = %d\n", message->status_code); 
	}

	//	soup_message_headers_foreach (message->response_headers, foreach_header, NULL);
	g_downloadable_set_status (download, G_DOWNLOADABLE_DOWNLOADING);
	
	if (download->priv->size == 0) 
		download->priv->size = soup_message_headers_get_content_length (message->response_headers);

}

static void on_download_progressed (SoupMessage *message, SoupBuffer *buffer, gpointer user_data)
{
	SoupDownload *download = SOUP_DOWNLOAD (user_data);
	g_assert (download != NULL);

	g_signal_emit_by_name (download, "download-progressed", buffer->length);
	download->priv->need_to_write = TRUE;
}

static void write_output_file (gpointer data)
{
	g_assert (data != NULL);

	GError *error = NULL;
	GDownloadable *download		 = G_DOWNLOADABLE (data);
	SoupDownload  *soup_download = SOUP_DOWNLOAD (data);

	if (soup_download->priv->need_to_write) {
		GFileOutputStream *ostream = g_file_append_to (download->priv->local_file, G_FILE_CREATE_NONE, NULL, &error);
		handle_error (error);

		SoupBuffer *buffer = soup_message_body_flatten (soup_download->priv->message->response_body);
		g_output_stream_write (G_OUTPUT_STREAM (ostream), buffer->data, buffer->length, NULL, &error);
		handle_error (error); 

		g_output_stream_close (G_OUTPUT_STREAM (ostream), NULL, &error);
		handle_error (error);

		download->priv->downloaded_size = soup_download->priv->prev_downloaded + buffer->length;
		soup_buffer_free (buffer);

		soup_download->priv->need_to_write = FALSE;
	}
} 

static void on_download_finished (SoupMessage *message, gpointer user_data)
{
	SoupDownload *download = SOUP_DOWNLOAD (user_data);
	g_assert (download != NULL);

	write_output_file (download);
	g_downloadable_set_status (G_DOWNLOADABLE(download), G_DOWNLOADABLE_COMPLETED);
} 
