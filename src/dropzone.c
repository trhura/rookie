/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * dropzone.c
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

#include "dropzone.h"
#include "main-window.h"
#include "rookie-settings.h"
#include "rookie-misc.h"

static GtkWidget *dropzone;
static GtkWidget *drawzone;
static gdouble prevx, prevy;

static gboolean		on_button_press		(GtkWidget* dropzone,
										 GdkEventButton *event,
										 gpointer user_data);

static gboolean		on_button_release	(GtkWidget* dropzone,
										 GdkEventButton *event,
										 gpointer user_data);

static gboolean		on_motion_notify	(GtkWidget* dropzone,
										 GdkEventMotion *event,
										 gpointer user_data);

static gboolean		on_drag_drop		(GtkWidget *widget,
										 GdkDragContext *context,
										 gint x,
										 gint y,
										 guint time,
										 gpointer user_data);

static void		on_drag_data_received (GtkWidget *widget,
									   GdkDragContext *context,
									   gint x,
									   gint y,
									   GtkSelectionData *selection_data,
									   guint target_type,
									   guint time,
									   gpointer data);

enum {
	TARGET_STRING,
	TARGET_URL,
};

static GtkTargetEntry targets [] = {
	{ "STRING",			0, TARGET_STRING },
	{ "text/plain",		0, TARGET_STRING },
	{ "text/uri-list",	0, TARGET_URL },
};

static guint n_targets = G_N_ELEMENTS (targets);

GtkWidget* create_dropzone ()
{
	dropzone = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	drawzone = gtk_label_new ("drag");

	gtk_window_set_decorated (GTK_WINDOW (dropzone), FALSE);
	gtk_window_set_skip_pager_hint (GTK_WINDOW (dropzone), TRUE);
	gtk_window_set_skip_taskbar_hint (GTK_WINDOW (dropzone), TRUE);
	gtk_window_set_keep_above (GTK_WINDOW (dropzone), TRUE);
	gtk_container_add (GTK_CONTAINER (dropzone), drawzone);
	gtk_widget_show (drawzone);

	gtk_drag_dest_set		(dropzone,
							 GTK_DEST_DEFAULT_MOTION |
							 GTK_DEST_DEFAULT_HIGHLIGHT,
							 targets, n_targets,
							 GDK_ACTION_COPY);

	gtk_widget_add_events	(dropzone,
							 GDK_BUTTON_MOTION_MASK	|
							 GDK_BUTTON_PRESS_MASK	|
							 GDK_BUTTON_RELEASE_MASK);

	gtk_window_set_geometry_hints (GTK_WINDOW (dropzone),
								   dropzone, NULL,
								   GDK_HINT_POS | GDK_HINT_USER_POS);

	g_signal_connect (dropzone, "button-press-event",
					  G_CALLBACK(on_button_press), NULL);
	g_signal_connect (dropzone, "motion-notify-event",
					  G_CALLBACK(on_motion_notify), NULL);
	g_signal_connect (dropzone, "button-release-event",
					  G_CALLBACK(on_button_release), NULL);
	g_signal_connect (dropzone, "drag-drop",
					  G_CALLBACK(on_drag_drop), NULL);
	g_signal_connect (dropzone, "drag-data-received", 
					  G_CALLBACK(on_drag_data_received), NULL);

	gtk_window_resize (GTK_WINDOW(dropzone),
					   rookie_settings_get_dropzone_width (),
					   rookie_settings_get_dropzone_height());

	gtk_window_move (GTK_WINDOW(dropzone),
					 rookie_settings_get_dropzone_xpos (),
					 rookie_settings_get_dropzone_ypos());

	return dropzone;
}

/*
 * When the application quits save the dropzone's position.
 */
void dropzone_save_state ()
{
	if (!gtk_widget_get_visible (dropzone))
		/* If dropzone is hidden, no need to save state */
		return;

	gint width, height, xpos, ypos;

	gtk_window_get_size (GTK_WINDOW(dropzone), &width, &height);
	gtk_window_get_position (GTK_WINDOW(dropzone), &xpos, &ypos);

	rookie_settings_set_dropzone_width  (width);
	rookie_settings_set_dropzone_height (height);
	rookie_settings_set_dropzone_xpos (xpos);
	rookie_settings_set_dropzone_ypos (ypos);
}

static gboolean
on_button_press (GtkWidget* dropzone,
				 GdkEventButton* event,
				 gpointer user_data)
{
	if (event->button == 1 ) {
		/* Left Mouse Click */
		prevx = event->x;
		prevy = event->y;

		GdkCursor *cursor = gdk_cursor_new (GDK_FLEUR);
		GdkWindow *window = gtk_widget_get_window (dropzone);
		gdk_window_set_cursor (window, cursor);
	}
	return FALSE;
}


static gboolean
on_button_release (GtkWidget* dropzone,
				   GdkEventButton* event,
				   gpointer user_data)
{
	gdk_window_set_cursor (gtk_widget_get_window (dropzone), NULL);
	return FALSE;
}

static gboolean
on_motion_notify (GtkWidget* dropzone,
				  GdkEventMotion *event,
				  gpointer user_data)
{

	gint curx, cury;
	GdkModifierType state;

	gdk_window_get_device_position (gtk_widget_get_root_window(dropzone),
									event->device, &curx, &cury, &state);

	if (state & GDK_BUTTON1_MASK) {
		gtk_window_move (GTK_WINDOW(dropzone), curx - prevx, cury- prevy);
	}

	return TRUE;
}

static gboolean
on_drag_drop (GtkWidget *widget,
			  GdkDragContext *context,
			  gint x, gint y,
			  guint time, gpointer data)
{
	GtkTargetList *target_list = gtk_target_list_new (targets, n_targets);
	GdkAtom atom = gtk_drag_dest_find_target (widget, context, target_list);

	if (atom == GDK_NONE)
		return FALSE;

	gtk_drag_get_data (widget, context, atom, time);
	gtk_target_list_unref (target_list);
	return TRUE;
}

static void
on_drag_data_received (GtkWidget *widget,
					   GdkDragContext *context,
					   gint x, gint y,
					   GtkSelectionData *selection_data,
					   guint target_type, guint time,
					   gpointer data)
{
	gchar *text = (gchar*) gtk_selection_data_get_text (selection_data);

	if (rookie_misc_is_valid_url (text))
		rookie_misc_add_download (text);

	gtk_drag_finish (context, TRUE, FALSE, time);
	g_free (text);
}
