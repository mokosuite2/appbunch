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

#include "input.h"
#include "wm.h"
#include "client.h"
#include "dialog_client.h"
#include "dock_client.h"
#include "splash_client.h"
#include "desktop_client.h"

Ecore_X_Window root_win = 0;

// client splash (ex above, ce ne puo' essere solo uno)
wm_client* splash_client = NULL;

// client desktop (ce ne puo' essere solo uno)
wm_client* desktop_client = NULL;

// client dock (ce ne puo' essere solo uno)
wm_client* dock_client = NULL;

// client input (ce ne puo' essere solo uno)
wm_client* input_client = NULL;

// coda di wm_client
static GQueue* windows = NULL;

// spazi strut riservati
guint reserved_top = 0;
guint reserved_bottom = 0;

// tasto di chiusura
#ifdef OPENMOKO
//#define CLOSE_KEY       "XF86Phone"
#define CLOSE_KEY       "F14"
#else
#define CLOSE_KEY       "F3"
#endif

// timer pressione close
static GTimer* close_timer = NULL;

#define DELAY_SHOW_DESKTOP      2

static Eina_Bool _ignore_renew_event(void* data, int type, void* event)
{
    return EINA_TRUE;
}

static Eina_Bool _mouse_down(void* data, int type, void* event)
{
    Ecore_Event_Mouse_Button* e = event;
    g_debug("[%s] buttons=%d, modifiers = %d, win=0x%x, cur=0x%x", __func__, e->buttons, e->modifiers, e->window, ecore_x_window_focus_get());

    // dai il focus
    Ecore_X_Window win = ecore_x_window_focus_get();
    wm_client *c = find_client(e->window);

    // tira su la finestra
    if (c != NULL && e->window != win)
        raise_client(c);

    return EINA_TRUE;
}

static Eina_Bool _key_down(void* data, int type, void* event)
{
    Ecore_Event_Key* e = event;
    g_debug("[%s] keyname=%s, key=%s, string=%s, modifiers = %d",
        __func__, e->keyname, e->key, e->string, e->modifiers);

    // chiudi finestra
    if (!strcasecmp(e->keyname, CLOSE_KEY)) {
        // fai partire il timer
        if (!close_timer) close_timer = g_timer_new();
        else g_timer_start(close_timer);
    }

    return EINA_TRUE;
}

static Eina_Bool _key_up(void* data, int type, void* event)
{
    Ecore_Event_Key* e = event;
    g_debug("[%s] keyname=%s, key=%s, string=%s, modifiers = %d",
        __func__, e->keyname, e->key, e->string, e->modifiers);

    // chiudi finestra
    if (!strcasecmp(e->keyname, CLOSE_KEY)) {
        if (close_timer)
            g_debug("[%s] elapsed: %f, desktop_client = %p",
                __func__, g_timer_elapsed(close_timer, NULL), desktop_client);

        // tempo passato! torna al desktop...
        if (close_timer && g_timer_elapsed(close_timer, NULL) >= DELAY_SHOW_DESKTOP && desktop_client) {
            show_desktop();
        }

        else {
            if (input_client && input_client->visible)
                input_win_hide();

            else
                // mandiamo il segnale di delete alla finestra che ha il focus, non alla finestra sotto il mouse!
                ecore_x_icccm_delete_window_send(ecore_x_window_focus_get(), e->timestamp);
        }
    }

    if (close_timer)
        // ferma tutto!
        g_timer_stop(close_timer);

    return EINA_TRUE;
}

static Eina_Bool _screen_changed(void *data, int type, void *event_info)
{
    static Ecore_X_Randr_Orientation previous_state = -1;
    Ecore_X_Event_Screen_Change* event = event_info;

    // gia' eseguito...
    //if (previous_state == event->rotation) return ECORE_CALLBACK_RENEW;

    g_debug("[%s] screen has changed to %dx%d, rotation %d", __func__, event->size.width, event->size.height, event->orientation);

    previous_state = event->orientation;

    // resize di tutte le finestre
    resize_all_clients();

    return EINA_TRUE;
}

