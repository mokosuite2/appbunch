#ifndef __MISC_H
#define __MISC_H

#include <glib.h>
#include <glib-object.h>
#include <time.h>

const char* fso_get_attribute(GHashTable* properties, const char* key);
int fso_get_attribute_int(GHashTable* properties, const char* key);

void g_value_free(gpointer data);

GValue* g_value_from_string(const char* string);
GValue* g_value_from_int(int number);

guint64 get_current_time(void);

char* get_time_repr_full(guint64 timestamp);
char* get_time_repr(guint64 timestamp);

time_t get_modification_time(const char* path);

#endif  /* __FSO_H */
