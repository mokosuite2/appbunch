#ifndef __MOKO_FSO_H
#define __MOKO_FSO_H

#include <frameworkd-glib/frameworkd-glib-dbus.h>
#include <frameworkd-glib/ousaged/frameworkd-glib-ousaged.h>

void fso_handlers_add(FrameworkdHandler* handlers);

void fso_init(void);

#endif  /* __MOKO_FSO_H */
