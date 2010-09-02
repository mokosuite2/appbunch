/*
 * Mokosuite
 * Virtual keyboard input window
 * Copyright (C) 2009-2010 Daniele Ricci <daniele.athome@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <signal.h>
#include <libmokosuite/mokosuite.h>
#include <Elementary.h>
#include <Ecore_X.h>
#include <fakekey/fakekey.h>

#include "input.h"

#define INPUT_WIDTH     640
#define INPUT_HEIGHT    234
#define INPUT_X         0
#define INPUT_Y         (480 - INPUT_HEIGHT)

static gboolean is_shift_down = FALSE;
static gboolean is_mouse_down = FALSE;

static Evas_Object* w = NULL;
static Evas_Object* kbd_layout = NULL;
static Evas_Object* kbd = NULL;
static FakeKey* fk = NULL;
static GHashTable* pressed_keys = NULL;

static gboolean _set_wm_strut(gpointer win);

static void show_win(int sig_num)
{
    if (w) {
        g_idle_add(_set_wm_strut, w);
        evas_object_show(w);
        elm_win_activate(w);
    }
}

static void hide_win(int sig_num)
{
    if (w) evas_object_hide(w);
}

static void each_release_key(gpointer key, gpointer value, gpointer data)
{
    edje_object_signal_emit((Evas_Object *) data, "release_key", (char*) key);
}

static void kbd_key_down(void *data, Evas_Object *obj, const char *emission, const char *source)
{
    char* key = NULL;
    char* work = g_strdup(source);

    key = strstr(work, ":");
    if (!key)
        key = (char *)work;
    else
        key++;

    if (!strcmp(key, "enter")) {
        // TODO press_shift();
        // key press!!
        fakekey_press_keysym(fk, XK_Return, 0);
        fakekey_release(fk);
    }

    else if (!strcmp(key, "backspace")) {
        // key press!!
        fakekey_press_keysym(fk, XK_BackSpace, 0);
        fakekey_release(fk);
    }

    else if (!strcmp(key, "shift")) {
        // TODO toggle_shift();
    }

    else if (!strcmp(key, ".?123") || !strcmp(key, "ABC") || !strcmp(key, "#+=") || !strcmp(key, ".?12")) {
        // lol
        //pass
    }

    else if (!strcmp(key, "&")) {
        //g_string_append(text, "&amp;");
        //edje_object_part_text_set(obj, "field", text->str);
        // TODO
    }

    else if (!strcmp(key, "<")) {
        //g_string_append(text, "&lt;");
        //edje_object_part_text_set(obj, "field", text->str);
        // TODO
    }

    else if (!strcmp(key, ">")) {
        //g_string_append(text, "&gt;");
        //edje_object_part_text_set(obj, "field", text->str);
        // TODO
    }

    else {
        if (is_shift_down) {
            // TODO release_shift();
            key[0] = toupper(key[0]);
        } else {
            key[0] = tolower(key[0]);
        }

        // key press!!!
        g_debug("Sending key press: %s (%c)", key, key[0]);
        fakekey_press(fk, (unsigned char *) key, 1, 0);
        fakekey_release(fk);
    }

    g_free(work);
}

static void kbd_mouse_over_key(void *data, Evas_Object *obj, const char *emission, const char *source)
{
    // mmm...
    if (!is_mouse_down || !strstr(source, ":")) return;

    char *work = g_strdup(source);
    char *part = NULL, *subpart = NULL;

    // sottoparte
    subpart = (strstr(work, ":") + 1);

    // parte
    *(subpart-1) = '\0';
    part = (char *) work;

    if (g_hash_table_lookup(pressed_keys, subpart))
        return;

    #if 0
    for k in self.pressed_keys.values():
        o.signal_emit("release_key", k)
    self.pressed_keys.clear()
    self.pressed_keys[subpart] = subpart
    o.signal_emit("press_key", subpart)
    #else
    Evas_Object* part_obj = edje_object_part_swallow_get(obj, part);

    g_hash_table_foreach(pressed_keys, each_release_key, part_obj);
    g_hash_table_remove_all(pressed_keys);
    g_hash_table_insert(pressed_keys, g_strdup(subpart), GINT_TO_POINTER(1));

    edje_object_signal_emit(part_obj, "press_key", subpart);
    #endif
}

static void kbd_mouse_out_key(void *data, Evas_Object *obj, const char *emission, const char *source)
{
    // mmm...
    if (!is_mouse_down || !strstr(source, ":")) return;

    char *work = g_strdup(source);
    char *part = NULL, *subpart = NULL;

    // sottoparte
    subpart = (strstr(work, ":") + 1);

    // parte
    *(subpart-1) = '\0';
    part = (char *) work;

    Evas_Object* part_obj = edje_object_part_swallow_get(obj, part);

    #if 0
    if subpart in self.pressed_keys:
        del self.pressed_keys[subpart]
        o.signal_emit("release_key", subpart)
    #else
    if (g_hash_table_lookup(pressed_keys, subpart)) {
        g_hash_table_remove(pressed_keys, subpart);
        edje_object_signal_emit(part_obj, "release_key", subpart);
    }
    #endif
}

static void kbd_mouse_down_key(void *data, Evas_Object *obj, const char *emission, const char *source)
{
    // mmm...
    if (!strstr(source, ":")) return;

    char *work = g_strdup(source);
    char *part = NULL, *subpart = NULL;

    // sottoparte
    subpart = (strstr(work, ":") + 1);

    // parte
    *(subpart-1) = '\0';
    part = (char *) work;

    is_mouse_down = TRUE;

    if (g_hash_table_lookup(pressed_keys, subpart))
        return;

    // TODO
    #if 0
    for k in self.pressed_keys.values():
        o.signal_emit("release_key", k)
    self.pressed_keys.clear()
    self.pressed_keys[subpart] = subpart
    #else
    Evas_Object* part_obj = edje_object_part_swallow_get(obj, part);

    g_hash_table_foreach(pressed_keys, each_release_key, part_obj);
    g_hash_table_remove_all(pressed_keys);
    g_hash_table_insert(pressed_keys, g_strdup(subpart), GINT_TO_POINTER(1));
    #endif

    edje_object_signal_emit(part_obj, "press_key", subpart);

    g_free(work);
}

static void kbd_mouse_up_key(void *data, Evas_Object *obj, const char *emission, const char *source)
{
    // mmm...
    if (!strstr(source, ":")) return;

    char *work = g_strdup(source);
    char *part = NULL, *subpart = NULL;

    // sottoparte
    subpart = (strstr(work, ":") + 1);

    // parte
    *(subpart-1) = '\0';
    part = (char *) work;

    Evas_Object* part_obj = edje_object_part_swallow_get(obj, part);

    is_mouse_down = FALSE;

    // TODO
    #if 0
    if subpart in self.pressed_keys:
        del self.pressed_keys[subpart]
        o.signal_emit("release_key", subpart)
        o.signal_emit("activated_key", subpart)
    #else
    if (g_hash_table_lookup(pressed_keys, subpart)) {
        g_hash_table_remove(pressed_keys, subpart);

        edje_object_signal_emit(part_obj, "release_key", subpart);
        edje_object_signal_emit(part_obj, "activated_key", subpart);
    }
    #endif
}

static void set_wm_strut(Evas_Object* win)
{
    Ecore_X_Window_State state;

    state = ECORE_X_WINDOW_STATE_SKIP_TASKBAR;
    ecore_x_netwm_window_state_set(elm_win_xwindow_get(win), &state, 1);

    state = ECORE_X_WINDOW_STATE_SKIP_PAGER;
    ecore_x_netwm_window_state_set(elm_win_xwindow_get(win), &state, 1);

    #if 0
    int wm_struct_vals[] = {
        0, /* left */
        0, /* right */
        0, /* top */
        0, /* bottom */
        0, /* left_start_y */
        0, /* left_end_y */
        0, /* right_start_y */
        0, /* right_end_y */
        0, /* top_start_x */
        0, /* top_end_x */
        0, /* bottom_start_x */
        1399 }; /* bottom_end_x */
    #endif

    gulong data[12] = { 0 };

    data[0] = 0;
    data[1] = 0;
    data[2] = 0;
    data[3] = INPUT_HEIGHT;

    /* if wm supports STRUT_PARTIAL it will ignore STRUT */
    ecore_x_window_prop_property_set(elm_win_xwindow_get(win), ECORE_X_ATOM_NET_WM_STRUT_PARTIAL,
        ECORE_X_ATOM_CARDINAL, 32, data, 12);

    ecore_x_window_prop_property_set(elm_win_xwindow_get(win), ECORE_X_ATOM_NET_WM_STRUT,
        ECORE_X_ATOM_CARDINAL, 32, data, 4);

    XWMHints            *wm_hints;
    wm_hints = XAllocWMHints();

    if (wm_hints)
    {
        wm_hints->input = False;
        wm_hints->flags = InputHint;
        XSetWMHints(ecore_x_display_get(), elm_win_xwindow_get(win), wm_hints );
        XFree(wm_hints);
    }

    XSizeHints           size_hints;

    size_hints.flags = PPosition | PSize | PMinSize;
    size_hints.x = 0;
    size_hints.y = 0;
    size_hints.width      =  INPUT_WIDTH;
    size_hints.height     =  INPUT_HEIGHT;
    size_hints.min_width  =  INPUT_WIDTH;
    size_hints.min_height =  INPUT_HEIGHT;

    XSetStandardProperties(ecore_x_display_get(), elm_win_xwindow_get(win), "Keyboard",
            NULL, 0, NULL, 0, &size_hints);
}

