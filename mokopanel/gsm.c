/*
 * Mokosuite
 * GSM signal panel applet
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
#include <libmokosuite/misc.h>
#include <frameworkd-glib/ousaged/frameworkd-glib-ousaged-dbus.h>
#include <frameworkd-glib/ousaged/frameworkd-glib-ousaged.h>
#include <frameworkd-glib/ogsmd/frameworkd-glib-ogsmd-dbus.h>
#include <frameworkd-glib/ogsmd/frameworkd-glib-ogsmd-network.h>
#include <frameworkd-glib/ogsmd/frameworkd-glib-ogsmd-device.h>
#include <frameworkd-glib/ogsmd/frameworkd-glib-ogsmd-sim.h>

#include "panel.h"
#include "gsm.h"
#include "idle.h"

#include <glib/gi18n-lib.h>

/* !!!!! FIXME FIXME FIXME !!!!! */
#ifdef OPENMOKO
#define SEGV_ADDRESS    0x60000
#else
//#define SEGV_ADDRESS    0x10
#define SEGV_ADDRESS    0xbfff0000
#endif

static FrameworkdHandler fsoHandlers = {0};

// lista applet gsm
static GPtrArray* gsm_applets = NULL;

// segnale GSM
static gint8 signal_strength = 0;

// stato modem GSM
static GsmStatus gsm_status = GSM_STATUS_DISABLED;

// flag offline mode (GetFunctionality)
static gboolean offline_mode = FALSE;

// flag SIM bloccata
static gboolean sim_locked = FALSE;

// FIXME workaround per bug elm_icon
static GsmIcon old_icon = GSM_ICON_DISABLED;
static int old_perc = 0;

static void update_operator(const char* operator)
{
    if (operator)
        idlescreen_update_operator(operator);
    else
        idlescreen_update_operator(_("(No service)"));
}

static void update_icon(Evas_Object* gsm)
{
    if (sim_locked) {
        gsm_applet_set_icon(gsm, GSM_ICON_DISABLED);
        update_operator(_("(SIM locked)"));
        return;
    }

    if (offline_mode) {
        gsm_applet_set_icon(gsm, GSM_ICON_OFFLINE);
        update_operator(NULL);
        return;
    }

    if (gsm_status == GSM_STATUS_DISABLED || gsm_status == GSM_STATUS_REGISTRATION_UNKNOWN) {
        gsm_applet_set_icon(gsm, GSM_ICON_DISABLED);
        update_operator(NULL);
        return;
    }

    if (gsm_status == GSM_STATUS_REGISTRATION_DENIED) {
        update_operator(_("(SOS only)"));
    }

    gsm_applet_set_icon(gsm, GSM_ICON_ONLINE);    
}

/* versione SEGV-safe di ogsmd_network_get_registration_status_from_dbus */
int FIXED_ogsmd_network_get_registration_status_from_dbus(GHashTable * properties)
{
    GValue *reg = NULL;
    const char *registration = NULL;

    // ma tu guarda...
    g_debug("(reg status) Properties: %p", properties);
    if ((properties == NULL || (guint32)properties < SEGV_ADDRESS)
        ||
        ((reg =
          g_hash_table_lookup(properties,
                  DBUS_NETWORK_PROPERTY_REGISTRATION)) == NULL))
        return NETWORK_PROPERTY_REGISTRATION_UNKNOWN;

    registration = g_value_get_string(reg);

    if (!strcmp
        (registration, DBUS_NETWORK_PROPERTY_REGISTRATION_UNREGISTERED)) {
        return NETWORK_PROPERTY_REGISTRATION_UNREGISTERED;
    }
    else if (!strcmp(registration, DBUS_NETWORK_PROPERTY_REGISTRATION_HOME)) {
        return NETWORK_PROPERTY_REGISTRATION_HOME;
    }
    else if (!strcmp(registration, DBUS_NETWORK_PROPERTY_REGISTRATION_BUSY)) {
        return NETWORK_PROPERTY_REGISTRATION_BUSY;
    }
    else if (!strcmp
        (registration, DBUS_NETWORK_PROPERTY_REGISTRATION_DENIED)) {
        return NETWORK_PROPERTY_REGISTRATION_DENIED;
    }
    else if (!strcmp
        (registration, DBUS_NETWORK_PROPERTY_REGISTRATION_ROAMING)) {
        return NETWORK_PROPERTY_REGISTRATION_ROAMING;
    }

    return NETWORK_PROPERTY_REGISTRATION_UNKNOWN;
}

