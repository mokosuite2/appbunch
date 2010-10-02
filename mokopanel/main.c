/*
 * Mokosuite
 * Panel, idle screen and shutdown window
 * Copyright (C) 2009-2010 Daniele Ricci <daniele.athome@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdlib.h>
#include <Elementary.h>
#include <libmokosuite/mokosuite.h>
#include <libmokosuite/settings-service.h>
#include <libmokosuite/notifications.h>

#include <dbus/dbus-glib-bindings.h>

#include <freesmartphone-glib/freesmartphone-glib.h>
#include <freesmartphone-glib/odeviced/input.h>

#define MOKO_PANEL_NAME             "org.mokosuite.panel"
#define MOKO_PANEL_SETTINGS_PATH    "/org/mokosuite/Panel/Settings"

/* path predefinito db panel (impostazioni e altro) */
#define PANELDB_PATH                SYSCONFDIR "/mokosuite/panel.db"

#include "panel.h"
#include "idle.h"
#include "shutdown.h"
#include "notifications-win.h"

static MokoPanel* main_panel = NULL;

/* esportabili */
MokoSettingsService* panel_settings = NULL;

#if 0
static gboolean test_after_notify(MokoPanel* panel)
{
    mokopanel_notification_queue(main_panel, "Io bene, tu?", "unread-message", "test unread message/2", MOKOPANEL_NOTIFICATION_FLAG_NONE);
    return FALSE;
}
#endif

int main(int argc, char* argv[])
{
    moko_factory_init(argc, argv, PACKAGE, VERSION);

    GError *e = NULL;
    DBusGProxy *driver_proxy;
    guint request_ret;

#warning system_bus symbol not correctly exported, using workaround
    if (!system_bus) {
        system_bus = dbus_g_bus_get(DBUS_BUS_SYSTEM, &e);

        if (e) {
            g_error("Unable to connect to system bus: %s", e->message);
            g_error_free(e);
            return EXIT_FAILURE;
        }
    }

    driver_proxy = dbus_g_proxy_new_for_name (system_bus,
            DBUS_SERVICE_DBUS,
            DBUS_PATH_DBUS,
            DBUS_INTERFACE_DBUS);
    if (!driver_proxy)
        g_error("Unable to connect to DBus interface. Exiting.");

    if (!org_freedesktop_DBus_request_name (driver_proxy,
            MOKO_PANEL_NAME, 0, &request_ret, &e)) {
        g_error("Unable to request panel service name: %s. Exiting.", e->message);
        g_error_free(e);
        return EXIT_FAILURE;
    }
    g_object_unref(driver_proxy);

    /*
    if (request_ret != EGG_DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        g_error("Could not become primary name owner. Exiting.");
        return EXIT_FAILURE;
    }
    */

    char* db_path = config_get_string("panel", "paneldb", PANELDB_PATH);
    panel_settings = moko_settings_service_new(MOKO_PANEL_SETTINGS_PATH, db_path, "panel");
    g_free(db_path);

    freesmartphone_glib_init();

    elm_theme_overlay_add(NULL, "elm/pager/base/panel");
    elm_theme_overlay_add(NULL, "elm/label/base/panel");
    elm_theme_overlay_add(NULL, "elm/bg/base/panel");

    main_panel = mokopanel_new("mokopanel", "Panel");

    // idle screen
    idlescreen_init(main_panel);

    // FIXME test icone
    #if 0
    mokopanel_notification_queue(main_panel, "Ciao zio come stai?", "unread-message", "test unread message/2",
        MOKOPANEL_NOTIFICATION_FLAG_NONE | MOKOPANEL_NOTIFICATION_FLAG_REPRESENT);
    //mokopanel_notification_queue(main_panel, "+393296061565", MOKOSUITE_DATADIR "call-end.png", NOTIFICATION_MISSED_CALL,
    //    MOKOPANEL_NOTIFICATION_FLAG_DONT_PUSH);
    //mokopanel_notification_queue(main_panel, "+39066520498", MOKOSUITE_DATADIR "call-end.png", NOTIFICATION_MISSED_CALL,
    //    MOKOPANEL_NOTIFICATION_FLAG_DONT_PUSH);
    g_timeout_add_seconds(1, (GSourceFunc) test_after_notify, main_panel);
    #endif
    // FIXME fine test icone

    return moko_factory_run();
}
