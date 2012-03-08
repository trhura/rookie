/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * download-dialog.h
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

#ifndef _DOWNLOAD_DIALOG_H_
#define _DOWNLOAD_DIALOG_H_

#include <glib-object.h>
#include <gtk/gtk.h>

#include "categories.h"
#include "g-downloadable-backends.h"

G_BEGIN_DECLS

#define DOWNLOAD_TYPE_DIALOG             (download_dialog_get_type ())
#define DOWNLOAD_DIALOG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), DOWNLOAD_TYPE_DIALOG, DownloadDialog))
#define DOWNLOAD_DIALOG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DOWNLOAD_TYPE_DIALOG, DownloadDialogClass))
#define DOWNLOAD_IS_DIALOG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DOWNLOAD_TYPE_DIALOG))
#define DOWNLOAD_IS_DIALOG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DOWNLOAD_TYPE_DIALOG))
#define DOWNLOAD_DIALOG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DOWNLOAD_TYPE_DIALOG, DownloadDialogClass))

typedef struct _DownloadDialogPrivate DownloadDialogPrivate;
typedef struct _DownloadDialogClass DownloadDialogClass;
typedef struct _DownloadDialog DownloadDialog;

struct _DownloadDialogClass
{
	GtkDialogClass parent_class;
};

struct _DownloadDialog
{
	GtkDialog parent_instance;

	DownloadDialogPrivate * priv;
};

GType download_dialog_get_type (void) G_GNUC_CONST;

GtkWidget*	 download_dialog_new ();
gchar*		 download_dialog_get_url (DownloadDialog *dialog);
gchar*		 download_dialog_get_basename (DownloadDialog *dialog); 
gchar*		 download_dialog_get_save_path (DownloadDialog *dialog);
gboolean	 download_dialog_get_start_downloading (DownloadDialog *dialog);

Category*	 download_dialog_get_category (DownloadDialog *dialog);
GDownloadableBackend *download_dialog_get_backend (DownloadDialog *dialog);

void		 download_dialog_set_url (DownloadDialog *dialog, const gchar *url);

G_END_DECLS

#endif /* _DOWNLOAD_DIALOG_H_ */ 
