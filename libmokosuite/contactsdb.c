#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "contactsdb.h"

#include <stdlib.h>
#include <string.h>
#include <phone-utils.h>
#include <libmokosuite/misc.h>

time_t contactsdb_timestamp = -1;
char* contactsdb_path = NULL;

#ifdef CONTACTSDB_SQLITE
#warning "Using SQLite contacts storage"
#include <sqlite3.h>

static GHashTable* contacts_lookup_cache = NULL;
static sqlite3 *db = NULL;

// TODO vari statement
// TODO static sqlite3_stmt* lookup_number_stm = NULL;

#define CONTACTS_DB_SCHEMA     \
    "CREATE TABLE IF NOT EXISTS contacts (\n" \
    "id INTEGER PRIMARY KEY,\n" \
    "default_phone INTEGER,\n" \
    "default_sms INTEGER,\n" \
    "default_mms INTEGER,\n" \
    "default_email INTEGER\n" \
    ")"

#define CONTACTS_FIELDS_DB_SCHEMA     \
    "CREATE TABLE IF NOT EXISTS contacts_fields (\n" \
    "contact_id INTEGER NOT NULL,\n" \
    "field_id INTEGER NOT NULL,\n" \
    "type INTEGER NOT NULL,\n" \
    "name TEXT NOT NULL,\n" \
    "value TEXT NOT NULL,\n" \
    "CONSTRAINT fields_pkey PRIMARY KEY (contact_id, field_id)\n" \
    ")"

// indice UNIQUE sul tipo del campo relativo all'ID
#define CONTACTS_FIELDS_DB_INDEX_ID_TYPE     \
    "CREATE UNIQUE INDEX IF NOT EXISTS contacts_fields_type on contacts_field (contact_id, type)"


// indice sul valore del campo
#define CONTACTS_FIELDS_DB_INDEX_VALUE     \
    "CREATE INDEX IF NOT EXISTS contacts_fields_value on contacts_field (value)"


#define ROWINDEX(row,col)     (ncolumn + (row * ncolumn) + col)

#else

#warning "Using Berkeley DB contacts storage"
#include <db.h>

// database delle chiavi 
static DB *kdb = NULL;

// valore di una chiave
typedef struct {
    guint32 contact_id;
    guint16 field_id;
} __attribute__ ((packed)) DBT_ContactKey;

// database dei contatti
static DB *db = NULL;

// chiave
typedef guint32 DBT_ContactId;

// valore
typedef struct {
    // numero di campi del contatto
    guint16 fields;
} __attribute__ ((packed)) DBT_Contact;

#endif

static void contactfield_free(gpointer data)
{
    if (data) {
        ContactField* f = data;
        g_free(f->name);
        g_free(f->value);
        g_free(f);
    }
}

#ifdef CONTACTSDB_SQLITE

ContactField* contactsdb_get_first_field(ContactEntry* e, ContactFieldType type)
{
    g_return_val_if_fail(e != NULL, NULL);

    ContactField* f;
    int i;

    for (i = 0; i < e->fields->len; i++) {
        f = g_ptr_array_index(e->fields, i);
        if (f->type == type)
            return f;
    }

    return NULL;
}

ContactField* contactsdb_get_default_field(ContactEntry* e, ContactDefaultType deftype)
{
    g_return_val_if_fail(e != NULL, NULL);

    ContactField* f;
    int i;

    int to_search = 0;
    switch (deftype) {
        case CONTACT_DEFAULT_PHONE:
            to_search = e->default_phone;
            break;
        case CONTACT_DEFAULT_SMS:
            to_search = e->default_sms;
            break;
        case CONTACT_DEFAULT_MMS:
            to_search = e->default_mms;
            break;
        case CONTACT_DEFAULT_EMAIL:
            to_search = e->default_email;
            break;
    }

    // niente default :(
    if (to_search <= 0) return NULL;

    for (i = 0; i < e->fields->len; i++) {
        f = g_ptr_array_index(e->fields, i);
        if (f->id == to_search)
            return f;
    }

    return NULL;
}

#define COL_CID         0
#define COL_DEF_PHONE   1
#define COL_DEF_SMS     2
#define COL_DEF_MMS     3
#define COL_DEF_EMAIL   4
#define COL_FID         5
#define COL_TYPE        6
#define COL_NAME        7
#define COL_VALUE       8

