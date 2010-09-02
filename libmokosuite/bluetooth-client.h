#ifndef __BLUETOOTH_CLIENT_H
#define __BLUETOOTH_CLIENT_H

#include <glib.h>
#include <dbus/dbus-glib.h>
#include <eggdbus/eggdbus.h>
#include "bluezbindings.h"
#include "bluetooth-agent.h"

bluezManager* bluetooth_client_manager_connect(void);

bluezAdapter* bluetooth_client_manager_get_default_adapter(bluezManager* manager);

char* bluetooth_client_adapter_get_name(bluezAdapter* adapter);

EggDBusHashMap* bluetooth_client_adapter_get_properties(bluezAdapter* adapter);

gboolean bluetooth_client_adapter_set_discoverable(bluezAdapter* adapter, gboolean discoverable);

gboolean bluetooth_client_adapter_is_discovering(bluezAdapter* adapter);

gboolean bluetooth_client_adapter_start_discovery(bluezAdapter* adapter);

gboolean bluetooth_client_adapter_create_device(bluezAdapter* adapter, const char* address,
    void (*callback)(bluezAdapter* , bluezDevice* , gpointer, GError*), gpointer data);

gboolean bluetooth_client_adapter_start_pairing(bluezAdapter* adapter, const char* agent_path, const char* address,
    void (*callback)(bluezAdapter* , bluezDevice* , gpointer, GError*), gpointer data);

bluezDevice* bluetooth_client_adapter_find_device(bluezAdapter* adapter, const char* address);

gboolean bluetooth_client_device_is_paired(bluezDevice* device);

EggDBusHashMap* bluetooth_client_device_get_properties(bluezDevice* device);

#endif  /* __BLUETOOTH_CLIENT_H */
