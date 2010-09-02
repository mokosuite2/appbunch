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

#include "wm.h"
#include "client.h"
#include "dialog_client.h"
#include "dock_client.h"
#include "splash_client.h"
#include "desktop_client.h"
#include "input_client.h"


Eina_Bool client_create_request(void* data, int type, void* event)
{
    // non gestire la finestra qui, potrebbe essere pericoloso
    Ecore_X_Event_Window_Create* e = event;
    g_debug("[%s] win=0x%x", __func__, e->win);

    wm_client* c = find_client(e->win);

    // maneggia! :D
    if (!c)
        manage_window(e->win);
    else
        g_warning("[%s] Client already managed! Eeeehh??? :-o", __func__);

    return EINA_TRUE;
}

Eina_Bool client_show_request(void* data, int type, void* event)
{
    Ecore_X_Event_Window_Show_Request *e = event;
    g_debug("[%s] win=0x%x", __func__, e->win);

    wm_client* c = find_client(e->win);
    // maneggia! :D
    if (!c)
        c = manage_window(e->win);

    c->visible = TRUE;
    ecore_x_window_show(c->win);
    raise_client(c);

    if (c == input_client)
        input_client_show(c);

    else if (c == dock_client)
        dock_client_show(c);

    return EINA_TRUE;
}

Eina_Bool client_visible(void* data, int type, void* event)
{
    Ecore_X_Event_Window_Visibility_Change* e = event;
    g_debug("[%s] win=0x%x", __func__, e->win);
    
    wm_client* c = find_client(e->win);
    // maneggia! :D
    if (!c)
        c = manage_window(e->win);

    c->visible = TRUE;
    raise_client(c);

    if (c == input_client)
        input_client_show(c);

    return EINA_TRUE;
}

Eina_Bool client_hide(void* data, int type, void* event)
{
    Ecore_X_Event_Window_Hide *e = event;
    g_debug("[%s] win=0x%x", __func__, e->win);

    wm_client* c = find_client(e->win);
    if (c != NULL) {
        c->visible = FALSE;

        if (c->type == ECORE_X_WINDOW_TYPE_DOCK)
            dock_client_hide(c);

        else if (c->type == ECORE_X_WINDOW_TYPE_UTILITY)
            input_client_hide(c);
    }

    raise_last(c);

    return EINA_TRUE;
}

Eina_Bool client_destroy(void* data, int type, void* event)
{
    Ecore_X_Event_Window_Destroy *e = event;
    g_debug("[%s] win=0x%x", __func__, e->win);

    wm_client* c = find_client(e->win);

    if (c != NULL) {
        if (c->type == ECORE_X_WINDOW_TYPE_DOCK)
            dock_client_destroy(c);

        else if (c->type == ECORE_X_WINDOW_TYPE_UTILITY)
            input_client_destroy(c);

        client_destroyed(c);
    }

    return EINA_TRUE;
}

Eina_Bool client_configure_request(void* data, int type, void* event)
{
    Ecore_X_Event_Window_Configure_Request *e = event;
    g_debug("[%s] x=%d, y=%d, w=%d, h=%d, border=%d, stack=%d, value_mask=%lu",
         __func__, e->x, e->y, e->w, e->h,
        e->border, e->detail, e->value_mask);

    wm_client* c = find_client(e->win);

    // maneggia! :D
    if (!c)
        c = manage_window(e->win);

    c->rect.x = e->x;
    c->rect.y = e->y;
    c->rect.w = e->w;
    c->rect.h = e->h;

    reconfigure_client(c);

    g_debug("[%s] Resizing window 0x%x to %dx%d and moving to %dx%d",
        __func__, e->win, c->rect.w, c->rect.h, c->rect.x, c->rect.y);

    #if 0
    ecore_x_window_configure(e->win, e->value_mask,
        c->rect.x, c->rect.y, c->rect.w, c->rect.h,
        e->border, e->abovewin, e->detail);

    // invia la notifica di configure effettuato
    ecore_x_icccm_move_resize_send(e->win,
        c->rect.x, c->rect.y, c->rect.w, c->rect.h);
    #else
    real_reconfigure_client(c);
    #endif
    

    if (c == dock_client)
        resize_all_clients();

    return EINA_TRUE;
}

Eina_Bool client_message(void* data, int type, void* event)
{
    Ecore_X_Event_Client_Message* e = event;
    g_debug("[%s] msg=%d, win=0x%x", __func__, e->message_type, e->win);
    /*
    g_debug("%d, %d, %d, %d, %d, %d, %d, %d, %d",
        ECORE_X_ATOM_NET_WM_STATE_DEMANDS_ATTENTION,
        ECORE_X_ATOM_NET_WM_STATE_ABOVE,
        ECORE_X_ATOM_NET_RESTACK_WINDOW,
        ECORE_X_ATOM_NET_ACTIVE_WINDOW,
        ECORE_X_ATOM_WM_TAKE_FOCUS,
        ECORE_X_ATOM_NET_WM_STATE_FULLSCREEN,
        ECORE_X_ATOM_NET_WM_WINDOW_TYPE,
        ECORE_X_ATOM_E_ILLUME_KEYBOARD_GEOMETRY,
        ECORE_X_ATOM_E_ILLUME_QUICKPANEL
    );
    */

    wm_client* c = find_client(e->win);
    g_return_val_if_fail(c != NULL, EINA_TRUE);

    // attivazione finestra
    if (e->message_type == ECORE_X_ATOM_NET_ACTIVE_WINDOW)
        raise_client(c);

    else if (e->message_type == ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_STATE)
        keyboard_state(e->data.l[0]);

    //else if (e->message_type == ECORE_X_ATOM_WM_STATE)

    return EINA_TRUE;
}

