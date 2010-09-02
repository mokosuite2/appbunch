#include <Elementary.h>
#include <glib.h>

#include <libmokosuite/mokosuite.h>
#include <libmokosuite/gui.h>
#include <libmokosuite/fso.h>

#include "menu-common.h"
#include "menu-network.h"
#include "menu-bluetooth.h"

#include <glib/gi18n-lib.h>

// finestra
static MokoWin* win = NULL;
static Evas_Object* menu = NULL;

static MenuItem* sw_bluetooth = NULL;
static MenuItem* sw_wifi = NULL;

void bt_toggle(gpointer data);

void bt_settings(gpointer data)
{
    menu_bluetooth_init();
}

void wi_settings(gpointer data)
{
    // TODO menu_wireless_init();
}

void menu_network_init(void)
{
    if (win != NULL) {
        mokowin_activate(win);
        return;
    }

    win = menu_window_new("mokosettings_network", _("Connections and network"), TRUE, &menu);
    if (!win) {
        g_critical("Cannot create network menu window.");
        return;
    }

    MenuItem* item = NULL;

    //--//
    item = g_new0(MenuItem, 1);
    item->label = _("WiFi");
    item->sublabel = _("Activate wireless");
    item->checkbox = TRUE;
    item->itc.item_style = "generic_sub";
    item->list = menu;
    //item->item_callback = wi_toggle;

    menu_window_item_add(menu, item);
    sw_wifi = item;
    elm_genlist_item_disabled_set(sw_wifi->item, TRUE);
    // TODO menu_wireless_init_item(sw_wifi, TRUE);

    //--//
    item = g_new0(MenuItem, 1);
    item->label = _("Wireless settings");
    item->sublabel = NULL;
    item->checkbox = FALSE;
    item->itc.item_style = "generic";
    item->list = menu;
    item->item_callback = wi_settings;

    menu_window_item_add(menu, item);

    //--//
    item = g_new0(MenuItem, 1);
    item->label = _("Bluetooth");
    item->sublabel = _("Activate bluetooth");
    item->checkbox = TRUE;
    item->itc.item_style = "generic_sub";
    item->list = menu;
    item->item_callback = bt_toggle;

    menu_window_item_add(menu, item);
    sw_bluetooth = item;
    elm_genlist_item_disabled_set(sw_bluetooth->item, TRUE);
    menu_bluetooth_init_item(sw_bluetooth, TRUE);

    //--//
    item = g_new0(MenuItem, 1);
    item->label = _("Bluetooth settings");
    item->sublabel = NULL;
    item->checkbox = FALSE;
    item->itc.item_style = "generic";
    item->list = menu;
    item->item_callback = bt_settings;

    menu_window_item_add(menu, item);

    // fine dei menu!!!
    mokowin_activate(win);
}
