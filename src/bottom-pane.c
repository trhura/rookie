/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * bottom-pane.h
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
#include "rookie-i18n.h"
#include "bottom-pane.h"

static gboolean selected;
static GDownloadable *selected_download;

static GtkWidget *notebook;
static GtkWidget *ptablabel;
static GtkWidget *pwidget;
static GtkWidget *plabel;

static GtkWidget *iwidget;
static GtkWidget *itablabel;
static GtkWidget *ilabel;

static GtkWidget *lwidget;
static GtkWidget *ltablabel;
static GtkWidget *llabel;

static GtkWidget *url;
static GtkWidget *location;
static GtkWidget *category;
static GtkWidget *size;
static GtkWidget *downloaded;
static GtkWidget *started;

static GtkWidget *notify;
static GtkWidget *open;
static GtkWidget *open_location;
static GtkTextBuffer *comment;

static GtkListStore *logs_store;

enum {
	INFORMATION_TAB_POSITION,
	PROPERTIES_TAB_POSITION,
	LOGS_TAB_POSITION
};

static void update_information_page ();
static void update_properties_page ();
static void update_logs_page ();
static void create_information_widget ();
static void update_information_widget ();
static void create_properties_widget ();
static void update_properties_widget ();
static void create_logs_widget ();
static void update_logs_widget ();

static void on_toggle (GtkToggleButton *button,
					   GDownloadableFinishAction action);

static gboolean on_comment_save (GtkWidget *widget,
								 GdkEventFocus *event,
								 gpointer data);

GtkWidget* create_bottom_pane ()
{
	notebook	= gtk_notebook_new ();
	itablabel	= gtk_label_new (_("Information"));
	ptablabel   = gtk_label_new (_("Properties"));
	ltablabel	= gtk_label_new (_("Logs"));
	plabel		= gtk_label_new ("Please select exactly one download to edit its properties.");
	ilabel		= gtk_label_new (_("Please select exactly one download to view its information."));
	llabel		= gtk_label_new (_("Please select exactly one download to view its logs."));

	create_information_widget ();
	create_properties_widget ();
	create_logs_widget ();

	g_object_ref (itablabel);
	g_object_ref (ilabel);
	g_object_ref (iwidget);
	g_object_ref (ptablabel);
	g_object_ref (plabel);
	g_object_ref (pwidget);
	g_object_ref (ltablabel);
	g_object_ref (llabel);
	g_object_ref (lwidget);

	update_bottom_pane (NULL);
	return notebook;
}

void update_bottom_pane (GDownloadable *download)
{
	gint selected_page =
		gtk_notebook_get_current_page (GTK_NOTEBOOK(notebook));

	if (download != NULL) {
		selected = TRUE;
		selected_download  = download;
	} else {
		selected = FALSE;
	}

	update_information_page ();
	update_properties_page ();
	update_logs_page ();

	gtk_notebook_set_current_page (GTK_NOTEBOOK(notebook), selected_page);
}

static void update_information_page ()
{
	static GtkWidget *page;

	if (selected) {
		update_information_widget ();
		page = iwidget;
	} else {
		page = ilabel;
	}

	gtk_notebook_remove_page (GTK_NOTEBOOK (notebook),
							  INFORMATION_TAB_POSITION);
	gtk_notebook_insert_page (GTK_NOTEBOOK (notebook), page, itablabel,
							  INFORMATION_TAB_POSITION);
}

static void
update_properties_page ()
{
	static GtkWidget *page;

	if (selected) {
		update_properties_widget ();
		page = pwidget;
	} else {
		page = plabel;
	}

	gtk_notebook_remove_page (GTK_NOTEBOOK (notebook),
							  PROPERTIES_TAB_POSITION);
	gtk_notebook_insert_page (GTK_NOTEBOOK (notebook), page, ptablabel,
							  PROPERTIES_TAB_POSITION);
}

static void
update_logs_page ()
{
	static GtkWidget *page;

	if (selected) {
		update_logs_widget ();
		page = lwidget;
	} else {
		page = llabel;
	}

	gtk_notebook_remove_page (GTK_NOTEBOOK (notebook),
							  LOGS_TAB_POSITION);
	gtk_notebook_insert_page (GTK_NOTEBOOK (notebook), page, ltablabel,
							  LOGS_TAB_POSITION);
}

