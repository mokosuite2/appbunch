
#include "mokosuite.h"

#include <string.h>
#include "bluetooth-agent.h"
#include "bluezbindings.h"

#define BLUEZ_SERVICE   "org.bluez"

#define BLUEZ_MANAGER_PATH  "/"
#define BLUEZ_MANAGER_INTERFACE "org.bluez.Manager"
#define BLUEZ_DEVICE_INTERFACE  "org.bluez.Device"

static EggDBusConnection *connection = NULL;

#define BLUETOOTH_AGENT_SERVICE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), \
                BLUETOOTH_TYPE_AGENT_SERVICE, BluetoothAgentServicePrivate))

typedef struct _BluetoothAgentServicePrivate BluetoothAgentServicePrivate;

struct _BluetoothAgentServicePrivate {
    gchar *busname;
    gchar *path;
    bluezAdapter *adapter;

    BluetoothAgentPasskeyFunc pincode_func;
    gpointer pincode_data;

    BluetoothAgentDisplayFunc display_func;
    gpointer display_data;

    BluetoothAgentPasskeyFunc passkey_func;
    gpointer passkey_data;

    BluetoothAgentConfirmFunc confirm_func;
    gpointer confirm_data;

    BluetoothAgentAuthorizeFunc authorize_func;
    gpointer authorize_data;

    BluetoothAgentCancelFunc cancel_func;
    gpointer cancel_data;
};

static void bluetooth_agent_service_iface_init (bluezAgentIface *iface);

G_DEFINE_TYPE_WITH_CODE (BluetoothAgentService, bluetooth_agent_service, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (BLUEZ_TYPE_AGENT,
                                                bluetooth_agent_service_iface_init)
                         );


static void handle_request_pin_code(bluezAgent *instance, const gchar *device, EggDBusMethodInvocation *method_invocation)
{
    // TODO
    g_debug("PIN code requested! Sending 0000");
    bluez_agent_handle_request_pin_code_finish(method_invocation, "0000");
}

static void handle_request_passkey(bluezAgent *instance, const gchar *device, EggDBusMethodInvocation *method_invocation)
{
    // TODO
    g_debug("Passkey requested!");
}

static void handle_request_confirmation(bluezAgent *instance, const gchar *device, guint passkey, EggDBusMethodInvocation *method_invocation)
{
    // TODO
    g_debug("Request confirmation!");
}

static void handle_authorize(bluezAgent *instance, const gchar *device, const gchar *uuid, EggDBusMethodInvocation *method_invocation)
{
    // TODO
    g_debug("Authorize!");
}

static void handle_cancel(bluezAgent *instance, EggDBusMethodInvocation *method_invocation)
{
    // TODO
    g_debug("Canceled.");
}

static void handle_release(bluezAgent *instance, EggDBusMethodInvocation *method_invocation)
{
    // TODO
    g_debug("Released!");
}


static void bluetooth_agent_service_init (BluetoothAgentService *object)
{
    BluetoothAgentServicePrivate *priv;

    priv = BLUETOOTH_AGENT_SERVICE_GET_PRIVATE (object);
    // TODO
}

static void bluetooth_agent_service_finalize (GObject *object)
{
    BluetoothAgentService *agent;
    BluetoothAgentServicePrivate *priv;

    agent = BLUETOOTH_AGENT_SERVICE (object);
    priv = BLUETOOTH_AGENT_SERVICE_GET_PRIVATE (agent);

    // TODO
}

static void
bluetooth_agent_service_class_init (BluetoothAgentServiceClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    gobject_class->finalize     = bluetooth_agent_service_finalize;

    connection = egg_dbus_connection_get_for_bus(EGG_DBUS_BUS_TYPE_SYSTEM);

    g_type_class_add_private (klass, sizeof (BluetoothAgentServicePrivate));
}

