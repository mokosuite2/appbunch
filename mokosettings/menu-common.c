#include <Elementary.h>
#include <glib.h>

#include <libmokosuite/mokosuite.h>

#include "menu-common.h"

#include <glib/gi18n-lib.h>

static void _close(void *data, Evas_Object *obj, void *event_info) {
    MokoWin* win = MOKO_WIN(data);
    gboolean child = GPOINTER_TO_INT(win->data);

    if (child)
        // nascondi finestra per un recupero successivo piu' veloce
        evas_object_hide(obj);
    else
        // esci subito
        moko_factory_exit();
}

static void _list_selected(void *data, Evas_Object *obj, void *event_info) {

    elm_genlist_item_selected_set((Elm_Genlist_Item*)event_info, FALSE);

    MenuItem* item = (MenuItem *)elm_genlist_item_data_get((const Elm_Genlist_Item *)event_info);

    if (item != NULL && item->item_callback != NULL)
        (item->item_callback)(item);

}

static Evas_Object* _menu_genlist_icon_get(void *data, Evas_Object * obj, const char *part)
{
    MenuItem *entry = (MenuItem *) data;

    if (!strcmp(part, "elm.swallow.end")) {

        if (entry->checkbox) {

            Evas_Object *check = elm_check_add(entry->list);
            elm_check_state_pointer_set(check, &entry->checked);
            evas_object_show(check);
    
            return check;
        }
    }

    return NULL;
}

static char* _menu_genlist_label_get(void *data, Evas_Object * obj, const char *part)
{
    MenuItem *entry = (MenuItem *) data;

    if (!strcmp(part, "elm.text"))
        return g_strdup(entry->label);

    else if (!strcmp(part, "elm.text.sub"))
        return (entry->sublabel != NULL) ? g_strdup(entry->sublabel) : NULL;

    return NULL;
}

/**
 * Imposta un'impostazione remota e riflette il cambiamento nel MenuItem.
 * Se il MenuItem e' di tipo checkbox, new_value non sara' usato
 */
void menu_item_replace_setting(MenuItem* item, DBusGProxy* proxy, const char* new_value, gboolean update_item)
{
    GError* e = NULL;
    gboolean new_state = FALSE;

    char* name = (char*) item->data1;
    char* fmt = (char*) item->data2;
    char* fmt2 = (char*) item->data3;

    if (item->checkbox)
        new_state = !item->checked;

    const char* new_value_real = item->checkbox ? (new_state ? "true" : "false") : new_value;

    if (!moko_settings_set_setting(proxy, name, new_value_real, &e)) {
        g_warning("Unable to set display_dim_usb: %s", e->message);
        g_error_free(e);
    } else {

        item->data4 = g_strdup(new_value_real);

        if (update_item) {
            if (item->checkbox)
                item->checked = new_state;

            if (item->checkbox)
                item->sublabel = (item->checked) ? fmt : (fmt2 != NULL ? fmt2 : fmt);
            else
                item->sublabel = g_strdup_printf(fmt, new_value);

            elm_genlist_item_update(item->item);
        }

    }
}

/**
 * Assegna un'impostazione remota ad un MenuItem.
 * fmt2 (se diverso da NULL) e' usato se il MenuItem e' di tipo checkbox e il valore e' falso
 */
void menu_item_assign_setting(MenuItem* item, DBusGProxy* proxy, const char* name, const char* default_val, const char* fmt, const char* fmt2)
{
    GError* e = NULL;

    if (proxy != NULL) {
        // salva la chiave e i formati
        item->data1 = g_strdup(name);
        item->data2 = g_strdup(fmt);
        item->data3 = g_strdup(fmt2);

        char* ret_value = moko_settings_get_setting(proxy, name, default_val, &e);

        if (e != NULL) {
            g_warning("Error getting %s: %s", name, e->message);
            g_error_free(e);

        } else {
            if (item->checkbox)
                item->checked = !strcmp(ret_value, "true");

            item->data4 = ret_value;
            item->sublabel = g_strdup_printf( (item->checked) ? fmt : (fmt2 != NULL ? fmt2 : fmt), ret_value);

            elm_genlist_item_disabled_set(item->item, FALSE);
        }
    }

    if (proxy == NULL || e != NULL)
        item->sublabel = _("(error)");

    elm_genlist_item_update(item->item);
}

void menu_window_item_add(Evas_Object* menu, MenuItem* item)
{
    item->itc.func.label_get = _menu_genlist_label_get;
    item->itc.func.icon_get = _menu_genlist_icon_get;

    item->list = menu;
    item->item = elm_genlist_item_append(menu, &item->itc, item,
        NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

}

void menu_window_item_add_array(Evas_Object* menu, MenuItem** menu_list)
{
    int i;
    for (i = 0; menu_list[i]->label != NULL; i++) {
        menu_window_item_add(menu, menu_list[i]);
    }
}

MokoWin* menu_window_new(const char* name, const char* title, gboolean child, Evas_Object** menu_ret)
{
    MokoWin *win = mokowin_new(name);
    if (!win) {
        g_critical("Cannot create menu window. Exiting.");
        return NULL;
    }

    win->data = GINT_TO_POINTER(child);
    elm_win_title_set(win->win, title);

    mokowin_create_vbox(win, FALSE);

    //evas_object_smart_callback_add(win, "delete,request", _close, win);
    //mokowin_delete_data_set(win, win);
    win->delete_callback = _close;

    Evas_Object *menu = elm_genlist_add(win->win);
    elm_genlist_bounce_set(menu, FALSE, FALSE);
    elm_genlist_horizontal_mode_set(menu, ELM_LIST_LIMIT);
    elm_genlist_homogeneous_set(menu, FALSE);

    evas_object_size_hint_weight_set(menu, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(menu, EVAS_HINT_FILL, EVAS_HINT_FILL);
    //elm_win_resize_object_add(win->win, menu);
    elm_box_pack_start(win->vbox, menu);

    evas_object_smart_callback_add(menu, "selected", _list_selected, NULL);

    evas_object_show(menu);
    *menu_ret = menu;

    return win;
}
