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

#ifndef __MOKOWM_H
#define __MOKOWM_H

#include <Ecore_X.h>
#include <Ecore_Evas.h>

#include "client.h"

/* -- client particolari -- */

// client above (ce ne puo' essere solo uno)
extern wm_client* above_client;

// client desktop (ce ne puo' essere solo uno)
extern wm_client* desktop_client;

// client dock (ce ne puo' essere solo uno)
extern wm_client* dock_client;

// client input (ce ne puo' essere solo uno)
extern wm_client* input_client;

extern Ecore_X_Window root_win;

// spazi strut riservati
extern guint reserved_top;
extern guint reserved_bottom;

wm_client* find_client(Ecore_X_Window win);

void reset_stack(void);

void raise_client(wm_client* c);
void raise_last(wm_client* this);
void raise_window(Ecore_X_Window win);
void show_desktop(void);

void reconfigure_all_clients(void);
void resize_all_clients(void);

void keyboard_state(Ecore_X_Virtual_Keyboard_State state);
void client_destroyed(wm_client* c);

void set_state(wm_client* c, Ecore_X_Window_State state, gboolean value);
void assign_client(wm_client* c);

wm_client* manage_window(Ecore_X_Window win);

void manage_all_windows(void);
void wm_init(void);


#endif  /* __MOKOWM_H */
