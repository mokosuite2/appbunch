#ifndef __GSM_H
#define __GSM_H

#include <libmokosuite/mokosuite.h>
#include <libmokosuite/settings-service.h>

gboolean gsm_is_ready(void);
gboolean gsm_can_call(void);

void gsm_online(void);
void gsm_offline(void);

void gsm_init(MokoSettingsService *settings);

#endif  /* __GSM_H */
