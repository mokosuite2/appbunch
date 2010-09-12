#include <Elementary.h>
#include <glib.h>

#include <libmokosuite/mokosuite.h>
#include <freesmartphone-glib/ousaged/usage.h>

#define EGG_DBUS_I_KNOW_API_IS_SUBJECT_TO_CHANGE
#include <libmokosuite/bluetooth-client.h>
#include <libmokosuite/bluetooth-agent.h>
#include <libmokosuite/bluezheadset.h>

#include "menu-common.h"
#include "menu-bluetooth.h"

#include <glib/gi18n-lib.h>

#define BTMENU_ADDRESS(x)       ((char*)x->data1)
#define BTMENU_DEVICE(x)        ((bluezDevice*)x->data2)
#define BTMENU_DEVICE_PROPS(x)  ((EggDBusHashMap*)x->data3)

// finestra
static MokoWin* win = NULL;
static Evas_Object* menu = NULL;
static Evas_Object* bt_menu = NULL;

static MenuItem* sw_bluetooth = NULL;
static MenuItem* ex_bluetooth = NULL;
static MenuItem* sw_visible = NULL;
static MenuItem* sw_devname = NULL;

static gboolean fso_signals_attached = FALSE;

static bluezManager* bt_manager = NULL;
static bluezAdapter* bt_adapter = NULL;
static BluetoothAgentService* bt_agent = NULL;

static char* bt_devname = NULL;
static gboolean bt_discoverable = FALSE;
static gboolean bt_discovering = FALSE;
static GHashTable* bt_devices_found = NULL;

static void bt_dbus_connect(void);
static void bt_dbus_disconnect(void);
static void bt_discovery_start(void);
static void bt_device_click(gpointer data);

static void bt_devname_update(void)
{
    if (bt_devname == NULL)
        sw_devname->sublabel = _("(unknown)");
    else
        // TODO MEMORY LEAK!!!
        sw_devname->sublabel = bt_devname;

    elm_genlist_item_disabled_set(sw_devname->item, FALSE);
    elm_genlist_item_update(sw_devname->item);
}

static void bt_devname_disable(void)
{
    sw_devname->sublabel = _("Activate bluetooth first.");
    elm_genlist_item_disabled_set(sw_devname->item, TRUE);
    elm_genlist_item_update(sw_devname->item);
}

static void bt_visible_update(void)
{
    sw_visible->checked = bt_discoverable;

    elm_genlist_item_disabled_set(sw_visible->item, FALSE);
    elm_genlist_item_update(sw_visible->item);
}

static void bt_visible_disable(void)
{
    sw_visible->checked = FALSE;
    elm_genlist_item_disabled_set(sw_visible->item, TRUE);
    elm_genlist_item_update(sw_visible->item);
}

static void bt_toggle_activating(MenuItem* item)
{
    if (item == NULL) {
        if (sw_visible != NULL) {
            bt_visible_disable();
            bt_devname_disable();
        }

        if (sw_bluetooth != NULL)
            bt_toggle_activating(sw_bluetooth);

        if (ex_bluetooth != NULL)
            bt_toggle_activating(ex_bluetooth);

    } else {
        item->itc.item_style = "generic_sub";
        item->sublabel = _("Activating bluetooth...");
        elm_genlist_item_disabled_set(item->item, TRUE);
        elm_genlist_item_update(item->item);
    }
}

static void bt_toggle_activated(MenuItem* item)
{
    if (item == NULL) {
        if (sw_visible != NULL) {
            bt_visible_update();
            bt_devname_update();
        }

        if (sw_bluetooth != NULL)
            bt_toggle_activated(sw_bluetooth);

        if (ex_bluetooth != NULL)
            bt_toggle_activated(ex_bluetooth);

        if (bt_menu != NULL)
            mokowin_menu_set(win, bt_menu);

    } else {
        item->itc.item_style = "generic";
        item->checked = TRUE;
        item->sublabel = NULL;
        elm_genlist_item_disabled_set(item->item, FALSE);
        elm_genlist_item_update(item->item);
    }
}

static void bt_toggle_deactivating(MenuItem* item)
{
    if (item == NULL) {
        if (sw_visible != NULL) {
            bt_visible_disable();
            bt_devname_disable();
        }

        if (sw_bluetooth != NULL)
            bt_toggle_deactivating(sw_bluetooth);

        if (ex_bluetooth != NULL)
            bt_toggle_deactivating(ex_bluetooth);

        if (bt_menu != NULL)
            mokowin_menu_set(win, NULL);

    } else {
        item->itc.item_style = "generic_sub";
        item->sublabel = _("Deactivating bluetooth...");
        elm_genlist_item_disabled_set(item->item, TRUE);
        elm_genlist_item_update(item->item);
    }
}

