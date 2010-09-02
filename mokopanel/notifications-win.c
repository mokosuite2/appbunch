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

#include <Elementary.h>
#include <Ecore_X.h>
#include <Ecore_X_Atoms.h>
#include <glib.h>
#include <libmokosuite/mokosuite.h>
#include <libmokosuite/settings-service.h>
#include <libmokosuite/fso.h>
#include <libmokosuite/misc.h>
#include <libmokosuite/notifications.h>
#include <frameworkd-glib/odeviced/frameworkd-glib-odeviced-dbus.h>
#include <frameworkd-glib/odeviced/frameworkd-glib-odeviced-idlenotifier.h>
#include <frameworkd-glib/ogsmd/frameworkd-glib-ogsmd-dbus.h>
#include <frameworkd-glib/ogsmd/frameworkd-glib-ogsmd-call.h>

#include "panel.h"
#include "idle.h"
#include "notifications-win.h"

#include <glib/gi18n-lib.h>

static MokoPanel* current_panel = NULL;

// TODO converti in bitmap :)
static GArray* calls = NULL;

static Evas_Object* notification_win = NULL;
static Evas_Object* notification_list = NULL;
static Elm_Genlist_Item_Class itc = {0};
static gboolean notification_show = FALSE;

extern MokoSettingsService* panel_settings;

// label notifiche
const char* notification_labels[][2] = {
    { "%d active call", "%d active calls" },
    { "%d missed call", "%d missed calls" },
    { "%d new message", "%d new messages" },
    { "%d unread USSD response", "%d unread USSD responses" }
};

// comandi notifiche
const char* notification_commands[] = {
    "/home/root/tmp/phone-activate.sh Frontend string:calls",
    "/home/root/tmp/phone-activate.sh Frontend string:log",
    "/usr/bin/phoneui-messages",
    "/home/root/tmp/phone-activate.sh"
};

// ListItem notifiche
Elm_Genlist_Item* notification_items[] = {
    NULL,
    NULL,
    NULL,
    NULL
};

static void call_status(gpointer data, const int id, const int status, GHashTable *props)
{
    g_debug("CallStatus[%d]=%d", id, status);
    int i;
    gboolean call_exists = FALSE;

    switch (status) {
        case CALL_STATUS_INCOMING:
        case CALL_STATUS_OUTGOING:
            // controlla se la chiamata e' gia presente
            for (i = 0; i < calls->len; i++) {
                // chiamata gia' aggiunta, esci
                if (g_array_index(calls, int, i) == id) {
                    call_exists = TRUE;
                    goto end;
                }
            }

            // aggiungi la chiamata
            g_array_append_val(calls, id);

        end:
            if (status == CALL_STATUS_INCOMING) {
                // disattiva idlescreen e screensaver
                screensaver_off();

                // richiedi display
                ousaged_request_resource("Display", NULL, NULL);
            }

            // disabilita idle screen
            idle_hide();
            break;

        case CALL_STATUS_ACTIVE:
            ousaged_release_resource("Display", NULL, NULL);

            break;

        case CALL_STATUS_RELEASE:
            // togli la chiamata dalla lista se presente
            for (i = 0; i < calls->len; i++) {
                if (g_array_index(calls, int, i) == id) {
                    g_array_remove_index_fast(calls, i);
                    break;
                }
            }

            ousaged_release_resource("Display", NULL, NULL);

            if (!calls->len) {
                // ultima chiamata chiusa, togli screensaver
                screensaver_off();
                idle_hide();
            }

            break;

        case CALL_STATUS_HELD:
            g_debug("Held call!");
            // tanto non funziona... :(
            break;
    }

}

#if 0
static int handle_call_status(const char* status)
{
    int st;

    if (!strcmp(status, DBUS_CALL_STATUS_INCOMING)) {
        st = CALL_STATUS_INCOMING;
    }
    else if (!strcmp(status, DBUS_CALL_STATUS_OUTGOING)) {
        // Display outgoing UI
    }
    else if (!strcmp(status, DBUS_CALL_STATUS_ACTIVE)) {
        st = CALL_STATUS_ACTIVE;
    }
    else if (!strcmp(status, DBUS_CALL_STATUS_RELEASE)) {
        st = CALL_STATUS_RELEASE;
    }
    else {
        st = CALL_STATUS_HELD;
    }

    return st;
}

static void list_calls(GError *e, GPtrArray * calls, gpointer data)
{
    g_return_if_fail(e == NULL);

    int i;

    for (i = 0; i < calls->len; i++) {
        GHashTable* props = (GHashTable *) g_ptr_array_index(calls, i);

        call_status(fso_get_attribute_int(props, "id"),
            handle_call_status(fso_get_attribute(props, "status")),
            props);
    }
}
#endif