// aggiorna lo stato della tastiera virtuale
void keyboard_state(Ecore_X_Virtual_Keyboard_State state)
{
    #if 0
    FIXME gestito da input method
    // disattiva keyboard
    if (state == ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_OFF || state == ECORE_X_VIRTUAL_KEYBOARD_STATE_UNKNOWN) {
        input_win_hide();
    }

    else {
        input_win_show();
    }

    // TODO altri tipi di tastiera?
    //else if (e->data.l[0] == ECORE_X_VIRTUAL_KEYBOARD_STATE_ON)
    #endif
}

void client_destroyed(wm_client* c)
{
    if (c == desktop_client)
        desktop_client = NULL;

    else if (c == dock_client)
        dock_client = NULL;

    else if (c == splash_client)
        splash_client = NULL;

    else if (c == input_client)
        input_client = NULL;

    g_queue_remove(windows, c);
    g_free(c);

    // tira su la finestra in cima alla pila
    raise_last(NULL);
}

// cerca il client data la finestra
wm_client* find_client(Ecore_X_Window win)
{
    GList* iter = windows->head;

    while (iter) {
        wm_client* c = iter->data;
        if (c->win == win) return c;

        iter = iter->next;
    }

    return NULL;
}

// tira su la finestra passata e le da il focus
void raise_window(Ecore_X_Window win)
{
    g_debug("[%s] Giving focus 0x%x (input_win=0x%x)", __func__, win, (input_client != NULL) ? input_client->win : NULL);
    ecore_x_window_focus(win);
    ecore_x_window_raise(win);

    ecore_x_events_allow_all();
}

void show_desktop(void)
{
    g_return_if_fail(desktop_client != NULL);

    g_debug("[%s] Raising desktop 0x%x (desktop_client=%p)",
        __func__, desktop_client->win, desktop_client);

    // porta il client al top della coda
    g_queue_remove(windows, desktop_client);
    g_queue_push_head(windows, desktop_client);

    // dai il focus alla finestra
    raise_window(desktop_client->win);

    // riorganizza lo stack
    reset_stack();
}

// tira su il client, rispettando l'organizzazione dello stack e modificando la coda delle finestre di conseguenza
void raise_client(wm_client* c)
{
    g_return_if_fail(c != NULL);

    g_debug("[%s] Raising window 0x%x (c=%p, desktop_client=%p)",
        __func__, c->win, c, desktop_client);

    if (c->visible && c != desktop_client && c != input_client && c != dock_client) {

        // porta il client al top della coda
        g_queue_remove(windows, c);
        g_queue_push_head(windows, c);

        // dai il focus alla finestra
        raise_window(c->win);
    }

    // riorganizza lo stack
    reset_stack();
}

void raise_last(wm_client* this)
{
    GList* iter = windows->head;

    while (iter) {
        wm_client* c = iter->data;
        if (this != c) {
            g_debug("[%s] raising window 0x%x", __func__, c->win);
            raise_client(c);
            break;
        }

        iter = iter->next;
    }
}

// riorganizza lo stack per tirare su le finestre privilegiate
void reset_stack(void)
{
    // l'input client deve avere solo la visibilita'
    if (input_client)
        ecore_x_window_raise(input_client->win);

    if (dock_client)
        ecore_x_window_raise(dock_client->win);

    if (splash_client)
        raise_window(splash_client->win);

    // cerca finestre a schermo intero
    GList* iter = windows->head;

    while (iter) {
        wm_client* c = iter->data;
        if (c->fullscreen) {
            g_debug("[%s] raising fullscreen window 0x%x", __func__, c->win);
            raise_window(c->win);
        }

        iter = iter->next;
    }
}


