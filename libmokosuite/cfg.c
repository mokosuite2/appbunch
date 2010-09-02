
#include "mokosuite.h"

#define CONFIG_FILE_PATH SYSCONFDIR     "/mokosuite.conf"
#define SAVE_SCHEDULE_TIMEOUT           3

static GKeyFile *config = NULL;
static guint save_job = 0;

static gboolean _save_job(gpointer data)
{
    save_job = 0;
    config_save();
    return FALSE;
}

static void schedule_save(void)
{
    if (!save_job)
        save_job = g_timeout_add_seconds(SAVE_SCHEDULE_TIMEOUT, _save_job, NULL);
}

void config_init(void)
{
    if (config) return;

    GError *error = NULL;

    config = g_key_file_new();

    g_key_file_load_from_file (config, CONFIG_FILE_PATH, G_KEY_FILE_NONE, &error);
    if (error != NULL)
    {
        g_debug("Error loading configuration, creating new");
        g_error_free(error);
        // TODO :D ehm...
    }
}

gboolean config_save(void)
{
    return g_file_set_contents(CONFIG_FILE_PATH, g_key_file_to_data(config, NULL, NULL), -1, NULL);
}

gboolean config_has_key(const char *group, const char *key)
{
    // nella documentazione non cita NULL per l'error... boh...
    return g_key_file_has_key(config, group, key, NULL);
}

char *config_get_string(const char *group, const char *key, const char *default_val)
{
    GError *error = NULL;
    char *ret = NULL;

    ret = g_key_file_get_string (config, group, key, &error);
    if (error != NULL)
    {
        g_error_free(error);
        ret = g_strdup(default_val);
    }

    return ret;
}

int config_get_integer(const char *group, const char *key, int default_val)
{
    GError *error = NULL;
    int ret;

    ret = g_key_file_get_integer (config, group, key, &error);
    if (error != NULL)
    {
        g_error_free(error);
        ret = default_val;
    }

    return ret;
}

void config_set_string(const char* group, const char* key, const char* value)
{
    g_key_file_set_string(config, group, key, value);
    schedule_save();
}

void config_set_integer(const char* group, const char* key, int value)
{
    g_key_file_set_integer(config, group, key, value);
    schedule_save();
}
