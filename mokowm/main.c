/*
 * Mokosuite
 * Window manager
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

#include <Ecore_X.h>
#include <Ecore_Input.h>
#include <Ecore.h>
#include <Eet.h>
#include <Eina.h>

#include <unistd.h>
#include <ctype.h>
#include <glib.h>
#include <glib-object.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "wm.h"
#include "input.h"


int main(int argc, char* argv[])
{
    g_type_init();
    g_set_prgname(PACKAGE);

    eina_init();
    eet_init();
    ecore_init();
    ecore_app_args_set(argc, (const char **)argv);

    gboolean disable_input = FALSE;
    gboolean input_landscape = FALSE;

    int c;
    opterr = 0;
    while ((c = getopt (argc, argv, "il")) != -1) {
        switch (c) {
            case 'i':
                disable_input = TRUE;
                break;

            case 'l':
                input_landscape = TRUE;
                break;

            default:
                if (isprint(optopt))
                    g_error("Unknown option: `-%c'.", optopt);
                else
                    g_error("Unknown option: `-\\x%x'.", optopt);
                return EXIT_FAILURE;
        }
        opterr = 0;
    }


    //ecore_file_init();
    //evas_init();
    //edje_init();
    //ecore_evas_init();

    if (!ecore_x_init(NULL)) {
        g_error("Cannot connect to X server. Exiting.");
        return EXIT_FAILURE;
    }

    ecore_x_netwm_init();

    // inizializza il wm
    wm_init();

    if (!disable_input)
        input_win_new(input_landscape);

    // gestisci tutte le finestre
    manage_all_windows();

    ecore_main_loop_begin();

    ecore_shutdown();
    eet_shutdown();
    eina_shutdown();

    return EXIT_SUCCESS;
}
