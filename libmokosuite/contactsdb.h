#ifndef __CONTACTSDB_H
#define __CONTACTSDB_H

#include <glib.h>

typedef enum {
    CONTACT_DEFAULT_PHONE = 0,
    CONTACT_DEFAULT_SMS,
    CONTACT_DEFAULT_MMS,
    CONTACT_DEFAULT_EMAIL
} ContactDefaultType;

typedef enum {
    CONTACT_FIELD_PHONE = 0,
    CONTACT_FIELD_NAME,
    CONTACT_FIELD_FIRST_NAME,
    CONTACT_FIELD_LAST_NAME,
    CONTACT_FIELD_EMAIL,
    CONTACT_FIELD_ADDRESS,
    CONTACT_FIELD_COMPANY,
    CONTACT_FIELD_OTHER
} ContactFieldType;

struct _ContactField {
    // id del campo
    int id;

    // tipo di campo
    int type;

    // nome del campo
    char* name;

    // valore del campo
    char* value;
};

struct _ContactEntry {
    // id del contatto
#ifdef CONTACTSDB_SQLITE
    gint64 id;
#else
    guint32 id;
#endif

    // campi del contatto
    GPtrArray* fields;

    // campi di default
    guint32 default_phone;
    guint32 default_sms;
    guint32 default_mms;
    guint32 default_email;

    // dati utente
    gpointer data;
};

typedef struct _ContactEntry ContactEntry;
typedef struct _ContactField ContactField;

typedef void (*ContactEntryFunc)(ContactEntry*, gpointer);

ContactField* contactsdb_get_first_field(ContactEntry* e, ContactFieldType type);
ContactField* contactsdb_get_default_field(ContactEntry* e, ContactDefaultType deftype);

void contactsdb_foreach_contact(
#ifdef CONTACTSDB_SQLITE
ContactFieldType name_type, ContactDefaultType number_type,
#else
const char* field_name,
#endif
ContactEntryFunc func, gpointer data);


ContactEntry* contactsdb_lookup_number(const char *peer);

void contactsdb_init(const char *db_path);

extern time_t contactsdb_timestamp;
extern char* contactsdb_path;

#endif  /* __CONTACTSDB_H */
