/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * main-window.c
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

#include "config.h"
#include "rookie-services.h"
#include "rookie-i18n.h"
#include "categories.h"
#include "category-dialog.h"
#include "rookie-settings.h"
#include "rookie-debug.h"
#include "rookie-misc.h"
#include "g-download-list-controller.h"
#include "sidepane.h"
#include "bottom-pane.h"
#include "main-window.h"
#include "preferences-dialog.h"

static void relayout_mainbox ();
static GtkActionGroup * create_action_group ();
static void init_actions (GtkActionGroup * action_group);
static void toggle_window_visible ();

static void on_new ();
static void on_quit ();
static void on_start ();
static void on_pause ();
static void on_remove ();
static void on_open ();
static void on_open_folder ();
static void on_delete ();
static void on_preferences ();
static void on_category ();
static void on_help ();
static void on_translate ();
static void on_report ();
static void on_about ();
static void on_properties ();
static void on_sidepane ();
static void on_bottompane ();
static void on_toolbar ();
static void on_statusbar ();

static void on_egg_icon_activate (GtkStatusIcon *, gpointer *);
static void on_selection_change (GtkTreeSelection * , gpointer);
static gboolean on_window_delete (GtkWidget *, GdkEvent *, gpointer);
static gboolean on_view_button_press (GtkWidget *, GdkEventButton *, gpointer);
static void on_egg_icon_popup_menu (GtkStatusIcon *, guint, guint, gpointer);
static void on_download_added (GDownloadList *, GDownloadable *, gpointer);
static void on_download_removed (GDownloadList *, GDownloadable *, gpointer);
static void g_downloadable_connect_signals (GDownloadable *, gpointer);

static GtkWidget * mainpopup;
static GtkWidget * viewpopup;
static GtkWidget * statpopup;
static GtkWidget * menubar;
static GtkWidget * toolbar;
static GtkWidget * statbar;
static GtkWidget * sidepane;
static GtkWidget * bottompane;

static GtkWidget * window;
static GtkWidget * mainbox;
static GtkWidget * hpaned;
static GtkWidget * vpaned;
static GtkWidget * view;
static GtkWidget * sw1;
static GtkWidget * sw2;
static GtkStatusIcon *eggicon;
static GtkTreeSelection *selection;

static GtkAction * a_start;
static GtkAction * a_pause;
static GtkAction * a_remove;
static GtkAction * a_open;
static GtkAction * a_open_location;
static GtkAction * a_delete;
static GtkAction * a_properties;

