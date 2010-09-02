#include <dbus/dbus-glib-bindings.h>
#include <string.h>

#include "settings-service.h"
#include "settings-service-glue.h"

G_DEFINE_TYPE(MokoSettingsService, moko_settings_service, G_TYPE_OBJECT)

#define SETTINGS_DB_SCHEMA  "CREATE TABLE IF NOT EXISTS settings (name VARCHAR(30) PRIMARY KEY NOT NULL, value TEXT(1024) NOT NULL);"

static void
moko_settings_service_set_property (GObject      *object,
                        guint         property_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
    MokoSettingsService *self = MOKO_SETTINGS_SERVICE(object);
    MokoSettingsServiceClass *klass = MOKO_SETTINGS_SERVICE_GET_CLASS(object);

    switch (property_id)
    {
        case PROP_DBUS_PATH:
            g_free (self->dbus_path);
            self->dbus_path = g_value_dup_string (value);

            /* Register DBUS path */
            dbus_g_connection_register_g_object(klass->connection, self->dbus_path, G_OBJECT (self));

            break;

        case PROP_DB_PATH:

            #ifdef SETTINGSDB_CFG
            config_init();
            #else
            if (self->db != NULL) {
                self->db->close(self->db, 0);
                self->db = NULL;
            }

            g_free (self->db_path);
            self->db_path = g_value_dup_string (value);

            if (db_create(&self->db, NULL, 0) != 0) {
                if (self->db != NULL) {
                    self->db->close(self->db, 0);
                    self->db = NULL;
                }
            }

            if (self->db != NULL && self->db->open(self->db, NULL, self->db_path, NULL, DB_HASH, DB_CREATE, 0) != 0) {
                self->db->close(self->db, 0);
                self->db = NULL;
            }

            if (self->db == NULL)
                g_warning("Unable to open application database; will not be able to store settings");
            #endif

            break;

        case PROP_APP_NAME:
            if (self->app_name) {
                g_free(self->app_name);
                self->app_name = NULL;
            }

            self->app_name = g_value_dup_string (value);
            break;

        default:
            /* We don't have any other property... */
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}

static void
moko_settings_service_get_property (GObject    *object,
                        guint       property_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
    MokoSettingsService *self = MOKO_SETTINGS_SERVICE(object);

    switch (property_id)
    {
        case PROP_DBUS_PATH:
            g_value_set_string (value, self->dbus_path);
            break;

        case PROP_DB_PATH:
            g_value_set_string (value, self->db_path);
            break;

        case PROP_APP_NAME:
            g_value_set_string (value, self->app_name);
            break;

        default:
            /* We don't have any other property... */
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}


static void
moko_settings_service_class_init(MokoSettingsServiceClass * klass)
{
    GError *error = NULL;
    GObjectClass *g_objectclass = G_OBJECT_CLASS(klass);

    g_objectclass->set_property = moko_settings_service_set_property;
    g_objectclass->get_property = moko_settings_service_get_property;

    /* registra la proprieta' path dbus */
    GParamSpec *p_dbus_path = g_param_spec_string ("dbus-path",
        "DBus path",
        "Set the DBus path to bind to",
        NULL /* default value */,
        G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

    g_object_class_install_property (g_objectclass, PROP_DBUS_PATH, p_dbus_path);

    /* registra la proprieta' path dbus */
    GParamSpec *p_db_path = g_param_spec_string ("db-path",
        "Database path",
        "Set the database path to store settings",
        NULL /* default value */,
        G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

    g_object_class_install_property (g_objectclass, PROP_DB_PATH, p_db_path);

    /* registra la proprieta' app name */
    GParamSpec *p_app_name = g_param_spec_string ("app-name",
        "App Name",
        "Set the app name to store settings",
        NULL /* default value */,
        G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

    g_object_class_install_property (g_objectclass, PROP_APP_NAME, p_app_name);

    /* Init the DBus connection, per-klass */
    klass->connection = dbus_g_bus_get (DBUS_BUS_SYSTEM, &error);
    if (klass->connection == NULL) {
        g_error("Unable to connect to dbus: %s", error->message);
        g_error_free (error);
        return;
    }

    dbus_g_object_type_install_info (MOKO_TYPE_SETTINGS_SERVICE,
            &dbus_glib_moko_settings_service_object_info);


}

static void
moko_settings_service_init(MokoSettingsService *object)
{
    // ehm... :)
    object->db = NULL;

    // inizializza lo storage in memoria
    object->store = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

    // mappa dei callback
    object->callbacks = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
}


MokoSettingsService *
moko_settings_service_new(const char *dbus_path, const char *db_path, const char *app_name)
{
    return g_object_new(MOKO_TYPE_SETTINGS_SERVICE, "dbus-path", dbus_path, "db-path", db_path, "app-name", app_name, NULL);
}

void moko_settings_service_callback_add(MokoSettingsService * object, const char *key, MokoSettingsCallback callback)
{
    g_hash_table_replace(object->callbacks, g_strdup(key), callback);
}

void moko_settings_service_callback_remove(MokoSettingsService * object, const char *key)
{
    g_hash_table_remove(object->callbacks, key);
}

gboolean moko_settings_service_get_setting(MokoSettingsService * object, const char *key, const char *default_val, char **value, GError **error)
{
    // resetta il puntatore (mannaggia a te!!...)
    *value = NULL;

    // tenta prima caricamento dallo store in memoria
    gpointer val = g_hash_table_lookup(object->store, key);
    if (val != NULL) {
        *value = g_strdup(val);

    } else {

        // ok, tentiamo nel database
        #ifdef SETTINGSDB_CFG
        *value = config_get_string(object->app_name, key, default_val);
        #else
        //g_debug("get_setting(%s) db = %p", key, object->db);
        if (object->db != NULL) {

            DBT db_key = {0}, db_data = {0};
            db_data.flags = DB_DBT_MALLOC;

            db_key.data = (void *) key;
            db_key.size = strlen(key) + 1;

            int ret = object->db->get(object->db, NULL, &db_key, &db_data, 0);
            if (ret == 0) {
                // copia puntatore :)
                *value = db_data.data;
                //g_debug("get_setting(%s) value = \"%s\"", key, *value);
            }

            else if (ret != DB_NOTFOUND) {
                g_warning("Error reading setting %s", key);
            }
        }
        #endif
    }

    // cavolo, niente da fare... valore di default :(
    if (*value == NULL) *value = g_strdup(default_val);

    return TRUE;
}

gboolean moko_settings_service_get_all_settings(MokoSettingsService * object, GHashTable **values, GError **error)
{
    // TODO
    return FALSE;
}

gboolean moko_settings_service_set_setting(MokoSettingsService * object, const char *key, const char *value, GError **error)
{
    #ifdef SETTINGSDB_CFG
    config_set_string(object->app_name, key, value);
    #else
    if (object->db != NULL) {
        // opera sul db
        DBT db_key = {0};
        db_key.data = (void *) key;
        db_key.size = strlen(key) + 1;

        if (value != NULL) {

            DBT db_data = {0};
            db_data.data = (void *) value;
            db_data.size = strlen(value) + 1;

            /*int ret = */
            object->db->put(object->db, NULL, &db_key, &db_data, 0);
            //g_debug("set_setting(%s) ret = %d", key, ret);
            // FIXME e gli errori??? :S

        } else {

            DBT db_key = {0};
            db_key.data = (void *) key;
            db_key.size = strlen(key) + 1;

            object->db->del(object->db, NULL, &db_key, 0);
        }

        object->db->sync(object->db, 0);
    }
    #endif

    if (value != NULL) {
        g_hash_table_replace(object->store, g_strdup(key), g_strdup(value));
    } else {
        g_hash_table_remove(object->store, key);
    }

    // richiama il callback se presente
    MokoSettingsCallback callback = g_hash_table_lookup(object->callbacks, key);
    if (callback != NULL)
        callback(object, key, value);

    return TRUE;
}
