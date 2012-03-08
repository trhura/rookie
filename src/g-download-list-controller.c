/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * g-download-list-controller.c
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
#include "g-download-list-controller.h"

#define G_DOWNLOAD_LIST_CONTROLLER_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE	\
												((o),					\
												 G_TYPE_DOWNLOAD_LIST_CONTROLLER, \
												 GDownloadListControllerPrivate))

static const gdouble REDRAW_SEC = 0.5;
static const gint ICON_SIZE		= 22;

struct _GDownloadListControllerPrivate
{
	GTimer * timer;

	GtkWidget *view;
	GtkTreeSelection *selection;
	GtkListStore *model;

	GDownloadableStatus status_filter;
	Category *category_filter;
	gboolean status_set;
	gboolean category_set;
};

enum Columns {
	DOWNLOAD_COLUMN, UID_COLUMN,
	ICONNAME_COLUMN, PIXBUF_COLUMN,
	FILENAME_COLUMN, PROGRESS_VALUE_COLUMN,
	PROGRESS_TEXT_COLUMN, SIZE_COLUMN,
	DOWNLOADED_COLUMN, SPEED_COLUMN,
	TOTAL_COLUMNS
};

G_DEFINE_TYPE (GDownloadListController, g_download_list_controller, G_TYPE_OBJECT);

static void		setup_tree_view  (GtkTreeView  *view);
static void		setup_tree_model (GtkListStore *model);
static gchar*	format_speed_for_display (gint speed);
static gchar*	format_status_for_display (GDownloadableStatus status, gint percent);
static void		g_downloadable_connect_signals (GDownloadable *download, gpointer data);
static void		g_download_list_controller_refresh_row (GtkTreeIter *iter);
static void		g_download_list_controller_append_row (GDownloadable *download, gpointer data);

static void on_download_added (GDownloadList * download_list, GDownloadable * download, gpointer data);
static void on_download_removed (GDownloadList * download_list, GDownloadable * download, gpointer data);

static void
g_download_list_controller_init (GDownloadListController *object)
{
	object->priv = G_DOWNLOAD_LIST_CONTROLLER_PRIVATE (object);

	object->priv->status_set = FALSE;
	object->priv->category_set = FALSE;
	object->priv->timer = g_timer_new ();

	object->priv->model = gtk_list_store_new (TOTAL_COLUMNS, G_TYPE_POINTER,
											  G_TYPE_INT, G_TYPE_STRING, GDK_TYPE_PIXBUF,
											  G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING,
											  G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

	object->priv->view	= gtk_tree_view_new_with_model (GTK_TREE_MODEL(object->priv->model));
	setup_tree_view (GTK_TREE_VIEW(object->priv->view));

	object->priv->selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (object->priv->view));
	gtk_tree_selection_set_mode (object->priv->selection, GTK_SELECTION_MULTIPLE);
}

static void
g_download_list_controller_finalize (GObject *object)
{
	GDownloadListController *controller = G_DOWNLOAD_LIST_CONTROLLER (object);

	g_timer_destroy (controller->priv->timer);
	// g_object_unref  (controller->priv->view);
	// g_object_unref  (controller->priv->model);

	G_OBJECT_CLASS (g_download_list_controller_parent_class)->finalize (object);
}

static void
g_download_list_controller_class_init (GDownloadListControllerClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	//	GObjectClass* parent_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (GDownloadListControllerPrivate));
	object_class->finalize = g_download_list_controller_finalize;
}

GDownloadListController * g_download_list_controller_get ()
{
	static GDownloadListController * controller = NULL;

	if (controller == NULL) {
		controller = g_object_new (G_TYPE_DOWNLOAD_LIST_CONTROLLER, NULL);

		GDownloadList * list = g_download_list_get ();

		if (list == NULL)
			g_error ("Download List must be intialized before Download List Controller");

		g_download_list_foreach ((GFunc)g_downloadable_connect_signals, NULL);
		g_signal_connect (list, "download-added", G_CALLBACK(on_download_added), controller);
		g_signal_connect (list, "download-removed", G_CALLBACK(on_download_removed), controller);

		setup_tree_model (controller->priv->model);
	}

	return controller;
}

