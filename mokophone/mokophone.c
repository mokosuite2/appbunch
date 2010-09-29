#include <dbus/dbus-glib-bindings.h>
#include <string.h>

#include "phonewin.h"
#include "callwin.h"
#include "mokophone.h"
#include "mokophone-service-glue.h"

G_DEFINE_TYPE(MokoPhoneService, moko_phone_service, G_TYPE_OBJECT)

static void
moko_phone_service_class_init(MokoPhoneServiceClass * klass)
{
    GError *error = NULL;

    /* Init the DBus connection, per-klass */
    klass->connection = dbus_g_bus_get (DBUS_BUS_SYSTEM, &error);
    if (klass->connection == NULL) {
        g_error("Unable to connect to dbus: %s", error->message);
        g_error_free (error);
        return;
    }

    dbus_g_object_type_install_info (MOKO_TYPE_PHONE_SERVICE,
            &dbus_glib_moko_phone_object_info);
}

static void
moko_phone_service_init(MokoPhoneService * object)
{
    GError *error = NULL;
    DBusGProxy *driver_proxy;
    guint request_ret;

    MokoPhoneServiceClass *klass =
        MOKO_PHONE_SERVICE_GET_CLASS(object);

    /* Register DBUS path */
    dbus_g_connection_register_g_object(klass->connection,
            MOKO_PHONE_PATH,
            G_OBJECT (object));

    /* Register the service name, the constant here are defined in dbus-glib-bindings.h */
    driver_proxy = dbus_g_proxy_new_for_name (klass->connection,
            DBUS_SERVICE_DBUS,
            DBUS_PATH_DBUS,
            DBUS_INTERFACE_DBUS);

    if (!org_freedesktop_DBus_request_name (driver_proxy,
            MOKO_PHONE_NAME, 0, &request_ret, &error)) {
        g_error("Unable to register service: %s", error->message);
        g_error_free (error);
    }
    g_object_unref(driver_proxy);
}


MokoPhoneService *
moko_phone_service_new(void)
{
    return g_object_new(MOKO_TYPE_PHONE_SERVICE, NULL);
}

gboolean moko_phone_frontend(MokoPhoneService *object, const char *section, GError **error)
{
    // il telefono e' la sezione predefinita
    int n_section = SECTION_PHONE;

    if (!strcasecmp(section, "log")) n_section = SECTION_LOG;
    else if (!strcasecmp(section, "contacts")) n_section = SECTION_CONTACTS;
    else if (!strcasecmp(section, "calls")) {
        phone_call_win_activate();
        return TRUE;
    }

    // non e' piu' necessario attivare la callwin, ora abbiamo il panel con le chiamate in corso ;)
    phone_win_activate(n_section, FALSE /*(n_section == SECTION_PHONE)*/);
    return TRUE;
}
