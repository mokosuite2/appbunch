/*
 * Mokosuite
 * Window manager
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

#include <Ecore_X.h>
#include <Ecore_Input.h>
#include <Ecore.h>
#include <Eet.h>
#include <Eina.h>

#include <glib.h>
#include <glib-object.h>

#include "dialog_client.h"
#include "client.h"
#include "wm.h"

#define DIALOG_MIN_WIDTH        100
#define DIALOG_MIN_HEIGHT       80

void dialog_client_reconfigure(wm_client *c)
{
    int rw, rh;

    ecore_x_window_geometry_get(root_win, NULL, NULL, &rw, &rh);
    g_debug("rect.w = %d, rect.h = %d, RW = %d, RH = %d", c->rect.w, c->rect.h, rw, rh);

    rh -= (reserved_top + reserved_bottom);

    if (c->rect.w > rw) c->rect.w = rw;
    if (c->rect.h > rh) c->rect.h = rh;

    if (c->rect.w < DIALOG_MIN_WIDTH) c->rect.w = DIALOG_MIN_WIDTH;
    if (c->rect.h < DIALOG_MIN_HEIGHT) c->rect.h = DIALOG_MIN_HEIGHT;

    // centrare la finestra
    c->rect.x = (rw / 2) - (c->rect.w / 2);
    c->rect.y = (rh / 2) - (c->rect.h / 2) + reserved_top;
}
