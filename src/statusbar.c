/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * statusbar.c
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
#include "statusbar.h"
#include "g-downloadable.h"
#include "g-download-list.h"

static GtkWidget* statlabel;

GtkWidget* create_statusbar ()
{
	GtkWidget* statbar = gtk_statusbar_new ();

	statlabel = gtk_label_new ("");
	gtk_box_pack_end (GTK_BOX(statbar), statlabel, FALSE, FALSE, 5);

	refresh_statlabel ();
	return statbar;
}


static void
count_downloads_by_status (GDownloadable *download,
						   gint *download_count_by_status)
{
	gint status	= g_downloadable_get_status (download);
	download_count_by_status[status]++;
	download_count_by_status[G_DOWNLOADABLE_TOTAL_STATUS]++;
}

void refresh_statlabel ()
{

	gint download_count_by_status[G_DOWNLOADABLE_TOTAL_STATUS+1] = { 0 };
	g_download_list_foreach ((GFunc) count_downloads_by_status, download_count_by_status);

	gint active  = download_count_by_status[G_DOWNLOADABLE_DOWNLOADING];
	gint done	 = download_count_by_status[G_DOWNLOADABLE_COMPLETED];
	gint all	 = download_count_by_status[G_DOWNLOADABLE_TOTAL_STATUS];
	gint left	 = all - done;

	gchar *string	  = g_strdup_printf ("%d left of %d downloads, %d running", left, all, active);
	gtk_label_set_markup (GTK_LABEL(statlabel), string);
}
