#include "mokophone.h"
#include "gsm.h"

#include <libmokosuite/gui.h>
#include <libmokosuite/contactsdb.h>
#include <libmokosuite/misc.h>
#include <libmokosuite/notifications.h>

#include "phonewin.h"
#include "logview.h"
#include "logentry.h"
#include "callwin.h"
#include "callsdb.h"

#include <glib/gi18n-lib.h>

static Elm_Genlist_Item_Class itc = {0};
static Elm_Genlist_Item_Class itc_sub = {0};
Evas_Object *log_list = NULL;

// FIXME bug del cazzo della genlist
static gboolean longpressed = FALSE;

// notifiche chiamate perse
static GSList* lostcall_ids = NULL;
extern DBusGProxy* panel_notifications;

/* -- sezione log -- */

static void _list_selected(void *data, Evas_Object *obj, void *event_info)
{
    elm_genlist_item_selected_set((Elm_Genlist_Item*)event_info, FALSE);

    // FIXME bug del cazzo della genlist
    if (longpressed) {
        longpressed = FALSE;
        return;
    }

    // finestra gestione chiamata
    CallEntry *e = (CallEntry *)elm_genlist_item_data_get((const Elm_Genlist_Item *)event_info);

    if (e != NULL)
        logentry_new(e);
}

static void _popup_click(gpointer popup, gpointer data, int index, gboolean final)
{
    CallEntry *c = (CallEntry *) data;

    g_debug("POPUP! Click to %d, final = %d", index, final);
    switch (index) {

        // chiamata
        case 1:
            if (c->peer != NULL)
                phone_win_call_internal(c->peer, NULL);

            break;

        // cancella entry
        case 2:
            callsdb_delete_call(c->id);
            elm_genlist_item_del((Elm_Genlist_Item *)c->data);

            break;

        default:
            break;
    }
}

// wrapper per logentry
static void _popup_clicked_wrapper(CallEntry* c, int choose)
{
    _popup_click(NULL, c, choose, TRUE);
}

static void _list_longpressed(void *data, Evas_Object *obj, void *event_info)
{
    // FIXME bug del cazzo della genlist
    longpressed = TRUE;

    CallEntry *e = (CallEntry *)elm_genlist_item_data_get((const Elm_Genlist_Item *)event_info);

    MokoPopupMenu *p = moko_popup_menu_new(phone_win_get_mokowin(), NULL, MOKO_POPUP_BUTTONS, _popup_click, e);

    if (e->peer != NULL) {
        char *s = g_strdup_printf(_("Call %s"), e->peer);
        moko_popup_menu_add(p, s, 1, FALSE);
        g_free(s);
    }

    moko_popup_menu_add(p, _("Delete log entry"), 2, FALSE);

    if (e->peer != NULL)
        moko_popup_menu_add(p, _("Add to contacts"), 3, FALSE);

    mokoinwin_activate(MOKO_INWIN(p));
}

static void _log_clear_clicked(void *data, Evas_Object *obj, void *event_info)
{
    // nascondi il menu
    mokowin_menu_hide(phone_win_get_mokowin());

    // TODO chiedi conferma?
    callsdb_truncate();
    elm_genlist_clear(log_list);
}

static void _log_call_clicked(void *data, Evas_Object *obj, void *event_info)
{
    CallEntry *c = (CallEntry *) data;

    if (c->peer != NULL)
        phone_win_call_internal(c->peer, NULL);
}

static char* log_genlist_label_get(const void *data, Evas_Object * obj, const char *part)
{
    // TODO
    CallEntry* call = (CallEntry *) data;
    //g_debug("Requesting label for part %s (data = %p)", part, data);

    if (!strcmp(part, "elm.text")) {

        if (call->data2) {
            ContactField* f = contactsdb_get_first_field((ContactEntry *) call->data2, CONTACT_FIELD_NAME);
            return g_strdup( (f != NULL) ? f->value : _("(no name)"));
        }

        else {

            if (call->peer)
                return g_strdup(call->peer);
            else
                return g_strdup(_("(no number)"));
        }

    } else if (!strcmp(part, "elm.text.sub")) {

        if (call->peer)
            return g_strdup(call->peer);
        else
            return g_strdup(_("(no number)"));

    } else if (!strcmp(part, "elm.text.right")) {

        return get_time_repr(call->timestamp);
    }


    return NULL;
}

