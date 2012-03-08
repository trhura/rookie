/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * category-dialog.c
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

#include "category-dialog.h"
#include "categories.h"
#include "rookie-misc.h"
#include "rookie-i18n.h"
#include "main-window.h"
#include "g-downloadable.h"

enum {
	VISIBLE_COLUMN,
	NAME_COLUMN,
	CATEGORY_COLUMN,
	TOTAL_COLUMNS,
};

static void append_category_to_model	  (Category				 *category,
										   GtkListStore			 *list_store);
static void on_category_selection_change  (GtkTreeSelection		 *selection,
										   gpointer				  user_data);
static void on_category_visibility_toggle (GtkCellRendererToggle *toggle,
										   gchar				 *path,
										   gpointer				  user_data);
static void on_category_name_change		  (GtkEditable			 *editable,
										   gpointer				  user_data);
static void on_category_pattern_change	  (GtkEditable			 *editable,
										   gpointer				  user_data);
static void on_category_save_path_change  (GtkFileChooser		 *chooser,
										   gpointer				  user_data);
static void on_new_category				  (GtkButton			 *button,
										   gpointer				  user_data);
static void on_remove_category			  (GtkButton			 *button,
										   gpointer				  user_data);
static void on_checkbutton_toggle		  (GtkToggleButton		 *button,
										   gpointer				  user_data);
static void backend_combo_append_text	  (GDownloadableBackend  *backend,
										   GtkComboBoxText		 *combo);
static void on_category_backend_change	  (GtkComboBox			 *combo,
										   gpointer				 *user_data);

static GtkWidget *nentry;
static GtkWidget *swidget;
static GtkWidget *bcombo;
static GtkWidget *pentry;
static GtkWidget *open;
static GtkWidget *notify;
static GtkWidget *open_location;
static GtkListStore *model;
static GtkTreeSelection *selection;