void create_main_window ()
{
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	GtkUIManager		*ui_manager		= gtk_ui_manager_new ();
	GtkActionGroup		*action_group	= create_action_group ();

	GError	 *error	= NULL;
	gchar	 *ui_path = rookie_misc_get_ui_path ("AppUI.ui");

	gtk_ui_manager_insert_action_group (ui_manager, action_group, 0);
	gtk_ui_manager_add_ui_from_file (ui_manager, ui_path, &error);
	handle_critical_error (error);
	g_free (ui_path);

	eggicon = gtk_status_icon_new_from_icon_name ("rookie");
	init_actions (action_group);

	menubar = gtk_ui_manager_get_widget (ui_manager, "/menubar");
	toolbar = gtk_ui_manager_get_widget (ui_manager, "/toolbar");
	statbar = gtk_statusbar_new ();
	mainbox = gtk_vbox_new (FALSE, 0);

	mainpopup = gtk_ui_manager_get_widget (ui_manager, "/m_popup");
	viewpopup = gtk_ui_manager_get_widget (ui_manager, "/v_popup");
	statpopup = gtk_ui_manager_get_widget (ui_manager, "/s_popup");

	gtk_toolbar_set_icon_size (GTK_TOOLBAR(toolbar),
							   GTK_ICON_SIZE_LARGE_TOOLBAR);
	gtk_toolbar_set_style (GTK_TOOLBAR(toolbar), GTK_TOOLBAR_BOTH);

	sw1	= gtk_scrolled_window_new (NULL, NULL);
	sw2 = gtk_scrolled_window_new (NULL, NULL);

	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(sw1), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(sw2), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	sidepane = create_sidepane ();
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW(sw1), sidepane);

	view	= g_download_list_controller_get_view ();
	gtk_container_add (GTK_CONTAINER (sw2), view);

	bottompane = create_bottom_pane ();

	vpaned = gtk_vpaned_new ();
	gtk_paned_add1 (GTK_PANED(vpaned), sw2);
	gtk_paned_add2 (GTK_PANED(vpaned), bottompane);

	hpaned = gtk_hpaned_new ();
	gtk_paned_add1 (GTK_PANED(hpaned), sw1);
	gtk_paned_add2 (GTK_PANED(hpaned), vpaned);

	gtk_paned_set_position (GTK_PANED(hpaned), rookie_settings_get_sidepane_width ());
	gtk_paned_set_position (GTK_PANED(vpaned), rookie_settings_get_bottompane_width ());

	gtk_window_add_accel_group (GTK_WINDOW(window), gtk_ui_manager_get_accel_group(ui_manager));
	gtk_window_set_title (GTK_WINDOW(window), _("Rookie Download Manager"));
	gtk_window_set_icon_name (GTK_WINDOW(window), "rookie");

	gtk_box_pack_start (GTK_BOX(mainbox), menubar, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(mainbox), toolbar, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(mainbox), hpaned, TRUE, TRUE, 0);
	gtk_box_pack_end (GTK_BOX(mainbox), statbar, FALSE, FALSE, 0);

	relayout_mainbox ();
	gtk_container_add (GTK_CONTAINER(window), mainbox);
	gtk_window_set_geometry_hints (GTK_WINDOW (window), window, NULL,
								   GDK_HINT_USER_SIZE | GDK_HINT_USER_POS | GDK_HINT_POS);

	gtk_window_resize (GTK_WINDOW(window),
					   rookie_settings_get_window_width (),
					   rookie_settings_get_window_height());

	rookie_settings_bind (ROOKIE_TOOLBAR_VISIBLE, toolbar, "visible");
	rookie_settings_bind (ROOKIE_STATUSBAR_VISIBLE, statbar, "visible");

	/* Signals */
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(view));

	g_download_list_foreach ((GFunc)g_downloadable_connect_signals, NULL);

	g_signal_connect (eggicon, "activate",
					  G_CALLBACK(on_egg_icon_activate), NULL);
	g_signal_connect (eggicon, "popup-menu",
					  G_CALLBACK(on_egg_icon_popup_menu), NULL);
	g_signal_connect (window, "delete-event",
					  G_CALLBACK(on_window_delete), NULL);

	g_signal_connect (g_download_list_get (), "download-added",
					  G_CALLBACK (on_download_added), NULL);
	g_signal_connect (g_download_list_get (), "download-removed",
					  G_CALLBACK (on_download_removed), NULL);
	g_signal_connect (view, "button-press-event",
					  G_CALLBACK(on_view_button_press), selection);

	g_signal_connect (selection, "changed",
					  G_CALLBACK(on_selection_change), NULL);
}

void relayout_mainbox ()
{
	gboolean mvisible = rookie_settings_get_visible (ROOKIE_SIDEPANE_VISIBLE);
	gboolean nvisible = rookie_settings_get_visible (ROOKIE_BOTTOMPANE_VISIBLE);

	GtkWidget *parent;
	GtkWidget *left_widget = nvisible ? vpaned : sw2;
	GtkWidget *main_widget = mvisible ? hpaned : left_widget;

	parent  = gtk_widget_get_parent (vpaned);
	if (parent != NULL) {
		g_object_ref (vpaned);
		gtk_container_remove (GTK_CONTAINER(parent), vpaned);
	}

	parent  = gtk_widget_get_parent (sw2);
	if (parent != NULL) {
		g_object_ref (sw2);
		gtk_container_remove (GTK_CONTAINER(parent), sw2);
	}

	parent  = gtk_widget_get_parent (hpaned);
	if (parent != NULL) {
		g_object_ref (hpaned);
		gtk_container_remove (GTK_CONTAINER(parent), hpaned);
	}

	if (nvisible) {
		gtk_paned_add1 (GTK_PANED (vpaned), sw2);
		g_object_unref (sw2);
	}

	if (mvisible) {
		gtk_paned_add2 (GTK_PANED (hpaned), left_widget);
		g_object_unref (left_widget);
	}

	gtk_box_pack_start  (GTK_BOX (mainbox), main_widget, TRUE, TRUE, 0);
	gtk_widget_show_all (mainbox);
}

