#include <Elementary.h>
#include <glib.h>

#include <libmokosuite/mokosuite.h>
#include <libmokosuite/gui.h>
#include <libmokosuite/fso.h>

#include <frameworkd-glib/odeviced/frameworkd-glib-odeviced-dbus.h>
#include <frameworkd-glib/odeviced/frameworkd-glib-odeviced-display.h>

#include "menu-common.h"
#include "menu-sounds.h"
#include "mokosettings.h"

#include <glib/gi18n-lib.h>

// finestra
static MokoWin* win = NULL;
static Evas_Object* menu = NULL;

static void di_volume_changed(MokoPopupSlider* popup, MenuItem* item, int value, gboolean final)
{
    if (popup->changed) {
        char* val = g_strdup_printf("%d", value);
        menu_item_replace_setting(item, phone_settings, val, final);
        g_free(val);
    }
}

static void di_volume(gpointer data)
{
    MenuItem* item = (MenuItem*) data;
    MokoPopupSlider* p = moko_popup_slider_new(win, item->label, atoi((const char*)item->data4), di_volume_changed, data);
    mokoinwin_activate(MOKO_INWIN(p));
}

static void di_play_sound(gpointer data)
{
    menu_item_replace_setting((MenuItem *)data, phone_settings, NULL, TRUE);
}

static void di_vibrate(gpointer data)
{
    menu_item_replace_setting((MenuItem *)data, phone_settings, NULL, TRUE);
}

void menu_sounds_init(void)
{
    if (win != NULL) {
        mokowin_activate(win);
        return;
    }

    win = menu_window_new("mokosettings_sounds", _("Sounds and notifications"), TRUE, &menu);
    if (!win) {
        g_critical("Cannot create sounds menu window.");
        return;
    }

    MenuItem* item = NULL;

    //--//
    item = g_new0(MenuItem, 1);
    item->label = _("Play ringtone");
    item->sublabel = _("Retreiving settings...");
    item->itc.item_style = "generic_sub";
    item->checkbox = TRUE;
    item->list = menu;
    item->item_callback = di_play_sound;

    menu_window_item_add(menu, item);
    elm_genlist_item_disabled_set(item->item, TRUE);
    menu_item_assign_setting(item, phone_settings, "call_notification_sound", "false",
        _("Play ringtone on incoming call"),
        _("Don't play any sound on incoming call"));

    //--//
    item = g_new0(MenuItem, 1);
    item->label = _("Vibrate on call");
    item->sublabel = _("Retreiving settings...");
    item->itc.item_style = "generic_sub";
    item->checkbox = TRUE;
    item->list = menu;
    item->item_callback = di_vibrate;

    menu_window_item_add(menu, item);
    elm_genlist_item_disabled_set(item->item, TRUE);
    menu_item_assign_setting(item, phone_settings, "call_notification_vibration", "false",
        _("Vibrate on incoming call"),
        _("Don't vibrate on incoming call"));

    #if 0
    //--//
    item = g_new0(MenuItem, 1);
    item->label = _("In call sound settings");
    item->itc.item_style = "separator";
    item->list = menu;

    menu_window_item_add(menu, item);
    #endif

    //--//
    item = g_new0(MenuItem, 1);
    item->label = _("Handset volume");
    item->sublabel = _("Retreiving settings...");
    item->itc.item_style = "generic_sub";
    item->list = menu;
    item->item_callback = di_volume;

    menu_window_item_add(menu, item);
    elm_genlist_item_disabled_set(item->item, TRUE);
    menu_item_assign_setting(item, phone_settings, "phone_handset_volume", NULL, _("%s%%"), NULL);

    //--//
    item = g_new0(MenuItem, 1);
    item->label = _("Speaker volume");
    item->sublabel = _("Retreiving settings...");
    item->itc.item_style = "generic_sub";
    item->list = menu;
    item->item_callback = di_volume;

    menu_window_item_add(menu, item);
    elm_genlist_item_disabled_set(item->item, TRUE);
    menu_item_assign_setting(item, phone_settings, "phone_speaker_volume", NULL, _("%s%%"), NULL);

    //--//
    item = g_new0(MenuItem, 1);
    item->label = _("Headset volume");
    item->sublabel = _("Retreiving settings...");
    item->itc.item_style = "generic_sub";
    item->list = menu;
    item->item_callback = di_volume;

    menu_window_item_add(menu, item);
    elm_genlist_item_disabled_set(item->item, TRUE);
    menu_item_assign_setting(item, phone_settings, "phone_headset_volume", NULL, _("%s%%"), NULL);

    // fine dei menu!!!
    mokowin_activate(win);
}
