#ifndef __MENU_BLUETOOTH_H
#define __MENU_BLUETOOTH_H

#include "menu-common.h"

#define RESOURCE_BLUETOOTH      "Bluetooth"

void menu_bluetooth_init_item(MenuItem* item, gboolean extra);

void menu_bluetooth_init(void);

#endif  /* __MENU_BLUETOOTH_H */
