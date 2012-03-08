/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * gio-download.c
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

#include <string.h>
#include "rookie-debug.h"
#include "gio-download.h" 
#include "g-downloadable-private.h"

G_DEFINE_TYPE (GioDownload, gio_download, G_TYPE_DOWNLOADABLE);

static void gio_download_pause (GDownloadable  *download);
static void gio_download_start (GDownloadable  *download, gboolean resume);
static void gio_download_serialize (GDownloadable *download, GKeyFile *keys, const gchar *group);
static void	gio_download_deserialize (GDownloadable * download, GKeyFile * keys, const gchar * group);
static void read_input_stream (GDownloadable *download); 
static void write_chunks_to_output_stream (GioDownload *download);

#define GIO_DOWNLOAD_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GIO_TYPE_DOWNLOAD, GioDownloadPrivate))

#define CHUNK_BUFF_SIZE 2048
#define NUM_OF_CHUNKS	50

#define MEM_CHUNK(chunk) ((MemChunk*) chunk)

typedef struct {
	void  *data;
	gsize size;
} MemChunk;

struct _GioDownloadPrivate
{
	GFileInputStream *input;
	GFileOutputStream *output;

	guchar *chunk_buff;
	GSList *chunks;
	gint num_chunks;
};

static MemChunk* mem_chunk_new (gsize size)
{
	g_return_val_if_fail (size > 0, NULL);

	MemChunk *chunk = MEM_CHUNK(g_malloc (sizeof (MemChunk)));
	chunk->data		= g_slice_alloc (size);
	chunk->size		= size;
	return chunk;
}

static MemChunk* mem_chunk_new_from_buffer (gsize size, gconstpointer buffer)
{
	g_return_val_if_fail (size > 0, NULL);
	MemChunk *chunk = mem_chunk_new (size);
	memcpy (chunk->data, buffer, size);
	return chunk;
}

static void mem_chunk_free (MemChunk *chunk)
{
	g_return_if_fail (chunk != NULL);
	g_slice_free1 (chunk->size, chunk->data);
	g_free (chunk); 
}

static MemChunk* flatten_mem_chunks (GSList *chunks)
{
	gsize chunk_size = 0;
	
	GSList *iter = chunks;
	while (iter != NULL) {
		chunk_size += MEM_CHUNK(iter->data)->size;
		iter = g_slist_next (iter);
	}

	MemChunk *chunk = mem_chunk_new (chunk_size);
	gsize copied_chunk_size = 0;
	iter = chunks;

	while (iter != NULL) {
		MemChunk *jchunk = MEM_CHUNK (iter->data); 
		memcpy (chunk->data + copied_chunk_size, jchunk->data, jchunk->size);
		copied_chunk_size += jchunk->size;
		iter = g_slist_next (iter);
	}

	return chunk;
}

static void
gio_download_init (GioDownload *object)
{
	object->priv = GIO_DOWNLOAD_PRIVATE (object); 
	object->priv->chunk_buff	= g_slice_alloc (CHUNK_BUFF_SIZE); 
}

static void
gio_download_finalize (GObject *object)
{
	GioDownload *download = GIO_DOWNLOAD (object);

	if (download->priv->input) {
		g_object_unref (download->priv->input);
	}
	
	if (download->priv->output) {
		write_chunks_to_output_stream (download);
		g_object_unref (download->priv->output);
	}
	
	g_slice_free1 (CHUNK_BUFF_SIZE, download->priv->chunk_buff);
	G_OBJECT_CLASS (gio_download_parent_class)->finalize (object);
}

static void
gio_download_class_init (GioDownloadClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	GDownloadableClass* parent_class = G_DOWNLOADABLE_CLASS (klass);

	g_type_class_add_private (klass, sizeof (GioDownloadPrivate));
	
	parent_class->start = gio_download_start;
	parent_class->pause = gio_download_pause;
	parent_class->serialize = gio_download_serialize;
	parent_class->deserialize = gio_download_deserialize;
	object_class->finalize = gio_download_finalize;
}

static void on_size_query_ready (GObject * object, GAsyncResult * result, gpointer data)
{
	GError *error = NULL;
	GFileInfo *info = g_file_query_info_finish (G_FILE(object), result, &error);
	GDownloadable *download = G_DOWNLOADABLE(data);
	g_assert (download != NULL);
	
	if (error) {
		handle_error (error);
		g_downloadable_set_status (download, G_DOWNLOADABLE_NETWORK_ERROR);
		
	} else {
		download->priv->size = g_file_info_get_attribute_uint64 (info, G_FILE_ATTRIBUTE_STANDARD_SIZE);
		g_object_unref (info);
	}
}

