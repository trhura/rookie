/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * categories.h
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
#ifndef _CATEGORIES_H_
#define _CATEGORIES_H_

#include <glib.h>
#include "rookie-misc.h"

typedef struct _Category Category;

void categories_init ();
void categories_finalize ();
gint categories_get_category_count ();
void categories_foreach_category (GFunc func, gpointer data);
void categories_remove_category (Category *category);

Category* categories_append_category ();
Category* categories_get_category_by_url (const gchar *url);
Category* categories_get_category_by_name (const gchar *name);
Category* categories_get_category_by_id (gint id);

gint			category_get_id (Category *category);
gint			category_get_priority (Category *category);
gint			category_get_finish_action (Category *category);
gboolean		category_get_visible (Category *category);
const gchar*	category_get_name (Category *category);
const gchar*	category_get_pattern (Category *category);
const gchar*	category_get_backend (Category *category);
const gchar*	category_get_path (Category *category);

void category_set_name (Category *category, const gchar *name);
void category_set_path (Category *category, const gchar *path);
void category_set_priority (Category *category, gint priority);
void category_set_visible  (Category *category, gboolean visible);
void category_set_backend (Category *category, const gchar *backend);
void category_set_pattern  (Category *category, const gchar *pattern);
void category_set_finish_action (Category *category, gint finish_action);
void category_add_finish_action (Category *category, int action);
void category_remove_finish_action (Category *category, int action);

#endif
