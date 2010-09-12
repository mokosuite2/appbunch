#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "callsdb.h"

#include <string.h>
#include <sys/stat.h>
#include <libmokosuite/misc.h>

#include <freesmartphone-glib/freesmartphone-glib.h>
#include <freesmartphone-glib/opimd/calls.h>

typedef struct {
    GHashTable* query;
    GTimer* timer;
    gpointer userdata;
} query_data_t;

static void _cb_query(GError* error, char* path, gpointer userdata)
{
    query_data_t* data = userdata;
    g_debug("[%s] query took %f seconds", __func__, g_timer_elapsed(data->timer, NULL));

    g_timer_destroy(data->timer);
}


void callsdb_foreach_call(CallEntryFunc func, gpointer data)
{
    g_return_if_fail(func != NULL);
    if (opimdCallsBus == NULL) return;

    query_data_t* cbdata = g_new0(query_data_t, 1);

    cbdata->timer = g_timer_new();
    cbdata->query = g_hash_table_new(g_str_hash, g_str_equal);
    cbdata->userdata = data;

    opimd_calls_query(cbdata->query, _cb_query, cbdata);
}

CallEntry* callsdb_get_call(gint64 id)
{
    // TODO
    return NULL;
}

gint64 callsdb_new_call(CallDirection direction, const char* peer,
    guint64 timestamp, guint64 duration,
    gboolean answered, gboolean is_new)
{
    if (opimdCallsBus == NULL) return 0;

    GTimer* t = g_timer_new();
    g_debug("[%s] query took %f seconds", __func__, g_timer_elapsed(t, NULL));
    g_timer_destroy(t);

    return -1;
}

void callsdb_set_call_new(gint64 id, gboolean is_new)
{
    if (opimdCallsBus == NULL) return;

    GTimer* t = g_timer_new();
    g_debug("[%s] query took %f seconds", __func__, g_timer_elapsed(t, NULL));
    g_timer_destroy(t);
}

gboolean callsdb_delete_call(gint64 id)
{
    if (opimdCallsBus == NULL) return FALSE;

    GTimer* t = g_timer_new();

    g_debug("[%s] query took %f seconds", __func__, g_timer_elapsed(t, NULL));
    g_timer_destroy(t);

    // TODO :)
    return FALSE;
}

gboolean callsdb_truncate(void)
{

    GTimer* t = g_timer_new();
    g_debug("[%s] query took %f seconds", __func__, g_timer_elapsed(t, NULL));
    g_timer_destroy(t);

    // TODO
    return FALSE;
}

void callsdb_init(void)
{
    opimd_calls_dbus_connect();

    if (opimdCallsBus == NULL)
        g_warning("Unable to connect to calls database; will not be able to log calls");
}

