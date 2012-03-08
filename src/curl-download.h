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

#ifndef _CURL_DOWNLOAD_H_
#define _CURL_DOWNLOAD_H_

#include <glib-object.h>
#include "g-downloadable.h"

G_BEGIN_DECLS

#define CURL_TYPE_DOWNLOAD             (curl_download_get_type ())
#define CURL_DOWNLOAD(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), CURL_TYPE_DOWNLOAD, CurlDownload))
#define CURL_DOWNLOAD_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), CURL_TYPE_DOWNLOAD, CurlDownloadClass))
#define CURL_IS_DOWNLOAD(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CURL_TYPE_DOWNLOAD))
#define CURL_IS_DOWNLOAD_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), CURL_TYPE_DOWNLOAD))
#define CURL_DOWNLOAD_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), CURL_TYPE_DOWNLOAD, CurlDownloadClass))

typedef struct _CurlDownloadPrivate CurlDownloadPrivate;
typedef struct _CurlDownloadClass CurlDownloadClass;
typedef struct _CurlDownload CurlDownload;

struct _CurlDownloadClass
{
	GDownloadableClass parent_class;
};

struct _CurlDownload
{
	GDownloadable parent_instance;

	CurlDownloadPrivate * priv;
};

GType curl_download_get_type (void) G_GNUC_CONST;
void curl_download_deserialize (GDownloadable * download, GKeyFile * keys, const gchar * group);
void curl_download_global_init ();
G_END_DECLS

#endif /* _CURL_DOWNLOAD_H_ */
