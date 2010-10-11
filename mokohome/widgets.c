#include <Elementary.h>
#include <glib.h>
#include <libmokosuite/mokosuite.h>

#include "widgets.h"
#include "launchers.h"
#include "desktop.h"

static void launcher_clicked(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
    Evas_Event_Mouse_Up *ev = event_info;
    if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;

    GError *err = NULL;

    char *cmd = g_strdup_printf("sh -c \"%s\"", (char *) data);
    g_spawn_command_line_async(cmd, &err);

    g_free(cmd);
    g_debug("Process spawned, error: %s", (err != NULL) ? err->message : "OK");

    if (err != NULL)
        g_error_free(err);
}

static gboolean _pressing(gpointer data)
{
    Evas_Object* wd = data;
    if (!evas_object_data_get(wd, "clicked")) {
        g_debug("Launcher pressed, starting drag");
        evas_object_data_set(wd, "clicked", (void*) TRUE);
        drag_start((int) evas_object_data_get(wd, "desktop_id"));
    }

    return FALSE;
}

static void launcher_pressed(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
    // already dragging -- leave
    if (drag_status >= 0) return;

    evas_object_data_set(obj, "clicked", (void*) FALSE);
    g_timeout_add_seconds(2, _pressing, obj);
}

static void launcher_released(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
    if (drag_status < 0) {
        evas_object_data_set(obj, "clicked", (void*) TRUE);
        launcher_clicked(data, e, obj, event_info);
        return;
    }

    g_debug("Launcher is being dragged, not launching");
}

Evas_Object* widget_launcher_new(Evas_Object* parent, Efreet_Desktop* d)
{
    Evas_Object *bt = launcher_new(parent, d);
    if (!bt) return NULL;

    // contenitore widget dragabile
    Evas_Object *wd = edje_object_add(evas_object_evas_get(parent));
    edje_object_file_set(wd, MOKOSUITE_DATADIR "theme.edj", "widget");
    edje_object_part_swallow(wd, "widget", bt);
    evas_object_size_hint_min_set(wd, LAUNCHER_WIDTH, LAUNCHER_HEIGHT);

    evas_object_event_callback_add(bt, EVAS_CALLBACK_MOUSE_DOWN, launcher_pressed, NULL);
    evas_object_event_callback_add(bt, EVAS_CALLBACK_MOUSE_UP, launcher_released, g_strdup(d->exec));

    evas_object_show(bt);
    evas_object_show(wd);

    return wd;
}
