#include <glib.h>
#include <string.h>
#include <frameworkd-glib/frameworkd-glib-dbus.h>
#include <frameworkd-glib/ousaged/frameworkd-glib-ousaged.h>
#include <frameworkd-glib/ousaged/frameworkd-glib-ousaged-dbus.h>
#include <frameworkd-glib/ogsmd/frameworkd-glib-ogsmd-dbus.h>
#include <frameworkd-glib/ogsmd/frameworkd-glib-ogsmd-device.h>
#include <frameworkd-glib/ogsmd/frameworkd-glib-ogsmd-sim.h>
#include <frameworkd-glib/ogsmd/frameworkd-glib-ogsmd-call.h>
#include <frameworkd-glib/ogsmd/frameworkd-glib-ogsmd-network.h>

#include "gsm.h"
#include "simauthwin.h"
#include "callwin.h"
#include "phonewin.h"

#include <glib/gi18n-lib.h>


static SimAuthWin *sim_auth = NULL;

static FrameworkdHandler* gsmHandlers = NULL;
static gboolean gsm_available = FALSE;
static gboolean gsm_ready = FALSE;
static gboolean sim_ready = FALSE;
static gboolean network_registered = FALSE;
static gboolean offline_mode = FALSE;

static void online_offline(void);
static gboolean request_gsm(gpointer data);
static gboolean get_auth_status(gpointer data);
static gboolean list_resources(gpointer data);
static gboolean register_network(gpointer data);

// eh...
static void network_status(GHashTable *status);
static void _get_network_status_callback(GError * error, GHashTable * status, gpointer data);

static void sim_ready_actions(void);
static void sim_auth_show(SimStatus type);
static void sim_auth_hide(void);
static void sim_auth_error(void);

static void _request_resource_callback(GError * error, gpointer userdata)
{
    // nessun errore -- tocca a ResourceChanged
    if (error == NULL) return;

    if (IS_USAGE_ERROR(error, USAGE_ERROR_USER_EXISTS)) {
        g_message("GSM has already been requested.");
        return;
    }

    g_debug("request resource error, try again in 1s");
    g_timeout_add(1000, request_gsm, NULL);
}

static void _list_resources_callback(GError *error, char **resources, gpointer userdata)
{
    /* if we successfully got a list of resources...
     * check if GSM is within them and request it if
     * so, otherwise wait for ResourceAvailable signal */
    if (error) {
        g_message("error: (%d) %s", error->code, error->message);
        g_timeout_add(1000, list_resources, NULL);
        return;
    }

    if (resources) {
        int i = 0;
        while (resources[i] != NULL) {
            g_debug("Resource %s available", resources[i]);
            if (!strcmp(resources[i], "GSM")) {
                gsm_available = TRUE;
                break;
            }
            i++;
        }

        if (gsm_available)
            request_gsm(NULL);
    }
}

static void _register_to_network_callback(GError * error, gpointer userdata)
{
    if (error == NULL) {
        g_debug("Successfully registered to the network");
        // non settare ora -- network_registered = TRUE;
        ogsmd_network_get_status(_get_network_status_callback, NULL);
    }
    else {
        g_debug("Registering to network failed: %s %s %d", error->message, g_quark_to_string(error->domain), error->code);
        if (!offline_mode && gsm_ready)
            g_timeout_add(10 * 1000, register_network, NULL);   // TODO: parametrizza il timeout
    }
}

static void _get_network_status_callback(GError * error, GHashTable * status, gpointer data)
{
    if (!error)
        network_status(status);

    else
        _register_to_network_callback(error, data);
}

/* GSM.SIM.GetAuthStatus */
static void _sim_get_auth_status_callback(GError *error, int status, gpointer data)
{
    g_debug("sim_auth_status_callback(%s,status=%d)", error ? "ERROR" : "OK", status);

    /* if no SIM is present inform the user about it and
     * stop retrying to authenticate the SIM */
    if (IS_SIM_ERROR(error, SIM_ERROR_NOT_PRESENT)) {
        g_message("SIM card not present.");
        // TODO
        return;
    }

    /* altri errori o stato sconosciuto, riprova tra 5s */
    if (error || status == SIM_UNKNOWN) {
        g_debug("Error: %s", error->message);
        g_timeout_add(5000, get_auth_status, NULL);
        return;
    }

    if (status != SIM_READY) {
        g_debug("SIM authentication started");
        sim_auth_show(SIM_PIN_REQUIRED);
        return;
    }

    g_debug("SIM authenticated");
    sim_auth_hide();

    // registra sulla rete
    register_network(NULL);
}

