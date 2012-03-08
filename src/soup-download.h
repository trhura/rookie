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

#ifndef _SOUP_DOWNLOAD_H_
#define _SOUP_DOWNLOAD_H_

#include <glib-object.h>
#include "g-downloadable.h"

G_BEGIN_DECLS

#define SOUP_TYPE_DOWNLOAD             (soup_download_get_type ())
#define SOUP_DOWNLOAD(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), SOUP_TYPE_DOWNLOAD, SoupDownload))
#define SOUP_DOWNLOAD_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), SOUP_TYPE_DOWNLOAD, SoupDownloadClass))
#define SOUP_IS_DOWNLOAD(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SOUP_TYPE_DOWNLOAD))
#define SOUP_IS_DOWNLOAD_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), SOUP_TYPE_DOWNLOAD))
#define SOUP_DOWNLOAD_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), SOUP_TYPE_DOWNLOAD, SoupDownloadClass))

typedef struct _SoupDownloadPrivate SoupDownloadPrivate;
typedef struct _SoupDownloadClass SoupDownloadClass;
typedef struct _SoupDownload SoupDownload;

struct _SoupDownloadClass
{
	GDownloadableClass parent_class;
};

struct _SoupDownload
{
	GDownloadable parent_instance;

	SoupDownloadPrivate *priv;
};

GType	soup_download_get_type (void) G_GNUC_CONST; 

G_END_DECLS

#endif /* _SOUP_DOWNLOAD_H_ */ 