static void
create_information_widget ()
{
	iwidget = gtk_grid_new ();

	url = gtk_label_new (NULL);
	GtkWidget *url_label = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL(url_label), "<b>URL:</b>");
	gtk_grid_attach (GTK_GRID(iwidget), url_label, -1, -1, 1, 1);
	gtk_grid_attach (GTK_GRID(iwidget), url, 0, -1, 1, 1);

	size = gtk_label_new (NULL);
	GtkWidget *size_label = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL(size_label), "<b>Size:</b>");
	gtk_grid_attach (GTK_GRID(iwidget), size_label, -1,  0, 1, 1);
	gtk_grid_attach (GTK_GRID(iwidget), size, 0, 0, 1, 1);

	location = gtk_label_new (NULL);
	GtkWidget *location_label = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL(location_label), "<b>Location:</b>");
	gtk_grid_attach (GTK_GRID(iwidget), location_label, -1, 1, 1, 1);
	gtk_grid_attach (GTK_GRID(iwidget), location, 0, 1, 1, 1);

	category = gtk_label_new (NULL);
	GtkWidget *category_label = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL(category_label), "<b>Category:</b>");
	gtk_grid_attach (GTK_GRID(iwidget), category_label, -1, 2, 1, 1);
	gtk_grid_attach (GTK_GRID(iwidget), category, 0, 2, 1, 1);

	downloaded = gtk_label_new (NULL);
	GtkWidget *downloaded_label = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL(downloaded_label), "<b>Downloaded:</b>");
	gtk_grid_attach (GTK_GRID(iwidget), downloaded_label, -1, 3, 1, 1);
	gtk_grid_attach (GTK_GRID(iwidget), downloaded, 0, 3, 1, 1);

	started = gtk_label_new (NULL);
	GtkWidget *started_label = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL(started_label), "<b>Started Date:</b>");
	gtk_grid_attach (GTK_GRID(iwidget), started_label, -1, 4, 1, 1);
	gtk_grid_attach (GTK_GRID(iwidget), started, 0, 4, 1, 1);

	gtk_grid_set_row_spacing (GTK_GRID(iwidget), 10);
	gtk_grid_set_column_spacing (GTK_GRID(iwidget), 10);

	gtk_label_set_selectable (GTK_LABEL (size), TRUE);
	gtk_label_set_selectable (GTK_LABEL (location), TRUE);
	gtk_label_set_selectable (GTK_LABEL (category), TRUE);
	gtk_label_set_selectable (GTK_LABEL (downloaded), TRUE);
	gtk_label_set_selectable (GTK_LABEL (started), TRUE);
	gtk_label_set_selectable (GTK_LABEL (url), TRUE);

	gtk_misc_set_alignment (GTK_MISC(url), 0.0, 0.5);
	gtk_misc_set_alignment (GTK_MISC(location), 0.0, 0.5);
	gtk_misc_set_alignment (GTK_MISC(category), 0.0, 0.5);
	gtk_misc_set_alignment (GTK_MISC(size), 0.0, 0.5);
	gtk_misc_set_alignment (GTK_MISC(downloaded), 0.0, 0.5);
	gtk_misc_set_alignment (GTK_MISC(started), 0.0, 0.5);
	gtk_misc_set_alignment (GTK_MISC(url_label), 1.0, 0.5);
	gtk_misc_set_alignment (GTK_MISC(location_label), 1.0, 0.5);
	gtk_misc_set_alignment (GTK_MISC(category_label), 1.0, 0.5);
	gtk_misc_set_alignment (GTK_MISC(size_label), 1.0, 0.5);
	gtk_misc_set_alignment (GTK_MISC(downloaded_label), 1.0, 0.5);
	gtk_misc_set_alignment (GTK_MISC(started_label), 1.0, 0.5);

	gtk_widget_set_margin_left (iwidget, 15);
	gtk_widget_set_margin_top (iwidget, 15);
	gtk_widget_show_all (iwidget);
}

static void
create_properties_widget ()
{
	pwidget = gtk_vbox_new (FALSE, 10);

	GtkWidget *finish_label = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL(finish_label), "<b>When Finished:</b>");

	GtkWidget *buttonbox = gtk_hbutton_box_new ();
	open   = gtk_check_button_new_with_label ("Open File");
	notify = gtk_check_button_new_with_label ("Show Notification");
	open_location = gtk_check_button_new_with_label ("Open Location");

	gtk_box_pack_start (GTK_BOX(buttonbox), notify, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(buttonbox), open, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(buttonbox), open_location, FALSE, FALSE, 0);

	GtkWidget *comment_label = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL(comment_label), "<b>Comment:</b>");

	GtkWidget *text_view  = gtk_text_view_new ();
	comment = gtk_text_view_get_buffer (GTK_TEXT_VIEW(text_view));
	gtk_widget_set_size_request (text_view, -1, 60);
	gtk_text_view_set_left_margin (GTK_TEXT_VIEW(text_view), 10);
	gtk_text_view_set_right_margin (GTK_TEXT_VIEW(text_view), 10);

	gtk_box_pack_start (GTK_BOX(pwidget), finish_label, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(pwidget), buttonbox, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(pwidget), comment_label, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(pwidget), text_view, FALSE, FALSE, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX(buttonbox), GTK_BUTTONBOX_START);

	gtk_misc_set_alignment (GTK_MISC(finish_label), 0.0, 0.5);
	gtk_misc_set_alignment (GTK_MISC(comment_label), 0.0, 0.5);

	gtk_widget_set_margin_left (pwidget, 15);
	gtk_widget_set_margin_left (buttonbox, 15);
	gtk_widget_set_margin_left  (text_view, 15);
	gtk_widget_set_margin_right (text_view, 20);
	gtk_widget_set_margin_top  (pwidget, 15);

	g_signal_connect (notify, "toggled", G_CALLBACK (on_toggle),
					  GINT_TO_POINTER (G_DOWNLOADABLE_FINISH_ACTION_NOTIFY));
	g_signal_connect (open, "toggled", G_CALLBACK (on_toggle),
					  GINT_TO_POINTER (G_DOWNLOADABLE_FINISH_ACTION_OPEN));
	g_signal_connect (open_location, "toggled", G_CALLBACK (on_toggle),
					  GINT_TO_POINTER (G_DOWNLOADABLE_FINISH_ACTION_OPEN_LOCATION));
	g_signal_connect (text_view, "focus-out-event", G_CALLBACK (on_comment_save), NULL);

	gtk_widget_show_all (pwidget);
}

