/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * gio-download.h
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

#ifndef _GIO_DOWNLOAD_H_
#define _GIO_DOWNLOAD_H_

#include <glib-object.h>
#include "g-downloadable.h"

G_BEGIN_DECLS

#define GIO_TYPE_DOWNLOAD             (gio_download_get_type ())
#define GIO_DOWNLOAD(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIO_TYPE_DOWNLOAD, GioDownload))
#define GIO_DOWNLOAD_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GIO_TYPE_DOWNLOAD, GioDownloadClass))
#define GIO_IS_DOWNLOAD(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GIO_TYPE_DOWNLOAD))
#define GIO_IS_DOWNLOAD_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GIO_TYPE_DOWNLOAD))
#define GIO_DOWNLOAD_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GIO_TYPE_DOWNLOAD, GioDownloadClass))

typedef struct _GioDownloadPrivate GioDownloadPrivate;
typedef struct _GioDownloadClass GioDownloadClass;
typedef struct _GioDownload GioDownload;

struct _GioDownloadClass
{
	GDownloadableClass parent_class;
};

struct _GioDownload
{
	GDownloadable parent_instance;

	GioDownloadPrivate * priv;
};

GType	gio_download_get_type (void) G_GNUC_CONST; 

G_END_DECLS

#endif /* _GIO_DOWNLOAD_H_ */
