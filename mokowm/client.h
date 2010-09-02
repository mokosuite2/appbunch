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

#ifndef __MOKOWM_CLIENTS_H
#define __MOKOWM_CLIENTS_H

#include <Ecore_X.h>
#include <Ecore_Evas.h>

/* client structure */
typedef struct {
    Ecore_X_Window win;
    Ecore_X_Window parent;
    Ecore_X_Window root;

    Ecore_X_Window_Type type;

    /* [_NET_]WM_CLASS */
    char* name;
    char* class;

    /* [_NET_]WM_NAME */
    char* title;

    gboolean fullscreen;
    gboolean skip_pager;
    gboolean skip_taskbar;

    gboolean visible;

    struct {
        int x;
        int y;
        int w;
        int h;
    } rect;

    pid_t pid;

} wm_client;


Eina_Bool client_create_request(void* data, int type, void* event);
Eina_Bool client_show_request(void* data, int type, void* event);
Eina_Bool client_visible(void* data, int type, void* event);
Eina_Bool client_hide(void* data, int type, void* event);
Eina_Bool client_destroy(void* data, int type, void* event);
Eina_Bool client_configure_request(void* data, int type, void* event);
Eina_Bool client_message(void* data, int type, void* event);
Eina_Bool client_property_change(void* data, int type, void* event);
Eina_Bool client_state_request(void* data, int type, void* event);

void reconfigure_client(wm_client *c);
void real_reconfigure_client(wm_client *c);


#endif  /* __MOKO_INPUT_H */