#if 0
gboolean bluetooth_agent_register(BluetoothAgent *agent, DBusGProxy *adapter)
{
    BluetoothAgentPrivate *priv = BLUETOOTH_AGENT_GET_PRIVATE(agent);
    DBusGProxy *proxy;
    GObject *object;
    GError *error = NULL;
    gchar *path;

    DBG("agent %p", agent);

    if (priv->adapter != NULL)
        return FALSE;

    priv->adapter = g_object_ref(adapter);

    path = g_path_get_basename(dbus_g_proxy_get_path(adapter));

    priv->path = g_strdup_printf("/org/bluez/agent/%s", path);

    g_free(path);

    proxy = dbus_g_proxy_new_for_name_owner(connection,
            dbus_g_proxy_get_bus_name(priv->adapter),
            dbus_g_proxy_get_path(priv->adapter),
            dbus_g_proxy_get_interface(priv->adapter), NULL);

    g_free(priv->busname);

    if (proxy != NULL) {
        priv->busname = g_strdup(dbus_g_proxy_get_bus_name(proxy));
        g_object_unref(proxy);
    } else
        priv->busname = g_strdup(dbus_g_proxy_get_bus_name(adapter));

    object = dbus_g_connection_lookup_g_object(connection, priv->path);
    if (object != NULL)
        g_object_unref(object);

    dbus_g_connection_register_g_object(connection,
                        priv->path, G_OBJECT(agent));

    dbus_g_proxy_call(priv->adapter, "RegisterAgent", &error,
                    DBUS_TYPE_G_OBJECT_PATH, priv->path,
                    G_TYPE_STRING, "DisplayYesNo",
                    G_TYPE_INVALID, G_TYPE_INVALID);

    if (error != NULL) {
        g_printerr("Agent registration failed: %s\n",
                            error->message);
        g_error_free(error);
    }

    return TRUE;
}

gboolean bluetooth_agent_service_unregister(BluetoothAgentService *agent)
{
    BluetoothAgentServicePrivate *priv = BLUETOOTH_AGENT_SERVICE_GET_PRIVATE(agent);
    GError *error = NULL;

    if (priv->adapter == NULL)
        return FALSE;

    dbus_g_proxy_call(priv->adapter, "UnregisterAgent", &error,
                    DBUS_TYPE_G_OBJECT_PATH, priv->path,
                    G_TYPE_INVALID, G_TYPE_INVALID);

    if (error != NULL) {
        g_printerr("Agent unregistration failed: %s\n",
                            error->message);
        g_error_free(error);
    }

    g_object_unref(priv->adapter);
    priv->adapter = NULL;

    g_free(priv->path);
    priv->path = NULL;

    return TRUE;
}
#endif

gboolean bluetooth_agent_service_setup(BluetoothAgentService *agent, const char *path)
{
    BluetoothAgentServicePrivate *priv = BLUETOOTH_AGENT_SERVICE_GET_PRIVATE(agent);
    EggDBusObjectProxy *proxy;

    if (priv->path != NULL)
        return FALSE;

    priv->path = g_strdup(path);

    proxy =  egg_dbus_connection_get_object_proxy(connection, BLUEZ_SERVICE, BLUEZ_MANAGER_PATH);

    g_free(priv->busname);

    if (proxy != NULL) {
        priv->busname = g_strdup(egg_dbus_object_proxy_get_name(proxy));
        g_object_unref(proxy);
    } else
        priv->busname = NULL;

    guint count = egg_dbus_connection_lookup_interface(connection, priv->path, NULL, NULL);
    g_debug("BluezAgent registered interfaces: %u", count);

    egg_dbus_connection_register_interface(connection, priv->path,
        BLUEZ_TYPE_AGENT, agent, G_TYPE_INVALID);

    return TRUE;
}

BluetoothAgentService* bluetooth_agent_service_new (void)
{
  return BLUETOOTH_AGENT_SERVICE (g_object_new (BLUETOOTH_TYPE_AGENT_SERVICE, NULL));
}

static void
bluetooth_agent_service_iface_init (bluezAgentIface *iface)
{
    iface->handle_request_pin_code = handle_request_pin_code;
    iface->handle_request_passkey = handle_request_passkey;
    iface->handle_request_confirmation = handle_request_confirmation;
    iface->handle_authorize = handle_authorize;
    iface->handle_cancel = handle_cancel;
    iface->handle_release = handle_release;

    // TODO
    iface->handle_display_passkey = NULL;
    iface->handle_confirm_mode = NULL;
}