static void setup_tree_view (GtkTreeView * view)
{
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;

	renderer = gtk_cell_renderer_pixbuf_new ();
	column   = gtk_tree_view_column_new_with_attributes ("", renderer,
														 "pixbuf", PIXBUF_COLUMN, NULL);
	g_object_set (column, "sort-column-id", ICONNAME_COLUMN, "sort-indicator", TRUE, NULL);
	gtk_tree_view_append_column (view, column);

	renderer = gtk_cell_renderer_text_new ();
	g_object_set (renderer, "ellipsize", PANGO_ELLIPSIZE_END, "scale", 0.9, NULL);
	column   = gtk_tree_view_column_new_with_attributes ("Filename", renderer,
														 "text", FILENAME_COLUMN, NULL);
	g_object_set (column, "sort-column-id", FILENAME_COLUMN, "sort-indicator", TRUE,
				  "resizable", TRUE,"min-width", 200, "expand", TRUE, NULL);
	gtk_tree_view_append_column (view, column);

	renderer = gtk_cell_renderer_progress_new ();
	g_object_set (renderer, "width", 300, NULL);
	column   = gtk_tree_view_column_new_with_attributes ("Progress", renderer,
														 "value", PROGRESS_VALUE_COLUMN,
														 "text", PROGRESS_TEXT_COLUMN, NULL);
	g_object_set (column, "sort-column-id", PROGRESS_VALUE_COLUMN, "sort-indicator", TRUE,
				  "resizable", TRUE,  "min-width", 200, NULL);
	gtk_tree_view_append_column (view, column);

	renderer = gtk_cell_renderer_text_new ();
	g_object_set (renderer, "xalign", 0.9, "scale", 0.9, NULL);
	column   = gtk_tree_view_column_new_with_attributes ("Size", renderer,
														 "text", SIZE_COLUMN, NULL);
	g_object_set (column, "sort-column-id", SIZE_COLUMN, "sort-indicator", TRUE,
				  "fixed-width", 100, "expand", FALSE, "sizing", GTK_TREE_VIEW_COLUMN_FIXED, NULL);
	gtk_tree_view_append_column (view, column);

	renderer = gtk_cell_renderer_text_new ();
	g_object_set (renderer, "xalign", 0.9, "scale", 0.9, NULL);
	column   = gtk_tree_view_column_new_with_attributes ("Downloaded", renderer,
														 "text", DOWNLOADED_COLUMN, NULL);
	g_object_set (column, "sort-column-id", DOWNLOADED_COLUMN, "sort-indicator", TRUE,
				  "fixed-width", 100, "expand", FALSE, "sizing", GTK_TREE_VIEW_COLUMN_FIXED, NULL);
	gtk_tree_view_append_column (view, column);

	renderer = gtk_cell_renderer_text_new ();
	g_object_set (renderer, "xalign", 0.9, "scale", 0.9, NULL);
	column   = gtk_tree_view_column_new_with_attributes ("Speed", renderer,
														 "text", SPEED_COLUMN, NULL);
	g_object_set (column, "sort-column-id", SPEED_COLUMN, "sort-indicator", TRUE,
				  "fixed-width", 100, "expand", FALSE, "sizing", GTK_TREE_VIEW_COLUMN_FIXED, NULL);
	gtk_tree_view_append_column (view, column);

	gtk_tree_view_set_search_column (view, FILENAME_COLUMN);
	gtk_tree_view_set_rules_hint (view, TRUE);
}

static void setup_tree_model (GtkListStore *model)
{
	gtk_list_store_clear (model);
	g_download_list_foreach ((GFunc)g_download_list_controller_append_row, NULL);
}

