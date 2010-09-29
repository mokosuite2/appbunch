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
#include <libmokosuite/misc.h>
#include <libmokosuite/notifications.h>
#include <freesmartphone-glib/freesmartphone-glib.h>
#include <freesmartphone-glib/odeviced/idlenotifier.h>
#include <freesmartphone-glib/ogsmd/call.h>
#include <freesmartphone-glib/ousaged/usage.h>
#include <freesmartphone-glib/opimd/calls.h>
#include <freesmartphone-glib/opimd/callquery.h>
#include <freesmartphone-glib/opimd/call.h>
#include <freesmartphone-glib/opimd/messagequery.h>
#include <freesmartphone-glib/opimd/messages.h>
#include <freesmartphone-glib/opimd/message.h>

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

static Evas_Object* operator_label = NULL;

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
    "/usr/bin/mokophone-activate.sh Frontend string:calls",
    "/usr/bin/mokophone-activate.sh Frontend string:log",
    "/usr/bin/phoneui-messages",
    "/usr/bin/mokophone-activate.sh"
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
                ousaged_usage_request_resource("Display", NULL, NULL);
            }

            // disabilita idle screen
            idle_hide();
            break;

        case CALL_STATUS_ACTIVE:
            ousaged_usage_release_resource("Display", NULL, NULL);

            break;

        case CALL_STATUS_RELEASE:
            // togli la chiamata dalla lista se presente
            for (i = 0; i < calls->len; i++) {
                if (g_array_index(calls, int, i) == id) {
                    g_array_remove_index_fast(calls, i);
                    break;
                }
            }

            ousaged_usage_release_resource("Display", NULL, NULL);

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

typedef struct {
    /* liberati subito */
    GHashTable* query;
    /* mantenuti fino alla fine */
    gpointer userdata;
    char* path;
} query_data_t;

static void call_query(GError* error, const char* path, gpointer userdata);
static void call_data(GError* error, GHashTable* props, gpointer data);

static void call_next(GError* error, GHashTable* row, gpointer userdata)
{
    query_data_t* data = userdata;

    if (error) {
        g_debug("[%s] Call row error: %s", __func__, error->message);

        // distruggi query
        opimd_callquery_dispose(data->path, NULL, NULL);

        // distruggi dati callback
        g_free(data->path);
        g_free(data);
        return;
    }

    // aggiungi! :)
    call_data(NULL, row, NULL);

    // prossimo risultato
    opimd_callquery_get_result(data->path, call_next, data);
}

static gboolean retry_call_query(gpointer userdata)
{
    query_data_t* data = userdata;
    opimd_calls_query(data->query, call_query, data);
    return FALSE;
}

static void call_query(GError* error, const char* path, gpointer userdata)
{
    query_data_t* data = userdata;
    if (error) {
        g_debug("Missed calls query error: (%d) %s", error->code, error->message);

        // opimd non ancora caricato? Riprova in 5 secondi
        if (FREESMARTPHONE_GLIB_IS_DBUS_ERROR(error, FREESMARTPHONE_GLIB_DBUS_ERROR_SERVICE_NOT_AVAILABLE)) {
            g_timeout_add_seconds(5, retry_call_query, data);
            return;
        }

        g_hash_table_destroy(data->query);
        g_free(data);
        return;
    }

    g_hash_table_destroy(data->query);

    data->path = g_strdup(path);
    opimd_callquery_get_result(data->path, call_next, data);
}

static void missed_calls(GError* error, gint missed, gpointer data)
{
    if (missed <= 0) return;

    // crea query chiamate senza risposta :)
    query_data_t* cbdata = g_new0(query_data_t, 1);

    cbdata->query = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_value_free);
    cbdata->userdata = data;

    g_hash_table_insert(cbdata->query, g_strdup("New"),
        g_value_from_int(1));
    g_hash_table_insert(cbdata->query, g_strdup("Answered"),
        g_value_from_int(0));
    g_hash_table_insert(cbdata->query, g_strdup("Direction"),
        g_value_from_string("in"));

    opimd_calls_query(cbdata->query, call_query, cbdata);
}

static void call_update(gpointer data, GHashTable* props)
{
    g_debug("Call has been modified - checking New attribute");
    // chiamata modificata - controlla new
    if (g_hash_table_lookup(props, "New") && !fso_get_attribute_int(props, "New")) {
        g_debug("New is 0 - removing missed call");
        mokopanel_notification_remove(current_panel, GPOINTER_TO_INT(data));
    }
}

static void call_remove(gpointer data)
{
    // chiamata cancellata - rimuovi sicuramente
    g_debug("Call has been deleted - removing missed call");
    mokopanel_notification_remove(current_panel, GPOINTER_TO_INT(data));
}

