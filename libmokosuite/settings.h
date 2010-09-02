#ifndef __MOKOSETTINGS_H
#define __MOKOSETTINGS_H

#include <glib.h>
#include <dbus/dbus-glib.h>

#define moko_settings_get_boolean(x)        (x != NULL && !strcasecmp(x, "true") ? TRUE : FALSE)
#define moko_settings_from_boolean(x)       (x ? "true" : "false")

#define moko_settings_get_integer(x)        (x != NULL ? atoi(x) : 0)

char* moko_settings_get_setting(DBusGProxy* proxy, const char* key, const char* default_val, GError** error);

void moko_settings_get_setting_async(DBusGProxy* proxy, const char* key, const char* default_val, void (*callback)
    (GError *, const char* ret_value, gpointer userdata), gpointer userdata);


gboolean moko_settings_set_setting(DBusGProxy* proxy, const char* key, const char* value, GError** error);

void moko_settings_set_setting_async(DBusGProxy* proxy, const char* key, const char* value, void (*callback)
    (GError *, gpointer userdata), gpointer userdata);


DBusGProxy* moko_settings_connect(const char* bus_name, const char* path);


#endif  /* __MOKOSETTINGS_H */
