#include <Elementary.h>
#include <glib.h>
#include <libmokosuite/mokosuite.h>

#include "desktop.h"
#include "widgets.h"

#define WIDGETS_COLUMNS     4
#define WIDGETS_ROWS        4

#ifdef QVGA
#define WIDGETS_PADDING0_HEIGHT     3
#else
#define WIDGETS_PADDING0_HEIGHT     5
#endif

#define WIDGETS_PADDING1_WIDTH       (235 * MOKOSUITE_SCALE_FACTOR)
#define WIDGETS_PADDING1_HEIGHT      (2 * MOKOSUITE_SCALE_FACTOR)
#define WIDGETS_PADDING2_WIDTH       (2 * MOKOSUITE_SCALE_FACTOR)
#define WIDGETS_PADDING2_HEIGHT      (2 * MOKOSUITE_SCALE_FACTOR)

#define LAUNCHER_HEIGHT (65 * MOKOSUITE_SCALE_FACTOR)

// per fare una cosa piu' precisa...
#ifdef QVGA
#define LAUNCHER_WIDTH  58
#else
#define LAUNCHER_WIDTH  115
#endif

#define LONG_PRESS_TIME     750
#define NUM_DESKTOPS        2

static int wx = 0;
static int wy = 0;
static GPtrArray* widgets[NUM_DESKTOPS];
static Evas_Object* desktops[NUM_DESKTOPS];

#if 0
struct widget_press_data {
    Evas_Object* scroller;
    GPtrArray* widgets;
};

static gboolean _widget_pressed(gpointer data)
{
    g_debug("Widget editing mode");
    struct widget_press_data* cb_data = data;

    int i;
    double val_ret = 0.0;
    edje_object_part_state_get(g_ptr_array_index(cb_data->widgets, 0), "widget_drag", &val_ret);

    if (val_ret == 1.0) {

        for (i = 0; i < cb_data->widgets->len; i++)
            edje_object_signal_emit(g_ptr_array_index(cb_data->widgets, i), "drag_end", "widget");

        elm_object_scroll_freeze_pop(cb_data->scroller);
    }

    else {
        for (i = 0; i < cb_data->widgets->len; i++)
            edje_object_signal_emit(g_ptr_array_index(cb_data->widgets, i), "drag_start", "widget");

        elm_object_scroll_freeze_push(cb_data->scroller);
    }

    g_free(data);
    return FALSE;
}

static void widget_pressed(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
    g_debug("Mouse down");
}

static void widget_released(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
    Evas_Event_Mouse_Up *ev = event_info;
    struct widget_press_data* cb_data = data;

    // stiamo scrollando il desktop
    if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;

    double val_ret = 0.0;
    edje_object_part_state_get(g_ptr_array_index(cb_data->widgets, 0), "widget_drag", &val_ret);

    if (val_ret == 1.0) {
        // TODO allineamento widget
    }

    g_debug("Mouse up");
}
#endif

static void add_widget(int desktop_id, Evas_Object* wd)
{
    elm_table_pack(desktops[desktop_id], wd, 1 + wx, 1 + (wy * 2), 1, 1);

    wx++;
    if (wx >= WIDGETS_COLUMNS) {
        wx = 0;
        wy++;
    }

    g_ptr_array_add(widgets[desktop_id], wd);
}

