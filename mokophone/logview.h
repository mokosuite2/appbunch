#ifndef __LOGVIEW_H
#define __LOGVIEW_H

#include <libmokosuite/gui.h>
#include <Elementary.h>
#include "callsdb.h"

void logview_add_call(CallEntry* e);

Evas_Object* logview_make_section(void);
Evas_Object* logview_make_menu(void);
void logview_reset_view(void);

#endif  /* __LOGVIEW_H */