static Evas_Object* log_genlist_icon_get(const void *data, Evas_Object * obj, const char *part)
{
    //g_debug("Requesting icon for part %s (data = %p)", part, data);

    CallEntry* call = (CallEntry *) data;
    MokoWin* win = phone_win_get_mokowin();

    if (!strcmp(part, "elm.swallow.icon")) {

        // icona bt_call
        Evas_Object *icon_call = elm_icon_add(win->win);
        elm_icon_file_set(icon_call, MOKOSUITE_DATADIR "call-start.png", NULL);
    
        elm_icon_smooth_set(icon_call, TRUE);
        elm_icon_scale_set(icon_call, TRUE, TRUE);
        evas_object_show(icon_call);
    
        // bt_call
        Evas_Object *bt_call = elm_button_add(win->win);
        elm_button_icon_set(bt_call, icon_call);

        evas_object_propagate_events_set(bt_call, FALSE);

        if (call->peer != NULL)
            evas_object_smart_callback_add(bt_call, "clicked", _log_call_clicked, data);
        else
            elm_object_disabled_set(bt_call, TRUE);

        return bt_call;
    }

    else if (!strcmp(part, "elm.swallow.end")) {

        Evas_Object *icon_type = elm_icon_add(win->win);

        char* type = NULL;

        if (call->direction == DIRECTION_OUTGOING)
            type = "out";

        else if (call->direction == DIRECTION_INCOMING) {

            if (call->answered) type = "in";
            else type = "missed";
        }

        char* file = g_strdup_printf(MOKOSUITE_DATADIR "log_call-%s.png", type);

        elm_icon_file_set(icon_type, file, NULL);

        g_free(file);
    
        //elm_icon_smooth_set(icon_type, TRUE);
        elm_icon_no_scale_set(icon_type, TRUE);
        elm_icon_scale_set(icon_type, FALSE, TRUE);
        evas_object_show(icon_type);

        return icon_type;
    }

    return NULL;
}

static Eina_Bool log_genlist_state_get(const void *data, Evas_Object * obj, const char *part)
{
    // TODO
    return FALSE;
}

static void log_genlist_del(const void *data, Evas_Object *obj)
{
    CallEntry* call = (CallEntry *) data;
    g_free(call->peer);
    g_free(call);
}

