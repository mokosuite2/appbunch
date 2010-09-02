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
#include <Edje.h>
#include <Evas.h>
#include <Ecore_X.h>
#include <fakekey/fakekey.h>
#include <ctype.h>

#include "input.h"

// meta-costanti LOOOL :D
static int INPUT_WIDTH;
static int INPUT_HEIGHT;
static int INPUT_X;
static int INPUT_Y;

#define LANDSCAPE_INPUT_WIDTH     640
#define LANDSCAPE_INPUT_HEIGHT    254 //234
#define LANDSCAPE_INPUT_X         0
#define LANDSCAPE_INPUT_Y         (480 - LANDSCAPE_INPUT_HEIGHT)

#define PORTRAIT_INPUT_WIDTH     480
#define PORTRAIT_INPUT_HEIGHT    254 //234
#define PORTRAIT_INPUT_X         0
#define PORTRAIT_INPUT_Y         (640 - PORTRAIT_INPUT_HEIGHT)

static gboolean is_shift_down = FALSE;
static gboolean is_mouse_down = FALSE;

static Ecore_X_Window xwin = 0;
static Ecore_Evas* ee = NULL;
static Evas_Object* kbd = NULL;
static FakeKey* fk = NULL;
static GHashTable* pressed_keys = NULL;

static void show_win(int sig_num)
{
    input_win_show();
}

static void hide_win(int sig_num)
{
    input_win_hide();
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

Ecore_X_Window input_xwin(void)
{
    return xwin;
}

void input_win_show(void)
{
    if (ee) {
        // FIXME ehm!
        if (INPUT_WIDTH == LANDSCAPE_INPUT_WIDTH)
            system("xrandr -o 1");

        ecore_evas_show(ee);
        ecore_x_window_raise(xwin);
    }
}

void input_win_hide(void)
{
    if (ee) {
        ecore_evas_hide(ee);
        // FIXME ehm!
        if (INPUT_WIDTH == LANDSCAPE_INPUT_WIDTH)
            system("xrandr -o 0");
    }
}

/* l'oggetto restituito e' statico */
Ecore_Evas* input_win_new(gboolean landscape)
{
    if (ee) return ee;
    evas_init();
    edje_init();
    ecore_evas_init(); // FIXME: check errors

    if (landscape) {
        INPUT_WIDTH = LANDSCAPE_INPUT_WIDTH;
        INPUT_HEIGHT = LANDSCAPE_INPUT_HEIGHT;
        INPUT_X = LANDSCAPE_INPUT_X;
        INPUT_Y = LANDSCAPE_INPUT_Y;
    }
    else {
        INPUT_WIDTH = PORTRAIT_INPUT_WIDTH;
        INPUT_HEIGHT = PORTRAIT_INPUT_HEIGHT;
        INPUT_X = PORTRAIT_INPUT_X;
        INPUT_Y = PORTRAIT_INPUT_Y;
    }

    g_debug("Creating input window");
    ee = ecore_evas_software_x11_new(NULL, 0, INPUT_X, INPUT_Y, INPUT_WIDTH, INPUT_HEIGHT);
    if (ee == NULL) {
        g_warning("Unable to create input window canvas.");
        return NULL;
    }

    xwin = ecore_evas_software_x11_window_get(ee);
    ecore_x_netwm_window_type_set(xwin, ECORE_X_WINDOW_TYPE_UTILITY);

    Evas* evas = ecore_evas_get(ee);

    kbd = edje_object_add(evas);

    char* edjfile = g_strdup_printf(MOKOSUITE_DATADIR "vkbd.%s.edj", landscape ? "landscape" : "portrait");
    edje_object_file_set(kbd, edjfile, "main");
    g_free(edjfile);

    evas_object_resize(kbd, INPUT_WIDTH, INPUT_HEIGHT);

    // eventi sulla tastiera
    edje_object_signal_callback_add(kbd, "key_down", "*", kbd_key_down, NULL);
    edje_object_signal_callback_add(kbd, "mouse_over_key", "*", kbd_mouse_over_key, NULL);
    edje_object_signal_callback_add(kbd, "mouse_out_key", "*", kbd_mouse_out_key, NULL);
    edje_object_signal_callback_add(kbd, "mouse,down,1", "*", kbd_mouse_down_key, NULL);
    edje_object_signal_callback_add(kbd, "mouse,down,1,*", "*", kbd_mouse_down_key, NULL);
    edje_object_signal_callback_add(kbd, "mouse,up,1", "*", kbd_mouse_up_key, NULL);

    evas_object_show(kbd);

    ecore_evas_title_set(ee, "Virtual keyboard");
    // NON MOSTRARE SUBITO -- ecore_evas_show(ee);

    /*
    self.on_mouse_down_add(self.on_mouse_down)
    self.on_mouse_up_add(self.on_mouse_up)
    self.on_key_down_add(self.on_key_down)
    */


    #if 0
    /* -- registrazione INPUT METHOD -- */
    TODO
    TODO
    TODO

    ecore_imf_init();

    Ecore_IMF_Context_Class class = {0};
    //class.

    Ecore_IMF_Context* ctx = ecore_imf_context_new(&class);

    /* -- fine INPUT METHOD -- */
    #endif

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

    return ee;
}
