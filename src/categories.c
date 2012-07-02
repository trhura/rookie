/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * categories.c
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

#include "categories.h"
#include "rookie-misc.h"
#include "rookie-debug.h"
#include "g-downloadable-backends.h"

struct _Category {
	gchar *name;
	gchar *pattern;
	gchar *backend;
	gchar *path;
	gint   priority;
	gint   id;
	gint   finish_action;
	gboolean visible;
	GSequenceIter *iter;
};

Category * category_new (const gchar *name);
void category_free (Category * );

static GSequence	*sequence;

static void load_categories ()
{
	GError *error = NULL;
	GKeyFile *keyfile = g_key_file_new ();
	gchar  *cpath = rookie_misc_get_category_file_path ();

	g_key_file_load_from_file (keyfile, cpath, G_KEY_FILE_NONE, &error);
	handle_critical_error (error);

	gsize group_count, i = 0;
	gchar **groups = g_key_file_get_groups (keyfile, &group_count);

	gchar *pattern, *path, *backend;
	gboolean visible;
	gint priority, finish_action;

	while (i < group_count ) {

		pattern	= g_key_file_get_string (keyfile, groups[i], "pattern", &error);
		handle_error (error);

		backend	= g_key_file_get_string (keyfile, groups[i], "backend", &error);
		handle_error (error);

		path	= g_key_file_get_string (keyfile, groups[i], "path", &error);
		handle_error (error);

		priority	= g_key_file_get_integer (keyfile, groups[i], "priority", &error);
		handle_error (error);

		visible	= g_key_file_get_boolean (keyfile, groups[i], "visible", &error);
		handle_error (error);

		finish_action = g_key_file_get_integer (keyfile, groups[i], "finish-action", &error);
		handle_error (error);

		Category* category  = category_new (groups[i]);

		category->id = i;
		category->priority = priority;
		category->visible  = visible;
		category->pattern  = (pattern == NULL) ? g_strdup ("*") : pattern;
		category->backend  = (backend == NULL) ? g_strdup (g_downloadable_backends_get_default ()->name) : backend;
		category->path	   = (path == NULL) ? g_strdup (g_getenv ("HOME")) : path;
		category->iter	   = g_sequence_append (sequence, category);
		category->finish_action = finish_action;

		i++;
	}

	g_free (cpath);
	g_strfreev (groups);
	g_key_file_free (keyfile);
}

static void
save_each_category (Category * category, GKeyFile *keyfile)
{
	g_key_file_set_string  (keyfile, category->name, "path", category->path);
	g_key_file_set_string  (keyfile, category->name, "pattern", category->pattern);
	g_key_file_set_string  (keyfile, category->name, "backend", category->backend);
	g_key_file_set_boolean (keyfile, category->name, "visible", category->visible);
	g_key_file_set_integer (keyfile, category->name, "priority", category->priority);
	g_key_file_set_integer (keyfile, category->name, "finish-action", category->finish_action);
}

void save_categories ()
{
	GError *error = NULL;
	gsize length;

	GKeyFile *keyfile  = g_key_file_new ();
	categories_foreach_category ((GFunc) save_each_category, keyfile);

	gchar *file	  = rookie_misc_get_category_file_path ();
	gchar *content = g_key_file_to_data (keyfile, &length, &error);
	handle_error (error);

	g_file_set_contents (file, content, length, &error);
	handle_error (error);

	g_free (file);
	g_free (content);
	g_key_file_free (keyfile);
}

void categories_foreach_category (GFunc func, gpointer data)
{
	GSequenceIter *iter = g_sequence_get_begin_iter (sequence);

	while (!g_sequence_iter_is_end (iter)) {
		Category *category = (Category*) g_sequence_get (iter);
		func (category, data);
		iter = g_sequence_iter_next (iter);
	}
}

void categories_init ()
{
	sequence = g_sequence_new ((GDestroyNotify)category_free);
	load_categories ();
}

void categories_finalize ()
{
	g_assert (sequence != NULL);

	save_categories ();
	g_sequence_free (sequence);
}

Category* category_new (const gchar *name)
{
	Category *category = (Category*) g_malloc (sizeof (Category));
	category->name	= g_strdup(name);

	return category;
}