static void _keyboard_click(void* data, Evas_Object* obj, void* event_info)
{
    system("killall -USR1 mokowm");
    notify_window_hide();
}

static void _close_handle_click(void* data, Evas_Object* obj, void* event_info)
{
    notify_window_hide();
}

static void _focus_out(void* data, Evas_Object* obj, void* event_info)
{
    g_debug("FOCUS OUT (%d)", notification_show);
    if (notification_show)
        notify_window_hide();
}

static void _list_selected(void *data, Evas_Object *obj, void *event_info)
{
    MokoNotification* n = (MokoNotification*) elm_genlist_item_data_get((const Elm_Genlist_Item *)event_info);

    elm_genlist_item_selected_set((Elm_Genlist_Item*)event_info, FALSE);

    // FIXME mamma mia che porcata! :S
    GError *err = NULL;

    g_debug("n = %p", n);
    g_debug("n->type = %d", n->type);
    char *cmd = g_strdup_printf("sh -c \"%s\"", notification_commands[n->type]);
    g_spawn_command_line_async(cmd, &err);

    g_free(cmd);
    g_debug("Process spawned, error: %s", (err != NULL) ? err->message : "OK");

    if (err != NULL)
        g_error_free(err);

    notify_window_hide();
}

static char* notification_genlist_label_get(const void *data, Evas_Object * obj, const char *part)
{
    MokoNotification* n = (MokoNotification*) data;

    if (!strcmp(part, "elm.text")) {

        // TODO
        return g_strdup_printf(_(notification_labels[n->type][(n->count == 1) ? 0 : 1]), n->count);

    } else if (!strcmp(part, "elm.text.sub")) {

        // TODO
        return (n->count == 1) ? g_strdup(n->text) : NULL;
    }

    return NULL;
}

static Evas_Object* notification_genlist_icon_get(const void *data, Evas_Object * obj, const char *part)
{
    MokoNotification* n = (MokoNotification*) data;

    if (!strcmp(part, "elm.swallow.icon")) {
        // icona notifica
        Evas_Object *icon = elm_icon_add(n->win);
        elm_icon_file_set(icon, n->icon, NULL);
        //evas_object_size_hint_min_set(icon, 100, 100);
        // TODO icona dimensionata correttamente? :S
        //elm_icon_smooth_set(icon, TRUE);
        elm_icon_no_scale_set(icon, TRUE);
        elm_icon_scale_set(icon, FALSE, FALSE);
        evas_object_show(icon);

        return icon;
    }

    return NULL;
}

static void notification_genlist_del(const void *data, Evas_Object *obj)
{
    MokoNotification* n = (MokoNotification*) data;
    g_free(n->text);
    g_free(n->icon);

    g_free(n);
}

static gboolean fso_connect(gpointer data)
{
    ogsmd_call_call_status_connect(call_status, NULL);
    //ogsmd_call_list_calls(list_calls, data);
    return FALSE;
}

/**
 * Inizializza le notifiche di chiamata nel pannello dato.
 * TODO implementazione multi-panel
 */
void notify_calls_init(MokoPanel* panel)
{
    dbus_connect_to_ogsmd_call();

    if (callBus == NULL) {
        g_error("Cannot connect to ogsmd (call). Exiting");
        return;
    }

    dbus_connect_to_odeviced_idle_notifier();

    if (odevicedIdleNotifierBus == NULL) {
        g_error("Cannot connect to odeviced (idle notifier). Exiting");
        return;
    }

    current_panel = panel;

    // array chiamate
    calls = g_array_new(TRUE, TRUE, sizeof(int));

    g_idle_add(fso_connect, panel);
}

/**
 * Aggiunge una notifica alla lista.
 */
