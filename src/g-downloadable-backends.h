/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * g-downloadable-backends.h
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

#ifndef _G_DOWNLOADABLE_BACKENDS_H_
#define _G_DOWNLOADABLE_BACKENDS_H_

#include <glib.h>
#include <glib-object.h>

struct _GDownloadableBackend {
	gint  id;
	GType type;
	gchar *name;
};

typedef struct _GDownloadableBackend GDownloadableBackend;

void g_downloadable_backends_init (); 
void g_downloadable_backends_finalize ();
void g_downloadable_backends_foreach  (GFunc func, gpointer user_data);

GDownloadableBackend * g_downloadable_backends_get_default (); 
GDownloadableBackend * g_downloadable_backends_by_name (const gchar * name);

#endif 
