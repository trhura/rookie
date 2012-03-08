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
#ifndef _ROOKIE_SETTINGS_H_
#define _ROOKIE_SETTINGS_H_

#include <glib.h>

typedef enum {
  ROOKIE_WINDOW_WIDTH,
  ROOKIE_WINDOW_HEIGHT, 
  ROOKIE_DROPZONE_WIDTH,
  ROOKIE_DROPZONE_HEIGHT,
  ROOKIE_DROPZONE_XPOS,
  ROOKIE_DROPZONE_YPOS,
  ROOKIE_SIDEPANE_WIDTH,
  ROOKIE_BOTTOMPANE_WIDTH,

  ROOKIE_TOOLBAR_VISIBLE,
  ROOKIE_SIDEPANE_VISIBLE,
  ROOKIE_DROPZONE_VISIBLE,
  ROOKIE_STATUSBAR_VISIBLE,
  ROOKIE_BOTTOMPANE_VISIBLE,

  ROOKIE_START_MINIMIZED,
  ROOKIE_START_NEW_DOWNLOADS
  
} RookieSetting;

void		 rookie_settings_init ();
void		 rookie_settings_finalize ();
GVariant*	 rookie_settings_get  (RookieSetting setting);
void		 rookie_settings_set  (RookieSetting setting, GVariant * value);

gint		 rookie_settings_get_window_width ();
void		 rookie_settings_set_window_width (gint width);
gint		 rookie_settings_get_window_height ();
void		 rookie_settings_set_window_height (gint height); 
gint		 rookie_settings_get_dropzone_width ();
void		 rookie_settings_set_dropzone_width (gint width);
gint		 rookie_settings_get_dropzone_height ();
void		 rookie_settings_set_dropzone_height (gint height);
gint		 rookie_settings_get_dropzone_xpos ();
void		 rookie_settings_set_dropzone_xpos (gint xpos);
gint		 rookie_settings_get_dropzone_ypos ();
void		 rookie_settings_set_dropzone_ypos (gint ypos);
gint		 rookie_settings_get_sidepane_width ();
void		 rookie_settings_set_sidepane_width (gint width);
gint		 rookie_settings_get_bottompane_width ();
void		 rookie_settings_set_bottompane_width (gint width);
gboolean	 rookie_settings_get_start_minimized ();

gboolean	 rookie_settings_get_visible (RookieSetting setting);
void		 rookie_settings_set_visible (RookieSetting setting, gboolean visible);
void		 rookie_settings_bind (RookieSetting setting, gpointer object, const gchar * property);

gboolean	 rookie_settings_get_start_minimized ();
void		 rookie_settings_set_start_minimzed (gboolean value);
gboolean	 rookie_settings_get_start_new_downloads ();
void		 rookie_settings_set_start_new_downloads (gboolean value);

#endif
