/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * g-download-list.h
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

#ifndef _G_DOWNLOAD_LIST_H_
#define _G_DOWNLOAD_LIST_H_

#include <glib-object.h>
#include "g-downloadable.h"

G_BEGIN_DECLS

#define G_TYPE_DOWNLOAD_LIST             (g_download_list_get_type ())
#define G_DOWNLOAD_LIST(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), G_TYPE_DOWNLOAD_LIST, GDownloadList))
#define G_DOWNLOAD_LIST_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), G_TYPE_DOWNLOAD_LIST, GDownloadListClass))
#define G_IS_DOWNLOAD_LIST(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), G_TYPE_DOWNLOAD_LIST))
#define G_IS_DOWNLOAD_LIST_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), G_TYPE_DOWNLOAD_LIST))
#define G_DOWNLOAD_LIST_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), G_TYPE_DOWNLOAD_LIST, GDownloadListClass))

typedef struct _GDownloadListPrivate GDownloadListPrivate;
typedef struct _GDownloadListClass GDownloadListClass;
typedef struct _GDownloadList GDownloadList;

struct _GDownloadListClass
{
	GObjectClass parent_class;
};

struct _GDownloadList
{
	GObject parent_instance;
	GDownloadListPrivate * priv;
};

GType g_download_list_get_type (void) G_GNUC_CONST;

GDownloadList*		g_download_list_get (void);
void	g_download_list_add (GDownloadable * download);
void	g_download_list_remove (GDownloadable * download);
void	g_download_list_foreach (GFunc func, gpointer data);

G_END_DECLS

#endif /* _G_DOWNLOAD_LIST_H_ */
