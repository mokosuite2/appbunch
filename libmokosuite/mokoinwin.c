
#include "mokosuite.h"
#include "gui.h"


void mokoinwin_activate(MokoInwin *mw)
{
    evas_object_show(mw->inwin);
    elm_win_inwin_activate(mw->inwin);
}

void mokoinwin_hide(MokoInwin *mw)
{
    evas_object_hide(mw->inwin);
}

void mokoinwin_destroy(MokoInwin *mw)
{
    if (mw != NULL) {

        // prima richiama il callback
        if (mw->delete_callback)
            (mw->delete_callback)(mw);

        // distruggi finestra
        evas_object_del(mw->inwin);
    
        // distruggi istanza
        g_free(mw);
    }
}

MokoInwin *mokoinwin_new(MokoWin *parent)
{
    MokoInwin *mw = g_new0(MokoInwin, 1);

    /* costruisci la finestra */
    mw->inwin = elm_win_inwin_add(parent->win);

    if(mw->inwin == NULL) {
        g_warning("Cannot instantiate inner window");
        g_free(mw);
        return NULL;
    }

    // MALEDETTO!!!!!
    //elm_win_resize_object_add(parent->win, mw->inwin);

    return mw;
}
