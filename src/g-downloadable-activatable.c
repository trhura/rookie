/*
 * gedit-app-activatable.h
 * This file is part of gedit
 *
 * Copyright (C) 2010 Steve Frécinaux
 * Copyright (C) 2010 Jesse van den Kieboom
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "g-downloadable-activatable.h"
#include "g-downloadable.h"


G_DEFINE_INTERFACE(GDownloadableActivatable, g_downloadable_activatable, G_TYPE_OBJECT)

void
g_downloadable_activatable_default_init (GDownloadableActivatableInterface *iface)
{
	static gboolean initialized = FALSE;

	if (!initialized)
	{
		g_object_interface_install_property (iface,
		                                     g_param_spec_object ("downloadable",
		                                                          "Downloadable",
		                                                          "A GDownloadable",
		                                                          G_TYPE_DOWNLOADABLE,
		                                                          G_PARAM_READWRITE |
		                                                          G_PARAM_CONSTRUCT_ONLY |
		                                                          G_PARAM_STATIC_STRINGS));

		initialized = TRUE;
	}
}

void
g_downloadable_activatable_activate (GDownloadableActivatable *activatable)
{
	GDownloadableActivatableInterface *iface;
	g_return_if_fail (G_DOWNLOADABLE_IS_ACTIVATABLE (activatable));

	iface = G_DOWNLOADABLE_ACTIVATABLE_GET_IFACE (activatable);

	if (iface->activate != NULL)
	{
		iface->activate (activatable);
	}
}

void
g_downloadable_activatable_deactivate (GDownloadableActivatable *activatable)
{
	GDownloadableActivatableInterface *iface;
	g_return_if_fail (G_DOWNLOADABLE_IS_ACTIVATABLE (activatable));

	iface = G_DOWNLOADABLE_ACTIVATABLE_GET_IFACE (activatable);

	if (iface->deactivate != NULL)
	{
		iface->deactivate (activatable);
	}
}


