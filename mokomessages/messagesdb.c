#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "messagesdb.h"

#include <string.h>
#include <sys/stat.h>
#include <phone-utils.h>
#include <libmokosuite/misc.h>

#include <freesmartphone-glib/freesmartphone-glib.h>
#include <freesmartphone-glib/opimd/messages.h>
#include <freesmartphone-glib/opimd/message.h>
#include <freesmartphone-glib/opimd/messagequery.h>

static GHashTable* threads = NULL;

static void handle_message_for_thread(GHashTable* row)
{
    const char* _peer = fso_get_attribute(row, "Peer");

    if (_peer != NULL) {
        char* peer = phone_utils_normalize_number(_peer);
        if (peer == NULL) peer = g_strdup(_peer);

        g_debug("Handling message %p with %s", row, peer);
        MessageThread* t = g_hash_table_lookup(threads, peer);

        const char* _direction = fso_get_attribute(row, "Direction");
        int direction = !strcasecmp(_direction, "in") ? DIRECTION_INCOMING : DIRECTION_OUTGOING;
        gboolean unread = (g_hash_table_lookup(row, "MessageRead") == NULL) ?
            FALSE : (fso_get_attribute_int(row, "MessageRead") == 0);

        int timestamp = fso_get_attribute_int(row, "Timestamp");

        if (!t) {
            t = g_new0(MessageThread, 1);
            t->peer = g_strdup(peer);
            t->content = g_strdup(fso_get_attribute(row, "Content"));
            t->timestamp = timestamp;

            g_hash_table_insert(threads, g_strdup(peer), t);
        }

        // messaggio non letto -- precedenza sugli altri
        if (unread) {
            if (t->total_count > 0) {
                // content
                if (t->content)
                    g_free(t->content);

                t->content = g_strdup(fso_get_attribute(row, "Content"));

                // timestamp
                t->timestamp = timestamp;
            }

            // unread count
            t->unread_count++;
        }

        // direction
        t->direction = direction;

        // total count
        t->total_count++;

        g_free(peer);
    }
}

typedef struct {
    /* liberati subito */
    GHashTable* query;
    /* mantenuti fino alla fine */
    GTimer* timer;
    MessageThreadFunc func;
    gpointer userdata;
    char* path;
} query_data_t;

static void _thread_userfunc(gpointer key, gpointer value, gpointer userdata)
{
    query_data_t* data = userdata;
    (data->func)((MessageThread*) value, data->userdata);
}

static void _cb_thread_query(GError* error, const char* path, gpointer userdata);

static void _cb_thread_next(GError* error, GHashTable* row, gpointer userdata)
{
    query_data_t* data = userdata;

    if (error) {
        g_debug("[%s] message row error: %s", __func__, error->message);
        g_debug("[%s] message loading took %f seconds", __func__, g_timer_elapsed(data->timer, NULL));
        g_timer_destroy(data->timer);

        // distruggi query
        opimd_messagequery_dispose(data->path, NULL, NULL);

        // richiama il callback per ogni thread trovato
        if (data->func)
            g_hash_table_foreach(threads, _thread_userfunc, data);

        // distruggi dati callback
        g_free(data->path);
        g_free(data);

        return;
    }

    handle_message_for_thread(row);

    opimd_messagequery_get_result(data->path, _cb_thread_next, data);
}

static gboolean _retry_thread_query(gpointer userdata)
{
    query_data_t* data = userdata;
    g_timer_start(data->timer);

    opimd_messages_query(data->query, _cb_thread_query, data);
    return FALSE;
}

static void _cb_thread_query(GError* error, const char* path, gpointer userdata)
{
    query_data_t* data = userdata;
    g_debug("[messagesdb_foreach_thread] query took %f seconds", g_timer_elapsed(data->timer, NULL));

    if (error) {
        g_debug("[messagesdb_foreach_thread] Thread query error: (%d) %s", error->code, error->message);

        // opimd non ancora caricato? Riprova in 5 secondi
        if (FREESMARTPHONE_GLIB_IS_DBUS_ERROR(error, FREESMARTPHONE_GLIB_DBUS_ERROR_SERVICE_NOT_AVAILABLE)) {
            g_timeout_add_seconds(5, _retry_thread_query, data);
            return;
        }

        g_hash_table_destroy(data->query);
        g_timer_destroy(data->timer);
        g_free(data);
        return;
    }

    g_hash_table_destroy(data->query);

    data->path = g_strdup(path);
    opimd_messagequery_get_result(data->path, _cb_thread_next, data);
}

void messagesdb_foreach_thread(MessageThreadFunc func, gpointer data)
{
    g_return_if_fail(func != NULL);
    if (opimdMessagesBus == NULL) return;

    query_data_t* cbdata = g_new0(query_data_t, 1);

    cbdata->timer = g_timer_new();
    cbdata->query = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_value_free);
    cbdata->userdata = data;
    cbdata->func = func;

    g_hash_table_insert(cbdata->query, g_strdup("_sortby"),
        g_value_from_string("Timestamp"));

    g_hash_table_insert(cbdata->query, g_strdup("_sortdesc"),
        g_value_from_int(1));

    opimd_messages_query(cbdata->query, _cb_thread_query, cbdata);
}

void messagesdb_init(MessageThreadFunc func, gpointer userdata)
{
    opimd_messages_dbus_connect();

    if (opimdMessagesBus == NULL)
        g_warning("Unable to connect to messages database; will not be able to read messages");

    threads = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
}
