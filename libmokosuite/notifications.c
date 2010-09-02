#include <glib.h>
#include <dbus/dbus-glib.h>

#include "mokosuite.h"
#include "notifications.h"
#include "notifications-glue.h"

static DBusGConnection* bus = NULL;

int moko_notifications_push(DBusGProxy* proxy, const char * text, const char * icon, int type, int flags, GError **error)
{
    int ret = -1;

    if (!org_mokosuite_Notifications_push_notification (proxy, text, icon, type, flags, &ret, error)) {
        g_warning("PushNotification: %s", (error != NULL && *error != NULL) ? (*error)->message : "unknown error");
        ret = -1;
    }

    return ret;
}

void moko_notifications_remove(DBusGProxy* proxy, int notification_id, GError **error)
{
    if (!org_mokosuite_Notifications_remove_notification (proxy, notification_id, error))
        g_warning("RemoveNotification: %s", (error != NULL && *error != NULL) ? (*error)->message : "unknown error");
}

DBusGProxy* moko_notifications_connect(const char* bus_name, const char* path)
{
    GError* error = NULL;
    bus = dbus_g_bus_get(DBUS_BUS_SYSTEM, &error);

    if (!bus) {
        g_critical("Couldn't connect to system bus (%s)", error->message);
        g_error_free(error);
        return NULL;
    }

    DBusGProxy* proxy = dbus_g_proxy_new_for_name(bus, bus_name, path, MOKO_NOTIFICATIONS_INTERFACE);
        if (proxy == NULL)
            g_warning("Couln't connect to the Mokosuite Notifications Interface");

    return proxy;
}