static void log_process_call(CallEntry* call, gpointer data)
{
    Elm_Genlist_Item_Class* cur_itc = &itc;
    if (call->peer) {
        call->data2 = contactsdb_lookup_number(call->peer);

        if (call->data2)
            cur_itc = &itc_sub;
    }

    Elm_Genlist_Item *it = elm_genlist_item_append((Evas_Object *) data, cur_itc, call,
        NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

    call->data = it;

    if (call->is_new && !call->answered && call->direction == DIRECTION_INCOMING) {
        char* text;
        if (call->data2 != NULL) {
            ContactField* f = contactsdb_get_first_field((ContactEntry *) call->data2, CONTACT_FIELD_NAME);
            text = g_strdup_printf(_("Missed call from %s - %s"), (f != NULL) ? f->value : _("(no name)"), call->peer);
        }

        else
            text = g_strdup_printf(_("Missed call from %s"), call->peer);

        call->data3 = GINT_TO_POINTER(moko_notifications_push(panel_notifications, text,
            MOKOSUITE_DATADIR "call-end.png", NOTIFICATION_MISSED_CALL, MOKOPANEL_NOTIFICATION_FLAG_REPRESENT, NULL));

        g_free(text);
        lostcall_ids = g_slist_append(lostcall_ids, call->data3);
    }
}

/* aggiunge una chiamata alla lista */
void logview_add_call(CallEntry* e)
{
    /* TODO ricerca binaria nella lista per trovare con meno passi possibili */

    // lookup!
    Elm_Genlist_Item_Class* cur_itc = &itc;
    if (e->peer) {
        e->data2 = contactsdb_lookup_number(e->peer);

        if (e->data2)
            cur_itc = &itc_sub;
    }

    // FIXME per ora aggiungi all'inizio
    Elm_Genlist_Item *it = elm_genlist_item_prepend(log_list, cur_itc, e,
        NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

    e->data = it;

    // chiamata persa?
    if (e->direction == DIRECTION_INCOMING && !e->answered) {
        char* text;
        if (e->data2 != NULL) {
            ContactField* f = contactsdb_get_first_field((ContactEntry *) e->data2, CONTACT_FIELD_NAME);
            text = g_strdup_printf(_("Missed call from %s - %s"), (f != NULL) ? f->value : _("(no name)"), e->peer);
        }

        else
            text = g_strdup_printf(_("Missed call from %s"), e->peer);

        e->data3 = GINT_TO_POINTER(moko_notifications_push(panel_notifications, text,
            MOKOSUITE_DATADIR "call-end.png", NOTIFICATION_MISSED_CALL, MOKOPANEL_NOTIFICATION_FLAG_REPRESENT, NULL));

        g_free(text);
        lostcall_ids = g_slist_append(lostcall_ids, e->data3);
    }
}

/* costruisce la sezione log */
Evas_Object* logview_make_section(void)
{
    // catturiamo la finestra :D
    MokoWin* win = phone_win_get_mokowin();

    // overlay per gli elementi della lista del log
    elm_theme_overlay_add(NULL, "elm/genlist/item/call/default");
    elm_theme_overlay_add(NULL, "elm/genlist/item_odd/call/default");

    log_list = elm_genlist_add(win->win);
    evas_object_size_hint_weight_set(log_list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    elm_genlist_bounce_set(log_list, FALSE, FALSE);
    evas_object_smart_callback_add(log_list, "selected", _list_selected, NULL);
    evas_object_smart_callback_add(log_list, "longpressed", _list_longpressed, NULL);

    itc.item_style = "call";
    itc.func.label_get = log_genlist_label_get;
    itc.func.icon_get = log_genlist_icon_get;
    itc.func.state_get = log_genlist_state_get;
    itc.func.del = log_genlist_del;

    itc_sub.item_style = "call_sub";
    itc_sub.func.label_get = log_genlist_label_get;
    itc_sub.func.icon_get = log_genlist_icon_get;
    itc_sub.func.state_get = log_genlist_state_get;
    itc_sub.func.del = log_genlist_del;

    evas_object_show(log_list);

    // carica subito le chiamate
    callsdb_foreach_call(log_process_call, log_list);

    // inializza quell'altro
    logentry_init(_popup_clicked_wrapper);

    return log_list;
}

/* costruisce il menu della sezione log */
Evas_Object* logview_make_menu(void)
{
    MokoWin* win = phone_win_get_mokowin();

    Evas_Object *bt_clear = elm_button_add(win->win);
    elm_button_label_set(bt_clear, _("Clear log"));

    evas_object_size_hint_weight_set(bt_clear, 1.0, 0.0);
    evas_object_size_hint_align_set(bt_clear, -1.0, 1.0);

    evas_object_smart_callback_add(bt_clear, "clicked", _log_clear_clicked, NULL);

    return bt_clear;
}

void logview_reset_view(void)
{
    #if 0
    // TODO come controllare questa cosa con opimd?
    // se il db e' stato modificato...
    time_t new_time = get_modification_time(callsdb_path);
    if (new_time != callsdb_timestamp) {
        callsdb_timestamp = new_time;

        elm_genlist_clear(log_list);
        callsdb_foreach_call(log_process_call, log_list);
    }
    #endif

    Elm_Genlist_Item *item = elm_genlist_first_item_get(log_list);

    if (item)
        elm_genlist_item_show(item);

    // per ora annulla qui le notifiche...
    if (lostcall_ids != NULL) {
        GSList* iter = lostcall_ids;

        while (iter) {
            int id = GPOINTER_TO_INT(iter->data);
            moko_notifications_remove(panel_notifications, id, NULL);

            iter = iter->next;
        }

        g_slist_free(lostcall_ids);
        lostcall_ids = NULL;

        callsdb_set_call_new(-1, FALSE);
    }
}