static void call_data(GError* error, GHashTable* props, gpointer data)
{
    if (error != NULL) {
        g_debug("Error getting new call data: %s", error->message);
        return;
    }

    const char* path = fso_get_attribute(props, "Path");

    // new
    gboolean is_new = (fso_get_attribute_int(props, "New") != 0);

    // answered
    gboolean answered = (fso_get_attribute_int(props, "Answered") != 0);

    // direction (incoming)
    const char* _direction = fso_get_attribute(props, "Direction");
    gboolean incoming = !strcasecmp(_direction, "in");

    g_debug("New call: answered=%d, new=%d, incoming=%d",
        answered, is_new, incoming);

    if (!answered && is_new && incoming) {
        const char* peer = fso_get_attribute(props, "Peer");
        char* text = g_strdup_printf(_("Missed call from %s"), peer);

        int id = mokopanel_notification_queue(current_panel,
            text, MOKOSUITE_DATADIR "call-end.png", NOTIFICATION_MISSED_CALL,
            MOKOPANEL_NOTIFICATION_FLAG_REPRESENT);
        g_free(text);

        // connetti ai cambiamenti della chiamata per la rimozione
        opimd_call_call_deleted_connect((char *) path, call_remove, GINT_TO_POINTER(id));
        opimd_call_call_updated_connect((char *) path, call_update, GINT_TO_POINTER(id));
    }
}

static void new_call(gpointer data, const char* path)
{
    g_debug("New call created %s", path);
    // ottieni dettagli chiamata per controllare se e' senza risposta
    opimd_call_get_multiple_fields(path, "Path,Peer,Answered,New,Direction", call_data, NULL);
}

static void message_query(GError* error, const char* path, gpointer userdata);
static void message_data(GError* error, GHashTable* props, gpointer data);

// solo per messaggi non letti :)
static void message_next(GError* error, GHashTable* row, gpointer userdata)
{
    query_data_t* data = userdata;

    if (error) {
        g_debug("[%s] Message row error: %s", __func__, error->message);

        // distruggi query
        opimd_messagequery_dispose(data->path, NULL, NULL);

        // distruggi dati callback
        g_free(data->path);
        g_free(data);
        return;
    }

    // aggiungi! :)
    message_data(NULL, row, NULL);

    // prossimo risultato
    opimd_messagequery_get_result(data->path, message_next, data);
}

static gboolean retry_message_query(gpointer userdata)
{
    query_data_t* data = userdata;
    opimd_calls_query(data->query, message_query, data);
    return FALSE;
}

static void message_query(GError* error, const char* path, gpointer userdata)
{
    query_data_t* data = userdata;
    if (error) {
        g_debug("Message query error: (%d) %s", error->code, error->message);

        // opimd non ancora caricato? Riprova in 5 secondi
        if (FREESMARTPHONE_GLIB_IS_DBUS_ERROR(error, FREESMARTPHONE_GLIB_DBUS_ERROR_SERVICE_NOT_AVAILABLE)) {
            g_timeout_add_seconds(5, retry_message_query, data);
            return;
        }

        g_hash_table_destroy(data->query);
        g_free(data);
        return;
    }

    g_hash_table_destroy(data->query);

    data->path = g_strdup(path);
    opimd_messagequery_get_result(data->path, message_next, data);
}

static void unread_messages(GError* error, gint unread, gpointer data)
{
    if (unread <= 0) return;

    // crea query messaggi entranti non letti
    query_data_t* cbdata = g_new0(query_data_t, 1);

    cbdata->query = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_value_free);
    cbdata->userdata = data;

    g_hash_table_insert(cbdata->query, g_strdup("MessageRead"),
        g_value_from_int(0));
    g_hash_table_insert(cbdata->query, g_strdup("Direction"),
        g_value_from_string("in"));

    opimd_messages_query(cbdata->query, message_query, cbdata);
}

static void message_update(gpointer data, GHashTable* props)
{
    g_debug("Message has been modified - checking MessageRead attribute");
    // chiamata modificata - controlla read
    if (g_hash_table_lookup(props, "MessageRead") && fso_get_attribute_bool(props, "MessageRead", TRUE)) {
        g_debug("MessageRead is 1 - removing message");
        mokopanel_notification_remove(current_panel, GPOINTER_TO_INT(data));
    }
}

static void message_remove(gpointer data)
{
    // messaggio cancellato - rimuovi sicuramente
    g_debug("Message has been deleted - removing unread message");
    mokopanel_notification_remove(current_panel, GPOINTER_TO_INT(data));
}

static void message_data(GError* error, GHashTable* props, gpointer data)
{
    if (error != NULL) {
        g_debug("Error getting new message data: %s", error->message);
        return;
    }

    const char* path = fso_get_attribute(props, "Path");

    // read
    gboolean is_read = (fso_get_attribute_bool(props, "MessageRead", TRUE) != 0);

    // direction (incoming)
    const char* _direction = fso_get_attribute(props, "Direction");
    gboolean incoming = !strcasecmp(_direction, "in");

    g_debug("New message: read=%d, incoming=%d",
        is_read, incoming);

    if (!is_read && incoming) {
        const char* peer = fso_get_attribute(props, "Peer");
        char* text = g_strdup_printf(_("New message from %s"), peer);

        int id = mokopanel_notification_queue(current_panel,
            text, MOKOSUITE_DATADIR "message-dock.png", NOTIFICATION_UNREAD_MESSAGE,
            MOKOPANEL_NOTIFICATION_FLAG_REPRESENT);
        g_free(text);

        // connetti ai cambiamenti della chiamata per la rimozione
        opimd_message_message_deleted_connect((char *) path, message_remove, GINT_TO_POINTER(id));
        opimd_message_message_updated_connect((char *) path, message_update, GINT_TO_POINTER(id));
    }
}

