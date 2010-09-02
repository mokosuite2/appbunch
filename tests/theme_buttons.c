#include <stdlib.h>
#include <Elementary.h>
#include <libmokosuite/gui.h>
#include <glib.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

static void _close(void *mokowin, Evas_Object *obj, void *event_info)
{
    elm_exit();
}

static Evas_Object* add_button(Evas_Object* parent, Evas_Object* vbox, const char* label)
{
    Evas_Object* btn = elm_button_add(parent);
    elm_button_label_set(btn, label);
    evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, 0.0);
    evas_object_size_hint_align_set(btn, -1.0, 0.0);

    elm_box_pack_end(vbox, btn);
    evas_object_show(btn);
    return btn;
}

static Evas_Object* add_check(Evas_Object* parent, Evas_Object* vbox, const char* label)
{
    Evas_Object* btn = elm_check_add(parent);
    if (label)
        elm_check_label_set(btn, label);
    evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, 0.0);
    evas_object_size_hint_align_set(btn, 0.5, 0.0);

    elm_box_pack_end(vbox, btn);
    evas_object_show(btn);
    return btn;
}

static void test_buttons()
{
    MokoWin* win = mokowin_new("test_buttons");
    mokowin_create_vbox(win, FALSE);

    evas_object_move(win->win, 0, 0);
    evas_object_resize(win->win, 480, 640);
    win->delete_callback = _close;

    add_button(win->win, win->vbox, "OK");
    add_button(win->win, win->vbox, "Cancel");
    add_button(win->win, win->vbox, "Terminate");
    add_button(win->win, win->vbox, "Wait");
    add_button(win->win, win->vbox, "TEST");

    add_check(win->win, win->vbox, NULL);

    mokowin_activate(win);
}

int main(int argc, char* argv[])
{
    g_set_prgname(PACKAGE);

    elm_init(argc, argv);
    if (!ecore_main_loop_glib_integrate())
        g_error("Ecore/GLib integration failed!");

    test_buttons();

    elm_run();
    elm_shutdown();

    return EXIT_SUCCESS;
}