GtkWidget* category_dialog_new ()
{
	GtkWidget *dialog	= gtk_dialog_new ();
	GtkWidget *action_area	= gtk_dialog_get_action_area (GTK_DIALOG (dialog));
	GtkWidget *content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog)); 

	model = gtk_list_store_new (TOTAL_COLUMNS, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_POINTER);
	GtkWidget *view		= gtk_tree_view_new_with_model (GTK_TREE_MODEL(model));
	categories_foreach_category ((GFunc)append_category_to_model, model);

	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	renderer = gtk_cell_renderer_toggle_new ();
	column	 = gtk_tree_view_column_new_with_attributes ("Visible", renderer, "active", VISIBLE_COLUMN, NULL); 
	g_signal_connect (renderer, "toggled", G_CALLBACK (on_category_visibility_toggle), model);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column	 = gtk_tree_view_column_new_with_attributes ("Category", renderer, "text", NAME_COLUMN, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW(view), TRUE);

	GtkWidget *scrolled	= gtk_scrolled_window_new (NULL, NULL);
	GtkWidget *grid		= gtk_grid_new ();// (3, 2, FALSE);
	GtkWidget *hbox		= gtk_hbox_new (FALSE, 10);
	GtkWidget *right_box = gtk_vbox_new (FALSE, 10);
	
	GtkWidget *nlabel	= gtk_label_new (_("Category Name:"));
	GtkWidget *slabel	= gtk_label_new (_("Default Save Path:"));
	GtkWidget *blabel	= gtk_label_new (_("Default Backend:"));
	GtkWidget *plabel	= gtk_label_new (_("URL Pattern:"));

	nentry	= gtk_entry_new ();
	pentry	= gtk_entry_new ();
	swidget	= gtk_file_chooser_button_new (_("Select Default Save Path"), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	bcombo  = gtk_combo_box_text_new ();

	g_downloadable_backends_foreach ((GFunc)backend_combo_append_text, bcombo);
	gtk_combo_box_set_active (GTK_COMBO_BOX (bcombo), 0);
	
	gtk_misc_set_alignment (GTK_MISC (nlabel), 0.9, 0.5);
	gtk_misc_set_alignment (GTK_MISC (slabel), 0.9, 0.5);
	gtk_misc_set_alignment (GTK_MISC (plabel), 0.9, 0.5);
	gtk_misc_set_alignment (GTK_MISC (blabel), 0.9, 0.5);

	gtk_widget_set_margin_left (grid, 10);
	gtk_grid_set_row_spacing (GTK_GRID (grid), 10);
	gtk_grid_set_column_spacing (GTK_GRID (grid), 10);

	gtk_widget_set_hexpand (nentry, TRUE);
	gtk_widget_set_hexpand (pentry, TRUE);
	gtk_widget_set_hexpand (swidget, TRUE);
	gtk_widget_set_hexpand (bcombo, TRUE);
	
	gtk_grid_attach (GTK_GRID (grid), nlabel,  -1, -1, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), nentry,   0, -1, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), plabel,  -1,  0, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), pentry,   0,  0, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), blabel,  -1,  1, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), bcombo,   0,  1, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), slabel,  -1,  2, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), swidget,  0,  2, 1, 1); 
	
	GtkWidget *on_finish = gtk_label_new (NULL);
	gtk_widget_set_margin_left (on_finish, 10);
	gtk_misc_set_alignment (GTK_MISC (on_finish), 0.0, 0.5);
	gtk_label_set_markup (GTK_LABEL (on_finish), "<b>Default Finish Action:</b>");

	gtk_widget_set_size_request (view, 170, -1);
	gtk_container_add (GTK_CONTAINER (scrolled), view);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start (GTK_BOX (hbox), scrolled, FALSE, FALSE, 0);

	GtkWidget *buttonbox = gtk_hbutton_box_new ();
	open   = gtk_check_button_new_with_label ("Open File");
	notify = gtk_check_button_new_with_label ("Show Notification"); 
	open_location = gtk_check_button_new_with_label ("Open Location");

	gtk_widget_set_margin_left (buttonbox, 20);
	gtk_box_set_homogeneous (GTK_BOX(buttonbox), FALSE);
	gtk_box_pack_start (GTK_BOX(buttonbox), notify, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(buttonbox), open, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(buttonbox), open_location, FALSE, FALSE, 0);
	
	gtk_box_pack_start (GTK_BOX (right_box), grid, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (right_box), on_finish, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (right_box), buttonbox, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), right_box, TRUE, TRUE, 0);
	
	gtk_box_pack_start (GTK_BOX (content_area), hbox, TRUE, TRUE, 8);
	
	GtkWidget *new		= gtk_button_new_from_stock (GTK_STOCK_NEW);
	GtkWidget *remove	= gtk_button_new_from_stock (GTK_STOCK_REMOVE);
	GtkWidget *close	= gtk_button_new_from_stock (GTK_STOCK_CLOSE);

	gtk_box_pack_start	(GTK_BOX(action_area), new, FALSE, FALSE, 0);
	gtk_box_pack_start	(GTK_BOX(action_area), remove, FALSE, FALSE, 0);
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog), close, GTK_RESPONSE_CLOSE);
	
	gtk_button_box_set_layout (GTK_BUTTON_BOX (action_area), GTK_BUTTONBOX_START);
	gtk_button_box_set_child_secondary (GTK_BUTTON_BOX (action_area), close, TRUE);

	gtk_window_resize (GTK_WINDOW (dialog), 650, 300);
	gtk_container_set_border_width (GTK_CONTAINER (dialog), 8);
	gtk_window_set_title (GTK_WINDOW (dialog), _("Edit Categories"));
	gtk_window_set_transient_for (GTK_WINDOW(dialog), GTK_WINDOW(get_main_window ()));

	gtk_widget_show_all (action_area);
	gtk_widget_show_all (content_area); 

	selection = gtk_tree_view_get_selection  (GTK_TREE_VIEW (view));
	g_signal_connect (new, "clicked", G_CALLBACK (on_new_category), NULL);
	g_signal_connect (remove, "clicked", G_CALLBACK (on_remove_category), NULL);
	g_signal_connect (nentry, "changed", G_CALLBACK (on_category_name_change), NULL); 
	g_signal_connect (pentry, "changed", G_CALLBACK (on_category_pattern_change), NULL);
	g_signal_connect (bcombo, "changed", G_CALLBACK (on_category_backend_change), NULL);
	g_signal_connect (selection, "changed", G_CALLBACK (on_category_selection_change), NULL);
	g_signal_connect (swidget, "selection-changed", G_CALLBACK (on_category_save_path_change), NULL);

	g_signal_connect (notify, "toggled", G_CALLBACK (on_checkbutton_toggle),
					  GINT_TO_POINTER (G_DOWNLOADABLE_FINISH_ACTION_NOTIFY));
	g_signal_connect (open, "toggled", G_CALLBACK (on_checkbutton_toggle),
					  GINT_TO_POINTER (G_DOWNLOADABLE_FINISH_ACTION_OPEN));
	g_signal_connect (open_location, "toggled", G_CALLBACK (on_checkbutton_toggle),
					  GINT_TO_POINTER (G_DOWNLOADABLE_FINISH_ACTION_OPEN_LOCATION)); 

	return dialog; 
}


static void append_category_to_model (Category * category, GtkListStore *list_store)
{
	GtkTreeIter iter;
	gtk_list_store_append (list_store, &iter);
	gtk_list_store_set	  (list_store, &iter,
						   VISIBLE_COLUMN, category_get_visible (category),
						   NAME_COLUMN, category_get_name (category),
						   CATEGORY_COLUMN, category,
						   -1);
}

static void on_category_visibility_toggle (GtkCellRendererToggle *toggle,
										   gchar				 *cpath,
										   gpointer				  user_data)
{
	GtkTreeIter  iter;
	Category	*category;
	GtkTreePath *path = gtk_tree_path_new_from_string (cpath);
		
	gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &iter, path);
	gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, CATEGORY_COLUMN, &category, -1);

	gboolean visible = !category_get_visible (category);
	category_set_visible (category,  visible);
	gtk_list_store_set (model, &iter, VISIBLE_COLUMN, visible, -1);
}

