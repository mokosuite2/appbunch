#include <libmokosuite/mokosuite.h>
#include <stdlib.h>


int main(int argc, char* argv[]) {
    g_debug(PACKAGE " version " VERSION " started");

    moko_factory_init(argc, argv, PACKAGE, VERSION);

    // TODO inizializza tutto!!!!!!!!! :-o

    return moko_factory_run();
}
