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
#include <curl/curl.h>
#include <glib/gprintf.h>

#include "rookie-debug.h"
#include "curl-download.h"
#include "g-downloadable-private.h"

G_DEFINE_TYPE (CurlDownload, curl_download, G_TYPE_DOWNLOADABLE);

#define CURL_DOWNLOAD_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), CURL_TYPE_DOWNLOAD, CurlDownloadPrivate))

static void curl_download_pause (GDownloadable  * download);
static void curl_download_start (GDownloadable  * download, gboolean resume);
static void curl_download_serialize (GDownloadable * download, GKeyFile * keys, const gchar * group);

struct _CurlDownloadPrivate
{
	CURL * curl;
	FILE * file;
};

static CURLM * curl_multi;

void curl_download_global_init ()
{
	curl_multi = curl_multi_init ();

	if (curl_multi == NULL)
		g_message ("cannot initialize curl multi");
}

static void curl_download_init (CurlDownload *object)
{
	/* TODO: Add initialization code here */
	object->priv = CURL_DOWNLOAD_PRIVATE (object);
}

static void
curl_download_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */
	curl_easy_cleanup (CURL_DOWNLOAD(object)->priv->curl);

	//	fclose (CURL_DOWNLOAD(object)->priv->file); FIXME:

	G_OBJECT_CLASS (curl_download_parent_class)->finalize (object);
}

static void
curl_download_class_init (CurlDownloadClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	GDownloadableClass* parent_class = G_DOWNLOADABLE_CLASS (klass);
	g_type_class_add_private (klass, sizeof (CurlDownloadPrivate));

	parent_class->start = curl_download_start;
	parent_class->pause = curl_download_pause;
	parent_class->serialize = curl_download_serialize;

	object_class->finalize = curl_download_finalize;
}

static void curl_download_pause (GDownloadable  * download)
{

}

static void curl_download_progressed (void * ptr, double dltotal, double dlnow, double ultotal, double ulnow)
{
	g_printf ("%f %f %f %f\n", dltotal, dlnow, ultotal, ulnow);
}

static size_t curl_download_write_file (void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	g_printf ("writing file ....\n");
	return fwrite(ptr, size, nmemb, stream);
}

static void curl_download_start (GDownloadable  * object, gboolean resume)
{
	CurlDownload * download  = CURL_DOWNLOAD (object);

	download->priv->curl = curl_easy_init ();

	if (download->priv->curl) {

		gchar * path = g_file_get_path (object->priv->local_file);
		download->priv->file = fopen (path, "a");
		g_free (path);

		curl_easy_setopt(download->priv->curl, CURLOPT_URL, object->priv->url);

		curl_easy_setopt(download->priv->curl, CURLOPT_WRITEFUNCTION, curl_download_write_file);
		curl_easy_setopt(download->priv->curl, CURLOPT_WRITEDATA, download->priv->file);

		curl_easy_setopt(download->priv->curl, CURLOPT_NOPROGRESS, 0L);
		curl_easy_setopt(download->priv->curl, CURLOPT_PROGRESSFUNCTION, curl_download_progressed);
		curl_easy_setopt(download->priv->curl, CURLOPT_PROGRESSDATA, object);

		gint running_handle;

		g_printf ("Curl multi performing ...\n");
		curl_multi_add_handle (curl_multi, download->priv->curl);

		if (CURLM_OK != curl_multi_perform (curl_multi, &running_handle)) {
			g_printf ("Curl multi perform failed ...\n");
		}
	}
}

static void curl_download_serialize (GDownloadable * download, GKeyFile * keys, const gchar * group)
{

}
