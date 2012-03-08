/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * download-dialog.c
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

#include "download-dialog.h"
#include "rookie-debug.h"
#include "rookie-misc.h"
#include "rookie-settings.h"
#include "rookie-i18n.h" 
#include "main-window.h"
#include "categories.h"
#include "g-downloadable-backends.h"

struct _DownloadDialogPrivate
{
	GtkWidget *url_entry;
	GtkWidget *save_as;
	GtkWidget *save_to;
	GtkWidget *more;

	GtkWidget *dialog_content;
	GtkWidget *main_box;
	GtkWidget *more_box;

	gboolean  showing_mainbox;
	gboolean  first_time;
	GtkWidget *separator;
	GtkWidget *backend;
	GtkWidget *category;
	GtkWidget *backend_label;
	GtkWidget *category_label; 
	GtkWidget *backend_options;	
	GtkWidget *start_downloading; 
};

#define DOWNLOAD_DIALOG_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), DOWNLOAD_TYPE_DIALOG, DownloadDialogPrivate))

static void on_more_clicked (GtkButton *button, gpointer user_data);
static void on_url_change (GtkEditable *editable, gpointer user_data);
static void on_save_as_change (GtkEditable *editable, gpointer user_data); 
static void backend_combo_append_text (GDownloadableBackend *backend, GtkComboBoxText *combo);
static void	category_combo_append_text (Category *category,	 GtkComboBoxText *combo);

G_DEFINE_TYPE (DownloadDialog, download_dialog, GTK_TYPE_DIALOG);

static void
download_dialog_init (DownloadDialog *object)
{
	GtkBuilder *builder = gtk_builder_new ();
	gchar  *ui_path	= rookie_misc_get_ui_path ("AddDownloadDialog.ui");
	GError *error = NULL;

	gtk_builder_add_from_file (builder,  ui_path, &error);
	handle_error (error);
	g_free (ui_path);
		
	object->priv = DOWNLOAD_DIALOG_PRIVATE (object);
	object->priv->url_entry =  (GtkWidget *) gtk_builder_get_object (builder, "url-entry");
	object->priv->save_as   =  (GtkWidget *) gtk_builder_get_object (builder, "save-as");
	object->priv->save_to   =  (GtkWidget *) gtk_builder_get_object (builder, "save-to");
	object->priv->main_box  =  (GtkWidget *) gtk_builder_get_object (builder, "main-content");
	object->priv->more_box  =  (GtkWidget *) gtk_builder_get_object (builder, "more-content");
	object->priv->start_downloading  =  (GtkWidget *) gtk_builder_get_object (builder, "start-downloading");

	/* Glade doesn't support GtkComboBoxText yet ;( */
	
	GtkWidget *table = (GtkWidget *) gtk_builder_get_object (builder, "more-table"); 
	object->priv->backend	= gtk_combo_box_text_new ();
	object->priv->category	= gtk_combo_box_text_new ();

	gtk_table_attach (GTK_TABLE (table),
					  object->priv->category, 1, 2, 0, 1,
					  GTK_EXPAND | GTK_FILL, 0, 0, 0);
	
	gtk_table_attach (GTK_TABLE (table),
					  object->priv->backend,  1, 2, 1, 2,
					  GTK_EXPAND | GTK_FILL, 0, 0, 0);

	gtk_widget_show (object->priv->category);
	gtk_widget_show (object->priv->backend);
	
	gtk_file_chooser_set_action (GTK_FILE_CHOOSER(object->priv->save_to),
								 GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);

	GtkWidget *content_area = gtk_dialog_get_content_area (GTK_DIALOG(object));
	GtkWidget *action_area  = gtk_dialog_get_action_area (GTK_DIALOG (object));
	
	GtkWidget *download	= gtk_button_new_with_mnemonic ("_Download");
	GtkWidget *cancel	= gtk_button_new_from_stock (GTK_STOCK_CANCEL);
	object->priv->more  = gtk_button_new_with_mnemonic ("_More"); 

	gtk_button_box_set_layout (GTK_BUTTON_BOX (action_area), GTK_BUTTONBOX_END);
	gtk_dialog_add_action_widget (GTK_DIALOG (object), cancel, GTK_RESPONSE_CANCEL); 
	gtk_dialog_add_action_widget (GTK_DIALOG (object), download, GTK_RESPONSE_OK);

	gtk_box_pack_start (GTK_BOX (action_area), object->priv->more, FALSE, FALSE, 0);
	gtk_button_box_set_child_secondary (GTK_BUTTON_BOX (action_area), object->priv->more, TRUE);
	gtk_widget_show_all (action_area);
	
	//	gtk_dialog_add_button (GTK_DIALOG (object), "_More", 0);
	//	gtk_button_box_set_child_secondary (GTK_BUTTON_BOX (action_area), more, TRUE); 

	object->priv->dialog_content = (GtkWidget *) gtk_builder_get_object (builder, "dialog-content");
	gtk_box_pack_start (GTK_BOX(content_area), object->priv->dialog_content, TRUE, TRUE, 0);

	object->priv->first_time = TRUE;
	object->priv->showing_mainbox = TRUE;
	
	gtk_window_set_default_size (GTK_WINDOW(object), 550, -1);
	gtk_container_set_border_width (GTK_CONTAINER (object), 8);
	gtk_window_set_title (GTK_WINDOW(object), _("Add download"));
	//	gtk_dialog_set_default_response (GTK_DIALOG(object), GTK_RESPONSE_OK);
	
	/* Signals */
	g_signal_connect (object->priv->url_entry, "changed", G_CALLBACK(on_url_change), object);
	g_signal_connect (object->priv->save_as, "changed", G_CALLBACK(on_save_as_change), object);
	g_signal_connect (object->priv->more, "clicked", G_CALLBACK (on_more_clicked), object);
	
	/* If current text in clipbard is valid url, set it as url entry text */
	GtkClipboard * clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
	gchar * text = gtk_clipboard_wait_for_text (clipboard); 
	
	if (text && rookie_misc_is_valid_url (text)) {
		gtk_entry_set_text (GTK_ENTRY(object->priv->url_entry), text);
		g_free (text);
	} 

	/* If main window is visible, center dialog for window */
	GtkWidget *window = get_main_window ();
	if (gtk_widget_get_visible (window))
		gtk_window_set_transient_for (GTK_WINDOW(object), GTK_WINDOW(window)); 
	
	g_object_ref (object->priv->main_box);
	g_object_ref (object->priv->more_box);
	g_object_unref (builder); 
}

