#ifndef MATCH_SEARCH_H
#define MATCH_SEARCH_H

#include "match_types.h"

void match_init(match_t *m, const match_cfg_t *cfg);
void match_set_enable(match_t *m, bool on);
void match_load_preset(match_t *m);
void match_tick(match_t *m); /* call every 20–100 ms */

#endif /* MATCH_SEARCH_H */