static int _contact_row(gpointer _data, int argc, char **argv, char **azColName)
{
    GHashTable* table = _data;
    ContactEntry* e = NULL;
    /*
    int i;

    for (i = 0; i < argc; i++) {
        g_debug("[%s] column[%d:%s]=%s", __func__, i, azColName[i], argv[i]);
    }
    */

    // cerca il contatto
    int cid = atol(argv[COL_CID]);
    e = g_hash_table_lookup(table, GINT_TO_POINTER(cid));
    if (!e) {
        //g_debug("[%s] creating new entry, id=%d", __func__, cid);
        e = g_new0(ContactEntry, 1);
        e->id = cid;
        e->fields = g_ptr_array_new_with_free_func(contactfield_free);

        if (argv[COL_DEF_PHONE] != NULL)
            e->default_phone = atol(argv[COL_DEF_PHONE]);

        if (argv[COL_DEF_SMS] != NULL)
            e->default_sms = atol(argv[COL_DEF_SMS]);

        if (argv[COL_DEF_MMS] != NULL)
            e->default_mms = atol(argv[COL_DEF_MMS]);

        if (argv[COL_DEF_EMAIL] != NULL)
            e->default_email = atol(argv[COL_DEF_EMAIL]);

        // inseriscila!
        g_hash_table_insert(table, GINT_TO_POINTER(cid), e);
    }

    // a questo punto ho la entry, vediamo se abbiamo qualche field da inserire
    if (argv[COL_FID] != NULL) {
        int fid = atol(argv[COL_FID]);

        ContactField* f = g_new0(ContactField, 1);
        f->id = fid;
        f->type = atol(argv[COL_TYPE]);
        f->name = g_strdup(argv[COL_NAME]);
        f->value = g_strdup(argv[COL_VALUE]);

        g_ptr_array_add(e->fields, f);

        // cache per lookup rapido
        if (f->type == CONTACT_FIELD_PHONE) {
            char* num = phone_utils_normalize_number(f->value);
            if (num == NULL) num = g_strdup(f->value);
            g_hash_table_insert(contacts_lookup_cache, num, e);
        }
    }

    return 0;
}

typedef struct {
    ContactEntryFunc callback;
    gpointer userdata;
} contacts_foreach_data_t;

//static void final_contacts_cb(gpointer key, gpointer value, gpointer _data)
static void final_contacts_cb(gpointer value, gpointer _data)
{
    contacts_foreach_data_t* data = _data;
    ContactEntry* e = value;

    if (e)
        (data->callback)(e, data->userdata);
}

static gint contacts_sort_func(gconstpointer _a, gconstpointer _b)
{
    ContactField* a = contactsdb_get_first_field((ContactEntry *) _a, CONTACT_FIELD_NAME);
    ContactField* b = contactsdb_get_first_field((ContactEntry *) _b, CONTACT_FIELD_NAME);

    if (a != NULL && b != NULL)
        return strcmp(a->value,b->value);

    return 0;
}

#endif

/**
 * Itera i contatti a scopo di visualizzazione finale (es. lista contatti)
 * @param field_name nome del campo da recuperare, NULL per tutti i campi
 * @param name_type tipo di campo da recuperare come nome del contatto
 * @param type tipo di numero default da recuperare come valore del contatto
 */
