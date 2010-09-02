#ifndef __MOKOMESSAGES_H
#define __MOKOMESSAGES_H

typedef struct {
    char* peer;
    char* text;
    guint64 timestamp;
    gboolean marked;
} thread_t;

typedef struct {
    char* text;
    guint64 timestamp;
    gboolean sent;
} sms_t;


#endif  /* __MOKOMESSAGES_H */
