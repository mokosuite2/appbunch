#ifndef __DESKTOP_H
#define __DESKTOP_H

#include <Elementary.h>
#include <glib.h>

extern int drag_status;

void drag_start(int desktop_id);
void drag_end(void);

Evas_Object* make_widgets(Evas_Object* win, Evas_Object* scroller);

#endif  /* __DESKTOP_H */