static void
download_dialog_finalize (GObject *object)
{
	DownloadDialog *dialog = DOWNLOAD_DIALOG (object);

	g_object_unref (dialog->priv->main_box);
	g_object_unref (dialog->priv->more_box);
	
	G_OBJECT_CLASS (download_dialog_parent_class)->finalize (object);
}

static void
download_dialog_class_init (DownloadDialogClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	//GtkDialogClass* parent_class = GTK_DIALOG_CLASS (klass);
	
	g_type_class_add_private (klass, sizeof (DownloadDialogPrivate));

	object_class->finalize = download_dialog_finalize;
}

static void on_url_change (GtkEditable * editable, gpointer data)
{
	GFile *file = g_file_new_for_uri (gtk_entry_get_text (GTK_ENTRY(editable)));
	DownloadDialog *dialog = DOWNLOAD_DIALOG (data);
	gchar *basename = g_file_get_basename (file);
	
	if (basename) {
			gtk_entry_set_text (GTK_ENTRY(dialog->priv->save_as), basename);
		gtk_editable_set_position (GTK_EDITABLE(dialog->priv->save_as), -1);
		g_free (basename);
	}

	const gchar *url = gtk_entry_get_text (GTK_ENTRY(dialog->priv->url_entry));
	Category *category = categories_get_category_by_url (url);

	if (category != NULL)
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(dialog->priv->save_to), category_get_path (category));
		
	g_object_unref (file); 
}

static void on_save_as_change (GtkEditable * editable, gpointer data)
{
	const gchar *filename = gtk_entry_get_text (GTK_ENTRY(editable));

	if (filename) {
		gchar *type = g_content_type_guess (filename, NULL, 0, NULL);
		gtk_entry_set_icon_from_gicon (GTK_ENTRY(editable), GTK_ENTRY_ICON_PRIMARY,
									   g_content_type_get_icon (type));
		g_free (type);
	} 
}

