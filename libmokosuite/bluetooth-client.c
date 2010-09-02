#include <glib.h>
#include <dbus/dbus-glib.h>

#include <eggdbus/eggdbus.h>

#include "bluezbindings.h"
#include "bluetooth-agent.h"
#include "bluetooth-client.h"

#define BLUEZ_SERVICE               "org.bluez"
#define BLUEZ_MANAGER_PATH          "/"
#define BLUEZ_MANAGER_INTERFACE     BLUEZ_SERVICE ".Manager"
#define BLUEZ_ADAPTER_INTERFACE     BLUEZ_SERVICE ".Adapter"

typedef struct {
    gpointer callback;
    gpointer data;
} CallbackData;

static EggDBusConnection* bus = NULL;

bluezManager* bluetooth_client_manager_connect(void)
{
    GError* error = NULL;
    if (!bus)
        bus = egg_dbus_connection_get_for_bus (EGG_DBUS_BUS_TYPE_SYSTEM);

    if (!bus) {
        g_critical("Couldn't connect to system bus (%s)", error->message);
        g_error_free(error);
        return NULL;
    }

    EggDBusObjectProxy* proxy = egg_dbus_connection_get_object_proxy(bus, BLUEZ_SERVICE, BLUEZ_MANAGER_PATH);
    if (proxy == NULL)
        g_warning("Couln't connect to bluetooth manager interface");

    return BLUEZ_QUERY_INTERFACE_MANAGER(proxy);
}

bluezAdapter* bluetooth_client_manager_get_default_adapter(bluezManager* manager)
{
    g_return_val_if_fail(bus != NULL, NULL);
    char* adapter = NULL;
    GError* e = NULL;

    if (!bluez_manager_default_adapter_sync (manager, EGG_DBUS_CALL_FLAGS_NONE, &adapter, NULL, &e)) {
        g_warning("Default adapter not found: %s", e->message);
        g_error_free(e);
        return NULL;
    }

    EggDBusObjectProxy* proxy = egg_dbus_connection_get_object_proxy(bus, BLUEZ_SERVICE, adapter);
    if (proxy == NULL)
        g_warning("Couln't connect to bluetooth adapter %s interface", adapter);

    g_free(adapter);
    return BLUEZ_QUERY_INTERFACE_ADAPTER(proxy);
}

char* bluetooth_client_adapter_get_name(bluezAdapter* adapter)
{
    g_return_val_if_fail(bus != NULL, NULL);

    return bluez_adapter_get_name(adapter);
}

EggDBusHashMap* bluetooth_client_adapter_get_properties(bluezAdapter* adapter)
{
    g_return_val_if_fail(bus != NULL, NULL);
    EggDBusHashMap* props = NULL;
    GError* e = NULL;

    if (!bluez_adapter_get_properties_sync(adapter, EGG_DBUS_CALL_FLAGS_NONE, &props, NULL, &e)) {
        g_warning("Unable to retrieve adapter properties: %s", e->message);
        g_error_free(e);
        return NULL;
    }

    return props;
}

gboolean bluetooth_client_adapter_set_discoverable(bluezAdapter* adapter, gboolean discoverable)
{
    g_return_val_if_fail(bus != NULL, FALSE);
    EggDBusVariant *flag;
    GError* e = NULL;

    flag = egg_dbus_variant_new_for_boolean(discoverable);

    if (!bluez_adapter_set_property_sync(adapter, EGG_DBUS_CALL_FLAGS_NONE, "Discoverable", flag, NULL, &e)) {
        g_warning("Unable to retrieve adapter properties: %s", e->message);
        g_error_free(e);
        return FALSE;
    }

    g_object_unref(flag);
    return TRUE;
}

gboolean bluetooth_client_adapter_is_discovering(bluezAdapter* adapter)
{
    g_return_val_if_fail(bus != NULL, FALSE);
    EggDBusVariant *flag = NULL;
    GError* e = NULL;

    if (!bluez_adapter_get_property_sync(adapter, EGG_DBUS_CALL_FLAGS_NONE, "Discovering", &flag, NULL, &e)) {
        g_warning("Unable to retrieve adapter discovery status: %s", e->message);
        g_error_free(e);
        return FALSE;
    }

    return egg_dbus_variant_get_boolean(flag);
}

gboolean bluetooth_client_adapter_start_discovery(bluezAdapter* adapter)
{
    g_return_val_if_fail(bus != NULL, FALSE);
    GError* e = NULL;

    if (!bluez_adapter_start_discovery_sync(adapter, EGG_DBUS_CALL_FLAGS_NONE, NULL, &e)) {
        g_warning("Unable to start device discovery: %s", e->message);
        g_error_free(e);
        return FALSE;
    }

    return TRUE;
}

