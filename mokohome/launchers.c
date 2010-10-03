#include <Elementary.h>
#include <glib.h>
#include <libmokosuite/mokosuite.h>

#include "launchers.h"

#define APPS_PATH       "/usr/share/applications"
#define COLUMNS         4       // 3

#ifdef QVGA
#define LAUNCHERS_PADDING0_HEIGHT     3
#else
#define LAUNCHERS_PADDING0_HEIGHT     5
#endif

#define LAUNCHERS_PADDING1_WIDTH       (235 * MOKOSUITE_SCALE_FACTOR)
#define LAUNCHERS_PADDING1_HEIGHT      (2 * MOKOSUITE_SCALE_FACTOR)
#define LAUNCHERS_PADDING2_WIDTH       (2 * MOKOSUITE_SCALE_FACTOR)
#define LAUNCHERS_PADDING2_HEIGHT      (2 * MOKOSUITE_SCALE_FACTOR)

static int x = 0;
static int y = 0;

struct launcher_click_data {
    Evas_Object* layout_edje;
    //Efreet_Desktop* entry;
    char* exec;
};

static void launcher_clicked(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
    Evas_Event_Mouse_Up *ev = event_info;
    if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;

    GError *err = NULL;
    struct launcher_click_data* cb_data = data;

    // chiudi launcher
    edje_object_signal_emit(cb_data->layout_edje, "collapse", "handle");

    char *cmd = g_strdup_printf("sh -c \"%s\"", cb_data->exec);
    g_spawn_command_line_async(cmd, &err);

    g_free(cmd);
    g_debug("Process spawned, error: %s", (err != NULL) ? err->message : "OK");

    if (err != NULL)
        g_error_free(err);
}