void contactsdb_foreach_contact(
#ifdef CONTACTSDB_SQLITE
ContactFieldType name_type, ContactDefaultType number_type,
#else
const char* field_name,
#endif
ContactEntryFunc func, gpointer data)
{
    g_return_if_fail(func != NULL);
    if (db == NULL) return;

#ifdef CONTACTSDB_SQLITE

    GTimer* t = g_timer_new();
    char *errmsg = NULL;

    GHashTable* table = g_hash_table_new(g_direct_hash, g_direct_equal);

    // TODO filtraggio per tipi

    if (sqlite3_exec(db, "SELECT c.id, c.default_phone, c.default_sms, c.default_mms, c.default_email, "
                "f.field_id, f.type, f.name, f.value FROM contacts c LEFT OUTER JOIN contacts_fields f on c.id = f.contact_id",
                _contact_row, table, &errmsg) == SQLITE_OK) {
        g_debug("[%s] query took %f seconds", __func__, g_timer_elapsed(t, NULL));

        // a questo punto abbiamo l'hashtable con tutti i campi, passali al callback e piano piano libera tutto
        contacts_foreach_data_t* cb_data = g_new0(contacts_foreach_data_t, 1);
        cb_data->callback = func;
        cb_data->userdata = data;

        // ottieni la lista dei valori e ordinala per campo NAME
        GList* values = g_hash_table_get_values(table);
        values = g_list_sort(values, contacts_sort_func);

        //g_hash_table_foreach(table, final_contacts_cb, cb_data);
        g_list_foreach(values, final_contacts_cb, cb_data);

        g_list_free(values);
        g_hash_table_destroy(table);
    }

    else {
        if (errmsg != NULL) {
            g_warning("[%s] %s", __func__, errmsg);
            sqlite3_free(errmsg);
        }
    }

    g_timer_destroy(t);

#else

    DBC* dbcp;
    int ret, i;
    guint32 psize = 0;
    char* field = NULL, *value = NULL;

    /* Acquire a cursor for the database. */
    if (db->cursor(db, NULL, &dbcp, 0) != 0) {
        g_warning("Unable to acquire a cursor to contacts database.");
        return;
    }

    DBT db_key = {0}, db_data = {0};

    /* Walk through the database */
    while ((ret = dbcp->c_get(dbcp, &db_key, &db_data, DB_NEXT)) == 0) {

        ContactEntry *e = g_new0(ContactEntry, 1);

        // l'id deve essere > 0, altrimenti salta tutto
        e->id = *((DBT_ContactId*)db_key.data);

        if (e->id > 0) {
            // array campi :)
            e->fields = g_ptr_array_new_with_free_func(contactfield_free);

            DBT_Contact* c = db_data.data;
            psize = sizeof(DBT_Contact);

            for (i = 0; i < c->fields; i++) {
                ContactField* f = NULL;

                field = (char *)(db_data.data + psize);
                value = (field + strlen(field) + 1);
                psize += strlen(field) + strlen(value) + 2;
                g_debug("field=%s, value=%s", field, value);

                // se il campo da cercare e' nullo, permetti inserimento
                // se non e' nullo, confronta con il nome corrente
                if (field_name != NULL ?
                    !strcmp(field, field_name) :
                    TRUE) {

                    // crea descrittore campo e aggiungilo alla lista
                    f = g_new0(ContactField, 1);
                    f->name = g_strdup(field);
                    f->value = g_strdup(value);
                    f->type = 0;

                    g_ptr_array_add(e->fields, f);

                    // non serve continuare tra i campi
                    if (field_name != NULL) break;
                }
            }

            (func)(e, data);
        }

        else g_free(e);
    }

    if (ret != DB_NOTFOUND) {
        g_warning("Unable to iterate contacts database cursor.");
        return;
    }

    /* Close the cursor. */
    dbcp->c_close(dbcp);
#endif
}

ContactEntry* contactsdb_lookup_number(const char *peer)
{
#ifdef CONTACTSDB_SQLITE
#if 0
    GTimer* t = g_timer_new();
    int rc;

    sqlite3_reset(lookup_number_stm);

    sqlite3_bind_int(lookup_number_stm, 1, (int) CONTACT_FIELD_PHONE);
    sqlite3_bind_string(lookup_number_stm, 2, g_strdup(peer), -1, g_free);

    rc = sqlite3_step (new_call_stm);
    g_debug("[%s] query took %f seconds", __func__, g_timer_elapsed(t, NULL));
    g_timer_destroy(t);

    if (rc == SQLITE_OK || rc == SQLITE_DONE) {
        e = g_new0(ContactEntry, 1);
        // TODO
    }
#else
    char* num = phone_utils_normalize_number(peer);
    ContactEntry* e = (ContactEntry *) g_hash_table_lookup(contacts_lookup_cache, ( num != NULL ) ? num : peer);
    g_free(num);

    return e;
#endif

#else
    // TODO
    return NULL;
#endif
}

