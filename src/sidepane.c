/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * sidepane.c
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

#include "sidepane.h"
#include "categories.h"
#include "g-download-list.h"
#include "g-download-list-controller.h"

enum {
	STATUS_STOCK_COLUMN,
	STATUS_TEXT_COLUMN,
	STATUS_STATUS_COLUMN,
	STATUS_TOTAL_COLUMNS
};

enum {
	CATEGORY_STOCK_COLUMN,
	CATEGORY_TEXT_COLUMN,
	CATEGORY_COLUMN,
	
	CATEGORY_TOTAL_COLUMNS
};

static GtkWidget * statusview;
static GtkWidget * categoryview;
static GtkListStore * statusmodel;
static GtkListStore * categorymodel;

static void create_status_view ();
static void create_category_view ();

static void append_category (Category * category, gpointer data);
static void on_status_change (GtkTreeSelection * selection, gpointer data);
static void on_category_change (GtkTreeSelection * selection, gpointer data);
static void g_downloadable_connect_signals (GDownloadable *download, gpointer user_data);
static void on_download_added (GDownloadList *list, GDownloadable *download, gpointer data);

GtkWidget* create_sidepane ()
{
	GtkWidget* sidepane = gtk_vbox_new (FALSE, 0);
  
	create_status_view ();
	create_category_view ();

	gtk_box_pack_start (GTK_BOX(sidepane), statusview, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(sidepane), categoryview, TRUE, TRUE, 0);

	return sidepane;
}

void create_status_view ()
{
	statusmodel = gtk_list_store_new (STATUS_TOTAL_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT, NULL);
	statusview	= gtk_tree_view_new_with_model (GTK_TREE_MODEL(statusmodel));
	
	/* Status */
	GtkCellRenderer * renderer;
	GtkTreeViewColumn * column;

	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, "Status");
		
	renderer = gtk_cell_renderer_pixbuf_new (); 
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
	gtk_tree_view_column_set_attributes (column, renderer, "stock-id", STATUS_STOCK_COLUMN, NULL); 

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	g_object_set (renderer, "scale", 0.9, "weight-set", TRUE, NULL);
	gtk_tree_view_column_set_attributes (column, renderer, "markup", STATUS_TEXT_COLUMN, NULL);

	gtk_tree_view_column_set_expand (column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(statusview), column);

	refresh_status_model ();

	g_download_list_foreach ((GFunc)g_downloadable_connect_signals, NULL);

	g_signal_connect (g_download_list_get (), "download-added",
					  G_CALLBACK (on_download_added), NULL);
	g_signal_connect (g_download_list_get (), "download-removed",
					  G_CALLBACK (refresh_status_model), NULL);
	g_signal_connect (gtk_tree_view_get_selection (GTK_TREE_VIEW(statusview)),
					  "changed", G_CALLBACK(on_status_change), NULL); 
}

void create_category_view ()
{
	categorymodel = gtk_list_store_new (CATEGORY_TOTAL_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER, NULL);
	categoryview  = gtk_tree_view_new_with_model (GTK_TREE_MODEL(categorymodel));

	refresh_category_model ();

	GtkCellRenderer * renderer;
	GtkTreeViewColumn * column;

	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, "Category");
									
 	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
	gtk_tree_view_column_set_attributes (column, renderer, "stock-id", CATEGORY_STOCK_COLUMN, NULL);
	
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	g_object_set (renderer, "scale", 0.9, NULL);
	gtk_tree_view_column_set_attributes (column, renderer, "markup", CATEGORY_TEXT_COLUMN, NULL);

	gtk_tree_view_column_set_expand (column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(categoryview), column);

	g_signal_connect (gtk_tree_view_get_selection (GTK_TREE_VIEW(categoryview)), "changed", G_CALLBACK(on_category_change), NULL); 
}

void refresh_category_model ()
{
	GtkTreeIter iter;

	gtk_list_store_clear  (categorymodel);
	gtk_list_store_append (categorymodel, &iter);
	gtk_list_store_set	  (categorymodel, &iter,
						   CATEGORY_STOCK_COLUMN, GTK_STOCK_DIRECTORY,
						   CATEGORY_TEXT_COLUMN, "All",
						   CATEGORY_COLUMN, NULL,
						   -1);

	categories_foreach_category ((GFunc) append_category, NULL);
}	

static void count_downloads_by_status (GDownloadable *download, gint *downloads_by_status)
{
	gint status	= g_downloadable_get_status (download);
	downloads_by_status[status]++;
}