static void bt_toggle_deactivated(MenuItem* item)
{
    if (item == NULL) {
        if (sw_visible != NULL) {
            bt_visible_disable();
            bt_devname_disable();
        }

        if (sw_bluetooth != NULL)
            bt_toggle_deactivated(sw_bluetooth);

        if (ex_bluetooth != NULL)
            bt_toggle_deactivated(ex_bluetooth);

        if (bt_menu != NULL)
            mokowin_menu_set(win, NULL);

    } else {
        item->itc.item_style = "generic_sub";
        item->checked = FALSE;
        item->sublabel = _("Activate bluetooth");
        elm_genlist_item_disabled_set(item->item, FALSE);
        elm_genlist_item_update(item->item);
    }
}

static void bt_adapter_changed(GObject* proxy, const char* adapter, gpointer data)
{
    g_debug("Default adapter changed: %s", adapter);
    bt_dbus_connect();
}

static void bt_adapter_removed(GObject* proxy, const char* adapter, gpointer data)
{
    g_debug("Adapter removed: %s", adapter);

    bt_adapter = NULL;
    bt_manager = NULL;
    bt_agent = NULL;

    bt_toggle_deactivated(NULL);
}

// distruttore MenuItem device trovato
static void bt_device_found_item_free(gpointer data)
{
    MenuItem* item = (MenuItem *)data;

    if (item->item)
        elm_genlist_item_del(item->item);

    g_free(item->label);
    g_free(item->data1);
    g_free(item);
}

static void bt_property_changed(GObject* proxy, const char* name, EggDBusVariant* value)
{
    g_debug("Property changed: %s", name);
    g_return_if_fail(value != NULL);

    if (!strcmp(name, "Discoverable")) {
        bt_discoverable = egg_dbus_variant_get_boolean(value);

        if (sw_visible != NULL)
            bt_visible_update();
    }

    else if (!strcmp(name, "Name")) {

        if (bt_devname != NULL)
            g_free(bt_devname);

        bt_devname = g_strdup(egg_dbus_variant_get_string(value));

        if (sw_visible != NULL)
            bt_devname_update();
    }

    else if (!strcmp(name, "Discovering")) {
        bt_discovering = egg_dbus_variant_get_boolean(value);
        g_debug("Discovering: %d", bt_discovering);

        if (bt_discovering) {

            if (bt_devices_found != NULL) {
                g_hash_table_destroy(bt_devices_found);
                bt_devices_found = NULL;
            }
        }
    }
}

static void bt_device_connected(bluezAdapter* adapter, bluezDevice* device, gpointer data, GError* error)
{
    g_return_if_fail(bt_adapter != NULL);
    g_debug("Connected to device! error = %p, device = %p", error, device);

    MenuItem* item = (MenuItem *)data;
    item->data2 = device;
    item->data3 = bluetooth_client_device_get_properties(device);

    bt_device_click(item);
}

static void bt_device_paired(bluezAdapter* adapter, bluezDevice* device, gpointer data, GError* error)
{
    g_return_if_fail(bt_adapter != NULL);
    g_debug("Paired to device! error = %p, device = %p", error, device);

    bluezHeadset* headset = BLUEZ_QUERY_INTERFACE_HEADSET(
        egg_dbus_interface_proxy_get_object_proxy(
            EGG_DBUS_INTERFACE_PROXY(device)
        )
    );

    if (headset != NULL) {
        GError* e = NULL;

        g_debug("Connecting to handset...");
        if (!bluez_headset_connect_sync(headset, 0, NULL, &e)) {
            g_debug("Error(connect): %s", e->message);
            g_error_free(e);
            e = NULL;
        }

        g_debug("Playing headset...");
        if (!bluez_headset_play_sync(headset, 0, NULL, &e)) {
            g_debug("Error(play): %s", e->message);
            g_error_free(e);
            return;
        }
    }
}

// callback click su device bluetooth
static void bt_device_click(gpointer data)
{
    g_return_if_fail(bt_adapter != NULL);

    MenuItem* item = (MenuItem *)data;

    // device gia' connesso
    if (item->data2 != NULL) {
        // TODO cosa facciamo?

        // se siamo gia' appaiati, connetti direttamente
        EggDBusVariant *paired = egg_dbus_hash_map_lookup(BTMENU_DEVICE_PROPS(item), "Paired");

        if (!egg_dbus_variant_get_boolean(paired)) {
            // TODO start pairing
            g_debug("Start pairing with %s", BTMENU_ADDRESS(item));
            if (!bluetooth_client_adapter_start_pairing(bt_adapter, "/org/mokosuite/Settings/BluezAgent",
                BTMENU_ADDRESS(item), bt_device_paired, item)) {
                // TODO visualizza errore
                g_warning("Error pairing to device %s", BTMENU_ADDRESS(item));
            }

        } else {
            // TODO connetti! ... ma a che? :S
            g_debug("Device already paired!");
            bt_device_paired(bt_adapter, BTMENU_DEVICE(item), item, NULL);
        }

        return;
    }

    // device inesistente, crealo
    if (!bluetooth_client_adapter_create_device(bt_adapter, BTMENU_ADDRESS(item), bt_device_connected, item)) {
        // TODO visualizza errore
        g_warning("Error connecting to device %s", BTMENU_ADDRESS(item));
    }
}