static void on_input_stream_close_ready (GObject * object, GAsyncResult * res, gpointer data)
{
	GError * error = NULL;
	g_input_stream_close_finish (G_INPUT_STREAM(object), res, &error);
	handle_critical_error (error);
}

static void on_output_stream_close_ready (GObject * object, GAsyncResult * res, gpointer data)
{
	GError * error = NULL;
	g_output_stream_close_finish (G_OUTPUT_STREAM(object), res, &error);
	handle_critical_error (error);
}

static void write_chunks_to_output_stream (GioDownload *download) 
{
	if (download->priv->chunks == NULL)
		return;
	
	GError *error = NULL;	
	download->priv->chunks = g_slist_reverse (download->priv->chunks); /* Important */
	MemChunk *chunk = flatten_mem_chunks (download->priv->chunks);
	
	g_output_stream_write  (G_OUTPUT_STREAM (download->priv->output), chunk->data, chunk->size, NULL, &error); 
	handle_critical_error (error);

	g_slist_free_full (download->priv->chunks, (GDestroyNotify)mem_chunk_free);
	download->priv->chunks = NULL;
	mem_chunk_free (chunk);
}

static void on_input_stream_read_ready (GObject * object, GAsyncResult * res, gpointer data)
{
	GError * error = NULL;
	GDownloadable *download = G_DOWNLOADABLE (data);
	GioDownload *gio_download = GIO_DOWNLOAD (data);
	g_assert (download != NULL);
	
	gssize size = g_input_stream_read_finish (G_INPUT_STREAM(object), res, &error);
	handle_critical_error (error);
	
	if (size < 0)  {
		g_downloadable_set_status (download, G_DOWNLOADABLE_NETWORK_ERROR);
		return;
	}
	
	if (size == 0 && download->priv->size == download->priv->downloaded_size) {
		/* Download Completed */
		write_chunks_to_output_stream (gio_download);

		g_input_stream_close_async (G_INPUT_STREAM(gio_download->priv->input), G_PRIORITY_DEFAULT,
									NULL, on_input_stream_close_ready, NULL);
		g_output_stream_close_async (G_OUTPUT_STREAM(gio_download->priv->output), G_PRIORITY_DEFAULT,
									 NULL, on_output_stream_close_ready, NULL);
		
		g_downloadable_set_status (download, G_DOWNLOADABLE_COMPLETED); 
		g_signal_emit_by_name (download, "download-completed");
			
		return;
	}

	if (gio_download->priv->num_chunks == NUM_OF_CHUNKS) {
		write_chunks_to_output_stream (gio_download);
		gio_download->priv->num_chunks = 0;
	} 

	MemChunk *chunk = mem_chunk_new_from_buffer (size, gio_download->priv->chunk_buff);
	gio_download->priv->chunks = g_slist_prepend (gio_download->priv->chunks, chunk);
	gio_download->priv->num_chunks++;

	g_signal_emit_by_name (data, "download-progressed", size);
	read_input_stream (download);
}

static void read_input_stream (GDownloadable * download)
{
	//g_message ("reading input stream ...\n");
	g_assert (download != NULL);
	
	if (download->priv->status != G_DOWNLOADABLE_DOWNLOADING)
		return;
	
	GioDownload *gio_download = GIO_DOWNLOAD (download); 

	g_input_stream_read_async (G_INPUT_STREAM(gio_download->priv->input), gio_download->priv->chunk_buff, CHUNK_BUFF_SIZE,
							   G_PRIORITY_DEFAULT, NULL, on_input_stream_read_ready, download);
	//g_message ("reading input stream ...\n");
} 

static void on_file_replace_ready (GObject *object, GAsyncResult *res, gpointer data)
{
	GError *error = NULL;
	GDownloadable *download = G_DOWNLOADABLE (data);
	GioDownload *gio_download = GIO_DOWNLOAD (data);
	g_assert (download != NULL);

	gio_download->priv->output = g_file_replace_finish (G_FILE(object), res, &error);
	//	GFileOutputStream *out_stream = g_file_replace_finish (G_FILE(object), res, &error);
	//	gio_download->priv->output	  = g_buffered_output_stream_new_sized (G_OUTPUT_STREAM(out_stream),
	//																	CHUNK_BUFF_SIZE);
	handle_critical_error (error);
	read_input_stream (download);
}

