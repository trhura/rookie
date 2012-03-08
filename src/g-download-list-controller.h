/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * g-download-list-controller.h
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

#ifndef _G_DOWNLOAD_LIST_CONTROLLER_H_
#define _G_DOWNLOAD_LIST_CONTROLLER_H_

#include <gtk/gtk.h>
#include <glib-object.h>

#include "g-downloadable.h"
#include "g-download-list.h"

G_BEGIN_DECLS

#define G_TYPE_DOWNLOAD_LIST_CONTROLLER             (g_download_list_controller_get_type ())
#define G_DOWNLOAD_LIST_CONTROLLER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), G_TYPE_DOWNLOAD_LIST_CONTROLLER, GDownloadListController))
#define G_DOWNLOAD_LIST_CONTROLLER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), G_TYPE_DOWNLOAD_LIST_CONTROLLER, GDownloadListControllerClass))
#define G_IS_DOWNLOAD_LIST_CONTROLLER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), G_TYPE_DOWNLOAD_LIST_CONTROLLER))
#define G_IS_DOWNLOAD_LIST_CONTROLLER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), G_TYPE_DOWNLOAD_LIST_CONTROLLER))
#define G_DOWNLOAD_LIST_CONTROLLER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), G_TYPE_DOWNLOAD_LIST_CONTROLLER, GDownloadListControllerClass))

typedef struct _GDownloadListControllerPrivate GDownloadListControllerPrivate;
typedef struct _GDownloadListControllerClass GDownloadListControllerClass;
typedef struct _GDownloadListController GDownloadListController;

struct _GDownloadListControllerClass
{
	GObjectClass parent_class;
};

struct _GDownloadListController
{
	GObject parent_instance;
	GDownloadListControllerPrivate * priv;
};

GType g_download_list_controller_get_type (void) G_GNUC_CONST;

GDownloadListController*		g_download_list_controller_get ();
GtkWidget*						g_download_list_controller_get_view ();
GtkTreeSelection*				g_download_list_controller_get_selection ();
GDownloadable*					g_download_list_controller_get_selected_download ();
void							g_download_list_controller_foreach_selected_downloads (GFunc func);
void							g_download_list_controller_set_status_filter (GDownloadableStatus status);
void							g_download_list_controller_set_category_filter (Category *category);

G_END_DECLS

#endif /* _G_DOWNLOAD_LIST_CONTROLLER_H_ */
