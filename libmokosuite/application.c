
#include "mokosuite.h"

#include <stdlib.h>
#include <Elementary.h>
#include <Ecore.h>

DBusGConnection *system_bus = NULL;

gboolean moko_factory_init(int argc, char *argv[], const char *package, const char *version)
{
    g_type_init();
    g_set_prgname(package);

    // TODO: inizializza Intl

    elm_init(argc, argv);

    // temi aggiuntivi e fix
    elm_theme_extension_add(NULL, MOKOSUITE_DATADIR "theme.edj");

    elm_theme_overlay_add(NULL, "elm/label/base_wrap/default");

    elm_theme_overlay_add(NULL, "elm/genlist/item/generic_sub/default");
    elm_theme_overlay_add(NULL, "elm/genlist/item_odd/generic_sub/default");

    elm_theme_overlay_add(NULL, "elm/genlist/item/generic/default");
    elm_theme_overlay_add(NULL, "elm/genlist/item_odd/generic/default");

    /* integrazione con GLib */
    if (!ecore_main_loop_glib_integrate())
        g_error("Ecore/GLib integration failed!");

    /* inizializza la configurazione di base */
    config_init();

    /* setup dbus server part */
    GError *error = NULL;

    g_debug("Creating system bus...");
    system_bus = dbus_g_bus_get(DBUS_BUS_SYSTEM, &error);
    g_debug("system_bus=%p, error=%p", system_bus, error);
    if (error) {
        g_error("%d: %s", error->code, error->message);
        g_error_free(error);
    }

    return TRUE;
}

int moko_factory_run(void)
{
    elm_run();
    elm_shutdown();

    return EXIT_SUCCESS;
}

void moko_factory_exit(void)
{
    elm_exit();
}
