#include <libmokosuite/mokosuite.h>
#include <libmokosuite/fso.h>

#include "threadwin.h"

int main(int argc, char* argv[]) {
    g_debug(PACKAGE " version " VERSION " started");

    moko_factory_init(argc, argv, PACKAGE, VERSION);

    // inizializza FSO
    fso_init();

    thread_win_init(NULL);

    // TEST mostra la finestra delle conversazioni
    thread_win_activate();

    return moko_factory_run();
}
