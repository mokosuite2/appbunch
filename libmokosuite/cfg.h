#ifndef __CONFIG_H
#define __CONFIG_H

/**
 * Carica la configurazione globale di base.
 */
void config_init(void);

/**
 * Salva la configurazione su file.
 * @return true se la configurazione è stata salvata con successo
 */
gboolean config_save(void);

/**
 * TODO
 */
gboolean config_has_key(const char *group, const char *key);

/**
 * Recupera una stringa dalla configurazione.
 * @param group nome della sezione
 * @param key nome della chiave
 * @param default_val facoltativo, valore di default se la chiave non è stata trovata
 * @return il valore trovato, altrimenti default_val
 */
char *config_get_string(const char *group, const char *key, const char *default_val);

/**
 * TODO
 */
int config_get_integer(const char *group, const char *key, int default_val);

/**
 * TODO
 */
void config_set_string(const char* group, const char* key, const char* value);

/**
 * TODO
 */
void config_set_integer(const char* group, const char* key, int value);

#endif  /* __CONFIG_H */
