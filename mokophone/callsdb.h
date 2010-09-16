#ifndef __CALLSDB_H
#define __CALLSDB_H

#include <glib.h>
#include <time.h>

typedef enum
{
    DIRECTION_OUTGOING,
    DIRECTION_INCOMING
} CallDirection;

struct _CallEntry {
    /* id chiamata */
    gint64 id;

    /* direzione chiamata */
    CallDirection direction;

    /* numero del chiamante/chiamato */
    char* peer;

    /* timestamp inizio chiamata */
    guint64 timestamp;

    /* durata chiamata in secondi */
    guint64 duration;

    /* chiamata risposta? */
    gboolean answered;

    /* chiamata letta? */
    gboolean is_new;

    /* dati utente (ListItem) */
    gpointer data;

    /* dati utente (ContactEntry) */
    gpointer data2;

    /* dati utente (ID notifica panel) */
    gpointer data3;
};

typedef struct _CallEntry CallEntry;

typedef void (*CallEntryFunc)(CallEntry*, gpointer);

void callsdb_foreach_call(CallEntryFunc func, gpointer data);

void callsdb_new_call(CallDirection direction, const char* peer,
    guint64 timestamp, guint64 duration,
    gboolean answered, gboolean is_new,
    CallEntryFunc func,
    gpointer userdata);

void callsdb_set_call_new(gint64 id, gboolean is_new);

gboolean callsdb_delete_call(gint64 id);

gboolean callsdb_truncate(void);

void callsdb_init();

#endif  /* __CALLSDB_H */