static void on_category_selection_change (GtkTreeSelection *selection, gpointer user_data)
{
	GtkTreeIter iter; 
	
	if (gtk_tree_selection_get_selected (selection, NULL, &iter)) {
		Category   *category;
		
		gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, CATEGORY_COLUMN, &category, -1);
		gchar *path = rookie_misc_expand_path (category_get_path (category));
	
		gtk_entry_set_text (GTK_ENTRY (nentry), category_get_name (category));
		gtk_entry_set_text (GTK_ENTRY (pentry), category_get_pattern (category)); 
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (swidget), path);

		GDownloadableBackend *backend = g_downloadable_backends_by_name (category_get_backend (category));
		gtk_combo_box_set_active (GTK_COMBO_BOX (bcombo), backend->id);

		gint action = category_get_finish_action (category);
		gboolean open_p = action & G_DOWNLOADABLE_FINISH_ACTION_OPEN;
		gboolean notify_p = action & G_DOWNLOADABLE_FINISH_ACTION_NOTIFY;
		gboolean open_location_p = action & G_DOWNLOADABLE_FINISH_ACTION_OPEN_LOCATION;
		
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(open), open_p);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(notify), notify_p);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(open_location), open_location_p);
		
		g_free (path);
	}
}


static void on_category_name_change (GtkEditable *editable, gpointer user_data)
{
	GtkTreeIter iter;
	Category    *category;
	const gchar	*name = gtk_entry_get_text (GTK_ENTRY (editable));
	
	if (gtk_tree_selection_get_selected (selection, NULL, &iter)) {
		gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, CATEGORY_COLUMN, &category, -1);
		
		category_set_name (category, name);
		gtk_list_store_set (model, &iter, NAME_COLUMN, name, -1);
	}
}

static void on_category_pattern_change (GtkEditable *editable, gpointer user_data)
{
	GtkTreeIter iter;
	Category    *category;
	const gchar	*pattern = gtk_entry_get_text (GTK_ENTRY(editable));
	
	if (gtk_tree_selection_get_selected (selection, NULL, &iter)) {
		gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, CATEGORY_COLUMN, &category, -1);
		
		category_set_pattern (category, pattern);
	}
}

static void on_category_save_path_change (GtkFileChooser *chooser, gpointer user_data)
{
	GtkTreeIter iter;
	Category   *category;
	gchar	   *save_path = gtk_file_chooser_get_current_folder (chooser);
	
	if (gtk_tree_selection_get_selected (selection, NULL, &iter)) {
		gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, CATEGORY_COLUMN, &category, -1);

		category_set_path (category, save_path);
		g_free (save_path);
	}
}

static void on_new_category (GtkButton *button, gpointer user_data)
{
	GtkTreeIter iter; 
	
	Category   *category = categories_append_category ();

	gtk_list_store_append (model, &iter);
	gtk_list_store_set	  (model, &iter,
						   VISIBLE_COLUMN, category_get_visible (category),
						   NAME_COLUMN, category_get_name (category),
						   CATEGORY_COLUMN, category,
						   -1);
	
	gtk_tree_selection_select_iter (selection, &iter);

}

static void on_remove_category (GtkButton *button, gpointer user_data)
{
	GtkTreeIter iter; 
	Category   *category;
	
	if (!gtk_tree_selection_get_selected (selection, NULL, &iter))
		return;

	gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, CATEGORY_COLUMN, &category, -1);
	gtk_list_store_remove (model, &iter);
	categories_remove_category (category);

	gchar *path = rookie_misc_expand_path ("~");
	
	gtk_entry_set_text (GTK_ENTRY (nentry), "");
	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (swidget), path);
	gtk_entry_set_text (GTK_ENTRY (pentry), "");

	g_free (path); 
} 

static void on_checkbutton_toggle (GtkToggleButton *button, gpointer user_data)
{
	GtkTreeIter iter; 
	Category   *category;
	gint action = GPOINTER_TO_INT (user_data);
	
	if (!gtk_tree_selection_get_selected (selection, NULL, &iter))
		return;

	gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, CATEGORY_COLUMN, &category, -1);

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button)))
		category_add_finish_action (category, action);
	else
		category_remove_finish_action (category, action); 
}


static void backend_combo_append_text (GDownloadableBackend *backend, GtkComboBoxText *combo)
{
	gtk_combo_box_text_append_text (combo, backend->name);
}

static void on_category_backend_change (GtkComboBox *combo, gpointer *user_data)
{
	GtkTreeIter iter;
	Category *category;
	gchar	 *backend  = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT(combo));
	
	if (gtk_tree_selection_get_selected (selection, NULL, &iter)) {
		gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, CATEGORY_COLUMN, &category, -1);
		category_set_backend (category, backend); 
	}
	
	g_free (backend);
} 
