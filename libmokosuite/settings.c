#include <glib.h>
#include <dbus/dbus-glib.h>

#include "mokosuite.h"
#include "settings-service.h"
#include "settings.h"
#include "settings-glue.h"

static DBusGConnection* bus = NULL;

void moko_settings_get_setting_async(DBusGProxy* proxy, const char* key, const char* default_val, void (*callback)
    (GError *, const char* ret_value, gpointer userdata), gpointer userdata)
{
    // TODO
}

void moko_settings_set_setting_async(DBusGProxy* proxy, const char* key, const char* value, void (*callback)
    (GError *, gpointer userdata), gpointer userdata)
{
    // TODO
}

char* moko_settings_get_setting(DBusGProxy* proxy, const char* key, const char* default_val, GError** error)
{
    char* ret = NULL;

    if (!org_mokosuite_Settings_get_setting (proxy, key, default_val, &ret, error)) {
        g_warning("GetSetting: %s", (error != NULL && *error != NULL) ? (*error)->message : "unknown error");
        ret = NULL;
    }

    return ret;
}

gboolean moko_settings_set_setting(DBusGProxy* proxy, const char* key, const char* value, GError** error)
{
    gboolean ret = FALSE;

    if (!(ret = org_mokosuite_Settings_set_setting (proxy, key, value, error))) {
        g_warning("SetSetting: %s", (error != NULL && *error != NULL) ? (*error)->message : "unknown error");
        ret = FALSE;
    }

    return ret;
}


DBusGProxy* moko_settings_connect(const char* bus_name, const char* path)
{
    GError* error = NULL;
    bus = dbus_g_bus_get(DBUS_BUS_SYSTEM, &error);

    if (!bus) {
        g_critical("Couldn't connect to system bus (%s)", error->message);
        g_error_free(error);
        return NULL;
    }

    DBusGProxy* proxy = dbus_g_proxy_new_for_name(bus, bus_name, path, MOKO_SETTINGS_INTERFACE);
        if (proxy == NULL)
            g_warning("Couln't connect to the Mokosuite Remote Settings Interface");

    return proxy;
}
