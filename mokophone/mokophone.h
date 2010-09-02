#ifndef __MOKOPHONE_H
#define __MOKOPHONE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <libmokosuite/mokosuite.h>

/* path predefinito db chiamate */
#define CALLSDB_PATH                SYSCONFDIR "/mokosuite/calls.db"

/* path predefinito db telefono (impostazioni e altro) */
#define PHONEDB_PATH                SYSCONFDIR "/mokosuite/phone.db"

/* path predefinito db contatti */
#define CONTACTSDB_PATH             SYSCONFDIR "/mokosuite/contacts.db"

#define MOKO_PHONE_NAME                 MOKOSUITE_SERVICE ".phone"
#define MOKO_PHONE_INTERFACE            MOKOSUITE_SERVICE ".Phone"
#define MOKO_PHONE_PATH                 MOKOSUITE_PATH "/Phone"

#define MOKO_PHONE_SETTINGS_PATH                 MOKO_PHONE_PATH "/Settings"

#include <glib-object.h>

#define MOKO_TYPE_PHONE_SERVICE            (moko_phone_service_get_type ())
#define MOKO_PHONE_SERVICE(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), MOKO_TYPE_PHONE_SERVICE, MokoPhoneService))
#define MOKO_PHONE_SERVICE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), MOKO_TYPE_PHONE_SERVICE, MokoPhoneServiceClass))
#define MOKO_IS_PHONE_SERVICE(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOKO_TYPE_PHONE_SERVICE))
#define MOKO_IS_PHONE_SERVICE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MOKO_TYPE_PHONE_SERVICE))
#define MOKO_PHONE_SERVICE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), MOKO_TYPE_PHONE_SERVICE, MokoPhoneServiceClass))


typedef struct _MokoPhoneService MokoPhoneService;
typedef struct _MokoPhoneServiceClass MokoPhoneServiceClass;

GType moko_phone_service_get_type(void);

struct _MokoPhoneService {
    GObject parent;
};

struct _MokoPhoneServiceClass {
    GObjectClass parent;
    DBusGConnection *connection;
};

gboolean moko_phone_frontend(MokoPhoneService *object, const char *section, GError **error);
gboolean moko_phone_list_unread_missed_calls(MokoPhoneService *object, gint64 id, const char *peer,
    guint64 timestamp, guint64 duration, gboolean answered, GError **error);
gboolean moko_phone_get_missed_call(MokoPhoneService *object, gint64 id, gpointer *out, GError **error);

MokoPhoneService *moko_phone_service_new(void);

#endif  /* __MOKOPHONE_H */