static void add_custom_widget(Evas_Object* win, int desktop_id, const char* icon, const char* name, Evas_Object* scroller)
{
    Evas_Object *ic, *lb;

    char* ic_path = NULL;
    if (!ic_path && (icon != NULL ? icon[0] != '/' : TRUE)) return;   // non aggiungere se l'icona non e' stata trovata

    ic = elm_icon_add(win);

    gboolean is_edj = (ic_path == NULL && g_str_has_suffix(icon, ".edj"));

    elm_icon_file_set(ic, (ic_path == NULL) ? icon : ic_path, is_edj ? "icon" : NULL );
    g_free(ic_path);

    elm_icon_smooth_set(ic, TRUE);
    elm_icon_scale_set(ic, TRUE, TRUE);
    evas_object_show(ic);
    //evas_object_size_hint_min_set(ic, 70, 70);

    Evas_Object *bt = edje_object_add(evas_object_evas_get(win));
    edje_object_file_set(bt, MOKOSUITE_DATADIR "theme.edj", "launcher");
    edje_object_part_swallow(bt, "icon", ic);

    lb = elm_label_add(win);
    elm_object_style_set(lb, "marker");
    elm_object_scale_set(lb, 0.65);

    elm_label_line_wrap_set(lb, TRUE);
    elm_label_label_set(lb, name);

    evas_object_size_hint_weight_set(lb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(lb, 0.5, 0.0);
    evas_object_show(lb);
    edje_object_part_swallow(bt, "title", lb);

    evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    //evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_size_hint_min_set(bt, LAUNCHER_WIDTH, LAUNCHER_HEIGHT);
    //evas_object_size_hint_max_set(bt, 100, 100);
    evas_object_show(bt);

    // contenitore widget dragabile
    Evas_Object *wd = edje_object_add(evas_object_evas_get(win));
    edje_object_file_set(wd, MOKOSUITE_DATADIR "theme.edj", "widget");
    edje_object_part_swallow(wd, "widget", bt);
    evas_object_size_hint_min_set(wd, LAUNCHER_WIDTH, LAUNCHER_HEIGHT);

    //evas_object_event_callback_add(wd, EVAS_CALLBACK_MOUSE_DOWN, widget_pressed, scroller);
    //evas_object_event_callback_add(wd, EVAS_CALLBACK_MOUSE_UP, widget_released, scroller);

    evas_object_show(bt);
    evas_object_show(wd);

    add_widget(desktop_id, wd);
}

static Evas_Object* add_launcher_widget(Evas_Object* parent, int desktop_id, const char* desktop_file)
{
    Efreet_Desktop* d = efreet_desktop_get(desktop_file);
    Evas_Object* wd = NULL;

    if (d) {
        wd = widget_launcher_new(parent, d);
        if (wd)
            add_widget(desktop_id, wd);

        efreet_desktop_free(d);
    }

    return wd;
}

static Evas_Object* prepare_table(Evas_Object* win, int cols, int rows)
{
    // riempi la tabella con le icone
    Evas_Object* tb, *pad;

    tb = elm_table_add(win);
    evas_object_size_hint_weight_set(tb, 0.0, 0.0);
    evas_object_size_hint_align_set(tb, 0.5, 0.5);
    elm_table_padding_set(tb, 0, WIDGETS_PADDING0_HEIGHT);

    pad = evas_object_rectangle_add(evas_object_evas_get(win));
    evas_object_size_hint_min_set(pad, WIDGETS_PADDING1_WIDTH, WIDGETS_PADDING1_HEIGHT);
    evas_object_size_hint_weight_set(pad, 0.0, 0.0);
    evas_object_size_hint_align_set(pad, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_table_pack(tb, pad, 1, 0, cols, 1);

    pad = evas_object_rectangle_add(evas_object_evas_get(win));
    evas_object_size_hint_min_set(pad, WIDGETS_PADDING1_WIDTH, WIDGETS_PADDING1_HEIGHT);
    evas_object_size_hint_weight_set(pad, 0.0, 0.0);
    evas_object_size_hint_align_set(pad, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_table_pack(tb, pad, 1, 11, cols, 1);

    pad = evas_object_rectangle_add(evas_object_evas_get(win));
    evas_object_size_hint_min_set(pad, WIDGETS_PADDING2_WIDTH, WIDGETS_PADDING2_HEIGHT);
    evas_object_size_hint_weight_set(pad, 0.0, 0.0);
    evas_object_size_hint_align_set(pad, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_table_pack(tb, pad, 0, 1, 1, 10);

    pad = evas_object_rectangle_add(evas_object_evas_get(win));
    evas_object_size_hint_min_set(pad, WIDGETS_PADDING2_WIDTH, WIDGETS_PADDING2_HEIGHT);
    evas_object_size_hint_weight_set(pad, 0.0, 0.0);
    evas_object_size_hint_align_set(pad, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_table_pack(tb, pad, cols+1, 1, 1, 10);

    return tb;
}

static void load_launchers(int desktop_id, Evas_Object* win)
{
    char* entry;
    char* key = g_strdup("launcher1");
    int i = 1;
    char* section = g_strdup_printf("home/%d", desktop_id + 1);

    while ((entry = config_get_string(section, key, NULL))) {
        add_launcher_widget(win, desktop_id, entry);

        g_free(entry);
        g_free(key);
        key = g_strdup_printf("launcher%d", ++i);
    }

    g_free(section);
}

Evas_Object* make_widgets(Evas_Object* win, Evas_Object* scroller)
{
    Evas_Object *mb, *bx, *tb;

    bx = elm_box_add(win);
    elm_box_homogenous_set(bx, TRUE);
    elm_box_horizontal_set(bx, TRUE);

    evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_weight_set(bx, 0.0, 0.0);
    evas_object_show(bx);

    // riempi la tabella con le icone
    tb = prepare_table(win, WIDGETS_COLUMNS, WIDGETS_ROWS);
    desktops[0] = tb;

    // TODO funzione di distruzione
    widgets[0] = g_ptr_array_new();

    // launcher da configurazione semplice
    load_launchers(0, win);

    mb = elm_mapbuf_add(win);
    evas_object_size_hint_weight_set(mb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(mb, 0.0, 0.0);

    elm_mapbuf_content_set(mb, tb);
    evas_object_show(tb);

    elm_box_pack_end(bx, mb);
    evas_object_show(mb);

    // tabella 2 :)
    wx = 0;
    wy = 0;
    tb = prepare_table(win, WIDGETS_COLUMNS, WIDGETS_ROWS);
    desktops[1] = tb;

    // TODO funzione di distruzione
    widgets[1] = g_ptr_array_new();

    // launcher da configurazione semplice
    load_launchers(1, win);

    mb = elm_mapbuf_add(win);
    evas_object_size_hint_weight_set(mb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(mb, 0.0, 0.0);

    elm_mapbuf_content_set(mb, tb);
    evas_object_show(tb);

    elm_box_pack_end(bx, mb);
    evas_object_show(mb);

    #if 0
    struct widget_press_data* cb_data = g_new0(struct widget_press_data, 1);
    cb_data->scroller = scroller;
    cb_data->widgets = widgets[0];

    g_timeout_add(LONG_PRESS_TIME * 2, _widget_pressed, cb_data);

    cb_data = g_new0(struct widget_press_data, 1);
    cb_data->scroller = scroller;
    cb_data->widgets = widgets[0];

    g_timeout_add(LONG_PRESS_TIME * 10, _widget_pressed, cb_data);
    #endif

    return bx;
}