static void _sim_send_auth_callback(GError *error, gpointer data)
{
    if (error != NULL) {
        g_debug("SendAuthCode error");

        sim_auth_error();
    }
}

static gboolean request_gsm(gpointer data)
{
    if (gsm_available) {
        g_debug("Request GSM resource");
        ousaged_request_resource("GSM", _request_resource_callback, NULL);
    }

    return FALSE;
}

static void _offline_callback(GError *error, gpointer userdata)
{
    g_debug("going offline");
    // TODO controlla errori set functionality
    network_registered = FALSE;
}

static void _online_callback(GError *error, gpointer userdata)
{
    g_debug("going online");
    // TODO controlla errori set functionality
    if (error) {
        g_message("SetFunctionality() error: (%d) %s", error->code, error->message);
    }

    get_auth_status(NULL);
}

static void online_offline(void)
{
    g_debug("online_offline: offline_mode = %d", offline_mode);

    if (offline_mode)
        ogsmd_device_set_functionality("airplane", FALSE, "", _offline_callback, NULL);

    else
        ogsmd_device_set_functionality("full", TRUE, "", _online_callback, NULL);
}

static gboolean get_auth_status(gpointer data)
{
    ogsmd_sim_get_auth_status(_sim_get_auth_status_callback, NULL);
    return FALSE;
}

static gboolean register_network(gpointer data)
{
    ogsmd_network_register(_register_to_network_callback, NULL);
    return FALSE;
}

static gboolean list_resources(gpointer data)
{
    ousaged_list_resources(_list_resources_callback, NULL);
    return FALSE;
}

/* chiamata quando la SIM e' pronta */
static void sim_ready_actions(void) {
    g_debug("SIM is ready!");
    sim_ready = TRUE;

    sim_auth_hide();
}

static void _pin_ok(void *simauth, const char *code) {
    SimAuthWin *s = SIMAUTH_WIN(simauth);

    switch (s->type) {
        case SIM_PIN_REQUIRED:
            moko_popup_status_activate(s->status, _("Checking..."));
            ogsmd_sim_send_auth_code(code, _sim_send_auth_callback, NULL);

            break;

        // TODO: altri case
        default:
            break;
    }
}

static void sim_auth_show(SimStatus type)
{
    if (sim_auth == NULL) {
        sim_auth = sim_auth_win_new(type);
        sim_auth->code_callback = _pin_ok;  // TODO: altri casi
    }

    mokowin_activate(MOKO_WIN(sim_auth));
}

static void sim_auth_error(void)
{
    if (sim_auth != NULL) {
        sim_auth_win_auth_error(sim_auth);
        mokoinwin_hide(MOKO_INWIN(sim_auth->status));
    }
}

static void sim_auth_hide(void)
{
    if (sim_auth != NULL) {
        sim_auth_win_destroy(sim_auth);
        sim_auth = NULL;
    }
}

static void offline_mode_changed(MokoSettingsService *settings, const char* key, const char* value)
{
    offline_mode = moko_settings_get_boolean(value);
    online_offline();
}

/* -- SIGNAL HANDLERS -- */

/* Usage.ResourceChanged signal */
static void resource_changed (const char *name, gboolean state, GHashTable *attributes)
{
    g_debug("Resource changed: %s (%d, current gsm_ready = %d)", name, state, gsm_ready);
    if (!strcmp(name, "GSM")) {

        if (gsm_ready ^ state) {
            gsm_ready = state;

            if (gsm_ready) {
                g_debug("GSM resource enabled, powering on modem");
                online_offline();
            }

        } else if (!gsm_ready) {
                // gsm disattivato, gestisci il recupero della risorsa
                g_debug("GSM resource disabled, requesting");
                request_gsm(NULL);
        }
    }
}

/* Usage.ResourceAvailable signal */
static void resource_available(const char *name, gboolean available)
{
    g_debug("resource %s is %s", name, available ? "now available" : "gone");
    if (!strcmp(name, "GSM")) {
        gsm_available = available;
        if (gsm_available)
            request_gsm(NULL);
    }
}

