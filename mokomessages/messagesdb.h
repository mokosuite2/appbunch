#ifndef __MESSAGESDB_H
#define __MESSAGESDB_H

#include <glib.h>
#include <time.h>

typedef enum
{
    DIRECTION_OUTGOING,
    DIRECTION_INCOMING
} MessageDirection;

struct _MessageThread {
    /* numero del contatto */
    char* peer;

    /* contenuto ultimo messaggio */
    char* content;

    /* direzione ultimo messaggio */
    MessageDirection direction;

    /* timestamp ultimo messaggio */
    guint64 timestamp;

    /* numero di messaggi non letti */
    int unread_count;

    /* numero di messaggi totali */
    int total_count;

    /* dati utente (ListItem) */
    gpointer data;

    /* dati utente (ContactEntry) */
    gpointer data2;
};

typedef struct _MessageThread MessageThread;

typedef void (*MessageThreadFunc)(MessageThread*, gpointer);

void messagesdb_foreach_thread(MessageThreadFunc func, gpointer data);

void messagesdb_init(MessageThreadFunc func, gpointer userdata);

#endif  /* __MESSAGESDB_H */
