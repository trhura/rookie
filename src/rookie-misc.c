/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * rookie-misc.c
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

#include <gio/gio.h>
#include <notify.h>

#include "rookie-misc.h"
#include "rookie-i18n.h"
#include "rookie-debug.h"
#include "rookie-settings.h"
#include "categories.h"
#include "download-dialog.h"
#include "g-downloadable.h"
#include "g-download-list.h"
#include "g-download-list-controller.h"
#include "main-window.h"
#include "dropzone.h"

gchar*
rookie_misc_get_ui_path (const gchar * name)
{
	const gchar *env = g_getenv ("ROOKIE_DATA_DIR");

	if (env == NULL)
		return g_build_filename (PACKAGE_DATA_DIR, "rookie", name, NULL);
	else
		return g_build_filename (env, name, NULL);
}

static gchar*
get_rookie_user_data_dir ()
{
	gchar *dir	= g_build_filename (g_get_user_data_dir (), "rookie", NULL);
	GFile *fil  = g_file_new_for_path (dir);
	GError *error = NULL;
	if (!g_file_query_exists (fil, NULL) &&
		!g_file_make_directory_with_parents (fil, NULL, &error))
		handle_critical_error (error);

	g_object_unref (fil);
	return dir;
}

gchar*
rookie_misc_get_state_file_path ()
{
	gchar *dir = get_rookie_user_data_dir ();
	gchar *ret = g_build_filename (dir, "downloads.ini", NULL);

	g_free (dir);
	return ret;
}

gchar*
rookie_misc_get_category_file_path ()
{
	GError *error = NULL;
	gchar *dir	  = get_rookie_user_data_dir ();
	GFile *fil	  = g_file_new_for_path (dir);
	gchar *spath  = g_build_filename (PACKAGE_DATA_DIR, "rookie",
									  "categories.ini", NULL);
	GFile *source = g_file_new_for_path (spath);
	GFile *dest	  = g_file_get_child (fil, "categories.ini");

	if (!g_file_query_exists (dest, NULL) &&
		!g_file_copy (source, dest, G_FILE_COPY_NONE, NULL, NULL, NULL, &error))
		handle_error (error);

	gchar *ret = g_build_filename (dir, "categories.ini", NULL);

	g_object_unref (source);
	g_object_unref (dest);
	g_object_unref (fil);
	g_free (spath);
	g_free (dir);

	return ret;
}

GFile*
rookie_misc_get_log_file (guint uid)
{
	gchar *dir = get_rookie_user_data_dir ();
	GFile *fil = g_file_new_for_path (dir);
	GFile *logDir = g_file_get_child (fil, "log");
	GError *error = NULL;

	if (!g_file_query_exists (logDir, NULL) &&
		!g_file_make_directory_with_parents (fil, NULL, &error))
		handle_critical_error (error);

	gchar *uid_str = g_strdup_printf ("%u", uid);
	GFile *logFile = g_file_get_child (logDir, uid_str);

	g_object_unref (logDir);
	g_object_unref (fil);
	g_free (uid_str);
	g_free (dir);

	return logFile;
}

gboolean
rookie_misc_is_valid_url (const gchar *url)
{
	g_return_val_if_fail (url != NULL, FALSE);

	gchar *schema	= g_uri_parse_scheme (url);
	gboolean ret	= ( schema != NULL );

	g_free (schema);
	return ret;
}


gboolean
rookie_misc_run_confirm_dialog (const gchar * title,
								const gchar * message,
								const gchar * ok_button,
								GtkWindow * modal)
{
	GtkWidget * dialog = gtk_message_dialog_new_with_markup (
								modal,
								GTK_DIALOG_MODAL,
								GTK_MESSAGE_QUESTION,
								GTK_BUTTONS_NONE,
								"<b>%s</b>\n\n%s",
								title,
								message);

	gtk_dialog_add_buttons (GTK_DIALOG(dialog),
							GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
							ok_button, GTK_RESPONSE_OK, NULL);
	gtk_window_set_geometry_hints (GTK_WINDOW (dialog),
								   dialog, NULL,
								   GDK_HINT_USER_SIZE | GDK_HINT_USER_POS);

	gtk_widget_set_size_request (dialog, 400, 40);
	int response = gtk_dialog_run (GTK_DIALOG(dialog));
	gtk_widget_destroy (dialog);

	return response == GTK_RESPONSE_OK;
}

void
rookie_misc_show_error_dialog (const gchar * title,
							   const gchar * message,
							   GtkWindow * modal)
{
	GtkWidget * dialog = gtk_message_dialog_new_with_markup (
									modal,
									GTK_DIALOG_MODAL,
									GTK_MESSAGE_ERROR,
									GTK_BUTTONS_CLOSE,
									"<b><big>%s</big></b>\n\n%s",
									title,
									message);

	gtk_window_set_geometry_hints (GTK_WINDOW (dialog),
								   dialog, NULL,
								   GDK_HINT_USER_SIZE | GDK_HINT_USER_POS);

	gtk_widget_set_size_request (dialog, 400, 40);
	gtk_dialog_run (GTK_DIALOG(dialog));
	gtk_widget_destroy (dialog);
}

