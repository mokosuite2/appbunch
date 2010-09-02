#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "callsdb.h"

#include <string.h>
#include <sys/stat.h>
#include <libmokosuite/misc.h>

time_t callsdb_timestamp = -1;
char* callsdb_path = NULL;

#ifdef CALLSDB_SQLITE
#include <sqlite3.h>

#ifdef USE_THREADS
static GAsyncQueue* cmd_queue = NULL;
#endif

static sqlite3 *db = NULL;

static sqlite3_stmt* new_call_stm = NULL;
static sqlite3_stmt* delete_call_stm = NULL;
static sqlite3_stmt* set_new_call_stm = NULL;
static sqlite3_stmt* set_new_all_stm = NULL;

#define CALLS_DB_SCHEMA     \
    "CREATE TABLE IF NOT EXISTS calls (\n" \
    "id INTEGER PRIMARY KEY,\n" \
    "peer TEXT,\n" \
    "direction INTEGER NOT NULL,\n" \
    "timestamp INTEGER NOT NULL,\n" \
    "duration INTEGER DEFAULT 0,\n" \
    "answered INTEGER DEFAULT 0,\n" \
    "new INTEGER DEFAULT 0\n" \
    ")"

#define ROWINDEX(row,col)     (ncolumn + (row * ncolumn) + col)

#else
#include <db.h>

static DB *db = NULL;

// chiave
typedef guint32 DBT_CallId;

// valore
typedef struct {
    guint64 timestamp;
    guint64 duration;

    CallDirection direction : 1;
    gboolean answered : 1;
    gboolean is_new : 1;
} __attribute__ ((packed)) DBT_Call;

#endif


void callsdb_foreach_call(CallEntryFunc func, gpointer data)
{
    g_return_if_fail(func != NULL);
    if (db == NULL) return;

#ifdef CALLSDB_SQLITE
    GTimer* t = g_timer_new();
    char **result = NULL, *errmsg = NULL;
    int nrow = 0, ncolumn = 0;
    int crow;

    // ordine per timestamp deprecato
    if (sqlite3_get_table(db, "SELECT * FROM calls ORDER BY id DESC", &result, &nrow, &ncolumn, &errmsg) == SQLITE_OK)
    {
        g_debug("[%s] query took %f seconds", __func__, g_timer_elapsed(t, NULL));

        if (nrow > 0) {

            for (crow = 0; crow < nrow; crow++) {

                CallEntry *e = g_new0(CallEntry, 1);

                // l'id deve essere > 0, altrimenti salta tutto
                e->id = g_ascii_strtoll(result[ROWINDEX(crow, 0)], NULL, 10);

                if (e->id > 0) {
                    e->peer = g_strdup(result[ROWINDEX(crow, 1)]);
                    e->direction = g_ascii_strtoull(result[ROWINDEX(crow, 2)], NULL, 10) ? DIRECTION_INCOMING : DIRECTION_OUTGOING;

                    e->timestamp = g_ascii_strtoull(result[ROWINDEX(crow, 3)], NULL, 10);
                    e->duration = g_ascii_strtoull(result[ROWINDEX(crow, 4)], NULL, 10);

                    e->answered = (g_ascii_strtoll(result[ROWINDEX(crow, 5)], NULL, 10) != 0);
                    e->is_new = (g_ascii_strtoll(result[ROWINDEX(crow, 6)], NULL, 10) != 0);

                    (func)(e, data);
                }

                else g_free(e);
            }

        }

        sqlite3_free_table(result);

        if (errmsg != NULL) {
            g_warning("Error reading call entry: %s", errmsg);
            sqlite3_free(errmsg);
        }

    }

    g_timer_destroy(t);
#else

    DBC* dbcp;
    int ret;

    /* Acquire a cursor for the database. */
    if (db->cursor(db, NULL, &dbcp, 0) != 0) {
        g_warning("Unable to acquire a cursor to calls database.");
        return;
    }

    DBT db_key = {0}, db_data = {0};

    /* Walk through the database */
    while ((ret = dbcp->c_get(dbcp, &db_key, &db_data, DB_PREV)) == 0) {
        printf("%lu : %.*s\n", *(u_long *)db_key.data, (int)db_data.size, (char *)db_data.data);

        CallEntry *e = g_new0(CallEntry, 1);

        // l'id deve essere > 0, altrimenti salta tutto
        e->id = *((DBT_CallId*)db_key.data);

        if (e->id > 0) {
            // il peer e' il primo elemento dei dati
            char* peer = (char *)db_data.data;
            if (strlen(peer))
                e->peer = g_strdup(db_data.data);

            DBT_Call* c = (db_data.data + strlen(peer) + 1);

            e->direction = c->direction;

            e->timestamp = c->timestamp;
            e->duration = c->duration;

            e->answered = c->answered;
            e->is_new = c->is_new;

            (func)(e, data);
        }

        else g_free(e);
    }

    if (ret != DB_NOTFOUND) {
        g_warning("Unable to iterate calls database cursor.");
        return;
    }

    /* Close the cursor. */
    dbcp->c_close(dbcp);

#endif
}

