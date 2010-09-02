/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2005-2008  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef __BLUETOOTH_AGENT_H
#define __BLUETOOTH_AGENT_H

#include <glib-object.h>
#include <dbus/dbus-glib.h>

G_BEGIN_DECLS

#define BLUETOOTH_TYPE_AGENT_SERVICE (bluetooth_agent_service_get_type())
#define BLUETOOTH_AGENT_SERVICE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), \
					BLUETOOTH_TYPE_AGENT_SERVICE, BluetoothAgentService))
#define BLUETOOTH_AGENT_SERVICE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), \
					BLUETOOTH_TYPE_AGENT_SERVICE, BluetoothAgentServiceClass))
#define BLUETOOTH_IS_AGENT_SERVICE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), \
							BLUETOOTH_TYPE_AGENT_SERVICE))
#define BLUETOOTH_IS_AGENT_SERVICE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), \
							BLUETOOTH_TYPE_AGENT_SERVICE))
#define BLUETOOTH_GET_AGENT_SERVICE_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), \
					BLUETOOTH_TYPE_AGENT_SERVICE, BluetoothAgentServiceClass))

typedef struct _BluetoothAgentService BluetoothAgentService;
typedef struct _BluetoothAgentServiceClass BluetoothAgentServiceClass;

struct _BluetoothAgentService {
	GObject parent;
};

struct _BluetoothAgentServiceClass {
	GObjectClass parent_class;
};

GType bluetooth_agent_service_get_type(void);

BluetoothAgentService *bluetooth_agent_service_new(void);

gboolean bluetooth_agent_service_setup(BluetoothAgentService *agent, const char *path);

gboolean bluetooth_agent_service_register(BluetoothAgentService *agent, DBusGProxy *adapter);
gboolean bluetooth_agent_service_unregister(BluetoothAgentService *agent);

typedef gboolean (*BluetoothAgentPasskeyFunc) (DBusGMethodInvocation *context,
					DBusGProxy *device, gpointer data);
typedef gboolean (*BluetoothAgentDisplayFunc) (DBusGMethodInvocation *context,
					DBusGProxy *device, guint passkey,
						guint entered, gpointer data);
typedef gboolean (*BluetoothAgentConfirmFunc) (DBusGMethodInvocation *context,
					DBusGProxy *device, guint passkey,
								gpointer data);
typedef gboolean (*BluetoothAgentAuthorizeFunc) (DBusGMethodInvocation *context,
					DBusGProxy *device, const char *uuid,
								gpointer data);
typedef gboolean (*BluetoothAgentCancelFunc) (DBusGMethodInvocation *context,
								gpointer data);

void bluetooth_agent_set_pincode_func(BluetoothAgentService *agent,
				BluetoothAgentPasskeyFunc func, gpointer data);
void bluetooth_agent_set_passkey_func(BluetoothAgentService *agent,
				BluetoothAgentPasskeyFunc func, gpointer data);
void bluetooth_agent_set_display_func(BluetoothAgentService *agent,
				BluetoothAgentDisplayFunc func, gpointer data);
void bluetooth_agent_set_confirm_func(BluetoothAgentService *agent,
				BluetoothAgentConfirmFunc func, gpointer data);
void bluetooth_agent_set_authorize_func(BluetoothAgentService *agent,
				BluetoothAgentAuthorizeFunc func, gpointer data);
void bluetooth_agent_set_cancel_func(BluetoothAgentService *agent,
				BluetoothAgentCancelFunc func, gpointer data);

G_END_DECLS

#endif /* __BLUETOOTH_AGENT_H */