static Evas_Object* prepare_table(Evas_Object* win, int cols, int rows)
{
    Evas_Object* tb, *pad;

    tb = elm_table_add(win);
    evas_object_size_hint_weight_set(tb, 0.0, 0.0);
    evas_object_size_hint_align_set(tb, 0.5, 0.5);
    elm_table_padding_set(tb, 0, LAUNCHERS_PADDING0_HEIGHT);

    pad = evas_object_rectangle_add(evas_object_evas_get(win));
    evas_object_size_hint_min_set(pad, LAUNCHERS_PADDING1_WIDTH, LAUNCHERS_PADDING1_HEIGHT);
    evas_object_size_hint_weight_set(pad, 0.0, 0.0);
    evas_object_size_hint_align_set(pad, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_table_pack(tb, pad, 1, 0, cols, 1);

    pad = evas_object_rectangle_add(evas_object_evas_get(win));
    evas_object_size_hint_min_set(pad, LAUNCHERS_PADDING1_WIDTH, LAUNCHERS_PADDING1_HEIGHT);
    evas_object_size_hint_weight_set(pad, 0.0, 0.0);
    evas_object_size_hint_align_set(pad, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_table_pack(tb, pad, 1, 11, cols, 1);

    pad = evas_object_rectangle_add(evas_object_evas_get(win));
    evas_object_size_hint_min_set(pad, LAUNCHERS_PADDING2_WIDTH, LAUNCHERS_PADDING2_HEIGHT);
    evas_object_size_hint_weight_set(pad, 0.0, 0.0);
    evas_object_size_hint_align_set(pad, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_table_pack(tb, pad, 0, 1, 1, 10);

    pad = evas_object_rectangle_add(evas_object_evas_get(win));
    evas_object_size_hint_min_set(pad, LAUNCHERS_PADDING2_WIDTH, LAUNCHERS_PADDING2_HEIGHT);
    evas_object_size_hint_weight_set(pad, 0.0, 0.0);
    evas_object_size_hint_align_set(pad, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_table_pack(tb, pad, cols+1, 1, 1, 10);

    return tb;
}

Evas_Object* launcher_new(Evas_Object* parent, Efreet_Desktop* d)
{
    Evas_Object *ic;

    char* ic_path = efreet_icon_path_find(LAUNCHER_ICON_THEME, d->icon, LAUNCHER_ICON_SIZE);
    if (!ic_path && (d->icon != NULL ? d->icon[0] != '/' : TRUE)) return NULL;   // non aggiungere se l'icona non e' stata trovata

    ic = elm_icon_add(parent);

    gboolean is_edj = (ic_path == NULL && g_str_has_suffix(d->icon, ".edj"));

    elm_icon_file_set(ic, (ic_path == NULL) ? d->icon : ic_path, is_edj ? "icon" : NULL );
    g_free(ic_path);

    elm_icon_smooth_set(ic, TRUE);
    elm_icon_scale_set(ic, TRUE, TRUE);
    evas_object_show(ic);
    evas_object_size_hint_max_set(ic, LAUNCHER_ICON_SIZE, LAUNCHER_ICON_SIZE);

    Evas_Object *bt = edje_object_add(evas_object_evas_get(parent));
    edje_object_file_set(bt, MOKOSUITE_DATADIR "theme.edj", "launcher");
    edje_object_part_swallow(bt, "icon", ic);

    // titolo
    edje_object_part_text_set(bt, "title", d->name);

    evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    //evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_size_hint_min_set(bt, LAUNCHER_WIDTH, LAUNCHER_HEIGHT);
    evas_object_show(bt);

    return bt;
}

static void add_launcher(Evas_Object* win, Evas_Object* layout_edje, Evas_Object* tb, Efreet_Desktop* d)
{
    Evas_Object *bt = launcher_new(win, d);
    if (!bt) return;

    elm_table_pack(tb, bt, 1 + x, 1 + (y * 2), 1, 1);

    struct launcher_click_data* data = g_new(struct launcher_click_data, 1);
    data->layout_edje = layout_edje;
    data->exec = g_strdup(d->exec);

    evas_object_event_callback_add(bt, EVAS_CALLBACK_MOUSE_UP, launcher_clicked, data);

    evas_object_show(bt);

    x++;
    if (x >= COLUMNS) {
        x = 0;
        y++;
    }
}

static gint _sort_desktop(gconstpointer _a, gconstpointer _b)
{
    const Efreet_Desktop* a = _a;
    const Efreet_Desktop* b = _b;
    char* name1 = g_utf8_casefold(a->name, -1);
    char* name2 = g_utf8_casefold(b->name, -1);

    gint cmp = g_utf8_collate(name1, name2);
    g_free(name1);
    g_free(name2);

    return cmp;
}

Evas_Object* make_launchers(Evas_Object* win, Evas_Object* layout_edje)
{
    Evas_Object* mb, *sc;

    // riempi la tabella con le icone
    Evas_Object* tb = prepare_table(win, COLUMNS, -1);

    GDir* dir = g_dir_open(APPS_PATH, 0, NULL);

    if (dir) {
        const char* name = NULL;
        char* full = NULL;
        Efreet_Desktop* d = NULL;
        GList* sorting = NULL;

        while ((name = g_dir_read_name(dir))) {
            full = g_strdup_printf("%s/%s", APPS_PATH, name);

            if (g_file_test(full, G_FILE_TEST_IS_REGULAR)) {

                d = efreet_desktop_get(full);

                if (d) {
                    sorting = g_list_insert_sorted(sorting, d, _sort_desktop);
                }
            }

            g_free(full);
        }
        g_dir_close(dir);

        if (sorting) {
            GList* iter = sorting;
            while (iter) {
                Efreet_Desktop* d = iter->data;
                add_launcher(win, layout_edje, tb, d);
                efreet_desktop_free(d);

                iter = iter->next;
            }
            g_list_free(sorting);
        }
    }

    mb = elm_mapbuf_add(win);
    elm_mapbuf_content_set(mb, tb);
    evas_object_show(tb);

    evas_object_show(mb);

    sc = elm_scroller_add(win);
    elm_object_style_set(sc, "desktop");

    elm_scroller_bounce_set(sc, FALSE, FALSE);
    elm_scroller_policy_set(sc, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);

    evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(sc, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_scroller_content_set(sc, tb);
    evas_object_show(tb);

    return sc;
}
