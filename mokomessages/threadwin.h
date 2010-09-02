#ifndef __THREADWIN_H
#define __THREADWIN_H

#include <libmokosuite/settings-service.h>

void thread_win_activate(void);
void thread_win_hide(void);

void thread_win_init(MokoSettingsService *settings);

#endif  /* __THREADWIN_H */