CallEntry* callsdb_get_call(
#ifdef CALLSDB_SQLITE
gint64 id
#else
guint32 id
#endif
)
{
    // TODO
    return NULL;
}

#ifdef CALLSDB_SQLITE
gint64
#else
guint32
#endif
callsdb_new_call(CallDirection direction, const char* peer,
    guint64 timestamp, guint64 duration,
    gboolean answered, gboolean is_new)
{
    if (db == NULL) return 0;

#ifdef CALLSDB_SQLITE
    gint64 retval = 0;

#ifdef USE_THREADS
    if (cmd_queue != NULL) {

        CallEntry* e = g_new0(CallEntry, 1);
        e->peer = g_strdup(peer);
        e->timestamp = timestamp;
        e->duration = duration;
        e->answered = answered;
        e->is_new = is_new;

        g_async_queue_push(cmd_queue, e);
        // niente retval
    }

    else {
#endif
    GTimer* t = g_timer_new();
    int rc;

    sqlite3_reset(new_call_stm);

    sqlite3_bind_text (new_call_stm, 1, g_strdup(peer), -1, g_free);
    sqlite3_bind_int (new_call_stm, 2, (int) direction);
    sqlite3_bind_int64 (new_call_stm, 3, (sqlite3_int64) timestamp);
    sqlite3_bind_int64 (new_call_stm, 4, (sqlite3_int64) duration);
    sqlite3_bind_int (new_call_stm, 5, (int) answered);
    sqlite3_bind_int (new_call_stm, 6, (int) is_new);

    rc = sqlite3_step (new_call_stm);
    g_debug("[%s] query took %f seconds", __func__, g_timer_elapsed(t, NULL));
    g_timer_destroy(t);

    if (rc == SQLITE_OK || rc == SQLITE_DONE) {
        retval = (gint64) sqlite3_last_insert_rowid(db);
    }

#ifdef USE_THREADS
    }   // cmd_queue != NULL
#endif

#else
    guint32 retval = 0;

    DBT_Call e = {0};
    e.direction = direction;
    e.timestamp = timestamp;
    e.duration = duration;
    e.answered = answered;
    e.is_new = is_new;

    if (peer == NULL)
        peer = "";

    size_t area_size = strlen(peer) + 1 + sizeof(e);

    void* area = g_malloc0(area_size);
    memcpy(area, peer, strlen(peer) + 1);
    memcpy(area + strlen(peer) + 1, &e, sizeof(e));

    DBT key = {0}, data = {0};
    data.data = area;
    data.size = area_size;

    if (db->put(db, NULL, &key, &data, DB_APPEND) == 0) {
        db->sync(db, 0);

        db_recno_t recno = *((db_recno_t*)key.data);
        retval = (DBT_CallId) recno;
    }

#endif

    callsdb_timestamp = get_modification_time(callsdb_path);
    return retval;
}

void callsdb_set_call_new(
#ifdef CALLSDB_SQLITE
gint64 id
#else
guint32 id
#endif
, gboolean is_new)
{
    if (db == NULL) return;

#ifdef CALLSDB_SQLITE
    GTimer* t = g_timer_new();
    int rc;

    sqlite3_stmt* stm = (id < 0) ? set_new_all_stm : set_new_call_stm;

    sqlite3_reset(stm);
    sqlite3_bind_int (stm, 1, (int) is_new);

    if (id >= 0)
        sqlite3_bind_int (stm, 2, (int) id);

    rc = sqlite3_step (stm);

    g_debug("[%s] query took %f seconds", __func__, g_timer_elapsed(t, NULL));
    g_timer_destroy(t);
#else
    // TODO
#endif

    callsdb_timestamp = get_modification_time(callsdb_path);
}