const char * FIXED_ogsmd_network_get_provider_from_dbus(GHashTable * properties)
{
    GValue *provider;
    g_debug("(provider) Properties: %p", properties);

    if (properties != NULL && (guint32)properties > SEGV_ADDRESS) {
        provider =
            g_hash_table_lookup(properties,
                        DBUS_NETWORK_PROPERTY_PROVIDER);
        return provider == NULL ? NULL : g_value_get_string(provider);
    }
    return NULL;
}

static void get_signal_strength_callback(GError *error, int strength, gpointer data)
{
    Evas_Object* gsm = (Evas_Object*) data;
        g_debug("GSM signal strength: %d", strength);

    signal_strength = (error == NULL && strength >= 0) ? strength : 0;

    update_icon(gsm);
}

static void get_functionality_callback(GError* error, char* level, gboolean autoregister, char* pin, gpointer data)
{
    Evas_Object* gsm = (Evas_Object *) data;

    // errore - gsm disattivo
    if (error != NULL) {
        gsm_status = GSM_STATUS_DISABLED;
        update_icon(gsm);
        return;
    }

    offline_mode = strcasecmp(level, "full");

    update_icon(gsm);
}

static void get_network_status_callback(GError *error, GHashTable *status, gpointer data)
{
    int status_n = FIXED_ogsmd_network_get_registration_status_from_dbus(status);
    const char* operator = FIXED_ogsmd_network_get_provider_from_dbus(status);
    g_debug("GSM network status: %d", status_n);

    if (status_n == NETWORK_PROPERTY_REGISTRATION_DENIED) {
        offline_mode = FALSE;
        gsm_status = GSM_STATUS_REGISTRATION_DENIED;
    }

    else if(status_n == NETWORK_PROPERTY_REGISTRATION_HOME ||
        status_n == NETWORK_PROPERTY_REGISTRATION_ROAMING) {
        gsm_status = GSM_STATUS_REGISTRATION_HOME;
        offline_mode = FALSE;
        update_operator(operator);
    }

    else if (status_n == NETWORK_PROPERTY_REGISTRATION_UNREGISTERED ||
        status_n == NETWORK_PROPERTY_REGISTRATION_UNKNOWN) {
        // controlla offline mode
        gsm_status = GSM_STATUS_REGISTRATION_UNKNOWN;
        ogsmd_device_get_functionality(get_functionality_callback, data);
    }

    else {
        gsm_status = GSM_STATUS_REGISTRATION_UNKNOWN;
    }

    update_icon((Evas_Object *)data);
}

static void get_device_status_callback(GError *error, const int status, gpointer data)
{
    g_debug("GSM Device status %d", status);

    switch (status) {
        case DEVICE_STATUS_ALIVE_SIM_LOCKED:
            sim_locked = TRUE;
            break;

        case DEVICE_STATUS_ALIVE_SIM_UNLOCKED:
        case DEVICE_STATUS_ALIVE_SIM_READY:
            sim_locked = FALSE;
            break;

        case DEVICE_STATUS_ALIVE_REGISTERED:
            // triggera registration status
            ogsmd_network_get_status(get_network_status_callback, data);
            break;

        case DEVICE_STATUS_UNKNOWN:
        case DEVICE_STATUS_CLOSED:
        case DEVICE_STATUS_INITIALIZING:
        case DEVICE_STATUS_ALIVE_NO_SIM:
        case DEVICE_STATUS_ALIVE_CLOSING:
            gsm_status = GSM_STATUS_DISABLED;
            break;
    }

    update_icon((Evas_Object *)data);
}