void set_state(wm_client* c, Ecore_X_Window_State state, gboolean value)
{
    g_return_if_fail(c != NULL);

    switch (state) {
        case ECORE_X_WINDOW_STATE_SKIP_TASKBAR:
            c->skip_taskbar = value;
            break;
        case ECORE_X_WINDOW_STATE_SKIP_PAGER:
            c->skip_pager = value;
            break;
        case ECORE_X_WINDOW_STATE_FULLSCREEN:
            c->fullscreen = value;
            break;

        default:
            g_debug("[%s] Not managing unknown state %d", __func__, state);
    }
}

void reconfigure_all_clients(void)
{
    GList* iter = windows->head;

    while (iter) {
        wm_client* c = iter->data;
        reconfigure_client(c);

        iter = iter->next;
    }
}

void resize_all_clients(void)
{
    GList* iter = windows->head;

    while (iter) {
        wm_client* c = iter->data;
        reconfigure_client(c);
        real_reconfigure_client(c);

        iter = iter->next;
    }
}

void assign_client(wm_client* c)
{
    if (c->type == ECORE_X_WINDOW_TYPE_DOCK) {
        dock_client = c;
    }

    else if (c->type == ECORE_X_WINDOW_TYPE_SPLASH) {
        splash_client = c;
    }

    else if (c->type == ECORE_X_WINDOW_TYPE_DESKTOP) {
        desktop_client = c;
    }

    else if (c->type == ECORE_X_WINDOW_TYPE_UTILITY) {
        input_client = c;
    }
}

// gestisce una finestra
wm_client* manage_window(Ecore_X_Window win)
{
    int rc, i;
    wm_client* c = NULL;
    g_debug("[%s] Managing window 0x%x", __func__, win);

    if ((c = find_client(win))) goto end;

    // crea un nuovo client
    c = g_new0(wm_client, 1);
    c->win = win;
    c->root = root_win;

    // nomi e titolo
    ecore_x_icccm_name_class_get(win, &c->name, &c->class);
    ecore_x_netwm_name_get(win, &c->title);
    g_debug("[%s] name = %s, class = %s, title = %s", __func__, c->name, c->class, c->title);

    // tipo
    rc = ecore_x_netwm_window_type_get(win, &c->type);
    g_debug("[%s] type_get(%d) = %d", __func__, rc, c->type);

    // stati
    Ecore_X_Window_State* state = NULL;
    unsigned int num = 0;
    rc = ecore_x_netwm_window_state_get(win, &state, &num);
    g_debug("[%s] state_num(%d) = %d", __func__, rc, num);

    for (i = 0; i < num; i++) {
        g_debug("[%s] state[%d] = %d", __func__, i, state[i]);
        set_state(c, state[i], TRUE);
    }

    rc = ecore_x_netwm_pid_get(win, &c->pid);
    g_debug("[%s] pid(%d) = %d", __func__, rc, c->pid);

    // geometria iniziale
    ecore_x_window_geometry_get(win, &c->rect.x, &c->rect.y, &c->rect.w, &c->rect.h);

    // pusha in coda perche' non sappiamo dove e' la finestra sullo stack
    g_queue_push_tail(windows, c);

    // evento mouse button per dare il focus
    ecore_x_window_button_grab(win, 1, ECORE_X_EVENT_MASK_MOUSE_DOWN, ECORE_X_EVENT_MASK_NONE, 1);
    ecore_x_window_button_grab(win, 3, ECORE_X_EVENT_MASK_MOUSE_DOWN, ECORE_X_EVENT_MASK_NONE, 1);

    // key grab :)
    ecore_x_window_key_grab(win, CLOSE_KEY, ECORE_X_EVENT_MASK_NONE, 1);

    // FIXME devo capire come funziona questo...
    ecore_x_event_mask_set(win, ECORE_X_EVENT_MASK_WINDOW_PROPERTY);

    // assegna per tipo
    assign_client(c);

    // alle finestre normali e' permesso il fullscreen
    if (c->type == ECORE_X_WINDOW_TYPE_NORMAL) {
        Ecore_X_Atom supp[] = {
            ECORE_X_ATOM_NET_WM_ACTION_FULLSCREEN
        };
        ecore_x_window_prop_atom_set(win, ECORE_X_ATOM_NET_WM_ALLOWED_ACTIONS, supp, 1);
    }

    // prima configurazione
    reconfigure_client(c);
    real_reconfigure_client(c);

    // TODO proprieta' iniziali?
    keyboard_state(ecore_x_e_virtual_keyboard_state_get(win));

end:
    return c;
}

