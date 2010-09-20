/* mokophone entry point */

#include "mokophone.h"
#include "phonewin.h"
#include "callwin.h"
#include "gsm.h"
#include "callsdb.h"
#include "sound.h"

#include <phone-utils.h>
#include <libmokosuite/contactsdb.h>
#include <libmokosuite/notifications.h>
#include <freesmartphone-glib/freesmartphone-glib.h>

static MokoPhoneService *phone_service = NULL;
static MokoSettingsService *settings_service = NULL;

// notifiche panel (esportate)
DBusGProxy* panel_notifications = NULL;

int main(int argc, char *argv[])
{
    char *db_path = NULL;

#ifdef BARBATRUCCO
    // FIXME fai il furbo eh...
    setenv("DISPLAY", ":0", 1);
    setenv("ELM_SCALE", "2", 1);
    setenv("ELM_FINGER", "80", 1);
    setenv("ELM_THEME", "gry", 1);
    // FIXME fine del furbo
#endif

    moko_factory_init(argc, argv, PACKAGE, VERSION);

#ifdef USE_THREADS
    g_thread_init(NULL);
#endif

    /* inizializza fso */
    freesmartphone_glib_init();

    /* inizializza le phone utils */
    phone_utils_init();

    // proxy notifiche panel
    panel_notifications = moko_notifications_connect(MOKOSUITE_SERVICE ".panel", MOKO_NOTIFICATIONS_DEFAULT_PATH);

    /* inizializza il servizio Phone */
    phone_service = moko_phone_service_new();

    /* inizializza il servizio Settings */
    db_path = config_get_string("phone", "phonedb", PHONEDB_PATH);
    settings_service = moko_settings_service_new(MOKO_PHONE_SETTINGS_PATH, (const char *)db_path, "phone");
    g_free(db_path);

    /* inizializza il database dei contatti */
    db_path = config_get_string("phone", "contactsdb", CONTACTSDB_PATH);
    contactsdb_init(db_path);
    g_free(db_path);

    gsm_init(settings_service);

    sound_init(settings_service);

    phone_win_init();
    phone_call_win_init(settings_service);

    return moko_factory_run();
}