static void bt_device_found(GObject* proxy, const char* address, EggDBusHashMap* values)
{
    g_return_if_fail(menu != NULL);

    if (bt_devices_found == NULL)
        bt_devices_found = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, bt_device_found_item_free);        

    g_debug("Device found: %s", address);
    MenuItem* old = g_hash_table_lookup(bt_devices_found, address);
    //g_debug("Old device: %p", old);

    if (old == NULL) {
        MenuItem* item = g_new0(MenuItem, 1);
        item->label = g_strdup(address);
        item->sublabel = "(device class)";
        item->checkbox = FALSE;
        item->itc.item_style = "generic_sub";
        item->data1 = g_strdup(address);
        item->data2 = bluetooth_client_adapter_find_device(bt_adapter, address);
        if (item->data2 != NULL)
            item->data3 = bluetooth_client_device_get_properties(BTMENU_DEVICE(item));

        item->item_callback = bt_device_click;

        // aggiungi solo se la lista e' stata creata
        menu_window_item_add(menu, item);

        old = item;

        g_hash_table_insert(bt_devices_found, g_strdup(address), item);

    }

    // FIXME temp
    EggDBusVariant* v = egg_dbus_hash_map_lookup(values, "Class");
    if (v != NULL)
        g_debug("Device class: 0x%x", egg_dbus_variant_get_uint(v));
    // FIXME temp


    // aggiornamenti valori
    EggDBusVariant* value = egg_dbus_hash_map_lookup(values, "Name");
    if (value != NULL) {

        if (!strcmp(old->label, address))
            g_free(old->label);

        const char* name = egg_dbus_variant_get_string(value);

        if (strcmp(old->label, name)) {
            g_debug("Discovered name for %s: %s", address, name);
            old->label = g_strdup(name);

            elm_genlist_item_update(old->item);
        }
    }

}

static void bt_dbus_connect(void)
{
    if (bt_manager == NULL) {
        bt_manager = bluetooth_client_manager_connect();
        if (!bt_manager)
            bt_toggle_deactivated(NULL);

        g_signal_connect(bt_manager, "default-adapter-changed", (GCallback) bt_adapter_changed, NULL);
        g_signal_connect(bt_manager, "adapter-removed", (GCallback) bt_adapter_removed, NULL);
    }

    if (bt_adapter == NULL && bt_manager != NULL) {

        bt_adapter = bluetooth_client_manager_get_default_adapter(bt_manager);
        if (bt_adapter != NULL) {
            g_signal_connect(bt_adapter, "property-changed", (GCallback) bt_property_changed, NULL);
            g_signal_connect(bt_adapter, "device-found", (GCallback) bt_device_found, NULL);

            EggDBusHashMap* bt_props = bluetooth_client_adapter_get_properties(bt_adapter);

            if (bt_props != NULL) {
                // stato discoverable
                EggDBusVariant* v = egg_dbus_hash_map_lookup(bt_props, "Discoverable");
                if (v != NULL)
                    bt_property_changed(NULL, "Discoverable", v);

                // nome device
                v = egg_dbus_hash_map_lookup(bt_props, "Name");
                if (v != NULL)
                    bt_property_changed(NULL, "Name", v);

                v = egg_dbus_hash_map_lookup(bt_props, "Discovering");
                if (v != NULL)
                    bt_property_changed(NULL, "Discovering", v);

                g_object_unref(bt_props);
            }

            // TODO creazione agent
            bt_agent = bluetooth_agent_service_new();
            bluetooth_agent_service_setup(bt_agent, "/org/mokosuite/Settings/BluezAgent");
        }
    }

    // abbiamo gia' tutto :)
    if (bt_adapter != NULL) {
        bt_toggle_activated(NULL);

        if (menu != NULL)
            bt_discovery_start();
    }
}

static void bt_dbus_disconnect(void)
{
    if (bt_adapter == NULL && bt_manager == NULL)
        bt_toggle_deactivated(NULL);
}

// imposta il MenuItem da gestire
void bt_extra_item(MenuItem* item)
{
    ex_bluetooth = item;
}