static gboolean _set_wm_strut(gpointer win)
{
    set_wm_strut((Evas_Object *) win);
    return FALSE;
}

/* l'oggetto restituito e' statico */
Evas_Object* input_win_new(void)
{
    if (w) return w;

    w = elm_win_add(NULL, "mokoinput", ELM_WIN_TOOLBAR);
    if (w == NULL) {
        g_warning("Unable to create input window.");
        return NULL;
    }

    elm_win_title_set(w, "Virtual keyboard");
    elm_win_borderless_set(w, TRUE);
    // TODO elm_win_keyboard_win_set(w, TRUE);

    evas_object_resize(w, INPUT_WIDTH, INPUT_HEIGHT);
    evas_object_move(w, INPUT_X, INPUT_Y);

    g_idle_add(_set_wm_strut, w);

    kbd_layout = elm_layout_add(w);
    elm_layout_file_set(kbd_layout, MOKOSUITE_DATADIR "vkbd.edj", "main");

    kbd = elm_layout_edje_get(kbd_layout);
    // eventi sulla tastiera
    edje_object_signal_callback_add(kbd, "key_down", "*", kbd_key_down, w);
    edje_object_signal_callback_add(kbd, "mouse_over_key", "*", kbd_mouse_over_key, w);
    edje_object_signal_callback_add(kbd, "mouse_out_key", "*", kbd_mouse_out_key, w);
    edje_object_signal_callback_add(kbd, "mouse,down,1", "*", kbd_mouse_down_key, w);
    edje_object_signal_callback_add(kbd, "mouse,down,1,*", "*", kbd_mouse_down_key, w);
    edje_object_signal_callback_add(kbd, "mouse,up,1", "*", kbd_mouse_up_key, w);

    /*
    self.on_mouse_down_add(self.on_mouse_down)
    self.on_mouse_up_add(self.on_mouse_up)
    self.on_key_down_add(self.on_key_down)
    */

    elm_win_resize_object_add(w, kbd_layout);
    evas_object_show(kbd_layout);

    // segnaletica ;)
    struct sigaction usr1;
    usr1.sa_handler = show_win;
    usr1.sa_flags = 0;
    sigaction(SIGUSR1, &usr1, NULL);

    struct sigaction usr2;
    usr2.sa_handler = hide_win;
    usr2.sa_flags = 0;
    sigaction(SIGUSR2, &usr2, NULL);

    // inizializza fakekey
    fk = fakekey_init(ecore_x_display_get());
    g_debug("FakeKey instance created (%p)", fk);

    // array tasti premuti
    pressed_keys = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

    return w;
}
