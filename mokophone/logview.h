#ifndef __LOGVIEW_H
#define __LOGVIEW_H

#include <libmokosuite/gui.h>
#include <Elementary.h>
#include "callsdb.h"

CallEntry* logview_add_call(gint64 id, CallDirection direction, const char* peer,
    guint64 timestamp, guint64 duration,
    gboolean answered, gboolean is_new);

Evas_Object* logview_make_section(void);
Evas_Object* logview_make_menu(void);
void logview_reset_view(void);

#endif  /* __LOGVIEW_H */
