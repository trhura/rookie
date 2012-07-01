/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * rookie-settings.c
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
#include "rookie-settings.h"

static GSettings * general;
static GSettings * state;

#define WINDOW_WIDTH_KEY		  "window-width"
#define WINDOW_HEIGHT_KEY		  "window-height"
#define SIDEPANE_WIDTH_KEY		  "sidebar-width"
#define BOTTOMPANE_WIDTH_KEY	  "bottompane-width"
#define START_MINIMIZED_KEY		  "start-minimized"
#define START_NEW_DOWNLOADS		  "start-new-downloads"

#define TOOLBAR_VISIBLE_KEY		  "show-toolbar"
#define SIDEPANE_VISIBLE_KEY	  "show-sidebar"
#define STATUSBAR_VISIBLE_KEY	  "show-statusbar"
#define BOTTOMPANE_VISIBLE_KEY	  "show-bottompane"

void rookie_settings_init ()
{
	general	 = g_settings_new ("net.launchpad.rookie");
	state	 = g_settings_new ("net.launchpad.rookie.state");
}

void rookie_settings_finalize ()
{
	g_settings_apply (general);
	g_settings_apply (state);
	g_object_unref (general);
	g_object_unref (state);
}

GVariant*  rookie_settings_get  (RookieSetting setting)
{
	switch (setting) {

	case ROOKIE_WINDOW_WIDTH:
		return g_settings_get_value (state, WINDOW_WIDTH_KEY);

	case ROOKIE_WINDOW_HEIGHT:
		return g_settings_get_value (state, WINDOW_HEIGHT_KEY);

	case ROOKIE_SIDEPANE_WIDTH:
		return g_settings_get_value (state, WINDOW_HEIGHT_KEY);

	default:
		g_warn_if_reached ();
		return NULL;
	}
}

void rookie_settings_set  (RookieSetting setting, GVariant *value)
{
	switch (setting) {

	case ROOKIE_WINDOW_WIDTH:
		g_settings_set_value (state, WINDOW_WIDTH_KEY, value);
		break;

	case ROOKIE_WINDOW_HEIGHT:
		g_settings_set_value (state, WINDOW_HEIGHT_KEY, value);
		break;

	case ROOKIE_SIDEPANE_WIDTH:
		g_settings_set_value (state, WINDOW_HEIGHT_KEY, value);
		break;

	default:
		g_warn_if_reached ();
	}
}

gint rookie_settings_get_window_width ()
{
	return g_settings_get_int (state, WINDOW_WIDTH_KEY);
}

void rookie_settings_set_window_width (gint width)
{
	g_settings_set_int (state, WINDOW_WIDTH_KEY, width);
}

gint rookie_settings_get_window_height ()
{
	return g_settings_get_int (state, WINDOW_HEIGHT_KEY);
}

void rookie_settings_set_window_height (gint height)
{
	g_settings_set_int (state, WINDOW_HEIGHT_KEY, height);
}

gint rookie_settings_get_sidepane_width ()
{
	return g_settings_get_int (state, SIDEPANE_WIDTH_KEY);
}

void rookie_settings_set_sidepane_width (gint width)
{
	g_settings_set_int (state, SIDEPANE_WIDTH_KEY, width);
}

gint rookie_settings_get_bottompane_width ()
{
	return g_settings_get_int (state, BOTTOMPANE_WIDTH_KEY);
}

void rookie_settings_set_bottompane_width (gint width)
{
	g_settings_set_int (state, BOTTOMPANE_WIDTH_KEY, width);
}

gboolean rookie_settings_get_visible (RookieSetting setting)
{
	switch (setting) {

	case  ROOKIE_TOOLBAR_VISIBLE:
		return g_settings_get_boolean (general, TOOLBAR_VISIBLE_KEY);

	case ROOKIE_SIDEPANE_VISIBLE:
		return g_settings_get_boolean (general, SIDEPANE_VISIBLE_KEY);

	case ROOKIE_STATUSBAR_VISIBLE:
		return g_settings_get_boolean (general, STATUSBAR_VISIBLE_KEY);

	case ROOKIE_BOTTOMPANE_VISIBLE:
		return g_settings_get_boolean (general, BOTTOMPANE_VISIBLE_KEY);

	default:
		g_warn_if_reached ();
		return FALSE;
	}
}

void rookie_settings_set_visible (RookieSetting setting, gboolean visible)
{
	switch (setting) {

	case  ROOKIE_TOOLBAR_VISIBLE:
		g_settings_set_boolean (general, TOOLBAR_VISIBLE_KEY, visible);
		break;

	case ROOKIE_SIDEPANE_VISIBLE:
		g_settings_set_boolean (general, SIDEPANE_VISIBLE_KEY, visible);
		break;

	case ROOKIE_STATUSBAR_VISIBLE:
		g_settings_set_boolean (general, STATUSBAR_VISIBLE_KEY, visible);
		break;

	case ROOKIE_BOTTOMPANE_VISIBLE:
		g_settings_set_boolean (general, BOTTOMPANE_VISIBLE_KEY, visible);
		break;

	default:
		g_warn_if_reached ();
	}
}

void rookie_settings_bind (RookieSetting setting, gpointer object, const gchar *property)
{
	switch (setting) {

	case  ROOKIE_TOOLBAR_VISIBLE:
		g_settings_bind (general, TOOLBAR_VISIBLE_KEY, object, property, G_SETTINGS_BIND_DEFAULT);
		break;

	case ROOKIE_SIDEPANE_VISIBLE:
		g_settings_bind (general, SIDEPANE_VISIBLE_KEY, object, property, G_SETTINGS_BIND_DEFAULT);
		break;

	case ROOKIE_STATUSBAR_VISIBLE:
		g_settings_bind (general, STATUSBAR_VISIBLE_KEY, object, property, G_SETTINGS_BIND_DEFAULT);
		break;

	case ROOKIE_BOTTOMPANE_VISIBLE:
		g_settings_bind (general, BOTTOMPANE_VISIBLE_KEY, object, property, G_SETTINGS_BIND_DEFAULT);
		break;

	case ROOKIE_SIDEPANE_WIDTH:
		g_settings_bind (state, SIDEPANE_WIDTH_KEY, object, property, G_SETTINGS_BIND_DEFAULT);
		break;

	case ROOKIE_BOTTOMPANE_WIDTH:
		g_settings_bind (state, BOTTOMPANE_WIDTH_KEY, object, property, G_SETTINGS_BIND_DEFAULT);
		break;

	case  ROOKIE_START_MINIMIZED:
		g_settings_bind (general, START_MINIMIZED_KEY, object, property, G_SETTINGS_BIND_DEFAULT);
		break;

	case ROOKIE_START_NEW_DOWNLOADS:
		g_settings_bind (general, START_NEW_DOWNLOADS, object, property, G_SETTINGS_BIND_DEFAULT);
		break;

	default:
		g_warn_if_reached ();
	}
}

gboolean rookie_settings_get_start_minimized ()
{
	return g_settings_get_boolean (general, START_MINIMIZED_KEY);
}

void rookie_settings_set_start_minimzed (gboolean value)
{
	g_settings_set_boolean (general, START_MINIMIZED_KEY, value);
}

gboolean rookie_settings_get_start_new_downloads ()
{
	return g_settings_get_boolean (general, START_NEW_DOWNLOADS);
}

void rookie_settings_set_start_new_downloads (gboolean value)
{
	g_settings_set_boolean (general, START_NEW_DOWNLOADS, value);
}