Eina_Bool client_property_change(void* data, int type, void* event)
{
    Ecore_X_Event_Window_Property* e = event;
    g_debug("[%s] win=0x%x, atom=%d", __func__, e->win, e->atom);
    /*
    g_debug("[%s] ECORE_X_ATOM_NET_WM_STATE=%d, ECORE_X_ATOM_WM_STATE=%d, ECORE_X_ATOM_NET_WM_STATE_FULLSCREEN=%d, "
        "ECORE_X_ATOM_NET_WM_ACTION_FULLSCREEN=%d, ECORE_X_ATOM_WINDOW=%d, ECORE_X_ATOM_WM_NORMAL_HINTS=%d", __func__,
        ECORE_X_ATOM_NET_WM_STATE,
        ECORE_X_ATOM_WM_STATE,
        ECORE_X_ATOM_NET_WM_STATE_FULLSCREEN,
        ECORE_X_ATOM_NET_WM_ACTION_FULLSCREEN,
        ECORE_X_ATOM_WINDOW, ECORE_X_ATOM_WM_NORMAL_HINTS);
    */

    wm_client* c = find_client(e->win);
    g_return_val_if_fail(c != NULL, EINA_TRUE);

    // cambio di tipo
    if (e->atom == ECORE_X_ATOM_NET_WM_WINDOW_TYPE) {
        int rc = ecore_x_netwm_window_type_get(c->win, &c->type);
        g_debug("[%s] New type (%d) = %d", __func__, rc, c->type);
        assign_client(c);

        // forse e' il caso di resettare tutto...
        resize_all_clients();
        reset_stack();
    }

    else if (e->atom == ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_STATE) {
        keyboard_state(ecore_x_e_virtual_keyboard_state_get(e->win));
    }

    return EINA_TRUE;
}

Eina_Bool client_state_request(void* data, int type, void* event)
{
    Ecore_X_Event_Window_State_Request* e = event;
    g_debug("[%s] win=0x%x, action=%d, source[0]=%d, source[1]=%d", __func__, e->win, e->action, e->state[0], e->state[1]);

    wm_client* c = find_client(e->win);
    g_return_val_if_fail(c != NULL, EINA_TRUE);

    if (e->state[0] == ECORE_X_WINDOW_STATE_FULLSCREEN) {
        g_debug("[%s] fullscreen state request for win 0x%x", __func__, e->win);

        if (e->action == ECORE_X_WINDOW_STATE_ACTION_REMOVE) {
            c->fullscreen = FALSE;
        }

        else if (e->action == ECORE_X_WINDOW_STATE_ACTION_ADD) {
            c->fullscreen = TRUE;
        }

        else if (e->action == ECORE_X_WINDOW_STATE_ACTION_TOGGLE) {
            c->fullscreen ^= c->fullscreen;
        }
    }

    resize_all_clients();
    reset_stack();
    return EINA_TRUE;
}


// resize guidato di un client -- riconfigura solo il rect
void reconfigure_client(wm_client *c)
{
    g_return_if_fail(c != NULL);

    if (c->type == ECORE_X_WINDOW_TYPE_DOCK) {
        dock_client_reconfigure(c);
    }

    else if (c->type == ECORE_X_WINDOW_TYPE_DIALOG) {
        dialog_client_reconfigure(c);
    }

    else if (c->type == ECORE_X_WINDOW_TYPE_SPLASH) {
        splash_client_reconfigure(c);
    }

    else if (c->type == ECORE_X_WINDOW_TYPE_DESKTOP) {
        desktop_client_reconfigure(c);
    }

    else if (c->type == ECORE_X_WINDOW_TYPE_UTILITY) {
        input_client_reconfigure(c);
    }

    else if (c->type == ECORE_X_WINDOW_TYPE_NORMAL) {
        ecore_x_window_geometry_get(root_win, &c->rect.x, NULL, &c->rect.w, &c->rect.h);

        // gestisci fullscreen
        if (!c->fullscreen) {
            c->rect.y = reserved_top;
            c->rect.h -= (reserved_top + reserved_bottom);
        }

        else {
            c->rect.y = c->rect.x = 0;
            ecore_x_window_geometry_get(root_win, NULL, NULL, &c->rect.w, &c->rect.h);
        }
    }

    // finestra sconosciuta -- posizionamento richiesto
    else {
        ecore_x_window_geometry_get(c->win, &c->rect.x, &c->rect.y, &c->rect.w, &c->rect.h);

        if (c->rect.y < reserved_top) c->rect.y += reserved_top;
        // TODO limite di altezza c->rect.h -= (reserved_top + reserved_bottom);
    }
}

// resize e reposition reale del client
void real_reconfigure_client(wm_client *c)
{
    g_return_if_fail(c != NULL);

    g_debug("[%s] Resizing window 0x%x to %dx%d and moving to %dx%d",
        __func__, c->win, c->rect.w, c->rect.h, c->rect.x, c->rect.y);

    ecore_x_window_move(c->win, c->rect.x, c->rect.y);
    ecore_x_window_resize(c->win, c->rect.w, c->rect.h);

    ecore_x_icccm_move_resize_send(c->win,
        c->rect.x, c->rect.y, c->rect.w, c->rect.h);
}
