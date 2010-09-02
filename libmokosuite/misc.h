#ifndef __MISC_H
#define __MISC_H

#include <glib.h>
#include <time.h>

const char* fso_get_attribute(GHashTable* properties, const char* key);
int fso_get_attribute_int(GHashTable* properties, const char* key);

guint64 get_current_time(void);

char* get_time_repr_full(guint64 timestamp);
char* get_time_repr(guint64 timestamp);

time_t get_modification_time(const char* path);

#endif  /* __FSO_H */