MokoNotification* notification_window_add(MokoPanel* panel, const char* text, const char* icon, int type)
{
    MokoNotification* n = NULL;

    if (notification_items[type]) {
        // incrementa il contatore
        Elm_Genlist_Item* item = notification_items[type];
        n = (MokoNotification *) elm_genlist_item_data_get(item);

        // TODO cambia sublabel ??
        n->count++;

        elm_genlist_item_update(item);
    }

    else {
        n = g_new0(MokoNotification, 1);

        n->list = notification_list;
        n->win = notification_win;
        n->panel = panel;
        n->text = g_strdup(text);
        n->icon = g_strdup(icon);
        n->type = type;
        n->count = 1;

        n->item = elm_genlist_item_append(notification_list, &itc, n,
            NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

        notification_items[type] = n->item;
    }

    return n;
}

/**
 * Rimuove una notifica dalla lista.
 */
void notification_window_remove(MokoNotification* n)
{
    n->count--;

    if (!n->count) {
        notification_items[n->type] = NULL;
        elm_genlist_item_del(n->item);
    }
}

void notify_window_init(MokoPanel* panel)
{
    Evas_Object* win = elm_win_add(NULL, "mokonotifications", ELM_WIN_DIALOG_BASIC);
    if (win == NULL) {
        g_critical("Cannot create notifications window; will not be able to read notifications");
        return;
    }

    elm_win_title_set(win, "Notifications");
    elm_win_borderless_set(win, TRUE);
    elm_win_sticky_set(win, TRUE);

    evas_object_smart_callback_add(win, "focus,in", _focus_out, NULL);

    // FIXME FIXME FIXME!!!
    evas_object_resize(win, 480, 600);
    evas_object_move(win, 0, 40);

    Ecore_X_Window_State state;

    state = ECORE_X_WINDOW_STATE_SKIP_TASKBAR;
    ecore_x_netwm_window_state_set(elm_win_xwindow_get(win), &state, 1);

    state = ECORE_X_WINDOW_STATE_SKIP_PAGER;
    ecore_x_netwm_window_state_set(elm_win_xwindow_get(win), &state, 1);

    state = ECORE_X_WINDOW_STATE_ABOVE;
    ecore_x_netwm_window_state_set(elm_win_xwindow_get(win), &state, 1);

    ecore_x_window_configure(elm_win_xwindow_get(win),
        ECORE_X_WINDOW_CONFIGURE_MASK_STACK_MODE,
        0, 0, 0, 0,
        0, 0,
        ECORE_X_WINDOW_STACK_ABOVE);

    Evas_Object* bg = elm_bg_add(win);
    evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_win_resize_object_add(win, bg);
    evas_object_show(bg);

    Evas_Object* vbox = elm_box_add(win);
    evas_object_size_hint_weight_set(vbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_win_resize_object_add(win, vbox);
    evas_object_show(vbox);

    // TODO intestazione
    // TEST pulsante tastiera :)
    Evas_Object* vkb = elm_button_add(win);
    elm_button_label_set(vkb, _("Keyboard"));
    evas_object_size_hint_weight_set(vkb, 0.0, 0.0);
    evas_object_size_hint_align_set(vkb, 1.0, 0.5);
    evas_object_smart_callback_add(vkb, "clicked", _keyboard_click, NULL);
    evas_object_show(vkb);

    elm_box_pack_start(vbox, vkb);

    Evas_Object* list = elm_genlist_add(win);
    elm_genlist_bounce_set(list, FALSE, FALSE);
    evas_object_smart_callback_add(list, "selected", _list_selected, NULL);

    evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);

    itc.item_style = "generic_sub";
    itc.func.label_get = notification_genlist_label_get;
    itc.func.icon_get = notification_genlist_icon_get;
    itc.func.del = notification_genlist_del;

    elm_box_pack_end(vbox, list);
    evas_object_show(list);

    Evas_Object* close_handle = elm_button_add(win);
    elm_button_label_set(close_handle, "[close]");
    evas_object_smart_callback_add(close_handle, "clicked", _close_handle_click, NULL);

    evas_object_size_hint_weight_set(close_handle, EVAS_HINT_EXPAND, 0.0);
    evas_object_size_hint_align_set(close_handle, EVAS_HINT_FILL, 0.0);

    elm_box_pack_end(vbox, close_handle);
    evas_object_show(close_handle);

    notification_win = win;
    notification_list = list;
    current_panel = panel;
}

void notify_window_start(void)
{
    g_return_if_fail(notification_win != NULL);

    if (!evas_object_visible_get(notification_win)) {

        evas_object_move(notification_win, 0, 40);
        evas_object_resize(notification_win, 480, 40);

        // notifica al pannello l'evento
        mokopanel_fire_event(current_panel, MOKOPANEL_CALLBACK_NOTIFICATION_START, NULL);
    }

    notify_window_show();
}

void notify_window_end(void)
{
    g_return_if_fail(notification_win != NULL);

    evas_object_move(notification_win, 0, 40);
    evas_object_resize(notification_win, 480, 600);
    notify_window_show();

    notification_show = TRUE;

    // notifica al pannello l'evento
    mokopanel_fire_event(current_panel, MOKOPANEL_CALLBACK_NOTIFICATION_END, NULL);
}

void notify_window_show(void)
{
    g_return_if_fail(notification_win != NULL);

    evas_object_show(notification_win);
    elm_win_activate(notification_win);
    evas_object_focus_set(notification_win, TRUE);
}

void notify_window_hide(void)
{
    g_return_if_fail(notification_win != NULL);

    evas_object_hide(notification_win);

    notification_show = FALSE;

    // notifica al pannello l'evento
    mokopanel_fire_event(current_panel, MOKOPANEL_CALLBACK_NOTIFICATION_HIDE, NULL);
}