static void g_download_list_controller_refresh_row (GtkTreeIter *iter)
{
	if (iter == NULL)
		return;

	GDownloadListController *controller = g_download_list_controller_get ();
	GtkListStore *model = controller->priv->model;

	GTimer *timer = controller->priv->timer;
	g_timer_stop (timer);

	if (g_timer_elapsed (timer, NULL) < REDRAW_SEC)
		return;

	g_timer_stop (timer);

	GDownloadable * download;
	gchar * prev_speed_string, * prev_progress_text;

	gtk_tree_model_get (GTK_TREE_MODEL(model), iter,
						DOWNLOAD_COLUMN, &download,
						PROGRESS_TEXT_COLUMN, &prev_progress_text,
						SPEED_COLUMN, &prev_speed_string, -1);

	g_return_if_fail (download != NULL);

	guint64 size = g_downloadable_get_size (download);
	guint64 downloaded = g_downloadable_get_downloaded_size (download);
	GDownloadableStatus status = g_downloadable_get_status (download);

	gint value =  size ?  (((gdouble)downloaded / (gdouble) size) * 100) : 0;
	gint speed = status == G_DOWNLOADABLE_DOWNLOADING ? g_downloadable_get_speed (download): 0;

	gtk_list_store_set (model, iter,
						PROGRESS_VALUE_COLUMN, value,
						SIZE_COLUMN, g_format_size_for_display (size),
						SPEED_COLUMN, format_speed_for_display (speed),
						PROGRESS_TEXT_COLUMN, format_status_for_display (status, value),
						DOWNLOADED_COLUMN, g_format_size_for_display (downloaded), -1);

	g_free (prev_speed_string);
	g_free (prev_progress_text);
}

GdkPixbuf* get_pixbuf_for_icon_name (const gchar * icon_name)
{
	GdkPixbuf * pixbuf = NULL;

	if (icon_name == NULL)
		return pixbuf;

	GError *error = NULL;
	GIcon  *icon = g_icon_new_for_string (icon_name, NULL);
	GtkIconTheme *icon_theme = gtk_icon_theme_get_default ();
	GtkIconInfo *icon_info = gtk_icon_theme_lookup_by_gicon (icon_theme, icon, ICON_SIZE, 0);

	pixbuf = gtk_icon_info_load_icon (icon_info, &error);
	gtk_icon_info_free (icon_info);
	handle_error (error);

	if (error) {
		pixbuf = gtk_icon_theme_load_icon (icon_theme, GTK_STOCK_FILE, ICON_SIZE, 0, &error);
		handle_error (error);
	}

	return pixbuf;
}

static void on_download_changed (GDownloadable * download, gpointer data)
{
	g_download_list_controller_refresh_row (g_downloadable_get_view_iter (download));
}

static void g_downloadable_connect_signals (GDownloadable *download, gpointer data)
{
	g_signal_connect (download, "download-progressed", G_CALLBACK (on_download_changed), data);
	g_signal_connect (download, "status-changed", G_CALLBACK (on_download_changed), data);
}

static void on_download_added (GDownloadList *download_list, GDownloadable *download, gpointer data)
{
	g_downloadable_connect_signals (download, data);
	g_download_list_controller_append_row (download, data);
}

static void on_download_removed (GDownloadList *download_list, GDownloadable *download, gpointer data)
{
	GDownloadListController  *controller = G_DOWNLOAD_LIST_CONTROLLER (data);
	GtkTreeIter *iter	= g_downloadable_get_view_iter (download);

	if (iter != NULL) {
		gtk_list_store_remove (controller->priv->model, iter);
	}
}

static void g_download_list_controller_append_row (GDownloadable *download, gpointer user_data)
{
	GDownloadListController  *controller = g_download_list_controller_get ();

	if (controller->priv->category_set &&
		controller->priv->category_filter != g_downloadable_get_category (download)) {
		g_downloadable_set_view_iter (download, NULL);
		return;
	}

	if (controller->priv->status_set &&
		controller->priv->status_filter != g_downloadable_get_status (download)) {
		g_downloadable_set_view_iter (download, NULL);
		return;
	}

	GtkTreeIter iter;

	gtk_list_store_append (controller->priv->model, &iter);
	g_downloadable_set_view_iter (download, gtk_tree_iter_copy(&iter));

	gint64 size = g_downloadable_get_size (download);
	gint64 downloaded = g_downloadable_get_downloaded_size (download);
	gint value =  size ? (double)downloaded / (double) size * 100 : 0;
	GDownloadableStatus status = g_downloadable_get_status (download);

	gtk_list_store_set (controller->priv->model, &iter,
						DOWNLOAD_COLUMN, download,
						PROGRESS_VALUE_COLUMN, value,
						SIZE_COLUMN, g_format_size_for_display (size),
						PROGRESS_TEXT_COLUMN, format_status_for_display(status, value),
						FILENAME_COLUMN, g_downloadable_get_basename (download),
						ICONNAME_COLUMN, g_downloadable_get_icon_name(download),
						DOWNLOADED_COLUMN, g_format_size_for_display (downloaded),
						PIXBUF_COLUMN, get_pixbuf_for_icon_name (g_downloadable_get_icon_name(download)),
						SPEED_COLUMN, format_speed_for_display (g_downloadable_get_speed (download)),
						-1);
}

