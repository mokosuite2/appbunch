#ifndef __MOKOAPP_H
#define __MOKOAPP_H

#include <glib.h>

gboolean moko_factory_init(int argc, char *argv[], const char *package, const char *version);

int moko_factory_run(void);

void moko_factory_exit(void);

#endif /* __MOKOAPP_H */