void category_free (Category *category)
{
	g_assert (category != NULL);

	g_free (category->name);
	g_free (category->path);
	g_free (category->pattern);
	g_free (category->backend);
	g_free (category);
}

gint categories_get_category_count ()
{
	g_assert (sequence != NULL);
	return g_sequence_get_length (sequence);
}

void categories_remove_category (Category *category)
{
	g_assert (sequence != NULL);
	g_assert (category != NULL);
	g_sequence_remove (category->iter);
}

Category* categories_append_category ()
{
	g_assert (sequence != NULL);

	Category *category = category_new ("Untitled");
	category->visible = TRUE;
	category->pattern = g_strdup("*");
	category->path	  = g_strdup(g_getenv("HOME"));
	category->iter	  = g_sequence_append (sequence, category);
	category->backend = g_strdup (g_downloadable_backends_get_default ()->name);

	return category;
}

Category* categories_get_category_by_url (const gchar *url)
{
	g_assert (sequence != NULL);

	GSequenceIter *iter = g_sequence_get_begin_iter (sequence);
	while (!g_sequence_iter_is_end (iter)) {
		Category *category = (Category*) g_sequence_get (iter);

		if (g_regex_match_simple (category->pattern, url, G_REGEX_CASELESS | G_REGEX_RAW, 0))
			return category;

		iter = g_sequence_iter_next (iter);
	}

	return NULL;
}


Category* categories_get_category_by_name (const gchar *name)
{
	g_assert (sequence != NULL);

	GSequenceIter *iter = g_sequence_get_begin_iter (sequence);
	while (!g_sequence_iter_is_end (iter)) {
		Category *category = (Category*) g_sequence_get (iter);

		if(g_strcmp0 (name, category->name) == 0)
			return category;

		iter = g_sequence_iter_next (iter);
	}

	return NULL;
}


Category* categories_get_category_by_id (gint id)
{
	g_assert (sequence != NULL);

	GSequenceIter *iter = g_sequence_get_begin_iter (sequence);
	while (!g_sequence_iter_is_end (iter)) {
		Category *category = (Category*) g_sequence_get (iter);

		if(id == category->id)
			return category;

		iter = g_sequence_iter_next (iter);
	}

	g_warn_if_reached ();
	return NULL;
}

gint category_get_priority (Category *category)
{
	g_assert (category != NULL);
	return category->priority;
}

gint category_get_id (Category *category)
{
	g_assert (category != NULL);
	return category->id;
}

gint category_get_finish_action (Category *category)
{
	g_assert (category != NULL);
	return category->finish_action;
}

gboolean category_get_visible (Category *category)
{
	g_assert (category != NULL);
	return category->visible;
}

const gchar* category_get_name (Category *category)
{
	g_assert (category != NULL);
	return category->name;
}

const gchar* category_get_pattern (Category *category)
{
	g_assert (category != NULL);
	return category->pattern;
}

const gchar* category_get_backend (Category *category)
{
	g_assert (category != NULL);
	return category->backend;
}

const gchar* category_get_path (Category *category)
{
	g_assert (category != NULL);
	return category->path;
}

void category_set_name (Category *category, const gchar *name)
{
	g_assert (category != NULL);
	g_free (category->name);
	category->name = g_strdup (name);
}

void category_set_path (Category *category, const gchar *path)
{
	g_assert (category != NULL);
	g_free (category->path);
	category->path = g_strdup (path);
}

void category_set_priority (Category *category, gint priority)
{
	g_assert (category != NULL);
	category->priority = priority;
}

void category_set_backend (Category *category, const gchar *backend)
{
	g_assert (category != NULL);
	g_free (category->backend);
	category->backend = g_strdup (backend);
}

void category_set_visible  (Category *category, gboolean visible)
{
	g_assert (category != NULL);
	category->visible = visible;
}

void category_set_pattern  (Category *category, const gchar *pattern)
{
	g_assert (category != NULL);
	g_free (category->pattern);
	category->pattern = g_strdup (pattern);
}

void category_set_finish_action (Category *category, gint finish_action)
{
	g_assert (category != NULL);
	category->finish_action = finish_action;
}

void category_add_finish_action (Category *category,
								 gint action)
{
	category->finish_action |= action;
}

void category_remove_finish_action (Category *category,
									gint action)
{
	category->finish_action &= ~action;
}