void bt_policy_set(GError *e, gpointer data)
{
    if (e != NULL) {
        g_warning("Error setting policy for bluetooth: %s", e->message);

        bt_toggle_deactivated(NULL);
        //return;
    }
}

void bt_toggle(gpointer data)
{
    MenuItem* item = (MenuItem*)data;

    if (item->checked) {
        bt_toggle_deactivating(NULL);

        ousaged_usage_set_resource_policy(RESOURCE_BLUETOOTH, RESOURCE_POLICY_AUTO, bt_policy_set, NULL);

    } else {
        bt_toggle_activating(NULL);

        ousaged_usage_set_resource_policy(RESOURCE_BLUETOOTH, RESOURCE_POLICY_ENABLED, bt_policy_set, NULL);
    }
}

void bt_state(GError *e, gboolean state, gpointer data)
{
    g_debug("Bluetooth status: %d", state);

    if (state) {
        bt_toggle_activating(NULL);

        bt_dbus_connect();

    } else {
        bt_toggle_deactivating(NULL);

        bt_dbus_disconnect();
    }

}

static void bt_discovery_start(void)
{
    if (!bt_discovering) {
        g_debug("Starting device discovery");

        bluetooth_client_adapter_start_discovery(bt_adapter);
    }
}

static void bt_visible(gpointer data)
{
    bluetooth_client_adapter_set_discoverable(bt_adapter, !sw_visible->checked);

    bt_visible_update();
}

static void bt_set_devname(gpointer data)
{
    // TODO
}

static void bt_resource_changed (gpointer data, const char *name, gboolean state, GHashTable *attributes)
{
    if (!strcmp(name, RESOURCE_BLUETOOTH))
        bt_state(NULL, state, NULL);
}

static void bt_discover_clicked(void* data, Evas_Object* obj, void* event_info)
{
    mokowin_menu_hide(win);
    bt_discovery_start();
}

void menu_bluetooth_init_item(MenuItem* item, gboolean extra)
{
    if (extra) {
        ex_bluetooth = item;
    }

    if (!fso_signals_attached) {
        // handler segnali FSO
        ousaged_usage_resource_changed_connect(bt_resource_changed, NULL);
        //ousaged_usage_resource_available_connect(usageResourceAvailable, NULL);

        fso_signals_attached = TRUE;
    }

    ousaged_usage_get_resource_state(RESOURCE_BLUETOOTH, bt_state, NULL);
}

void menu_bluetooth_init(void)
{
    if (win != NULL) {
        mokowin_activate(win);
        return;
    }

    win = menu_window_new("mokosettings_bluetooth", _("Bluetooth settings"), TRUE, &menu);
    if (!win) {
        g_critical("Cannot create bluetooth menu window.");
        return;
    }

    mokowin_menu_enable(win);

    // menu bluetooth
    Evas_Object *bt_discover = elm_button_add(win->win);
    elm_button_label_set(bt_discover, _("Discover devices"));

    evas_object_size_hint_weight_set(bt_discover, EVAS_HINT_EXPAND, 0.0);
    evas_object_size_hint_align_set(bt_discover, EVAS_HINT_FILL, 1.0);

    evas_object_smart_callback_add(bt_discover, "clicked", bt_discover_clicked, NULL);

    bt_menu = bt_discover;

    // comincia con gli item
    MenuItem* item = NULL;

    //--//
    item = g_new0(MenuItem, 1);
    item->label = _("Bluetooth");
    item->sublabel = _("Activate bluetooth");
    item->checkbox = TRUE;
    item->itc.item_style = "generic_sub";
    item->item_callback = bt_toggle;

    menu_window_item_add(menu, item);
    sw_bluetooth = item;
    elm_genlist_item_disabled_set(sw_bluetooth->item, TRUE);

    //--//
    item = g_new0(MenuItem, 1);
    item->label = _("Visible");
    item->sublabel = _("Make the device visible to others");
    item->checkbox = TRUE;
    item->itc.item_style = "generic_sub";
    item->item_callback = bt_visible;

    menu_window_item_add(menu, item);
    sw_visible = item;
    elm_genlist_item_disabled_set(sw_visible->item, TRUE);

    //--//
    item = g_new0(MenuItem, 1);
    item->label = _("Device name");
    item->sublabel = _("Activate bluetooth first.");
    item->checkbox = FALSE;
    item->itc.item_style = "generic_sub";
    item->item_callback = bt_set_devname;

    menu_window_item_add(menu, item);
    sw_devname = item;
    elm_genlist_item_disabled_set(sw_devname->item, TRUE);


    // finito, inizializza lo switch bluetooth
    menu_bluetooth_init_item(sw_bluetooth, FALSE);

    mokowin_activate(win);
}
