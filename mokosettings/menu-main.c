#include <Elementary.h>
#include <glib.h>

#include <libmokosuite/mokosuite.h>
#include <libmokosuite/gui.h>

#include "menu-common.h"
#include "menu-main.h"
#include "menu-network.h"
#include "menu-sounds.h"
#include "menu-display.h"

#include <glib/gi18n-lib.h>

// finestra
static MokoWin* win = NULL;
static Evas_Object* menu = NULL;

static void menu_network(gpointer data)
{
    menu_network_init();
}

static void menu_sounds(gpointer data)
{
    menu_sounds_init();
}

static void menu_display(gpointer data)
{
    menu_display_init();
}

void menu_main_init(void)
{
    win = menu_window_new("mokosettings_main", _("Settings"), FALSE, &menu);
    if (!win) {
        g_error("Cannot create main menu window. Exiting.");
        return; // mai raggiunto...
    }

    MenuItem* item = NULL;

    //--//
    item = g_new0(MenuItem, 1);
    item->label = _("Connections and network");
    item->sublabel = _("Manage network devices and connections");
    item->itc.item_style = "generic_sub";
    item->list = menu;
    item->item_callback = menu_network;

    menu_window_item_add(menu, item);

    //--//
    item = g_new0(MenuItem, 1);
    item->label = _("Call settings");
    item->sublabel = _("Call forwarding, call divert, call log...");
    item->itc.item_style = "generic_sub";
    item->list = menu;
    //item->item_callback = menu_network;

    menu_window_item_add(menu, item);

    //--//
    item = g_new0(MenuItem, 1);
    item->label = _("Sounds and notifications");
    item->sublabel = _("Manage notifications");
    item->itc.item_style = "generic_sub";
    item->list = menu;
    item->item_callback = menu_sounds;

    menu_window_item_add(menu, item);

    //--//
    item = g_new0(MenuItem, 1);
    item->label = _("Display settings");
    item->sublabel = _("Backlight and display idle time");
    item->itc.item_style = "generic_sub";
    item->list = menu;
    item->item_callback = menu_display;

    menu_window_item_add(menu, item);

    mokowin_activate(win);
}