gboolean callsdb_delete_call(
#ifdef CALLSDB_SQLITE
gint64 id
#else
guint32 id
#endif
)
{
    if (db == NULL) return FALSE;

#ifdef CALLSDB_SQLITE
    GTimer* t = g_timer_new();
    int rc;

    sqlite3_reset(delete_call_stm);

    sqlite3_bind_int (delete_call_stm, 1, (int) id);

    rc = sqlite3_step (delete_call_stm);

    g_debug("[%s] query took %f seconds", __func__, g_timer_elapsed(t, NULL));
    g_timer_destroy(t);

    callsdb_timestamp = get_modification_time(callsdb_path);
    return (rc == SQLITE_OK || rc == SQLITE_DONE);
#else
    DBT key = {0};
    key.data = (void *)&id;
    key.size = sizeof(id);

    int ret = db->del(db, NULL, &key, 0);

    callsdb_timestamp = get_modification_time(callsdb_path);
    return (ret == 0 || ret == DB_KEYEMPTY);
#endif
}

void callsdb_truncate(void)
{

    // TODO riporta risultato cancellazione?
    if (db != NULL) {
#ifdef CALLSDB_SQLITE
        GTimer* t = g_timer_new();
        sqlite3_exec(db, "DELETE FROM calls", NULL, NULL, NULL);
        g_debug("[%s] query took %f seconds", __func__, g_timer_elapsed(t, NULL));
        g_timer_destroy(t);
#else
        guint32 countp;
        db->truncate(db, NULL, &countp, 0);
        db->sync(db, 0);
#endif
    }
}

#ifdef USE_THREADS
gpointer db_thread(gpointer data)
{
    CallEntry *e;

    while((e = (CallEntry *) g_async_queue_pop(cmd_queue))) {
        GTimer* t = g_timer_new();
        int rc;
        gint32 retval;

        sqlite3_reset(new_call_stm);

        sqlite3_bind_text (new_call_stm, 1, e->peer, -1, g_free);
        sqlite3_bind_int (new_call_stm, 2, (int) e->direction);
        sqlite3_bind_int64 (new_call_stm, 3, (sqlite3_int64) e->timestamp);
        sqlite3_bind_int64 (new_call_stm, 4, (sqlite3_int64) e->duration);
        sqlite3_bind_int (new_call_stm, 5, (int) e->answered);
        sqlite3_bind_int (new_call_stm, 6, (int) e->is_new);

        rc = sqlite3_step (new_call_stm);
        g_debug("[%s] query took %f seconds", __func__, g_timer_elapsed(t, NULL));
        g_timer_destroy(t);

        // il peer e' gia' stato deallocato da sqlite
        g_free(e);

        if (rc == SQLITE_OK || rc == SQLITE_DONE) {
            retval = (gint64) sqlite3_last_insert_rowid(db);
            g_debug("[%s] rowid = %d", __func__, retval);
        }
    }

    return NULL;
}
#endif

void callsdb_init(const char *db_path)
{
#ifdef CALLSDB_SQLITE
    if (sqlite3_open(db_path, &db) != SQLITE_OK)
        goto fail;

    // db aperto, crea lo schema
    else
        sqlite3_exec(db, CALLS_DB_SCHEMA, NULL, NULL, NULL);

    if (sqlite3_prepare_v2(db, "INSERT INTO calls (peer, direction, timestamp, duration, answered, new) VALUES (?, ?, ?, ?, ?, ?)",
        -1, &new_call_stm, NULL) != SQLITE_OK) goto fail;

    if (sqlite3_prepare_v2(db, "DELETE FROM calls WHERE id = ?", -1, &delete_call_stm, NULL) != SQLITE_OK)
        goto fail;

    if (sqlite3_prepare_v2(db, "UPDATE calls SET new = ? WHERE id = ?", -1, &set_new_call_stm, NULL) != SQLITE_OK)
        goto fail;

    if (sqlite3_prepare_v2(db, "UPDATE calls SET new = ?", -1, &set_new_all_stm, NULL) != SQLITE_OK)
        goto fail;

    goto end;

fail:
    if (db != NULL) {
        sqlite3_close(db);
        db = NULL;
    }

end:

#ifdef USE_THREADS
    {
        // crea thread di scrittura
        GError* e = NULL;
        if (!g_thread_create(db_thread, NULL, TRUE, &e)) {
            g_warning("[%s] Unable to create database thread: %s", __func__, e->message);
            g_error_free(e);
        }

        else {
            cmd_queue = g_async_queue_new();
        }
    }
#endif


#else

    if (db_create(&db, NULL, 0) != 0) {
        if (db != NULL) {
            db->close(db, 0);
            db = NULL;
        }
    }

    if (db != NULL && db->open(db, NULL, db_path, NULL, DB_RECNO, DB_CREATE, 0) != 0) {
        db->close(db, 0);
        db = NULL;
    }

#endif

    if (db == NULL)
        g_warning("Unable to open calls database; will not be able to log calls");

    else {
        // recupera il timestamp del database
        callsdb_timestamp = get_modification_time(db_path);
        callsdb_path = g_strdup(db_path);
    }
}

