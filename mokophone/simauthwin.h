#ifndef __SIMAUTHWIN_H
#define __SIMAUTHWIN_H

#include <glib.h>
#include <libmokosuite/gui.h>
#include <frameworkd-glib/ogsmd/frameworkd-glib-ogsmd-sim.h>

struct _SimAuthWin {
    /* la finestra -- per il cast a MokoWin */
    MokoWin win;

    /* tipo di autenticazione -- cosa stiamo richiedendo */
    SimStatus type;

    /* widget vari*/
    Evas_Object* keypad;
    Evas_Object* keypad_edje;

    /* codice in corso di digitazione */
    GString* code;

    /* popup status per verifica */
    MokoPopupStatus* status;

    /* callback richiamato alla conferma del codice */
    void (*code_callback)(void *simauthwin, const char *code);

};

typedef struct _SimAuthWin SimAuthWin;

#define SIMAUTH_WIN(x)     ((SimAuthWin*)x)

void sim_auth_win_auth_error(SimAuthWin *s);

void sim_auth_win_destroy(SimAuthWin *s);
SimAuthWin* sim_auth_win_new(SimStatus type);

#endif  /* __SIMAUTHWIN_H */