static void new_incoming_message(gpointer data, const char* path)
{
    g_debug("New incoming message %s", path);
    opimd_message_get_multiple_fields(path, "Path,Peer,Direction,MessageRead,Content", message_data, NULL);
}

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
    //g_debug("FOCUS OUT (%d)", notification_show);
    if (notification_show)
        notify_window_hide();
}

static void _list_selected(void *data, Evas_Object *obj, void *event_info)
{
    MokoNotification* n = (MokoNotification*) elm_genlist_item_data_get((const Elm_Genlist_Item *)event_info);

    elm_genlist_item_selected_set((Elm_Genlist_Item*)event_info, FALSE);

    // FIXME mamma mia che porcata! :S
    GError *err = NULL;

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
    // call status per i cambiamenti delle chiamate
    ogsmd_call_call_status_connect(call_status, data);

    // TODO un giorno faremo anche questo -- ogsmd_call_list_calls(list_calls, data);

    // nuove chiamate senza risposte :)
    opimd_calls_new_call_connect(new_call, data);

    // ottieni le chiamate senza risposta attuali
    opimd_calls_get_new_missed_calls(missed_calls, data);

    // nuovi messaggi entranti
    opimd_messages_incoming_message_connect(new_incoming_message, data);

    // ottieni i messaggi entranti non letti attuali
    opimd_messages_get_unread_messages(unread_messages, data);

    return FALSE;
}

/**
 * Inizializza le notifiche di chiamata nel pannello dato.
 * TODO implementazione multi-panel
 */
void notify_calls_init(MokoPanel* panel)
{
    ogsmd_call_dbus_connect();

    if (ogsmdCallBus == NULL) {
        g_error("Cannot connect to ogsmd (call). Exiting");
        return;
    }

    odeviced_idlenotifier_dbus_connect();

    if (odevicedIdlenotifierBus == NULL) {
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
    Evas_Object* win = elm_win_add(NULL, "mokonotifications", ELM_WIN_BASIC);
    if (win == NULL) {
        g_critical("Cannot create notifications window; will not be able to read notifications");
        return;
    }

    elm_win_title_set(win, "Notifications");
    elm_win_borderless_set(win, TRUE);
    elm_win_sticky_set(win, TRUE);

    evas_object_smart_callback_add(win, "focus,out", _focus_out, NULL);
    evas_object_smart_callback_add(win, "delete,request", _focus_out, NULL);

    #if 0
    // FIXME FIXME FIXME!!!
    evas_object_resize(win, 480, 600);
    evas_object_move(win, 0, 40);
    #endif

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

    // intestazione
    Evas_Object* hdrbox = elm_box_add(win);
    elm_box_horizontal_set(hdrbox, TRUE);
    evas_object_size_hint_weight_set(hdrbox, EVAS_HINT_EXPAND, 0.0);
    evas_object_size_hint_align_set(hdrbox, EVAS_HINT_FILL, 0.0);
    evas_object_show(hdrbox);

    // operatore gsm
    operator_label = elm_label_add(win);
    evas_object_size_hint_weight_set(operator_label, EVAS_HINT_EXPAND, 0.0);
    evas_object_size_hint_align_set(operator_label, 0.1, 0.5);
    evas_object_show(operator_label);

    elm_box_pack_start(hdrbox, operator_label);

    // TEST pulsante tastiera :)
    Evas_Object* vkb = elm_button_add(win);
    elm_button_label_set(vkb, _("Keyboard"));
    evas_object_size_hint_weight_set(vkb, 0.0, 0.0);
    evas_object_size_hint_align_set(vkb, 1.0, 0.5);
    evas_object_smart_callback_add(vkb, "clicked", _keyboard_click, NULL);
    evas_object_show(vkb);

    elm_box_pack_end(hdrbox, vkb);

    // aggiungi l'intestazione alla finestra
    elm_box_pack_start(vbox, hdrbox);

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
        // notifica al pannello l'evento
        mokopanel_fire_event(current_panel, MOKOPANEL_CALLBACK_NOTIFICATION_START, NULL);
    }
}

void notify_window_end(void)
{
    g_return_if_fail(notification_win != NULL);

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

void notify_window_update_operator(const char* operator)
{
    if (operator_label) {
        char* op = g_strdup_printf("<b><font_size=10>%s</></b>", operator);
        elm_label_label_set(operator_label, op);
        g_free(op);
    }
}