static void
create_logs_widget ()
{
	logs_store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
	lwidget = gtk_tree_view_new_with_model (GTK_TREE_MODEL(logs_store));

	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;

	renderer = gtk_cell_renderer_text_new ();
	g_object_set (renderer,
				  "scale", 0.9,
				  NULL);
	column   = gtk_tree_view_column_new_with_attributes ("Time", renderer,
														 "text", 0,
														 NULL);
	g_object_set (column,
				  "min-width", 200,
				  "expand", FALSE,
				  NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(lwidget), column);

	renderer = gtk_cell_renderer_text_new ();
	g_object_set (renderer,
				  "ellipsize", PANGO_ELLIPSIZE_END,
				  "scale", 0.9,
				  NULL);
	column   = gtk_tree_view_column_new_with_attributes ("Time", renderer,
														 "text", 1,
														 NULL);
	g_object_set (column,
				  "min-width", 300,
				  "expand", TRUE,
				  NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(lwidget), column);

	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW(lwidget), TRUE);
	gtk_tree_view_set_headers_visible  (GTK_TREE_VIEW(lwidget), FALSE);
	gtk_widget_show_all (lwidget);
}

static void
update_information_widget ()
{
	gchar *path = g_downloadable_get_save_path (selected_download);
	gchar *size_str = g_format_size_for_display (g_downloadable_get_size (selected_download));
	gchar *downloaded_str = g_format_size_for_display (g_downloadable_get_downloaded_size (selected_download));

	gchar time_str[128];
	time_t time = g_downloadable_get_start_time (selected_download);
	struct tm *started_time = localtime (&time);
	strftime (time_str, 128, "%a %d %b %R %p", started_time);

	gtk_label_set_text (GTK_LABEL (size), size_str);
	gtk_label_set_text (GTK_LABEL (location), path);
	gtk_label_set_text (GTK_LABEL (downloaded), downloaded_str);
	gtk_label_set_text (GTK_LABEL (started), time_str);
	gtk_label_set_text (GTK_LABEL (url), g_downloadable_get_url (selected_download));

	Category *cat = g_downloadable_get_category (selected_download);

	if (cat == NULL)
		gtk_label_set_text (GTK_LABEL (category), _("Unknown"));
	else
		gtk_label_set_text (GTK_LABEL (category), category_get_name (cat));

	g_free (path);
	g_free (size_str);
	g_free (downloaded_str);
}

static void
update_properties_widget ()
{
	gboolean sensitive = ! g_downloadable_is_completed (selected_download);

	gtk_widget_set_sensitive (notify, sensitive);
	gtk_widget_set_sensitive (open, sensitive);
	gtk_widget_set_sensitive (open_location, sensitive);

	if (sensitive) {
		gint action = g_downloadable_get_finish_action (selected_download);

		gboolean notify_p = action & G_DOWNLOADABLE_FINISH_ACTION_NOTIFY;
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(notify), notify_p);

		gboolean open_p = action & G_DOWNLOADABLE_FINISH_ACTION_OPEN;
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(open), open_p);

		gboolean open_location_p = action & G_DOWNLOADABLE_FINISH_ACTION_OPEN_LOCATION;
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(open_location), open_location_p);
	}

	const gchar * comment_str = g_downloadable_get_comment (selected_download);
	gtk_text_buffer_set_text (comment, comment_str, -1);
}

static void
update_logs_widget ()
{

}

static void
on_toggle (GtkToggleButton *button,
		   GDownloadableFinishAction action)
{
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button)))
		g_downloadable_add_finish_action (selected_download, action);
	else
		g_downloadable_remove_finish_action (selected_download, action);
}

static gboolean
on_comment_save (GtkWidget *widget,
				 GdkEventFocus *event,
				 gpointer data)
{
	gchar *text;
	g_object_get (comment, "text", &text, NULL);
	g_downloadable_set_comment (selected_download, text);

	return FALSE;
}
