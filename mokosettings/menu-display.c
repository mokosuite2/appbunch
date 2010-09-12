#include <Elementary.h>
#include <glib.h>

#include <libmokosuite/mokosuite.h>
#include <libmokosuite/gui.h>

#include <freesmartphone-glib/odeviced/display.h>

#include "menu-common.h"
#include "menu-display.h"
#include "mokosettings.h"

#include <glib/gi18n-lib.h>

// valori di default
#define DEFAULT_BACKLIGHT       "90"
#define DEFAULT_IDLE_TIME       "30"
#define DEFAULT_DIM_ON_USB      "false"

// finestra
static MokoWin* win = NULL;
static Evas_Object* menu = NULL;

static void di_backlight_changed(MokoPopupSlider* popup, MenuItem* item, int value, gboolean final)
{
    g_debug("Backlight changed=%d, cb_value=%d, value=%d, orig_value=%d, final=%d",
        popup->changed, value, popup->value, popup->orig_value, final);

    if (popup->changed) {

        if (final && popup->value != popup->orig_value) {
            char* val = g_strdup_printf("%d", value);
            menu_item_replace_setting(item, panel_settings, val, final);
            g_free(val);

        } else {
            // imposta la backlight al volo :)
            odeviced_display_set_brightness(value, NULL, NULL);
        }
    } else {
        odeviced_display_set_brightness(popup->orig_value, NULL, NULL);
    }
}

static void di_backlight(gpointer data)
{
    MenuItem* item = (MenuItem*) data;
    MokoPopupSlider* p = moko_popup_slider_new(win, item->label, atoi((const char*)item->data4), di_backlight_changed, data);
    mokoinwin_activate(MOKO_INWIN(p));
}

static void idle_time_click(MokoPopupMenu* popup, MenuItem* item, int index, gboolean final)
{
    if (final && popup->orig_index != index) {
        char* val = g_strdup_printf("%d", index);
        menu_item_replace_setting(item, panel_settings, val, final);
        g_free(val);
    }
}

static void di_idle_time(gpointer data)
{
    MenuItem* item = (MenuItem*) data;

    MokoPopupMenu *p = moko_popup_menu_new(win, NULL, MOKO_POPUP_CHECKS_OK, idle_time_click, item);

    int value = atoi((const char*)item->data4);

    // bei pulsantini :D
    moko_popup_menu_add(p, _("5 seconds"), 10, (value == 10));
    moko_popup_menu_add(p, _("15 seconds"), 15, (value == 15));
    moko_popup_menu_add(p, _("30 seconds"), 30, (value == 30));
    moko_popup_menu_add(p, _("1 minute"), 60, (value == 60));
    moko_popup_menu_add(p, _("5 minutes"), 300, (value == 300));
    //moko_popup_menu_add(p, _("10 minutes"), 600, (value == 600));
    //moko_popup_menu_add(p, _("20 minutes"), 1200, (value == 1200));
    moko_popup_menu_add(p, _("Never"), -1, (value == -1));

    mokoinwin_activate(MOKO_INWIN(p));
}

static void get_brightness(MenuItem* item)
{
    menu_item_assign_setting(item, panel_settings, "display_brightness", DEFAULT_BACKLIGHT, _("%s%%"), NULL);
}

static void get_dim_on_usb(MenuItem* item)
{
    menu_item_assign_setting(item, panel_settings, "display_dim_usb", DEFAULT_DIM_ON_USB,
        _("Dim display even with connected USB"),
        _("Don't dim display when USB is connected"));
}

static void set_dim_on_usb(gpointer data)
{
    menu_item_replace_setting((MenuItem *)data, panel_settings, NULL, TRUE);
}

static void get_idle_time(MenuItem* item)
{
    menu_item_assign_setting(item, panel_settings, "display_idle_time", DEFAULT_IDLE_TIME, _("%s seconds"), NULL);
}

void menu_display_init(void)
{
    if (win != NULL) {
        mokowin_activate(win);
        return;
    }

    win = menu_window_new("mokosettings_display", _("Display settings"), TRUE, &menu);
    if (!win) {
        g_critical("Cannot create display menu window.");
        return;
    }

    MenuItem* item = NULL;

    //--//
    item = g_new0(MenuItem, 1);
    item->label = _("Backlight");
    item->sublabel = _("Retreiving settings...");
    item->itc.item_style = "generic_sub";
    item->list = menu;
    item->item_callback = di_backlight;

    menu_window_item_add(menu, item);
    elm_genlist_item_disabled_set(item->item, TRUE);
    get_brightness(item);

    //--//
    item = g_new0(MenuItem, 1);
    item->label = _("Dim on USB");
    item->sublabel = _("Retreiving settings...");
    item->itc.item_style = "generic_sub";
    item->checkbox = TRUE;
    item->list = menu;
    item->item_callback = set_dim_on_usb;

    menu_window_item_add(menu, item);
    elm_genlist_item_disabled_set(item->item, TRUE);
    get_dim_on_usb(item);

    //--//
    item = g_new0(MenuItem, 1);
    item->label = _("Display idle time");
    item->sublabel = _("Retreiving settings...");
    item->itc.item_style = "generic_sub";
    item->list = menu;
    item->item_callback = di_idle_time;

    menu_window_item_add(menu, item);
    elm_genlist_item_disabled_set(item->item, TRUE);
    get_idle_time(item);

    // fine dei menu!!!
    mokowin_activate(win);
}
