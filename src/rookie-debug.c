/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * rookie-debug.c
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
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <time.h>
#include <glib.h>
#include <glib/gprintf.h> 

static void info_handler (const gchar *, GLogLevelFlags, const gchar *, gpointer);
static void debug_handler (const gchar *, GLogLevelFlags, const gchar *, gpointer);
static void message_handler (const gchar *, GLogLevelFlags, const gchar *, gpointer);
static void warning_handler (const gchar *, GLogLevelFlags, const gchar *, gpointer);
static void critical_handler (const gchar *, GLogLevelFlags, const gchar *, gpointer);

void rookie_debug_init ()
{
	g_log_set_handler (NULL, G_LOG_LEVEL_INFO, info_handler, NULL);
	g_log_set_handler (NULL, G_LOG_LEVEL_DEBUG, debug_handler, NULL); 
	g_log_set_handler (NULL, G_LOG_LEVEL_MESSAGE, message_handler, NULL); 
	g_log_set_handler (NULL, G_LOG_LEVEL_WARNING, warning_handler, NULL);
	g_log_set_handler (NULL, G_LOG_LEVEL_CRITICAL, critical_handler, NULL); 
}

static void _log (const gchar *prefix, const gchar *message)
{
	gchar buf[128];
	time_t now = time (NULL);

	struct tm * local_time = localtime (&now); 
	strftime (buf, 128, "%a %b %d %I:%M:%S %p %Y", local_time);
	g_fprintf (stdout, "%s [%s] %s\n", buf, prefix, message);
}

static void info_handler (const gchar			*log_domain,
						  GLogLevelFlags		log_level,
						  const gchar			*message,
						  gpointer				user_data)
{
	_log ("INFO", message);
}


static void debug_handler (const gchar			*log_domain,
						   GLogLevelFlags		log_level,
						   const gchar			*message,
						   gpointer				user_data)
{
	_log ("DBG", message);
}



static void message_handler (const gchar		*log_domain,
							 GLogLevelFlags		log_level,
							 const gchar		*message,
							 gpointer			user_data)
{
	_log ("MSG", message);
}


static void warning_handler (const gchar		*log_domain,
							 GLogLevelFlags		log_level,
							 const gchar		*message,
							 gpointer			user_data)
{
	_log ("WRN", message);
}



static void critical_handler (const gchar		*log_domain,
							  GLogLevelFlags	log_level,
							  const gchar		*message,
							  gpointer			user_data)
{
	_log ("CRT", message);
}
