#ifndef __MOKOSETTINGS_SERVICE_H
#define __MOKOSETTINGS_SERVICE_H

#include "mokosuite.h"

#define MOKO_SETTINGS_INTERFACE            MOKOSUITE_SERVICE ".Settings"

#include <glib.h>
#include <glib-object.h>
#include <dbus/dbus-glib.h>
#include <db.h>

enum
{
    PROP_0,

    PROP_DBUS_PATH,
    PROP_DB_PATH,
    PROP_APP_NAME
};

G_BEGIN_DECLS

#define MOKO_TYPE_SETTINGS_SERVICE            (moko_settings_service_get_type ())
#define MOKO_SETTINGS_SERVICE(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), MOKO_TYPE_SETTINGS_SERVICE, MokoSettingsService))
#define MOKO_SETTINGS_SERVICE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), MOKO_TYPE_SETTINGS_SERVICE, MokoSettingsServiceClass))
#define MOKO_IS_SETTINGS_SERVICE(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOKO_TYPE_SETTINGS_SERVICE))
#define MOKO_IS_SETTINGS_SERVICE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MOKO_TYPE_SETTINGS_SERVICE))
#define MOKO_SETTINGS_SERVICE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), MOKO_TYPE_SETTINGS_SERVICE, MokoSettingsServiceClass))


typedef struct _MokoSettingsService MokoSettingsService;
typedef struct _MokoSettingsServiceClass MokoSettingsServiceClass;

GType moko_settings_service_get_type(void);

struct _MokoSettingsService {
    GObject parent;

    /*< private >*/
    char *dbus_path;
    char *db_path;
    DB *db;
    char* app_name;
    GHashTable *store;
    GHashTable *callbacks;
};

struct _MokoSettingsServiceClass {
    GObjectClass parent;

    DBusGConnection *connection;
};

typedef void (*MokoSettingsCallback)(MokoSettingsService *object, const char *key, const char *value);

/**
 * Aggiunge un callback per un cambiamento di un settaggio.
 * TODO
 */
void moko_settings_service_callback_add(MokoSettingsService * object, const char *key, MokoSettingsCallback callback);

/**
 * Rimuove un callback per un cambiamento di un settaggio.
 * TODO
 */
void moko_settings_service_callback_remove(MokoSettingsService * object, const char *key);

/**
 * Recupera un settaggio.
 * @param key chiave del settaggio
 * @param default_val valore di default se il settaggio non è stato trovato
 */
gboolean moko_settings_service_get_setting(MokoSettingsService * object, const char *key, const char *default_val, char **value, GError **error);

/**
 * Imposta un settaggio.
 * @param key chiave del settaggio
 * @param value valore del settaggio
 */
gboolean moko_settings_service_set_setting(MokoSettingsService * object, const char *key, const char *value, GError **error);

/**
 * Recupera tutti i settaggi.
 */
gboolean moko_settings_service_get_all_settings(MokoSettingsService * object, GHashTable **values, GError **error);

/**
 * Crea un server D-Bus per le impostazioni remote.
 * @param dbus_path path D-Bus per il server
 * @param db_path percorso del database di configurazione
 * @return un nuovo server D-Bus per le impostazioni remote
 */
MokoSettingsService *moko_settings_service_new(const char *dbus_path, const char *db_path, const char *app_name);


G_END_DECLS

#endif  /* __MOKOSETTINGS_SERVICE_H */
