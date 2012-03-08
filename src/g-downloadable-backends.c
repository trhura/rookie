/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * g-downloadable-backends.c
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

#include <glib.h>
#include "gio-download.h"
#include "soup-download.h"
#include "g-downloadable-backends.h"

#define GIO_NAME		"gio-backend"
#define SOUP_NAME		"soup-backend"

static GDownloadableBackend * gio_backend;
static GDownloadableBackend * soup_backend;

GDownloadableBackend * g_downloadable_backend_new (const gchar * name, GType type)
{
	static gint id = 0;
	
	GDownloadableBackend * backend = (GDownloadableBackend*) g_malloc (sizeof (GDownloadableBackend));
	backend->name = g_strdup(name);
	backend->type = type;
	backend->id	  = id++;
	
	return backend;
}

void g_downloadable_backend_destroy (GDownloadableBackend *backend)
{
	g_free (backend->name);
	g_free (backend);
}

void g_downloadable_backends_init ()
{
	gio_backend   = g_downloadable_backend_new (GIO_NAME, GIO_TYPE_DOWNLOAD);
	soup_backend  = g_downloadable_backend_new (SOUP_NAME, SOUP_TYPE_DOWNLOAD);
}

void g_downloadable_backends_finalize ()
{
	g_downloadable_backend_destroy (gio_backend);
	g_downloadable_backend_destroy (soup_backend);
}

GDownloadableBackend * g_downloadable_backends_get_default ()
{
	//return gio_backend;
	return gio_backend;
}

GDownloadableBackend * g_downloadable_backends_by_name (const gchar * name)
{
	if (g_strcmp0 (GIO_NAME, name) == 0) 		
		return gio_backend;

	if (g_strcmp0 (SOUP_NAME, name) == 0) 		
		return soup_backend;

	g_warn_if_reached ();
	return NULL;
} 

void g_downloadable_backends_foreach  (GFunc func, gpointer user_data)
{
	func (gio_backend, user_data);
	func (soup_backend, user_data);
}