void manage_all_windows(void)
{
    // gestisci tutte le finestre
    int num = 0, i;
    wm_client* c;
    Ecore_X_Window* list = ecore_x_window_children_get(root_win, &num);
    for (i = 0; i < num; i++) {
        c = manage_window(list[i]);
    }
    g_free(list);

    resize_all_clients();
    reset_stack();
}

void wm_init(void)
{
    // coda delle finestre
    windows = g_queue_new();

    // inizializza wm
    root_win = ecore_x_window_root_first_get();

    if (!ecore_x_window_manage(root_win)) {
        g_error("Unable to manage root window. Exiting.");
        return;
    }

    // quali proprieta' supportiamo? :)
    Ecore_X_Atom supp[] = {
        ECORE_X_ATOM_NET_WM_STATE,
        ECORE_X_ATOM_NET_WM_STATE_FULLSCREEN,
        ECORE_X_ATOM_NET_WM_STATE_SKIP_PAGER,
        ECORE_X_ATOM_NET_WM_STATE_SKIP_TASKBAR
    };
    ecore_x_window_prop_atom_set(root_win, ECORE_X_ATOM_NET_SUPPORTED, supp, 4);

    // key grab :)
    ecore_x_window_key_grab(root_win, CLOSE_KEY, ECORE_X_EVENT_MASK_NONE, 1);

    // passive grab
    ecore_x_passive_grab_replay_func_set(_ignore_renew_event, NULL);

    // eventi gestiti dal modulo client
    // FIXME crea solo rogne -- ecore_event_handler_add(ECORE_X_EVENT_WINDOW_CREATE, client_create_request, NULL);
    ecore_event_handler_add(ECORE_X_EVENT_WINDOW_VISIBILITY_CHANGE, client_visible, NULL);
    ecore_event_handler_add(ECORE_X_EVENT_WINDOW_SHOW_REQUEST, client_show_request, NULL);
    ecore_event_handler_add(ECORE_X_EVENT_WINDOW_HIDE, client_hide, NULL);
    ecore_event_handler_add(ECORE_X_EVENT_WINDOW_DESTROY, client_destroy, NULL);
    ecore_event_handler_add(ECORE_X_EVENT_WINDOW_CONFIGURE_REQUEST, client_configure_request, NULL);
    ecore_event_handler_add(ECORE_X_EVENT_CLIENT_MESSAGE, client_message, NULL);
    ecore_event_handler_add(ECORE_X_EVENT_WINDOW_PROPERTY, client_property_change, NULL);
    ecore_event_handler_add(ECORE_X_EVENT_WINDOW_STATE_REQUEST, client_state_request, NULL);

    // eventi gestiti dal modulo wm
    ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_DOWN, _mouse_down, NULL);
    ecore_event_handler_add(ECORE_X_EVENT_WINDOW_CONFIGURE, _ignore_renew_event, NULL);
    ecore_event_handler_add(ECORE_EVENT_KEY_DOWN, _key_down, NULL);
    ecore_event_handler_add(ECORE_EVENT_KEY_UP, _key_up, NULL);

    //ecore_event_handler_add(ECORE_X_EVENT_WINDOW_STATE_REQUEST, _state_request, NULL);
    //ecore_event_handler_add(ECORE_X_EVENT_PING, _ping, NULL);

    // eventi rotazione
    ecore_x_randr_events_select(root_win, TRUE);
    ecore_event_handler_add(ECORE_X_EVENT_SCREEN_CHANGE, _screen_changed, NULL);
}
