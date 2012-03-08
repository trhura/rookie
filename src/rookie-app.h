/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * rookie
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

#ifndef _ROOKIE_APP_H_
#define _ROOKIE_APP_H_

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ROOKIE_TYPE_APP				(rookie_app_get_type ())
#define ROOKIE_APP(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), ROOKIE_TYPE_APP, RookieApp))
#define ROOKIE_APP_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), ROOKIE_TYPE_APP, RookieAppClass))
#define ROOKIE_IS_APP(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), ROOKIE_TYPE_APP))
#define ROOKIE_IS_APP_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), ROOKIE_TYPE_APP))
#define ROOKIE_APP_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), ROOKIE_TYPE_APP, RookieAppClass))

typedef struct _RookieAppClass RookieAppClass;
typedef struct _RookieApp RookieApp;

struct _RookieAppClass
{
	GtkApplicationClass parent_class;
};

struct _RookieApp
{
	GtkApplication parent_instance;
};

GType rookie_app_get_type (void) G_GNUC_CONST;

RookieApp*		rookie_app_new ();
int				rookie_app_run (RookieApp *ap, int argc, char **argv);

G_END_DECLS

#endif /* _ROOKIE_APP_H_ */