static gchar * format_speed_for_display (gint speed)
{
	if (speed == 0)
		return "-";

	gchar * d = g_format_size_for_display (speed);
	gchar * r = g_strdup_printf ("%s/s", d);
	g_free (d);

	return r;
}

static gchar * format_status_for_display (GDownloadableStatus status, gint percent)
{
	switch (status) {
	case G_DOWNLOADABLE_COMPLETED:
		return g_strdup ("Done");

	case G_DOWNLOADABLE_DOWNLOADING:
		return g_strdup_printf("%d%%", percent);

	case G_DOWNLOADABLE_PAUSED:
		return g_strdup ("Paused");

	case G_DOWNLOADABLE_CONNECTING:
		return g_strdup ("Connecting ...");

	case G_DOWNLOADABLE_NETWORK_ERROR:
		/* TODO: Show errror details */
		return  g_strdup ("Network Error");

	default:
		g_warn_if_reached ();
		return NULL;
	}
}

GtkWidget * g_download_list_controller_get_view ()
{
	return g_download_list_controller_get()->priv->view;
}

GtkTreeSelection * g_download_list_controller_get_selection ()
{
	return g_download_list_controller_get()->priv->selection;
}

void g_download_list_controller_foreach_selected_downloads (GFunc func)
{
	/* Since func may modify the model, e.g, remove/delete download,
	 * We need to create a pointer array of selected downloads first
	 * Then call func on those downloads.		*/

	GDownloadListController *controller = g_download_list_controller_get ();
	GPtrArray *download_array = g_ptr_array_new ();

	GtkTreeIter   iter;
	GtkTreePath   *path;
	GtkTreeModel  *model;
	GDownloadable *download;

	GList *list  = gtk_tree_selection_get_selected_rows (controller->priv->selection, &model);
	GList *liter = list;

	while (liter) {
		path = (GtkTreePath *) liter->data;

		if (gtk_tree_model_get_iter (model, &iter, path)) {
			gtk_tree_model_get (model, &iter, DOWNLOAD_COLUMN, &download, -1);
			g_ptr_array_add (download_array, download);
		}

		gtk_tree_path_free (path);
		liter = g_list_next (liter);
	}

	g_ptr_array_foreach (download_array, func, NULL);
	g_ptr_array_free (download_array, FALSE);
	g_list_free (list);
}


GDownloadable* g_download_list_controller_get_selected_download ()
{
	GDownloadListController *controller = g_download_list_controller_get ();
	GtkTreeModel  *model;
	GDownloadable *download;
	GtkTreeIter  iter;

	if (gtk_tree_selection_count_selected_rows (controller->priv->selection) != 1) {
		return NULL;
	}

	GList *list  = gtk_tree_selection_get_selected_rows (controller->priv->selection, &model);
	GtkTreePath *path = list->data;

	if (gtk_tree_model_get_iter (model, &iter, path)) {
		gtk_tree_model_get (model, &iter, DOWNLOAD_COLUMN, &download, -1);
	}

	gtk_tree_path_free (path);
	g_list_free (list);

	return download;
}

void g_download_list_controller_set_status_filter (GDownloadableStatus status)
{
	GDownloadListController *controller = g_download_list_controller_get ();

	if (status == -1) {
		controller->priv->status_set = FALSE;

	} else {
		controller->priv->status_filter = status;
		controller->priv->status_set = TRUE;

	}
	setup_tree_model (controller->priv->model);
}

void g_download_list_controller_set_category_filter (Category *category)
{
	GDownloadListController *controller = g_download_list_controller_get ();

	if (category == NULL) {
		controller->priv->category_set = FALSE;

	} else {
		controller->priv->category_filter = category;
		controller->priv->category_set = TRUE;
	}

	setup_tree_model (controller->priv->model);
}
