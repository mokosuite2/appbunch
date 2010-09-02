#ifndef __MENU_COMMON_H
#define __MENU_COMMON_H

#include <libmokosuite/mokosuite.h>
#include <libmokosuite/gui.h>
#include <Elementary.h>
#include <glib.h>

struct _MenuItem {
    char* label;
    char* sublabel;

    gboolean checkbox;
    Eina_Bool checked;

    Evas_Object* list;
    Elm_Genlist_Item* item;
    Elm_Genlist_Item_Class itc;

    void (*item_callback)(gpointer);
    gpointer data1;
    gpointer data2;
    gpointer data3;
    gpointer data4;
};

typedef struct _MenuItem MenuItem;

void menu_item_replace_setting(MenuItem* item, DBusGProxy* proxy, const char* new_value, gboolean update_item);
void menu_item_assign_setting(MenuItem* item, DBusGProxy* proxy, const char* name, const char* default_val, const char* fmt, const char* fmt2);

void menu_window_item_add(Evas_Object* menu, MenuItem* item);
void menu_window_item_add_array(Evas_Object* menu, MenuItem** menu_list);

MokoWin* menu_window_new(const char* name, const char* title, gboolean child, Evas_Object** menu_ret);

#endif  /* __MENU_COMMON_H */