GtkWidget* get_main_window ()
{
	return window;
}

void init_actions (GtkActionGroup * action_group)
{
	a_start		= gtk_action_group_get_action (action_group, "a_start");
	a_pause		= gtk_action_group_get_action (action_group, "a_pause");
	a_remove	= gtk_action_group_get_action (action_group, "a_remove");
	a_open		= gtk_action_group_get_action (action_group, "a_open");
	a_delete	= gtk_action_group_get_action (action_group, "a_delete");
	a_properties = gtk_action_group_get_action (action_group, "a_properties");
	a_open_location	= gtk_action_group_get_action (action_group, "a_open_location");

	gtk_action_set_sensitive (a_remove, FALSE);
	gtk_action_set_sensitive (a_properties, FALSE);
	gtk_action_set_sensitive (a_start, FALSE);
	gtk_action_set_sensitive (a_pause, FALSE);
	gtk_action_set_sensitive (a_open, FALSE);
	gtk_action_set_sensitive (a_open_location, FALSE);
	gtk_action_set_sensitive (a_delete, FALSE);
}

void show_main_window ()
{
	gtk_widget_show (window);
}

GtkActionGroup * create_action_group ()
{
	GtkActionEntry a_file_menu = {
		"a_file_menu", NULL, _("_File")
	};

	GtkActionEntry a_edit_menu = {
		"a_edit_menu", NULL, _("_Edit")
	};

	GtkActionEntry a_view_menu = {
		"a_view_menu", NULL, _("_View")
	};

	GtkActionEntry a_help_menu = {
		"a_help_menu", NULL, _("_Help")
	};

	GtkActionEntry a_tool_menu = {
		"a_tool_menu", NULL, _("_Tool")
	};

	GtkActionEntry a_new = {
		"a_new", GTK_STOCK_ADD, _("_New"),
		"<control>n", _("Create a new download"),
		G_CALLBACK(on_new)
	};

	GtkActionEntry a_add = {
		"a_add", GTK_STOCK_ADD, _("_Add Download"),
		NULL, _("Add a new download"),
		G_CALLBACK(on_new)
	};

	GtkActionEntry a_quit = {
		"a_quit", GTK_STOCK_QUIT, _("_Quit"),
		"<control>q", _("Quit this application"),
		G_CALLBACK(on_quit)
	};

	GtkActionEntry a_start = {
		"a_start", GTK_STOCK_MEDIA_PLAY, _("_Start"),
		"<control>s", _("Start or resume selected downloads"),
		G_CALLBACK(on_start)
	};

	GtkActionEntry a_pause = {
		"a_pause", GTK_STOCK_MEDIA_PAUSE, _("_Pause"),
		"<control>p", _("Pause selected downloads"),
		G_CALLBACK(on_pause)
	};

	GtkActionEntry a_remove = {
		"a_remove", GTK_STOCK_REMOVE, _("_Remove"),
		NULL, _("Remove selected downloads"),
		G_CALLBACK(on_remove)
	};

	GtkActionEntry a_open = {
		"a_open", GTK_STOCK_FILE, ("_Open File"),
		NULL, _("Open selected files"),
		G_CALLBACK(on_open)
	};

	GtkActionEntry a_open_location = {
		"a_open_location", GTK_STOCK_DIRECTORY, _("Open _Location"),
		NULL, _("Open locations of selected files"),
		G_CALLBACK(on_open_folder)
	};

	GtkActionEntry a_delete = {
		"a_delete", GTK_STOCK_DELETE, _("_Delete File"),
		NULL, _("Move selected downloads to trash"),
		G_CALLBACK(on_delete)
	};

	GtkActionEntry a_preferences = {
		"a_preferences", GTK_STOCK_PREFERENCES, _("Pr_eferences"),
		NULL, _("Edit application settings"),
		G_CALLBACK(on_preferences)
	};

	GtkToggleActionEntry a_toolbar = {
		"a_toolbar", NULL, ("_Toolbar"),
		NULL, _("Show or hide toolbar"), G_CALLBACK(on_toolbar),
		rookie_settings_get_visible (ROOKIE_TOOLBAR_VISIBLE)
	};

	GtkToggleActionEntry a_statusbar = {
		"a_statusbar", NULL, _("_Statusbar"),
		NULL, _("Show or hide statusbar"), G_CALLBACK(on_statusbar),
		rookie_settings_get_visible (ROOKIE_STATUSBAR_VISIBLE)
	};

	GtkToggleActionEntry a_sidepane = {
		"a_sidepane", NULL, _("_Side Pane"),
		NULL, _("Show or hide side pane"), G_CALLBACK(on_sidepane),
		rookie_settings_get_visible (ROOKIE_SIDEPANE_VISIBLE)
	};

	GtkToggleActionEntry a_bottompane = {
		"a_bottompane", NULL, _("_Bottom Pane"),
		NULL, _("Show or hide bottom pane"), G_CALLBACK(on_bottompane),
		rookie_settings_get_visible (ROOKIE_BOTTOMPANE_VISIBLE)
	};

	GtkActionEntry a_category = {
		"a_category", NULL, _("Manage Categories"),
		NULL, _("Manange categories"), G_CALLBACK(on_category)
	};

	GtkActionEntry a_help = {
		"a_help", GTK_STOCK_HELP, _("_Get Help Online"),
		"F1", NULL, G_CALLBACK(on_help)
	};

	GtkActionEntry a_translate = {
		"a_translate", NULL, _("_Translate This Application"),
		NULL, _("Open launchpad to translate this application"),
		G_CALLBACK(on_translate)
	};

	GtkActionEntry a_report = {
		"a_report", NULL, _("_Report Problems"),
		NULL, NULL,
		G_CALLBACK(on_report)
	};

	GtkActionEntry a_about = {
		"a_about", GTK_STOCK_ABOUT, _("_About"),
		NULL, _("About this application"),
		G_CALLBACK(on_about)
	};

	GtkActionEntry a_properties = {
		"a_properties", GTK_STOCK_PROPERTIES, _("_Download Properties"),
		NULL, _("View Download Properties"),
		G_CALLBACK(on_properties)
	};

	GtkActionEntry normal_actions[] = {
		a_file_menu,
		a_edit_menu,
		a_view_menu,
		a_help_menu,
		a_tool_menu,

		a_new,
		a_add,
		a_quit,

		a_start,
		a_pause,
		a_remove,
		a_open,
		a_open_location,
		a_delete,
		a_preferences,

		a_category,

		a_help,
		a_translate,
		a_report,
		a_about,

		a_properties
	};

	GtkToggleActionEntry toggle_actions[] = {
		a_toolbar,
		a_statusbar,
		a_sidepane,
		a_bottompane
	};

	int n1 = G_N_ELEMENTS (normal_actions);
	int n2 = G_N_ELEMENTS (toggle_actions);

	GtkActionGroup * ret = gtk_action_group_new ("Default");
	gtk_action_group_add_actions (ret, normal_actions, n1, NULL);
	gtk_action_group_add_toggle_actions (ret, toggle_actions, n2, NULL);

	return ret;
}

