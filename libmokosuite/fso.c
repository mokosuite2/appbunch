#include <glib.h>
#include <frameworkd-glib/frameworkd-glib-dbus.h>
#include <frameworkd-glib/ousaged/frameworkd-glib-ousaged.h>
#include <frameworkd-glib/ousaged/frameworkd-glib-ousaged-dbus.h>
#include <frameworkd-glib/ogsmd/frameworkd-glib-ogsmd-dbus.h>
#include <frameworkd-glib/ogsmd/frameworkd-glib-ogsmd-device.h>
#include <frameworkd-glib/ogsmd/frameworkd-glib-ogsmd-sim.h>
#include <frameworkd-glib/ogsmd/frameworkd-glib-ogsmd-call.h>
#include <frameworkd-glib/ogsmd/frameworkd-glib-ogsmd-network.h>

static GPtrArray* fsoList = NULL;
static FrameworkdHandler internalFsoHandlers = {0};

#define LOOP_HANDLERS(name, ...) ({ \
    int i; \
    for (i = 0; i < fsoList->len; i++) { \
        FrameworkdHandler* hd = (FrameworkdHandler *)g_ptr_array_index(fsoList, i); \
        if (hd->name) \
            hd->name(__VA_ARGS__); \
    } \
})

/* Usage.ResourceChanged */
static void usageResourceChanged (const char *name, gboolean state, GHashTable *attributes)
{
    LOOP_HANDLERS(usageResourceChanged, name, state, attributes);
}

/* Usage.ResourceAvailable */
static void usageResourceAvailable (const char *name, gboolean available)
{
    LOOP_HANDLERS(usageResourceAvailable, name, available);
}

/* Device.IdleNotifier.State */
static void deviceIdleNotifierState(const int state)
{
    LOOP_HANDLERS(deviceIdleNotifierState, state);
}

/* Device.Input.Event */
static void deviceInputEvent(char* source, char* action, int duration)
{
    LOOP_HANDLERS(deviceInputEvent, source, action, duration);
}

/* Device.PowerSupply.Capacity */
static void devicePowerSupplyCapacity(const int energy)
{
    LOOP_HANDLERS(devicePowerSupplyCapacity, energy);
}

/* Device.PowerSupply.Status */
static void devicePowerSupplyStatus(const char* status)
{
    LOOP_HANDLERS(devicePowerSupplyStatus, status);
}

/* GSM.Network.SignalStrength */
static void networkSignalStrength(const int strength)
{
    LOOP_HANDLERS(networkSignalStrength, strength);
}

/* GSM.Network.Status */
static void networkStatus(GHashTable* status)
{
    LOOP_HANDLERS(networkStatus, status);
}

/* GSM.Call.CallStatus */
static void callCallStatus(const int id, const int status, GHashTable *props)
{
    LOOP_HANDLERS(callCallStatus, id, status, props);
}

/* GSM.Device.DeviceStatus */
static void gsmDeviceStatus(const int status)
{
    LOOP_HANDLERS(gsmDeviceStatus, status);
}

/* GSM.SIM.AuthStatus */
static void simAuthStatus(const int status)
{
    LOOP_HANDLERS(simAuthStatus, status);
}

/**
 * Registra i callback passati con libframeworkd-glib.
 */
void fso_handlers_add(FrameworkdHandler* handlers)
{
    g_return_if_fail(fsoList != NULL);

    g_ptr_array_add(fsoList, handlers);
}

/**
 * Inizializza i callback di FSO.
 */
void fso_init(void)
{
    fsoList = g_ptr_array_new();

    internalFsoHandlers.usageResourceChanged = usageResourceChanged;
    internalFsoHandlers.usageResourceAvailable = usageResourceAvailable;

    internalFsoHandlers.deviceIdleNotifierState = deviceIdleNotifierState;
    internalFsoHandlers.deviceInputEvent = deviceInputEvent;
    internalFsoHandlers.devicePowerSupplyCapacity = devicePowerSupplyCapacity;
    internalFsoHandlers.devicePowerSupplyStatus = devicePowerSupplyStatus;

    internalFsoHandlers.networkStatus = networkStatus;
    internalFsoHandlers.networkSignalStrength = networkSignalStrength;

    internalFsoHandlers.gsmDeviceStatus = gsmDeviceStatus;
    internalFsoHandlers.simAuthStatus = simAuthStatus;
    internalFsoHandlers.callCallStatus = callCallStatus;

    // connetti i segnali
    frameworkd_handler_connect(&internalFsoHandlers);

    dbus_connect_to_ousaged();

    if (ousagedBus == NULL) {
        g_error("Cannot connect to ousaged. Exiting");
        return;
    }

}
