/*
 * Mokosuite
 * Battery status panel applet
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

#include <Elementary.h>
#include <glib.h>
#include <libmokosuite/mokosuite.h>
#include <libmokosuite/fso.h>
#include <frameworkd-glib/odeviced/frameworkd-glib-odeviced-dbus.h>
#include <frameworkd-glib/odeviced/frameworkd-glib-odeviced-powersupply.h>

#include "panel.h"
#include "battery.h"

// chissa perche' non e' definita...
int odeviced_power_supply_get_status_from_dbus(const char *status);

static FrameworkdHandler fsoHandlers = {0};

// lista applet batterie
static GPtrArray* batteries = NULL;

// flag batteria in carica
gboolean battery_charging = FALSE;

// flag occupazione display su connessione usb
gboolean battery_resource_display = FALSE;

// FIXME workaround per bug elm_icon
static int old_perc = -1;
static gboolean old_charging = FALSE;

static void get_capacity_callback(GError *error, int energy, gpointer data)
{
    Evas_Object* bat = (Evas_Object*) data;

    char *ic = NULL;
    int perc = -1;

    if (error == NULL) {

        //g_debug("Energy value: %d", energy);

        if (energy <= 0)
            perc = (battery_charging) ? 10 : 0;
        else if (energy > 0 && energy <= 10)
            perc = 10;
        else if (energy > 10 && energy <= 30)
            perc = 20;
        else if (energy > 30 && energy <= 50)
            perc = 40;
        else if (energy > 50 && energy <= 70)
            perc = 60;
        else if (energy > 70 && energy <= 95)
            perc = 80;
        else if (energy > 95)
            perc = 100;

    }

    if (old_perc == perc && old_charging == battery_charging) {
        //g_debug("FIXME workaround to elm_icon bug - not setting battery icon");
        return;
    }

    old_perc = perc;
    old_charging = battery_charging;

    if (perc < 0)
        ic = g_strdup(MOKOSUITE_DATADIR "battery-unknown.png");
    else
        ic = g_strdup_printf(MOKOSUITE_DATADIR "%s-%d.png", (battery_charging) ? "charging" : "battery", perc);

    elm_icon_file_set(bat, ic, NULL);
    elm_icon_no_scale_set(bat, TRUE);
    #ifdef QVGA
    elm_icon_scale_set(bat, FALSE, TRUE);
    #else
    elm_icon_scale_set(bat, TRUE, TRUE);
    #endif

    evas_object_size_hint_min_set(bat, ICON_SIZE, ICON_SIZE);
    evas_object_size_hint_align_set(bat, 0.5, 0.5);
    g_free(ic);
}

static void set_charging(GError *error, int power_status)
{
    battery_charging = (error == NULL && power_status == DEVICE_POWER_STATE_CHARGING);

    #if 0
    if (battery_charging && battery_resource_display)
        ousaged_request_resource("Display", NULL, NULL);
    else
        // meglio rilasciare comunque il display...
        ousaged_release_resource("Display", NULL, NULL);
    #endif
}

static void get_power_status_callback(GError *error, int status, gpointer data)
{
    set_charging(error, status);
    odeviced_power_supply_get_capacity(get_capacity_callback, data);
}

static void capacity_changed(const int energy)
{
    if (batteries == NULL) return;

    int i;
    for (i = 0; i < batteries->len; i++)
        get_capacity_callback(NULL, energy, g_ptr_array_index(batteries, i));
}

static void power_status_changed(const char* status)
{
    g_debug("Power status changed to %s", status);
    if (batteries == NULL || status == NULL) return;

    int status_n = odeviced_power_supply_get_status_from_dbus(status);
    set_charging(NULL, status_n);
    g_debug("Power status changed to %s (%d)", status, status_n);

    int i;
    for (i = 0; i < batteries->len; i++) {
        get_capacity_callback(NULL, status_n, g_ptr_array_index(batteries, i));   
    }
}

static gboolean battery_fso_connect_timeout(gpointer data)
{
    if (odevicedPowerSupplyBus != NULL)
        odeviced_power_supply_get_power_status(get_power_status_callback, data);

    return TRUE;
}

static gboolean battery_fso_connect(gpointer data)
{
    dbus_connect_to_odeviced_power_supply();

    if (odevicedPowerSupplyBus == NULL)
        g_critical("Cannot connect to odeviced (PowerSupply)");

    // richiedi valore iniziale carica batteria
    if (odevicedPowerSupplyBus != NULL)
        odeviced_power_supply_get_power_status(get_power_status_callback, data);

    // per ovviare al cazzo di bug del full... sigh...
    g_timeout_add_seconds(5, battery_fso_connect_timeout, data);

    return FALSE;
}

Evas_Object* battery_applet_new(MokoPanel* panel)
{
    // TODO free func
    if (batteries == NULL)
        batteries = g_ptr_array_new();

    Evas_Object* bat = elm_icon_add(panel->win);

    elm_icon_file_set(bat, MOKOSUITE_DATADIR "battery-unknown.png", NULL);
    elm_icon_no_scale_set(bat, TRUE);
    #ifdef QVGA
    elm_icon_scale_set(bat, FALSE, TRUE);
    #else
    elm_icon_scale_set(bat, TRUE, TRUE);
    #endif

    evas_object_size_hint_min_set(bat, ICON_SIZE, ICON_SIZE);
    evas_object_size_hint_align_set(bat, 0.5, 0.5);
    evas_object_show(bat);

    // connetti segnali FSO
    fsoHandlers.devicePowerSupplyCapacity = capacity_changed;
    fsoHandlers.devicePowerSupplyStatus = power_status_changed;

    fso_handlers_add(&fsoHandlers);

    // richiesta dati iniziale
    g_idle_add(battery_fso_connect, bat);

    g_ptr_array_add(batteries, bat);

    return bat;
}