static void add_contact(const char* cname, const char* cphone)
{
    g_debug("TEST CONTACT INSERT (%s=%s)", cname, cphone);

#ifdef CONTACTSDB_SQLITE
#else
    char* name = "name";
    char* value = (char *) cname;
    char* name2 = "phone";
    char* value2 = (char *) cphone;

    ssize_t area_size = sizeof(DBT_Contact) + strlen(name) + strlen(value) + 2 +
        strlen(name2) + strlen(value2) + 2;
    void* area = g_malloc0(area_size);

    DBT_Contact * c = area;
    c->fields = 2;

    memcpy(area + sizeof(DBT_Contact), name, strlen(name) + 1);
    memcpy(area + sizeof(DBT_Contact) + strlen(name) + 1, value, strlen(value) + 1);

    memcpy(area + sizeof(DBT_Contact) + strlen(name) + strlen(value) + 2, name2, strlen(name2) + 1);
    memcpy(area + sizeof(DBT_Contact) + strlen(name) + strlen(value) + 2 + strlen(name2) + 1, value2, strlen(value2) + 1);

    DBT key = {0}, data = {0};
    data.data = area;
    data.size = area_size;

    guint32 retval = 0;

    if (db->put(db, NULL, &key, &data, DB_APPEND) == 0) {
        db->sync(db, 0);

        db_recno_t recno = *((db_recno_t*)key.data);
        retval = recno;
    }

    g_debug("RECNO=%d", retval);
    /* TODO chiave di lookup
    key.data = retval;
    key.size = sizeof(retval);

    data.data =
    data.size =

    if (retval > 0 && kdb->put(kdb, NULL, &key, &data, DB_APPEND) == 0) {
        kdb->sync(kdb, 0);

        g_debug("KEY OK!");
    }

    */
#endif
}

void contactsdb_init(const char *db_path)
{
#ifdef CONTACTSDB_SQLITE
    contacts_lookup_cache = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

    if (sqlite3_open(db_path, &db) != SQLITE_OK)
        goto fail;

    // db aperto, crea lo schema
    else {
        sqlite3_exec(db, CONTACTS_DB_SCHEMA, NULL, NULL, NULL);
        sqlite3_exec(db, CONTACTS_FIELDS_DB_SCHEMA, NULL, NULL, NULL);
        sqlite3_exec(db, CONTACTS_FIELDS_DB_INDEX_VALUE, NULL, NULL, NULL);
        sqlite3_exec(db, CONTACTS_FIELDS_DB_INDEX_ID_TYPE, NULL, NULL, NULL);
    }

    // TODO prepara statement
    #if 0
    if (sqlite3_prepare_v2(db, "SELECT c.id, c.default_phone, c.default_sms, c.default_mms, c.default_email, "
                "f.field_id, f.type, f.name, f.value FROM contacts c LEFT OUTER JOIN contacts_fields f on c.id = f.contact_id "
                "where f.type = ? and f.value = ?",
        -1, &lookup_number_stm, NULL) != SQLITE_OK) goto fail;
    #endif

    goto end;

fail:
    if (db != NULL) {
        sqlite3_close(db);
        db = NULL;
    }

end:

#else

    // apri database dei contatti
    if (db_create(&db, NULL, 0) != 0) {
        if (db != NULL) {
            db->close(db, 0);
            db = NULL;
        }
    }

    if (db != NULL && db->open(db, NULL, db_path, "contacts", DB_RECNO, DB_CREATE, 0) != 0) {
        db->close(db, 0);
        db = NULL;
    }

    if (db != NULL) {
        // apri database delle chiavi
        if (db_create(&kdb, NULL, 0) != 0) {
            if (kdb != NULL) {
                kdb->close(kdb, 0);
                db->close(db, 0);
                kdb = NULL;
                db = NULL;
            }
        }

        if (kdb != NULL && kdb->open(kdb, NULL, db_path, "contacts_key", DB_BTREE, DB_CREATE, 0) != 0) {
            kdb->close(kdb, 0);
            db->close(db, 0);
            kdb = NULL;
            db = NULL;
        }
    }

#endif

    if (db == NULL)
        g_warning("Unable to open contacts database; will not be able to store contacts");

    else {
        // recupera il timestamp del database
        contactsdb_timestamp = get_modification_time(db_path);
        contactsdb_path = g_strdup(db_path);
    }
}