/* Callbacks */
static void
on_download_status_changed (GDownloadable * download,
							gpointer data)
{
	/* When the status of a download changed, we might need to update actions, if it is
	 * selected. For now, we just simply emit selection changed to update action.		*/
	g_signal_emit_by_name (selection, "changed");
	refresh_status_model ();
}

static void
g_downloadable_connect_signals (GDownloadable *download,
								gpointer data)
{
	g_signal_connect (download, "status-changed",
					  G_CALLBACK (on_download_status_changed), data);
}

static void
on_download_added (GDownloadList *list,
				   GDownloadable *download,
				   gpointer data)
{
	g_downloadable_connect_signals (download, data);
	refresh_status_model ();
	refresh_category_model ();
}

static void
on_download_removed (GDownloadList *list,
					 GDownloadable *download,
					 gpointer data)
{
	refresh_status_model ();
	refresh_category_model ();
}

static void on_new ()
{
	rookie_misc_add_download (NULL);
}


static void on_quit ()
{
	rookie_misc_quit ();
}


static void
start_download (GDownloadable *download,
				gpointer data)
{
	if (g_downloadable_can_start (download))
		g_downloadable_start (download, FALSE);
}

static void on_start ()
{
	g_download_list_controller_foreach_selected_downloads ((GFunc) start_download);
}

static void
pause_download (GDownloadable *download,
				gpointer data)
{
	if (g_downloadable_can_pause (download))
		g_downloadable_pause (download);
}