/* GSM.SimReadyStatus */
static void sim_ready_status(gboolean ready)
{
    /* La SIM era gia' pronta (dal segnale ReadyStatus) */
    if (sim_ready) {
        g_debug("SIM was already ready, checking registration status...");
        ogsmd_network_get_status(_get_network_status_callback, NULL);
        return;
    }

    g_debug("SimReadyStatus: ready %d", ready);
    if (ready)
        sim_ready_actions();
}

/* GSM.SimAuthStatus */
static void sim_auth_status(const int status)
{
    if (status == SIM_READY) {
        sim_auth_hide();
        register_network(NULL);
    }
}

/* GSM.CallStatus */
static void call_status(const int id, const int status, GHashTable* properties)
{
    // :)
    phone_call_win_call_status(id, status, properties);
}

/* GSM.IncomingUssd */
static void incoming_ussd(int mode, const char* message)
{
    phone_win_ussd_reply(mode, message);
}

/* GSM.NetworkStatus */
static void network_status(GHashTable *status)
{
    if (!status) {
        g_debug("got no status from NetworkStatus?!");
        return;
    }

    GValue *tmp = g_hash_table_lookup(status, "registration");
    if (tmp) {

        const char *registration = g_value_get_string(tmp);
        g_debug("network_status (registration=%s)", registration);

        if (!strcmp(registration, "home")) {
            network_registered = TRUE;
        }

        else {
            network_registered = FALSE;
            if (!offline_mode && gsm_ready) {
                g_message("scheduling registration to network");
                g_timeout_add(10 * 1000, register_network, NULL);   // TODO: parametrizza il timeout
            }
        }

        /*
        if (!strcmp(registration, "unregistered")) {
            g_message("scheduling registration to network");
            g_timeout_add(10 * 1000,    // FIXME parametrizzabile
                    register_network, NULL);
        }
        */
    }
    else {
        g_debug("got NetworkStatus without registration?!?");
    }
}

/* GSM.DeviceStatus */
static void gsm_device_status(const int status)
{
    // TODO
}

/* -- funzioni pubbliche -- */

gboolean gsm_is_ready(void)
{
    return (gsm_ready && sim_ready);
}

gboolean gsm_can_call(void)
{
    return (gsm_ready && network_registered);
}

void gsm_online(void)
{
    offline_mode = FALSE;
    online_offline();
}

void gsm_offline(void)
{
    offline_mode = TRUE;
    online_offline();
}

void gsm_init(MokoSettingsService *settings)
{
    gsmHandlers = frameworkd_handler_new();
    gsmHandlers->usageResourceChanged = resource_changed;
    gsmHandlers->usageResourceAvailable = resource_available;
    gsmHandlers->simReadyStatus = sim_ready_status;
    gsmHandlers->gsmDeviceStatus = gsm_device_status;
    gsmHandlers->simAuthStatus = sim_auth_status;
    gsmHandlers->callCallStatus = call_status;
    gsmHandlers->incomingUssd = incoming_ussd;
    gsmHandlers->networkStatus = network_status;

    // connetti i segnali
    frameworkd_handler_connect(gsmHandlers);

    dbus_connect_to_ousaged();

    if (ousagedBus == NULL) {
        g_error("Cannot connect to ousaged. Exiting");
        return;
    }

    dbus_connect_to_ogsmd_device();

    if (deviceBus == NULL) {
        g_error("Cannot connect to ogsmd (device). Exiting");
        return;
    }

    dbus_connect_to_ogsmd_sim();

    if (simBus == NULL) {
        g_error("Cannot connect to ogsmd (sim). Exiting");
        return;
    }

    dbus_connect_to_ogsmd_call();

    if (callBus == NULL) {
        g_error("Cannot connect to ogsmd (call). Exiting");
        return;
    }

    // gestione offline mode
    char *str_offline_mode = NULL;
    moko_settings_service_get_setting(settings, "offline_mode", "false", &str_offline_mode, NULL);

    offline_mode = moko_settings_get_boolean(str_offline_mode);

    // listener offline mode
    moko_settings_service_callback_add(settings, "offline_mode", offline_mode_changed);

    // pinga fsogsmd per sicurezza, cosi' si attiva :)
    ogsmd_device_get_device_status(NULL, NULL);

    // comincia...
    list_resources(NULL);
}
