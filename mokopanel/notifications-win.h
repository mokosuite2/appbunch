/*
 * Mokosuite
 * Miscellaneous panel notifications
 * Copyright (C) 2009-2010 Daniele Ricci <daniele.athome@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef __PANEL_NOTIFICATIONS_H
#define __PANEL_NOTIFICATIONS_H

#include "panel.h"

struct _MokoNotification {
    Evas_Object* list;
    Elm_Genlist_Item* item;
    Evas_Object* win;

    MokoPanel* panel;
    char* text;
    char* icon;
    int type;

    unsigned int count;
};

typedef struct _MokoNotification MokoNotification;

void notify_calls_init(MokoPanel* panel);

MokoNotification* notification_window_add(MokoPanel* panel, const char* text, const char* icon, int type);
void notification_window_remove(MokoNotification* n);

void notify_window_init(MokoPanel* panel);

void notify_window_start(void);
void notify_window_end(void);

void notify_window_show(void);
void notify_window_hide(void);

#endif  /* __PANEL_NOTIFICATIONS_H */
