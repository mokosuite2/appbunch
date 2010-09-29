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
    int id;

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
};

typedef struct _CallEntry CallEntry;

typedef void (*CallEntryFunc)(CallEntry*, gpointer);

void callsdb_foreach_call(CallEntryFunc func, gpointer data);

void callsdb_set_call_new(int id, gboolean is_new);

gboolean callsdb_delete_call(int id);

gboolean callsdb_truncate(void);

void callsdb_init(CallEntryFunc func, gpointer userdata);

#endif  /* __CALLSDB_H */
