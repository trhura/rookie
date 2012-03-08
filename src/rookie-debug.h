/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * rookie-debug.h
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

#define rookie_debug(message, ...)										\
	do {																\
		g_debug ("%s (%s)" message, G_STRLOC, G_STRFUNC, ## __VA_ARGS__); \
	} while (0)

#define handle_error(error)							\
	do {											\
		if (error) {								\
			rookie_debug ("%s", error->message);	\
			g_clear_error (&error);					\
		}											\
	} while (0)

#define handle_critical_error(error)				\
	do {											\
		if (error) {								\
			rookie_debug ("%s", error->message);	\
			g_clear_error (&error);					\
		}											\
	} while (0)

void rookie_debug_init (void); 