static void on_pause ()
{
	g_download_list_controller_foreach_selected_downloads ((GFunc) pause_download);
}

static void
remove_download (GDownloadable *download,
				 gpointer data)
{
	gchar * message = g_strdup_printf (_("%s is not not compelete yet. Do you really want to remove it?"),
									   g_downloadable_get_basename (download));

	if (!g_downloadable_is_completed (download)) {
		if (rookie_misc_run_confirm_dialog (_("Incomplete Download"), message,
											_("_Remove"), GTK_WINDOW(window)))
			g_downloadable_delete (download);
		else
			goto _remove_download_release;
	}

	g_download_list_remove (download);

_remove_download_release:
	g_free (message);
}

static void on_remove ()
{
	g_download_list_controller_foreach_selected_downloads ((GFunc) remove_download);
}


static void open_download (GDownloadable * download, gpointer data)
{
	if (g_downloadable_is_completed (download)) {
		g_downloadable_open (download);
	}
}

static void on_open ()
{
	g_download_list_controller_foreach_selected_downloads ((GFunc) open_download);
}

static void on_open_folder ()
{
	g_download_list_controller_foreach_selected_downloads ((GFunc) g_downloadable_open_location);
}

static void delete_download (GDownloadable * download, gpointer data)
{
	gchar * message = g_strdup_printf (_("Do you really want to delete %s?"),
									   g_downloadable_get_basename (download));

	if (rookie_misc_run_confirm_dialog (_("Delete Download"), message ,
											 _("_Delete"), GTK_WINDOW(window))) {
		g_downloadable_delete (download);
		g_download_list_remove (download);
	}

	g_free (message);
}

static void on_delete ()
{
	g_download_list_controller_foreach_selected_downloads ((GFunc) delete_download);
}

static void on_preferences ()
{
	static GtkWidget *dialog = NULL;

	if (dialog == NULL)
		dialog = preferences_dialog_new ();

	gtk_dialog_run (GTK_DIALOG(dialog));
	gtk_widget_hide (dialog);
}

static void on_category ()
{
	static GtkWidget *dialog = NULL;

	if (dialog == NULL)
		dialog = category_dialog_new ();

	gtk_dialog_run (GTK_DIALOG(dialog));
	gtk_widget_hide (dialog);

	refresh_category_model ();
}

static void on_help ()
{
	gtk_show_uri (NULL, "https://answers.launchpad.net/rookie", GDK_CURRENT_TIME, NULL);
}

static void on_translate ()
{
	gtk_show_uri (NULL, "https://translations.launchpad.net/rookie", GDK_CURRENT_TIME, NULL);
}

static void on_report ()
{
	gtk_show_uri (NULL, "https://bugs.launchpad.net/rookie/+filebug", GDK_CURRENT_TIME, NULL);
}

static void on_about ()
{
	gchar * authors[] = {"Thura Hlaing <trhura@gmail.com>", NULL};

	const char * license[] = {
		N_("This program is free software; you can redistribute it and/or modify "
		   "it under the terms of the GNU General Public License as published by "
		   "the Free Software Foundation; either version 3 of the License, or "
		   "(at your option) any later version.\n"),
		N_("This program is distributed in the hope that it will be useful, "
		   "but WITHOUT ANY WARRANTY; without even the implied warranty of "
                   "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
		   "GNU General Public License for more details.\n"),
		N_("You should have received a copy of the GNU General Public License "
                   "along with this program; if not, write to the Free Software "
		   "Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.")
	};

	char * license_trans;

	license_trans = g_strconcat (_(license[0]), "\n", _(license[1]), "\n",
								 _(license[2]), "\n", NULL);

	gtk_show_about_dialog (GTK_WINDOW(window),
						   "authors", authors,
						   "program-name", "Rookie",
						   "comments", _("A download manager for Gnome"),
						   "copyright", "Copyright © 2010-2011 Thura Hlaing",
						   "license", license_trans,
						   "logo-icon-name", "rookie",
						   "wrap-license", TRUE,
						   "version", PACKAGE_VERSION,
						   "website", "https://launchpad.net/rookie",
						   NULL);

	g_free (license_trans);
}

static void on_properties ()
{

}

static void on_toolbar ()
{
	gboolean visible;
	g_object_get (toolbar, "visible", &visible, NULL);
	g_object_set (toolbar, "visible", !visible, NULL);
}