static void on_file_append_to_ready (GObject * object, GAsyncResult * res, gpointer data)
{
	GError *error = NULL; 
	GDownloadable *download = G_DOWNLOADABLE (data); 
	GioDownload *gio_download = GIO_DOWNLOAD (data);
	g_assert (download != NULL);
	
	GFileOutputStream *out_stream = g_file_append_to_finish (G_FILE(object), res, &error); 
	handle_error (error);
	
	if (out_stream != NULL)  {
		gio_download->priv->output = out_stream;
		if (g_seekable_seek (G_SEEKABLE(gio_download->priv->input), download->priv->downloaded_size, G_SEEK_SET, NULL, NULL)) {
			read_input_stream (download);
			return; 
		} 
	}

	g_file_replace_async (download->priv->local_file, NULL, FALSE, 0, G_PRIORITY_DEFAULT,
						  NULL, on_file_replace_ready,  download);
}

static void on_file_read_ready (GObject * object, GAsyncResult * res, gpointer data)
{
	GError *error = NULL;
	GDownloadable *download = G_DOWNLOADABLE (data);
	GioDownload   *gio_download = GIO_DOWNLOAD (data);
	g_assert (download != NULL);
	
	gio_download->priv->input = g_file_read_finish (G_FILE(object), res, &error); 
	handle_critical_error (error);
	
	if (gio_download->priv->input == NULL) {
		// TODO: Error details
		g_downloadable_set_status (download, G_DOWNLOADABLE_NETWORK_ERROR);
	} else {
		if (g_seekable_can_seek(G_SEEKABLE(gio_download->priv->input))
			&& g_file_query_exists (download->priv->local_file, NULL)) {
			//g_message ("appeding file ...\n");
			g_file_append_to_async (download->priv->local_file, 0,G_PRIORITY_DEFAULT,
									NULL, on_file_append_to_ready, download); 
		} else {
			//g_message ("replacing file ...\n");
			g_file_replace_async (download->priv->local_file, NULL, FALSE, 0, G_PRIORITY_DEFAULT,
								  NULL, on_file_replace_ready,  download);
		}
		g_downloadable_set_status (download, G_DOWNLOADABLE_DOWNLOADING);
	}
}

static void after_mount_enclosing_volume (GDownloadable * download)
{
	//g_message ("get size and read after mounting ...\n");
	if (download->priv->size ==  0)
		g_file_query_info_async (download->priv->remote_file,
								 G_FILE_ATTRIBUTE_STANDARD_SIZE,
								 0, G_PRIORITY_DEFAULT, NULL,
								 on_size_query_ready, download);
	
	g_file_read_async (download->priv->remote_file, G_PRIORITY_DEFAULT, NULL,
					   on_file_read_ready, download); 
}

static void on_mount_enclosing_ready (GObject * object, GAsyncResult * res, gpointer data)
{
	GError *error = NULL;
	
	if (g_file_mount_enclosing_volume_finish (G_FILE(object), res, &error))
		after_mount_enclosing_volume (G_DOWNLOADABLE(data)); 
	else
		handle_error (error); 
}

static void on_find_enclosing_mount_ready (GObject * object, GAsyncResult * res, gpointer data)
{
	GError *error = NULL;
	GDownloadable *download = G_DOWNLOADABLE (data);
	GMount *mount = g_file_find_enclosing_mount_finish (G_FILE(object), res, &error); 
	g_assert (download != NULL);
	
	if (mount) {
		handle_error (error);
		
		GMountOperation * operation = g_mount_operation_new ();
		g_mount_operation_set_anonymous (operation, TRUE); 
		g_file_mount_enclosing_volume (download->priv->remote_file, 0, operation, NULL, on_mount_enclosing_ready, data);

		// TODO: ask username & password
		g_object_unref (operation);
		g_object_unref (mount);		
	} else {
		after_mount_enclosing_volume (download);
	} 
}

static void gio_download_start (GDownloadable  * download, gboolean resume)
{
	g_downloadable_set_status (download, G_DOWNLOADABLE_CONNECTING);

	//g_message ("finding enclosing mount ...\n");
	g_file_find_enclosing_mount_async (download->priv->remote_file,
									   G_PRIORITY_DEFAULT, NULL,
									   on_find_enclosing_mount_ready, download); 
}

static void gio_download_pause (GDownloadable *download)
{
	g_downloadable_set_status (download, G_DOWNLOADABLE_PAUSED);
}


static void gio_download_serialize (GDownloadable* download,
									GKeyFile * keys,
									const gchar * group)
{
		/* Nothing needs to be done */ 
}

void gio_download_deserialize (GDownloadable * download,
							   GKeyFile * keys,
							   const gchar * group)
{
	/* Nothing needs to be done */ 
} 