static void on_more_clicked (GtkButton *button, gpointer user_data)
{
	DownloadDialog *dialog = DOWNLOAD_DIALOG (user_data);

	if (dialog->priv->showing_mainbox) {
		gtk_container_remove (GTK_CONTAINER (dialog->priv->dialog_content), dialog->priv->main_box);
		gtk_container_add (GTK_CONTAINER (dialog->priv->dialog_content), dialog->priv->more_box);
		gtk_button_set_label (GTK_BUTTON(dialog->priv->more), "_Back");
	} else {
		gtk_container_remove (GTK_CONTAINER (dialog->priv->dialog_content), dialog->priv->more_box);
		gtk_container_add (GTK_CONTAINER (dialog->priv->dialog_content), dialog->priv->main_box); 
		gtk_button_set_label (GTK_BUTTON(dialog->priv->more), "_More");
		gtk_window_resize (GTK_WINDOW(dialog), 550, 120);
	}

	if (dialog->priv->first_time) {
		g_downloadable_backends_foreach ((GFunc)backend_combo_append_text, dialog->priv->backend);
		categories_foreach_category  ((GFunc)category_combo_append_text, dialog->priv->category);

		const gchar *url = gtk_entry_get_text (GTK_ENTRY(dialog->priv->url_entry));
		Category *category = categories_get_category_by_url (url);

		if (category != NULL) {
			gint position = category_get_id (category);
			gtk_combo_box_set_active (GTK_COMBO_BOX (dialog->priv->category), position);

			GDownloadableBackend *backend = g_downloadable_backends_by_name (category_get_backend (category));
			gtk_combo_box_set_active (GTK_COMBO_BOX (dialog->priv->backend), backend->id);
			
		} else {
			gtk_combo_box_set_active (GTK_COMBO_BOX (dialog->priv->backend), 0);
			gtk_combo_box_set_active (GTK_COMBO_BOX (dialog->priv->category), 0);
		}

		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->priv->start_downloading),
									  rookie_settings_get_start_new_downloads ());
		
		dialog->priv->first_time = FALSE;
	}

	dialog->priv->showing_mainbox = ! dialog->priv->showing_mainbox;

}

GtkWidget * download_dialog_new ()
{
	return GTK_WIDGET (g_object_new (DOWNLOAD_TYPE_DIALOG, NULL));
}

gchar * download_dialog_get_url (DownloadDialog * dialog)
{
	return gtk_editable_get_chars (GTK_EDITABLE(dialog->priv->url_entry), 0, -1);
}

gchar * download_dialog_get_basename (DownloadDialog * dialog)
{
	return gtk_editable_get_chars (GTK_EDITABLE(dialog->priv->save_as), 0, -1);
}

gboolean download_dialog_get_start_downloading (DownloadDialog *dialog)
{
	return gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->priv->start_downloading));
}

Category* download_dialog_get_category (DownloadDialog *dialog)
{
	gchar *category_name = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT(dialog->priv->category));
	Category *category =  categories_get_category_by_name (category_name);
	
	g_free(category_name);
	return category;
}

GDownloadableBackend *download_dialog_get_backend (DownloadDialog *dialog)
{
	gchar *backend_name = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT(dialog->priv->backend));
	GDownloadableBackend *backend = g_downloadable_backends_by_name (backend_name);

	g_free (backend_name);
	return backend;
}

gchar * download_dialog_get_save_path (DownloadDialog * dialog)
{
	gchar * uri = gtk_file_chooser_get_uri (GTK_FILE_CHOOSER(dialog->priv->save_to));
	GFile * file = g_file_new_for_uri (uri);
	g_free (uri);

	gchar * path  = g_file_get_path (file);
	g_object_unref (file);
	return path;
}

void download_dialog_set_url (DownloadDialog * dialog, const gchar * url)
{
	gtk_entry_set_text (GTK_ENTRY(dialog->priv->url_entry), url);
} 

static void backend_combo_append_text (GDownloadableBackend *backend, GtkComboBoxText *combo)
{
	gtk_combo_box_text_append_text (combo, backend->name);
}

static void category_combo_append_text (Category *category, GtkComboBoxText *combo)
{
	gtk_combo_box_text_append_text (combo, category_get_name (category)); 
}
