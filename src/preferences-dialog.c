/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * preferences-dialog.c
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

#include "main-window.h"
#include "rookie-i18n.h"
#include "rookie-settings.h"
#include "preferences-dialog.h"

static GtkWidget* create_general_page ();

GtkWidget*  preferences_dialog_new ()
{

	GtkWidget *dialog	= gtk_dialog_new_with_buttons (_("Preferences"), GTK_WINDOW(get_main_window()),
													   GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
													   GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE, NULL);
	
	GtkWidget *action_area	= gtk_dialog_get_action_area (GTK_DIALOG (dialog));
	GtkWidget *content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
	GtkWidget *notebook		= gtk_notebook_new ();

	gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
							  create_general_page (),
							  gtk_label_new (_("General")));

	gtk_box_pack_start (GTK_BOX (content_area), notebook, TRUE, TRUE, 10);
	
	gtk_window_resize (GTK_WINDOW (dialog), 450, 300); 
	gtk_container_set_border_width (GTK_CONTAINER (dialog), 8); 
	gtk_window_set_title (GTK_WINDOW (dialog), _("Edit Preferences"));
	gtk_window_set_transient_for (GTK_WINDOW(dialog), GTK_WINDOW(get_main_window ()));

	gtk_widget_show_all (action_area);
	gtk_widget_show_all (content_area);

	return dialog;
}

static GtkWidget* create_general_page ()
{
	GtkWidget *box = gtk_vbox_new (FALSE, 10);
	GtkWidget *behavior_label  = gtk_label_new (NULL);
	GtkWidget *start_minimized = gtk_check_button_new_with_label (_("Minimize to tray on startup"));
	GtkWidget *start_new_downloads = gtk_check_button_new_with_label (_("Start new downloads automatically")); 
	
	gtk_misc_set_alignment (GTK_MISC(behavior_label), 0.0, -1);
	gtk_label_set_markup (GTK_LABEL(behavior_label), _("<b>Behavior</b>"));

	gtk_box_pack_start (GTK_BOX(box), behavior_label, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(box), start_minimized, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(box), start_new_downloads, FALSE, FALSE, 0);

	gtk_widget_set_margin_left (box, 10);
	gtk_widget_set_margin_top  (box, 10);
	gtk_widget_set_margin_left (start_minimized, 10);
	gtk_widget_set_margin_left (start_new_downloads, 10);

	rookie_settings_bind (ROOKIE_START_MINIMIZED, start_minimized, "active");
	rookie_settings_bind (ROOKIE_START_NEW_DOWNLOADS, start_new_downloads, "active");
	
	return box;
} 
