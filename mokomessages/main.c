#include <libmokosuite/mokosuite.h>
#include <freesmartphone-glib/freesmartphone-glib.h>
#include <phone-utils.h>

#include "threadwin.h"
#include "messagesdb.h"

int main(int argc, char* argv[]) {
    g_debug(PACKAGE " version " VERSION " started");

    moko_factory_init(argc, argv, PACKAGE, VERSION);

    freesmartphone_glib_init();
    phone_utils_init();

    // TODO
    messagesdb_init(NULL, NULL);

    thread_win_init(NULL);

    // TEST mostra la finestra delle conversazioni
    thread_win_activate();

    return moko_factory_run();
}
