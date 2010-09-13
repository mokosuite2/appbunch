#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "callsdb.h"

#include <string.h>
#include <sys/stat.h>
#include <libmokosuite/misc.h>

#include <freesmartphone-glib/freesmartphone-glib.h>
#include <freesmartphone-glib/opimd/calls.h>
#include <freesmartphone-glib/opimd/callquery.h>

typedef struct {
    /* liberati subito */
    GHashTable* query;
    GTimer* timer;
    /* mantenuti fino alla fine */
    CallEntryFunc func;
    gpointer userdata;
    char* path;
} query_data_t;

static void _cb_next(GError* error, GHashTable* row, gpointer userdata)
{
    query_data_t* data = userdata;

    if (error) {
        g_debug("[%s] Call row error: %s", __func__, error->message);
        g_free(data->path);
        g_free(data);
        return;
    }

    //g_debug("[%s] Call row entry %p", __func__, row);
    const char* _peer = fso_get_attribute(row, "Peer");

    if (_peer != NULL) {
        CallEntry *e = g_new0(CallEntry, 1);

        // l'id deve essere > 0, altrimenti salta tutto
        e->id = fso_get_attribute_int(row, "EntryId");
        //g_debug("[%s] Call entry id %lld", __func__, e->id);

        e->peer = g_strdup(_peer);
        const char* _direction = fso_get_attribute(row, "Direction");
        e->direction = !strcasecmp(_direction, "in") ? DIRECTION_INCOMING : DIRECTION_OUTGOING;

        e->timestamp = fso_get_attribute_int(row, "Timestamp");

        const char* _duration = fso_get_attribute(row, "Duration");
        e->duration = (_duration != NULL) ? g_ascii_strtoull(_duration, NULL, 10) : 0;

        e->answered = (fso_get_attribute_int(row, "Answered") != 0);
        e->is_new = (fso_get_attribute_int(row, "New") != 0);

        //g_debug("Processing call to %s (new=%d, answered=%d, direction=%d (IN=%d, OUT=%d)",
        //        e->peer, e->is_new, e->answered, e->direction, DIRECTION_INCOMING, DIRECTION_OUTGOING);
        (data->func)(e, data->userdata);
    }

    opimd_callquery_get_result(data->path, _cb_next, data);
}

static void _cb_query(GError* error, char* path, gpointer userdata)
{
    query_data_t* data = userdata;
    g_debug("[%s] query took %f seconds", __func__, g_timer_elapsed(data->timer, NULL));
    g_timer_destroy(data->timer);
    g_hash_table_destroy(data->query);

    data->path = g_strdup(path);
    opimd_callquery_get_result(data->path, _cb_next, data);
}


void callsdb_foreach_call(CallEntryFunc func, gpointer data)
{
    g_return_if_fail(func != NULL);
    if (opimdCallsBus == NULL) return;

    query_data_t* cbdata = g_new0(query_data_t, 1);

    cbdata->timer = g_timer_new();
    cbdata->query = g_hash_table_new(g_str_hash, g_str_equal);
    cbdata->userdata = data;
    cbdata->func = func;

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

