#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "callsdb.h"

#include <string.h>
#include <sys/stat.h>
#include <libmokosuite/misc.h>

#include <freesmartphone-glib/freesmartphone-glib.h>
#include <freesmartphone-glib/opimd/calls.h>
#include <freesmartphone-glib/opimd/call.h>
#include <freesmartphone-glib/opimd/callquery.h>

typedef struct {
    /* liberati subito */
    GHashTable* query;
    /* mantenuti fino alla fine */
    GTimer* timer;
    CallEntryFunc func;
    gpointer userdata;
    char* path;
} query_data_t;

static void _cb_next(GError* error, GHashTable* row, gpointer userdata)
{
    query_data_t* data = userdata;

    if (error) {
        g_debug("[%s] Call row error: %s", __func__, error->message);
        g_debug("[%s] Call log loading took %f seconds", __func__, g_timer_elapsed(data->timer, NULL));
        g_timer_destroy(data->timer);

        g_free(data->path);
        g_free(data);
        return;
    }

    const char* _peer = fso_get_attribute(row, "Peer");

    if (_peer != NULL) {
        CallEntry *e = g_new0(CallEntry, 1);

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

static void _cb_query(GError* error, const char* path, gpointer userdata)
{
    query_data_t* data = userdata;
    g_debug("[%s] query took %f seconds", __func__, g_timer_elapsed(data->timer, NULL));
    g_hash_table_destroy(data->query);

    if (error) {
        g_debug("[%s] Call query error: %s", __func__, error->message);
        g_free(data);
        return;
    }

    data->path = g_strdup(path);
    opimd_callquery_get_result(data->path, _cb_next, data);
}


void callsdb_foreach_call(CallEntryFunc func, gpointer data)
{
    g_return_if_fail(func != NULL);
    if (opimdCallsBus == NULL) return;

    query_data_t* cbdata = g_new0(query_data_t, 1);

    cbdata->timer = g_timer_new();
    cbdata->query = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
    cbdata->userdata = data;
    cbdata->func = func;

    g_hash_table_insert(cbdata->query, g_strdup("_sortby"),
        g_value_from_string("Timestamp"));

    g_hash_table_insert(cbdata->query, g_strdup("_sortdesc"),
        g_value_from_int(1));

    opimd_calls_query(cbdata->query, _cb_query, cbdata);
}


typedef struct {
    /* liberati subito */
    GHashTable* query;
    /* mantenuti fino alla fine */
    GTimer* timer;
    CallEntry* entry;
    CallEntryFunc func;
    gpointer userdata;
} new_call_data_t;

static void _cb_new_call(GError* error, const char* path, gpointer userdata)
{
    new_call_data_t* data = userdata;
    g_debug("[callsdb_new_call] query took %f seconds", g_timer_elapsed(data->timer, NULL));

    g_hash_table_destroy(data->query);
    g_timer_destroy(data->timer);

    if (error) {
        g_debug("[callsdb_new_call] New call error: %s", error->message);
        g_free(data->entry->peer);
        g_free(data->entry);
        g_free(data);
        return;
    }

    // estrai id dal path
    const char* last_slash = g_strrstr(path, "/");
    if (last_slash) {
        last_slash++;
        data->entry->id = g_ascii_strtoll(last_slash, NULL, 10);
    }

    g_debug("[callsdb_new_call] New call created: %s", path);
    data->func(data->entry, data->userdata);

    g_free(data);
}

void callsdb_new_call(CallDirection direction, const char* peer,
    guint64 timestamp, guint64 duration,
    gboolean answered, gboolean is_new,
    CallEntryFunc func,
    gpointer userdata)
{
    if (opimdCallsBus == NULL) return;

    new_call_data_t* cbdata = g_new0(new_call_data_t, 1);
    cbdata->timer = g_timer_new();
    cbdata->func = func;
    cbdata->userdata = userdata;
    cbdata->query = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

    // popola dati per l'inserimento
    g_hash_table_insert(cbdata->query, g_strdup("Direction"),
        g_value_from_string(direction == DIRECTION_INCOMING ? "in" : "out"));
    g_hash_table_insert(cbdata->query, g_strdup("Peer"),
        g_value_from_string(peer));
    g_hash_table_insert(cbdata->query, g_strdup("Timestamp"),
        g_value_from_int(timestamp));

    char* _duration = g_strdup_printf("%llu", duration);
    g_hash_table_insert(cbdata->query, g_strdup("Duration"),
        g_value_from_string(_duration));
    g_free(_duration);

    g_hash_table_insert(cbdata->query, g_strdup("Answered"),
        g_value_from_int(answered));
    g_hash_table_insert(cbdata->query, g_strdup("New"),
        g_value_from_int(is_new));

    // prepara la entry per il callback
    cbdata->entry = g_new0(CallEntry, 1);
    cbdata->entry->direction = direction;
    cbdata->entry->peer = g_strdup(peer);
    cbdata->entry->timestamp = timestamp;
    cbdata->entry->duration = duration;
    cbdata->entry->answered = answered;
    cbdata->entry->is_new = is_new;

    opimd_calls_add(cbdata->query, _cb_new_call, cbdata);
}


typedef struct {
    GHashTable* query;
    GTimer* timer;
} set_call_new_data_t;

static void _cb_set_call_new(GError* error, gpointer userdata)
{
    if (error) {
        g_debug("[callsdb_set_call_new] Set new call error: %s", error->message);
    }

    set_call_new_data_t* data = userdata;
    g_debug("[callsdb_set_call_new] query took %f seconds", g_timer_elapsed(data->timer, NULL));

    g_timer_destroy(data->timer);
    g_hash_table_destroy(data->query);
    g_free(data);
}

void callsdb_set_call_new(gint64 id, gboolean is_new)
{
    // FIXME FIXME bunk update FIXME FIXME
    if (id < 0) return;

    if (opimdCallsBus == NULL) return;

    char* path = g_strdup_printf("/org/freesmartphone/PIM/Calls/%lld", id);

    set_call_new_data_t* cbdata = g_new0(set_call_new_data_t, 1);
    cbdata->timer = g_timer_new();
    cbdata->query = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

    g_hash_table_insert(cbdata->query, g_strdup("New"), g_value_from_int(is_new));

    opimd_call_update(path, cbdata->query, _cb_set_call_new, cbdata);
    g_free(path);
}


static void _cb_delete(GError* error, gpointer userdata)
{
    // ignore error...?
    GTimer* t = userdata;
    g_debug("[callsdb_delete_call] query took %f seconds", g_timer_elapsed(t, NULL));
    g_timer_destroy(t);
}

gboolean callsdb_delete_call(gint64 id)
{
    if (opimdCallsBus == NULL) return FALSE;

    GTimer* t = g_timer_new();
    char* path = g_strdup_printf("/org/freesmartphone/PIM/Calls/%lld", id);
    opimd_call_delete(path, _cb_delete, t);
    g_free(path);

    return TRUE;
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

