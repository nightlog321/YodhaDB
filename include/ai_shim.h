#ifndef YODHA_AI_SHIM_H
#define YODHA_AI_SHIM_H

#include <stdint.h>
#include "predicate.h"

/*
 * Parse AI-produced JSON into a YodhaPredicate.
 *
 * Expected JSON format (strict):
 *
 * {
 *   "column": "user_age",
 *   "operator": ">",
 *   "value": 25
 * }
 *
 * Returns:
 *   0  on success
 *  -1  on parse or validation failure
 */
int yodha_parse_ai_json(
    const char *json,
    YodhaPredicate *out
);

#endif /* YODHA_AI_SHIM_H */