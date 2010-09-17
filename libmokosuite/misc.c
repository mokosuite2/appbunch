#include "mokosuite.h"
#include "misc.h"

#include <glib-object.h>
#include <glib/gi18n-lib.h>
#include <sys/stat.h>

#define DAYS(x)     (24*60*60*x)

/**
 * Restituisce un attributo in una HashTable proveniente da FSO.
 */
const char* fso_get_attribute(GHashTable* properties, const char* key)
{
    const char* value = NULL;
    GValue* gval = NULL;

    gval = g_hash_table_lookup(properties, key);

    if (gval != NULL) value = g_value_get_string(gval);

    return value;
}

int fso_get_attribute_int(GHashTable* properties, const char* key)
{
    int value = 0;
    GValue* gval = NULL;

    gval = g_hash_table_lookup(properties, key);
    //g_debug("Type=%s", G_VALUE_TYPE_NAME(gval));

    if (gval != NULL) value = g_value_get_int(gval);

    return value;
}

void g_value_free(gpointer data)
{
    GValue* value = data;
    g_value_unset(value);
    g_free(value);
}

GValue* g_value_from_string(const char* string)
{
    GValue* value = g_new0(GValue, 1);
    g_value_init(value, G_TYPE_STRING);
    g_value_set_string(value, string);

    return value;
}

GValue* g_value_from_int(int number)
{
    GValue* value = g_new0(GValue, 1);
    g_value_init(value, G_TYPE_INT);
    g_value_set_int(value, number);

    return value;
}

guint64 get_current_time(void)
{
    GTimeVal tv = {0};
    g_get_current_time(&tv);

    return (guint64)tv.tv_sec;
}

/**
 * Restituisce una rappresentazione sensata del timestamp passato.
 * La stringa è già tradotta e andrà liberata.
 */
char* get_time_repr(guint64 timestamp)
{
    guint64 now = get_current_time();
    gint64 diff = now - timestamp;  // differenza

    struct tm* timestamp_tm = localtime((const time_t*)&timestamp);

    char* ret = NULL;
    char strf[100+1] = {0, };

    //g_debug("now(%llu) - timestamp(%llu) = %lld", now, timestamp, diff);

    if (diff < DAYS(1)) {
        strftime(strf, 100, "%H:%M", timestamp_tm);
        ret = g_strdup(strf);
    }

    else if (diff < DAYS(2)) {
        ret = g_strdup(_("Yesterday"));
    }

    // FIXME calcolo dinamico giorni
    else if (diff < DAYS(3)) {
        ret = g_strdup(_("2 days ago"));
    }

    //else if (diff > DAYS(2) && diff < DAYS(... TODO

    else {
        strftime(strf, 100, "%d %b", timestamp_tm);
        ret = g_strdup(strf);
    }

    //g_debug("Returning time repr: \"%s\"", ret);
    return ret;
}

char* get_time_repr_full(guint64 timestamp)
{
    char strf[100+1] = {0, };
    struct tm* timestamp_tm = localtime((const time_t*)&timestamp);

    strftime(strf, 100, "%F %T", timestamp_tm);
    return g_strdup(strf);
}

time_t get_modification_time(const char* path)
{
    struct stat st = {0};
    return (!stat(path, &st)) ? st.st_mtime : -1;
}