static void _adapter_create_device(GObject* object, GAsyncResult* res, gpointer data)
{
    GError* e = NULL;
    char* path = NULL;
    bluezDevice* device = NULL;
    CallbackData* dt = (CallbackData*)data;

    void (*callback)(bluezAdapter* , bluezDevice* , gpointer, GError*) = NULL;
    callback = dt->callback;

    if (!bluez_adapter_create_device_finish (BLUEZ_ADAPTER(object), &path, res, &e)) {
        g_warning("Create device failed: %s", e->message);
    } else {
        // recupera il device
        EggDBusObjectProxy* proxy = egg_dbus_connection_get_object_proxy(bus, BLUEZ_SERVICE, path);
        if (proxy == NULL) {
            g_warning("Couln't connect to bluetooth device %s interface", path);
            e = g_error_new(G_IO_CHANNEL_ERROR, 0, "Cannot connect to device D-Bus interface");

        } else {
            device = BLUEZ_QUERY_INTERFACE_DEVICE(proxy);
        }
    }

    // richiama il callback!!!
    callback(BLUEZ_ADAPTER(object), device, dt->data, e);

    if (e != NULL)
        g_error_free(e);

    if (path != NULL)
        g_free(path);

    g_free(dt);
}

gboolean bluetooth_client_adapter_create_device(bluezAdapter* adapter, const char* address,
    void (*callback)(bluezAdapter* , bluezDevice* , gpointer, GError*), gpointer data)
{
    CallbackData* dt = g_new(CallbackData, 1);
    dt->callback = callback;
    dt->data = data;

    if (bluez_adapter_create_device(adapter, EGG_DBUS_CALL_FLAGS_NONE, address, NULL, _adapter_create_device, dt) <= 0) {
        g_warning("Cannot create device for %s", address);
        g_free(dt);
        return FALSE;
    }

    return TRUE;
}

gboolean bluetooth_client_adapter_start_pairing(bluezAdapter* adapter, const char* agent_path, const char* address,
    void (*callback)(bluezAdapter* , bluezDevice* , gpointer, GError*), gpointer data)
{
    CallbackData* dt = g_new(CallbackData, 1);
    dt->callback = callback;
    dt->data = data;

    if (bluez_adapter_create_paired_device(adapter, EGG_DBUS_CALL_FLAGS_NONE,
        address, agent_path, NULL, NULL, _adapter_create_device, dt) <= 0) {

        g_warning("Cannot start pairing for %s", address);
        g_free(dt);
        return FALSE;
    }

    return TRUE;
}

bluezDevice* bluetooth_client_adapter_find_device(bluezAdapter* adapter, const char* address)
{
    g_return_val_if_fail(bus != NULL, NULL);
    char* device = NULL;
    GError* e = NULL;

    if (!bluez_adapter_find_device_sync (adapter, EGG_DBUS_CALL_FLAGS_NONE, address, &device, NULL, &e)) {
        g_warning("Device not found: %s", e->message);
        g_error_free(e);
        return NULL;
    }

    EggDBusObjectProxy* proxy = egg_dbus_connection_get_object_proxy(bus, BLUEZ_SERVICE, device);
    if (proxy == NULL)
        g_warning("Couln't connect to bluetooth device %s interface", device);

    g_free(device);
    return BLUEZ_QUERY_INTERFACE_DEVICE(proxy);
}

gboolean bluetooth_client_device_is_paired(bluezDevice* device)
{
    g_return_val_if_fail(bus != NULL, FALSE);
    EggDBusVariant *flag = NULL;
    GError* e = NULL;

    if (!bluez_device_get_property_sync(device, EGG_DBUS_CALL_FLAGS_NONE, "Paired", &flag, NULL, &e)) {
        g_warning("Unable to retrieve device pairing status: %s", e->message);
        g_error_free(e);
        return FALSE;
    }

    return egg_dbus_variant_get_boolean(flag);
}

EggDBusHashMap* bluetooth_client_device_get_properties(bluezDevice* device)
{
    g_return_val_if_fail(bus != NULL, NULL);
    EggDBusHashMap* props = NULL;
    GError* e = NULL;

    if (!bluez_device_get_properties_sync(device, EGG_DBUS_CALL_FLAGS_NONE, &props, NULL, &e)) {
        g_warning("Unable to retrieve device properties: %s", e->message);
        g_error_free(e);
        return NULL;
    }

    return props;
}