static void get_auth_status_callback(GError *error, int status, gpointer data)
{
    g_debug("SIM auth status %d", status);

    if (status == SIM_READY) {
        sim_locked = FALSE;
        // probabile rientro da modalita' offline
        ogsmd_device_get_functionality(get_functionality_callback, data);
    }

    else if (status != SIM_UNKNOWN)
        sim_locked = TRUE;

    update_icon((Evas_Object *)data);
}

#if 0
static void get_resource_state_callback(GError *error, gboolean state, gpointer data)
{
    if (!state) {
        gsm_status = GSM_STATUS_DISABLED;
        update_icon((Evas_Object *) data);
    }
}

static void resource_changed (const char *name, gboolean state, GHashTable *attributes)
{
    if (gsm_applets == NULL) return;

    g_debug("Resource changed: %s (%d)", name, state);
    if (!strcmp(name, "GSM")) {
        // GSM disattivato...
        int i;
        for (i = 0; i < gsm_applets->len; i++)
            get_resource_state_callback(NULL, state, g_ptr_array_index(gsm_applets, i));
    }
}
#endif

static void signal_strength_changed(const int strength)
{
    if (gsm_applets == NULL) return;

    int i;
    for (i = 0; i < gsm_applets->len; i++)
        get_signal_strength_callback(NULL, strength, g_ptr_array_index(gsm_applets, i));
}

static void network_status_changed(GHashTable* status)
{
    if (gsm_applets == NULL) return;

    int i;
    for (i = 0; i < gsm_applets->len; i++)
        get_network_status_callback(NULL, status, g_ptr_array_index(gsm_applets, i));   
}

static void device_status_changed(const int status)
{
    if (gsm_applets == NULL) return;

    int i;
    for (i = 0; i < gsm_applets->len; i++)
        get_device_status_callback(NULL, status, g_ptr_array_index(gsm_applets, i));
}

static void auth_status_changed(const int status)
{
    if (gsm_applets == NULL) return;

    int i;
    for (i = 0; i < gsm_applets->len; i++)
        get_auth_status_callback(NULL, status, g_ptr_array_index(gsm_applets, i));
}

static gboolean gsm_fso_connect(gpointer data)
{
    dbus_connect_to_ousaged();
    if (ousagedBus == NULL)
        g_critical("Cannot connect to ousaged; will not be able to detect GSM resource change");

    dbus_connect_to_ogsmd_network();
    if (networkBus == NULL)
        g_critical("Cannot connect to ogsmd (network)");

    dbus_connect_to_ogsmd_device();
    if (deviceBus == NULL)
        g_critical("Cannot connect to ogsmd (device)");

    dbus_connect_to_ogsmd_sim();
    if (simBus == NULL)
        g_critical("Cannot connect to ogsmd (sim)");

    #if 0
    // richiedi valore iniziale policy risorsa per l'offline mode
    if (ousagedBus != NULL)
        ousaged_get_resource_state("GSM", get_resource_state_callback, data);
    #endif

    // richiedi valori iniziali gsm
    if (networkBus != NULL) {
        ogsmd_network_get_status(get_network_status_callback, data);
        ogsmd_network_get_signal_strength(get_signal_strength_callback, data);
    }

    // richiedi stato iniziale modem gsm
    if (deviceBus != NULL)
        ogsmd_device_get_device_status(get_device_status_callback, data);

    if (simBus != NULL)
        ogsmd_sim_get_auth_status(get_auth_status_callback, data);

    return FALSE;
}

