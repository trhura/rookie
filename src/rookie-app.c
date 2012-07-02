/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * rookie-app.c
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

#include <notify.h>
#include <config.h>
#include <libpeas/peas.h>

#include "categories.h"
#include "rookie-app.h"
#include "rookie-services.h"
#include "rookie-settings.h"
#include "main-window.h"
#include "g-downloadable-backends.h"

G_DEFINE_TYPE (RookieApp, rookie_app, GTK_TYPE_APPLICATION);

/* Signal Callbacks */
static int on_command_line (GApplication *application,
							GApplicationCommandLine *cmdline);

static void
rookie_app_init (RookieApp *object)
{
	g_signal_connect (object, "command-line",
					  G_CALLBACK(on_command_line), NULL);
}

static void
rookie_app_finalize (GObject *object)
{
	G_OBJECT_CLASS (rookie_app_parent_class)->finalize (object);
}

static void
rookie_app_class_init (RookieAppClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
//	GApplicationClass* parent_class = G_APPLICATION_CLASS (klass);
	object_class->finalize = rookie_app_finalize;
}

RookieApp *
rookie_app_new ()
{
	g_type_init ();
	return g_object_new (ROOKIE_TYPE_APP,
						 "application-id", "net.launchpad.rookie",
						 "flags", G_APPLICATION_HANDLES_COMMAND_LINE,
						 NULL);
}

int
rookie_app_run (RookieApp *application,
				int argc,
				char **argv)
{
	int status = g_application_run (G_APPLICATION(application), argc, argv);

	if (!g_application_get_is_registered (G_APPLICATION(application))) {
		g_warning ("Unable to register application.");
		return 1;
	}

	if (g_application_get_is_remote (G_APPLICATION(application))) {
		g_warning ("Another application instance is already running.");
		return 1;
	}

	g_downloadable_backends_init (); /* Must be before categories_init */
	rookie_settings_init ();
	categories_init ();

	PeasEngine* engine = peas_engine_get_default ();

	gchar* plugin_dir  = rookie_misc_get_plugins_dir ();
	peas_engine_add_search_path (engine, plugin_dir, plugin_dir);
	g_free (plugin_dir);

	plugin_dir  = g_build_filename (PACKAGE_DATA_DIR, "rookie", "plugins", NULL);
	peas_engine_add_search_path (engine, plugin_dir, plugin_dir);
	g_free (plugin_dir);

	peas_engine_enable_loader (engine, "gjs");
	peas_engine_enable_loader (engine, "python");
	peas_engine_enable_loader (engine, "seed");

	create_main_window ();

	if (!rookie_settings_get_start_minimized ())
		show_main_window ();

	notify_init (PACKAGE_NAME);
	gtk_main ();

	return status;
}

static int
on_command_line (GApplication *application,
				 GApplicationCommandLine *cmdline)
{
  gchar **argv;
  gint argc;
  gint i;

  argv = g_application_command_line_get_arguments (cmdline, &argc);

  for (i = 0; i < argc; i++)
	  g_print ("handling argument %s remotely\n", argv[i]);

  g_strfreev (argv);
  return 0;
}
