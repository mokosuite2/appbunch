#ifndef __MOKOSETTINGSAPP_H
#define __MOKOSETTINGSAPP_H

#include <libmokosuite/mokosuite.h>
#include <libmokosuite/fso.h>

#define DBUS_PANEL_SERVICE               "org.mokosuite.panel"
#define DBUS_PANEL_SETTINGS_PATH    "/org/mokosuite/Panel/Settings"

#define DBUS_PHONE_SERVICE          "org.mokosuite.phone"
#define DBUS_PHONE_SETTINGS_PATH    "/org/mokosuite/Phone/Settings"

extern DBusGProxy* panel_settings;
extern DBusGProxy* phone_settings;

#endif  /* __MOKOSETTINGSAPP_H */