void refresh_status_model ()
{
	GtkTreeIter iter;
	
	gtk_list_store_clear (statusmodel);

	gint downloads_by_status[G_DOWNLOADABLE_TOTAL_STATUS] = { 0 };
	g_download_list_foreach ((GFunc) count_downloads_by_status, downloads_by_status);

	gint active  = downloads_by_status[G_DOWNLOADABLE_DOWNLOADING];
	gint paused  = downloads_by_status[G_DOWNLOADABLE_PAUSED];
	gint connect = downloads_by_status[G_DOWNLOADABLE_CONNECTING];
	gint stopped = downloads_by_status[G_DOWNLOADABLE_NETWORK_ERROR];
	gint done	 = downloads_by_status[G_DOWNLOADABLE_COMPLETED];
	gint all	 = active + paused + stopped + done + connect;

	gchar *all_string	  = g_strdup_printf ("All (%d)", all);
	gchar *active_string  = g_strdup_printf ("Downloading (%d)", active);
	gchar *paused_string  = g_strdup_printf ("Paused (%d)", paused);
	gchar *stopped_string = g_strdup_printf ("Stopped (%d)", stopped); 
	gchar *done_string	  = g_strdup_printf ("Done (%d)", done);
	
	gtk_list_store_append (statusmodel, &iter);
	gtk_list_store_set (statusmodel, &iter,
						STATUS_STOCK_COLUMN, GTK_STOCK_DIRECTORY,
						STATUS_TEXT_COLUMN, all_string,
						STATUS_STATUS_COLUMN, -1, -1);

	gtk_list_store_append (statusmodel, &iter);
	gtk_list_store_set (statusmodel, &iter,
						STATUS_STOCK_COLUMN,  GTK_STOCK_GO_DOWN,
						STATUS_TEXT_COLUMN, active_string,
						STATUS_STATUS_COLUMN, G_DOWNLOADABLE_DOWNLOADING, -1);
	
	gtk_list_store_append (statusmodel, &iter);
	gtk_list_store_set (statusmodel, &iter,
						STATUS_STOCK_COLUMN, GTK_STOCK_MEDIA_PAUSE,
						STATUS_TEXT_COLUMN, paused_string,
						STATUS_STATUS_COLUMN, G_DOWNLOADABLE_PAUSED, -1);
	
	gtk_list_store_append (statusmodel, &iter);
	gtk_list_store_set (statusmodel, &iter,
						STATUS_STOCK_COLUMN, GTK_STOCK_STOP,
						STATUS_TEXT_COLUMN, stopped_string,
						STATUS_STATUS_COLUMN, G_DOWNLOADABLE_NETWORK_ERROR, -1);
	
	gtk_list_store_append (statusmodel, &iter);
	gtk_list_store_set (statusmodel, &iter,
						STATUS_STOCK_COLUMN, GTK_STOCK_APPLY,
						STATUS_TEXT_COLUMN, done_string,
						STATUS_STATUS_COLUMN, G_DOWNLOADABLE_COMPLETED, -1);
}

static void append_category (Category * category, gpointer data)
{
	GtkTreeIter iter;

	if (category_get_visible (category)) {
		gtk_list_store_append (categorymodel, &iter);
		gtk_list_store_set	  (categorymodel, &iter,
							   CATEGORY_STOCK_COLUMN, GTK_STOCK_FILE,
							   CATEGORY_TEXT_COLUMN,  category_get_name (category),
							   CATEGORY_COLUMN, category, -1);
	}
}

static void on_status_change (GtkTreeSelection * selection, gpointer data)
{
	GtkTreeIter iter;
	gint status = 0;
	
	if (gtk_tree_selection_get_selected (selection, NULL, &iter)) {
		gtk_tree_model_get (GTK_TREE_MODEL(statusmodel), &iter, STATUS_STATUS_COLUMN, &status, -1);
		g_download_list_controller_set_status_filter (status);
	}
}

static void on_category_change (GtkTreeSelection * selection, gpointer data)
{
	GtkTreeIter iter;
	Category *category;
	
	if (gtk_tree_selection_get_selected (selection, NULL, &iter)) {
		gtk_tree_model_get (GTK_TREE_MODEL(categorymodel), &iter, CATEGORY_COLUMN, &category, -1);
		g_download_list_controller_set_category_filter (category);
	}
} 

static void on_download_status_changed (GDownloadable * download, gpointer data)
{
	refresh_status_model ();
}

static void g_downloadable_connect_signals (GDownloadable *download, gpointer data)
{
	g_signal_connect (download, "status-changed", G_CALLBACK (on_download_status_changed), data); 
} 

static void on_download_added (GDownloadList *list, GDownloadable *download, gpointer data)
{
	g_downloadable_connect_signals (download, data);
	refresh_status_model ();
} 
