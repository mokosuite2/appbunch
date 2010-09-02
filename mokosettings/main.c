#include "mokosettings.h"
#include "menu-main.h"

DBusGProxy* panel_settings = NULL;
DBusGProxy* phone_settings = NULL;

#if 0
gboolean test_settings(gpointer data)
{
    DBusGProxy *s = (DBusGProxy*)data;
    GError *e = NULL;

    //gboolean r = moko_settings_set_setting(s, "offline_mode", "false", NULL);
    //g_debug("Set offline_mode to false: %d", r);

    char *v = moko_settings_get_setting(s, "offline_mode", NULL, &e);
    if (v == NULL && e != NULL) {
        g_debug("Error getting offline_mode: %s", e->message);
        g_error_free(e);
        return FALSE;
    }

    g_debug("Offline_mode: \"%s\"", v);
    g_free(v);

    return FALSE;
}
#endif

int main(int argc, char *argv[])
{
    g_debug(PACKAGE " version " VERSION " started");

    moko_factory_init(argc, argv, PACKAGE, VERSION);

    // inizializza FSO
    fso_init();

    #if 0
    DBusGProxy *s = moko_settings_connect("org.mokosuite.phone", "/org/mokosuite/Phone/Settings");
    if (s == NULL)
        g_error("Cannot connect to phone remote settings interface. Exiting.");

    g_idle_add(test_settings, s);
    #endif

    // Panel settings
    panel_settings = moko_settings_connect(DBUS_PANEL_SERVICE, DBUS_PANEL_SETTINGS_PATH);
    if (panel_settings == NULL)
        g_critical("Unable to connect to panel idle settings.");

    // Phone settings
    phone_settings = moko_settings_connect(DBUS_PHONE_SERVICE, DBUS_PHONE_SETTINGS_PATH);
    if (phone_settings == NULL)
        g_critical("Unable to connect to phone settings.");

    menu_main_init();

    return moko_factory_run();
}