void gsm_applet_set_icon(Evas_Object* gsm, GsmIcon icon)
{
    char *ic = NULL;
    int perc = 0;

    switch (icon) {
        case GSM_ICON_DISABLED:
            ic = g_strdup(MOKOSUITE_DATADIR "gsm-null.png");
            break;

        case GSM_ICON_OFFLINE:
            ic = g_strdup(MOKOSUITE_DATADIR "gsm-offline.png");
            break;

        case GSM_ICON_ONLINE: //[*ti amuzzoooooooo troppoooooooooooooooooooooooooooo!!!:)!!!Bacino!!!:)!*]
            g_debug("GSM online icon, strength = %d", signal_strength);

            if (signal_strength <= 0)
                perc = 0;
            else if (signal_strength > 0 && signal_strength <= 30)
                perc = 1;
            else if (signal_strength > 30 && signal_strength <= 55)
                perc = 2;
            else if (signal_strength > 55 && signal_strength <= 80)
                perc = 3;
            else if (signal_strength > 80)
                perc = 4;

            ic = g_strdup_printf(MOKOSUITE_DATADIR "gsm-%d.png", perc);

            break;

    }

    if (old_icon == icon && old_perc == perc) {
        g_debug("FIXME working around elm_icon bug - not setting GSM icon");
        g_free(ic);
        return;
    }

    g_debug("Setting GSM icon to %d, signal %d (old icon %d, old signal %d)\nfilename=%s", icon, perc, old_icon, old_perc, ic);
    old_icon = icon;
    old_perc = perc;

    if (ic != NULL) {
        elm_icon_file_set(gsm, ic, NULL);
        g_free(ic);

        // maledetto!!!
        elm_icon_no_scale_set(gsm, TRUE);
        #ifdef QVGA
        elm_icon_scale_set(gsm, FALSE, TRUE);
        #else
        elm_icon_scale_set(gsm, TRUE, TRUE);
        #endif
        evas_object_size_hint_align_set(gsm, 0.5, 0.5);
        evas_object_size_hint_min_set(gsm, ICON_SIZE, ICON_SIZE);
    }
}

#if 0
static gboolean next_icon(gpointer applet)
{
    Evas_Object* gsm = applet;
    static GsmIcon current_icon = GSM_ICON_DISABLED;

    gsm_applet_set_icon(gsm, current_icon);

    current_icon++;
    if (current_icon > GSM_ICON_ONLINE) {
        current_icon = GSM_ICON_ONLINE;
        signal_strength += 20;
        if (signal_strength > 100)
            signal_strength = 0;
    }

    return TRUE;
}
#endif

Evas_Object* gsm_applet_new(MokoPanel* panel)
{
    // TODO free func
    if (gsm_applets == NULL)
        gsm_applets = g_ptr_array_new();

    Evas_Object* gsm = elm_icon_add(panel->win);

    elm_icon_file_set(gsm, MOKOSUITE_DATADIR "gsm-null.png", NULL);
    elm_icon_no_scale_set(gsm, TRUE);
    #ifdef QVGA
    elm_icon_scale_set(gsm, FALSE, TRUE);
    #else
    elm_icon_scale_set(gsm, TRUE, TRUE);
    #endif

    evas_object_size_hint_align_set(gsm, 0.5, 0.5);
    evas_object_size_hint_min_set(gsm, ICON_SIZE, ICON_SIZE);
    evas_object_show(gsm);

    // connetti segnali FSO
    if (fsoHandlers.networkStatus == NULL) {
        fsoHandlers.networkStatus = network_status_changed;
        fsoHandlers.networkSignalStrength = signal_strength_changed;
        #if 0
        fsoHandlers.usageResourceChanged = resource_changed;
        #endif
        fsoHandlers.gsmDeviceStatus = device_status_changed;
        fsoHandlers.simAuthStatus = auth_status_changed;

        fso_handlers_add(&fsoHandlers);
    }

    // richiesta dati iniziale
    g_idle_add(gsm_fso_connect, gsm);

    g_ptr_array_add(gsm_applets, gsm);

    // TEST icona
    //g_timeout_add_seconds(5, next_icon, gsm);

    return gsm;
}
