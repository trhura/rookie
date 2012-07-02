/*
 * g-downloadable-activatable.h
 * This file is part of gedit
 *
 * Copyright (C) 2010 - Steve Frécinaux
 * Copyright (C) 2010 - Jesse van den Kieboom
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

#ifndef __G_DOWNLOADABLE_ACTIVATABLE_H__
#define __G_DOWNLOADABLE_ACTIVATABLE_H__

#include <glib-object.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define G_DOWNLOADABLE_TYPE_ACTIVATABLE		(g_downloadable_activatable_get_type ())
#define G_DOWNLOADABLE_ACTIVATABLE(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), G_DOWNLOADABLE_TYPE_ACTIVATABLE, GDownloadableActivatable))
#define G_DOWNLOADABLE_ACTIVATABLE_IFACE(obj)	(G_TYPE_CHECK_CLASS_CAST ((obj), G_DOWNLOADABLE_TYPE_ACTIVATABLE, GDownloadableActivatableInterface))
#define G_DOWNLOADABLE_IS_ACTIVATABLE(obj)	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), G_DOWNLOADABLE_TYPE_ACTIVATABLE))
#define G_DOWNLOADABLE_ACTIVATABLE_GET_IFACE(obj)	(G_TYPE_INSTANCE_GET_INTERFACE ((obj), G_DOWNLOADABLE_TYPE_ACTIVATABLE, GDownloadableActivatableInterface))

typedef struct _GDownloadableActivatable           GDownloadableActivatable; /* dummy typedef */
typedef struct _GDownloadableActivatableInterface  GDownloadableActivatableInterface;

struct _GDownloadableActivatableInterface
{
	GTypeInterface g_iface;

	/* Virtual public methods */
	void	(*activate)		(GDownloadableActivatable *activatable);
	void	(*deactivate)		(GDownloadableActivatable *activatable);
};

/*
 * Public methods
 */
GType	 g_downloadable_activatable_get_type	(void)  G_GNUC_CONST;

void	 g_downloadable_activatable_activate	(GDownloadableActivatable *activatable);
void	 g_downloadable_activatable_deactivate	(GDownloadableActivatable *activatable);

G_END_DECLS

#endif /* __G_DOWNLOADABLE_ACTIVATABLE_H__ */
