#ifndef __MOKONOTIFICATIONS_H
#define __MOKONOTIFICATIONS_H

#include "mokosuite.h"
#include "panel.h"

#define MOKO_NOTIFICATIONS_INTERFACE            MOKOSUITE_SERVICE ".Notifications"
#define MOKO_NOTIFICATIONS_DEFAULT_PATH         MOKOSUITE_PATH "/Panel/0/Notifications"

enum {
    NOTIFICATION_ACTIVE_CALL,
    NOTIFICATION_MISSED_CALL,
    NOTIFICATION_UNREAD_MESSAGE,
    NOTIFICATION_UNREAD_USSD,
    // TODO
};

enum {
    MOKOPANEL_NOTIFICATION_FLAG_NONE = 0,
    // non pusha il testo della notifica
    MOKOPANEL_NOTIFICATION_FLAG_DONT_PUSH = 1 << 0,
    // ripresenta il testo della notifica appena si chiude lo screensaver
    MOKOPANEL_NOTIFICATION_FLAG_REPRESENT = 1 << 1
};

int moko_notifications_push(DBusGProxy* proxy, const char * text, const char * icon, int type, int flags, GError **error);
void moko_notifications_remove(DBusGProxy* proxy, int notification_id, GError **error);

DBusGProxy* moko_notifications_connect(const char* bus_name, const char* path);

#endif  /* __MOKONOTIFICATIONS_H */
