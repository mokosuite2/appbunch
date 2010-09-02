#ifndef __MOKOSUITE_H
#define __MOKOSUITE_H

#include <glib.h>
#include <dbus/dbus-glib.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "application.h"
#include "settings.h"
#include "cfg.h"

#define MOKOSUITE_SERVICE                    "org.mokosuite"
#define MOKOSUITE_PATH                       "/org/mokosuite"

#define MOKOSUITE_DATADIR                   DATADIR "/mokosuite/"

#ifdef QVGA
#define MOKOSUITE_SCALE_FACTOR              1.0
#else
#define MOKOSUITE_SCALE_FACTOR              2.0
#endif

/* bus di sistema */
extern DBusGConnection *system_bus;

#endif  /* __MOKOSUITE_H */