static void on_statusbar ()
{
	gboolean visible;
	g_object_get (statbar, "visible", &visible, NULL);
	g_object_set (statbar, "visible", !visible, NULL);
}

static
void on_sidepane ()
{
	gboolean visible  = rookie_settings_get_visible (ROOKIE_SIDEPANE_VISIBLE);
	rookie_settings_set_visible (ROOKIE_SIDEPANE_VISIBLE, !visible);
	relayout_mainbox ();
}

static void on_bottompane ()
{
	gboolean visible  = rookie_settings_get_visible (ROOKIE_BOTTOMPANE_VISIBLE);
	rookie_settings_set_visible (ROOKIE_BOTTOMPANE_VISIBLE, !visible);
	relayout_mainbox ();
}

void toggle_window_visible ()
{
	if (gtk_widget_get_visible (window)) {
		main_window_save_state ();
		gtk_widget_hide (window);
	} else {
		gtk_window_present (GTK_WINDOW(window));
	}
}

static void
on_egg_icon_activate (GtkStatusIcon *icon,
					  gpointer *data)
{
	toggle_window_visible ();
}

static void
on_egg_icon_popup_menu (GtkStatusIcon *icon,
						guint button,
						guint time,
						gpointer data)
{
	gtk_menu_popup (GTK_MENU(statpopup), NULL, NULL,
					gtk_status_icon_position_menu,
					icon, button, time);
}

static gboolean
on_window_delete (GtkWidget *widget,
				  GdkEvent *event,
				  gpointer data)
{
	gtk_widget_hide (widget);
	return TRUE;
}

static void
update_actions_for_download (GDownloadable *download,
							 gpointer data)
{
	static gboolean flag1, flag2, flag3;

	if (download == NULL) {
		flag1 = flag2 = flag3 = TRUE;
		return;
	}

	if (flag1 && g_downloadable_is_completed (download)) {
		/* If any of selected files is finished */
		gtk_action_set_sensitive (a_open, TRUE);
		gtk_action_set_sensitive (a_delete, TRUE);
		flag1 = FALSE;
	}

	if (flag2 && g_downloadable_can_pause (download)) {
		gtk_action_set_sensitive (a_pause, TRUE);
		flag2 = FALSE;
	}

	if (flag3 && g_downloadable_can_start (download)) {
		gtk_action_set_sensitive (a_start, TRUE);
		flag3 = FALSE;
	}
}

static
void on_selection_change (GtkTreeSelection *selection,
						  gpointer data)
{
	int selected = gtk_tree_selection_count_selected_rows (selection);

	update_bottom_pane (g_download_list_controller_get_selected_download ());

	gtk_action_set_sensitive (a_remove, selected > 0);
	gtk_action_set_sensitive (a_open_location, selected > 0);
	gtk_action_set_sensitive (a_properties, selected == 0);
	gtk_action_set_sensitive (a_start, FALSE);
	gtk_action_set_sensitive (a_pause, FALSE);
	gtk_action_set_sensitive (a_open, FALSE);
	gtk_action_set_sensitive (a_delete, FALSE);

	update_actions_for_download (NULL, NULL);
	g_download_list_controller_foreach_selected_downloads ((GFunc)update_actions_for_download);
}

static gboolean
on_view_button_press (GtkWidget *widget,
					  GdkEventButton *event,
					  gpointer data)
{
	if (event->type == GDK_BUTTON_PRESS && event->button == 3)
		if (gtk_tree_selection_count_selected_rows (GTK_TREE_SELECTION(data))) {
			gtk_menu_popup (GTK_MENU(viewpopup), NULL, NULL, NULL, NULL, event->button, event->time);
			return TRUE;
		}

	return FALSE;
}

void	main_window_save_state ()
{
	if (!gtk_widget_get_visible (window))
		return;

	gint width, height, xpos, ypos;

	gtk_window_get_size (GTK_WINDOW(window), &width, &height);
	gtk_window_get_position (GTK_WINDOW(window), &xpos, &ypos);

	rookie_settings_set_window_width  (width);
	rookie_settings_set_window_height (height);

	rookie_settings_set_sidepane_width (gtk_paned_get_position (GTK_PANED(hpaned)));
	rookie_settings_set_bottompane_width (gtk_paned_get_position (GTK_PANED(vpaned)));
}