void
rookie_misc_add_download (const gchar * url)
{
	GtkWidget * dialog = download_dialog_new (url);

	if (url)
		download_dialog_set_url (DOWNLOAD_DIALOG(dialog), url);

	GtkWidget *window = get_main_window ();

	if (gtk_window_is_active (GTK_WINDOW(window)))
		gtk_window_set_transient_for (GTK_WINDOW(dialog), GTK_WINDOW(window));

	gchar *turl, *basename;
	gchar *save_path = NULL;
	int response;

	while (GTK_RESPONSE_OK == (response = gtk_dialog_run (GTK_DIALOG(dialog)))){

		turl = download_dialog_get_url (DOWNLOAD_DIALOG(dialog));
		basename = download_dialog_get_basename (DOWNLOAD_DIALOG(dialog));
		save_path = download_dialog_get_save_path (DOWNLOAD_DIALOG(dialog));

		if (g_strcmp0 (turl, "") == 0) {
			rookie_misc_show_error_dialog (_("Empty URL"),
										   _("The URL must not be empty"),
										   GTK_WINDOW(dialog));
			continue;
		}

		if (!rookie_misc_is_valid_url (turl)) {
			rookie_misc_show_error_dialog (_("Invalid URL"),
										   _("The URL you have entered is not valid. Please correct it."),
										   GTK_WINDOW(dialog));
			continue;
		}

		if (g_strcmp0 (basename, "") == 0) {
			rookie_misc_show_error_dialog (_("Empty Filename"),
										   _("The filename must not be empty"),
										   GTK_WINDOW(dialog));
			continue;
		}

		/*Check if there is any existing file */
		GFile * save_folder = g_file_new_for_path (save_path);
		GFile * save_file = g_file_get_child (save_folder, basename);

		if (g_file_query_exists (save_file, NULL)) {
			gchar * message = g_strdup_printf(_("There is already a file \"%s\". Are you sure you want to overwrite it?"), basename);

			if (!rookie_misc_run_confirm_dialog (_("File exists"), message,
													  "_Overwrite", GTK_WINDOW(dialog))) {
				g_object_unref (save_file);
				g_object_unref (save_folder);
				continue;
			}

			g_file_delete (save_file, NULL, NULL);
		}

		g_object_unref (save_file);

		/* Check Write Permission */
		GError * error = NULL;
		GFileInfo * info = g_file_query_info (save_folder, "access::can-write", G_FILE_QUERY_INFO_NONE,
											  NULL, &error);
		handle_error (error);

		if (g_file_info_get_attribute_boolean (info, "access::can-write") == FALSE) {
			rookie_misc_show_error_dialog (_("Permission Denied"),
							   _("You don't have permission to save file in that folder."),
							   GTK_WINDOW(dialog));
			g_object_unref (info);
			g_object_unref (save_folder);
			continue;
		}

		g_free (save_path);
		g_free (basename);
		g_free (turl);
		break;
	}

	if (response == GTK_RESPONSE_OK) {

		save_path = download_dialog_get_save_path (DOWNLOAD_DIALOG(dialog));
		gchar *purl = g_strdup (download_dialog_get_url (DOWNLOAD_DIALOG(dialog)));
		gchar *path = g_build_filename (save_path, download_dialog_get_basename (DOWNLOAD_DIALOG(dialog)), NULL);

		GDownloadableBackend *backend = download_dialog_get_backend (DOWNLOAD_DIALOG(dialog));
		GDownloadable *download = g_downloadable_create_download (purl, path, backend);
		Category *category = download_dialog_get_category (DOWNLOAD_DIALOG(dialog));
		g_downloadable_set_category (download, category);
		g_download_list_add (download);

		if (download_dialog_get_start_downloading (DOWNLOAD_DIALOG (dialog))) {
			g_downloadable_start (download, FALSE);
		}
	}

	g_free (save_path);
	gtk_widget_destroy (dialog);
}

void
rookie_misc_quit ()
{
	g_object_unref (g_download_list_controller_get ());
	g_object_unref (g_download_list_get ());

	main_window_save_state ();
	categories_finalize ();
	rookie_settings_finalize ();

	gtk_main_quit ();
}

gchar*
rookie_misc_expand_path (const gchar* path)
{
	g_return_val_if_fail (path != NULL, NULL);
	gchar * ret;

	if (path[0] == '~')
		ret = g_strdup_printf ("%s%s", g_getenv ("HOME"), path+1);
	else
		ret = g_strdup (path);

	return ret;
}

void
rookie_misc_show_notification	(const gchar *summary,
								 const gchar *body,
								 const gchar *icon)
{
	if (notify_is_initted ()) {
		NotifyNotification *notification;
		GError *error = NULL;

		notification = notify_notification_new (summary, body, icon);
		notify_notification_set_timeout (notification, NOTIFY_EXPIRES_DEFAULT);
		notify_notification_show (notification, &error);
		handle_error (error);
	}
}
