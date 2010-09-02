#include <libmokosuite/mokosuite.h>
#include <libmokosuite/gui.h>
#include <libmokosuite/misc.h>
#include <libmokosuite/settings-service.h>

#include "mokomessages.h"

#include <glib/gi18n-lib.h>

/* finestra principale */
static MokoWin* win = NULL;

/* lista conversazioni */
static Evas_Object* th_list;

static void _delete(void* mokowin, Evas_Object* obj, void* event_info)
{
    mokowin_hide((MokoWin *)mokowin);
}

static void _list_selected(void *data, Evas_Object *obj, void *event_info) {

    elm_genlist_item_selected_set((Elm_Genlist_Item*)event_info, FALSE);
    // TODO

    thread_t* t = (thread_t *) elm_genlist_item_data_get((Elm_Genlist_Item*)event_info);
    msg_list_init(t);
    msg_list_activate();
}

void thread_win_activate(void)
{
    g_return_if_fail(win != NULL);

    mokowin_activate(win);
}

void thread_win_hide(void)
{
    g_return_if_fail(win != NULL);

    mokowin_hide(win);
}

static char* _newmsg_genlist_label_get(const void *data, Evas_Object * obj, const char *part)
{
    if (!strcmp(part, "elm.text"))
        return g_strdup(_("New message"));

    else if (!strcmp(part, "elm.text.sub"))
        return g_strdup(_("Compose new message"));

    return NULL;
}

static char* _th_genlist_label_get(const void *data, Evas_Object * obj, const char *part)
{
    const thread_t* t = data;

    if (!strcmp(part, "elm.text"))
        return g_strdup(t->peer);

    else if (!strcmp(part, "elm.text.sub"))
        return g_strdup(t->text);

    else if (!strcmp(part, "elm.text.right"))
        return get_time_repr(t->timestamp);

    return NULL;
}

static void _list_realized(void *data, Evas_Object *obj, void *event_info)
{
    //g_debug("Item has been realized, marking");
    thread_t* t = (thread_t *) elm_genlist_item_data_get((Elm_Genlist_Item*)event_info);
    if (t && t->marked) {
        Evas_Object* e = (Evas_Object *) elm_genlist_item_object_get((Elm_Genlist_Item*)event_info);
        edje_object_signal_emit(e, "elm,marker,enable", "elm");
    }
}

static Elm_Genlist_Item_Class test_itc = {0};

static void test_messages(Evas_Object* list)
{
    test_itc.item_style = "thread";
    test_itc.func.label_get = _th_genlist_label_get;

    thread_t* t;
    t = g_new0(thread_t, 1);
    t->peer = "155";
    t->text = "Le abbiamo addebitato 9 euro per l'offerta del cazzo aggiuntivo.";
    t->timestamp = time(NULL);
    t->marked = TRUE;

    elm_genlist_item_append(list, &test_itc, t, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

    t = g_new0(thread_t, 1);
    t->peer = "+393296061565";
    t->text = "Ciau more ti amo troppiximo!! Lo sai oggi ti ho fatto un regalino!:)!";
    t->timestamp = time(NULL);
    t->marked = FALSE;

    elm_genlist_item_append(list, &test_itc, t, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
}

static Evas_Object* thread_list(void)
{
    // overlay per gli elementi della lista del log
    elm_theme_overlay_add(NULL, "elm/genlist/item/thread/default");
    elm_theme_overlay_add(NULL, "elm/genlist/item_odd/thread/default");

    Evas_Object* list = elm_genlist_add(win->win);
    elm_genlist_bounce_set(list, FALSE, FALSE);
    elm_genlist_horizontal_mode_set(list, ELM_LIST_LIMIT);
    elm_genlist_homogeneous_set(list, FALSE);

    evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_smart_callback_add(list, "selected", _list_selected, NULL);
    evas_object_smart_callback_add(list, "realized", _list_realized, NULL);

    // aggiungi il primo elemento "Nuovo messaggio"
    Elm_Genlist_Item_Class *itc = g_new0(Elm_Genlist_Item_Class, 1);
    itc->item_style = "generic_sub";
    itc->func.label_get = _newmsg_genlist_label_get;

    elm_genlist_item_append(list, itc, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

    test_messages(list);

    evas_object_show(list);
    return list;
}

static Evas_Object* make_menu(void)
{
    Evas_Object *m = elm_table_add(win->win);
    elm_table_homogenous_set(m, TRUE);
    evas_object_size_hint_weight_set(m, 1.0, 0.0);
    evas_object_size_hint_align_set(m, -1.0, 1.0);

    /* pulsante nuovo messaggio */
    Evas_Object *bt_compose = mokowin_menu_hover_button(win, m, _("Compose"), 0, 0, 1, 1);

    /* pulsante cancella tutto */
    Evas_Object *bt_del_all = mokowin_menu_hover_button(win, m, _("Delete all"), 1, 0, 1, 1);

    /* pulsante impostazioni */
    Evas_Object *bt_settings = mokowin_menu_hover_button(win, m, _("Settings"), 2, 0, 1, 1);

    return m;
}

void thread_win_init(MokoSettingsService *settings)
{
    win = mokowin_new("mokomessages");
    if (win == NULL) {
        g_error("[ThreadWin] Cannot create main window. Exiting");
        return;
    }

    win->delete_callback = _delete;

    elm_win_title_set(win->win, _("Messaging"));
    //elm_win_borderless_set(win->win, TRUE);

    mokowin_create_vbox(win, FALSE);
    mokowin_menu_enable(win);

    mokowin_menu_set(win, make_menu());

    th_list = thread_list();
    elm_box_pack_start(win->vbox, th_list);

    // TODO carica le conversazioni :)
    // TODO g_idle_add(...);
}
