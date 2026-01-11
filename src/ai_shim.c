#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include "ai_shim.h"

/* Skip spaces and quotes */
static const char *skip_ws_quotes(const char *p) {
    while (*p && (isspace(*p) || *p == '"'))
        p++;
    return p;
}

static int parse_string_field(
    const char *json,
    const char *key,
    char *out,
    size_t out_sz
) {
    const char *p = strstr(json, key);
    if (!p)
        return -1;

    p = strchr(p, ':');
    if (!p)
        return -1;
    p++;

    p = skip_ws_quotes(p);

    size_t i = 0;
    while (*p && *p != '"' && !isspace(*p) && i + 1 < out_sz) {
        out[i++] = *p++;
    }
    out[i] = '\0';

    return i > 0 ? 0 : -1;
}

static int parse_int_field(
    const char *json,
    const char *key,
    int64_t *value
) {
    const char *p = strstr(json, key);
    if (!p)
        return -1;

    p = strchr(p, ':');
    if (!p)
        return -1;
    p++;

    while (*p && !isdigit(*p))
        p++;

    if (!*p)
        return -1;

    *value = strtoll(p, NULL, 10);
    return 0;
}

int yodha_parse_ai_json(
    const char *json,
    YodhaPredicate *out
) {
    if (!json || !out)
        return -1;

    char column[64];
    char op[8];

    /* Parse column */
    if (parse_string_field(json, "\"column\"", column, sizeof(column)) != 0)
        return -1;

    if (strcmp(column, "user_age") != 0)
        return -1;

    out->column_id = 1; /* user_age */

    /* Parse operator */
    if (parse_string_field(json, "\"operator\"", op, sizeof(op)) != 0)
        return -1;

    if (strcmp(op, ">") == 0)
        out->op = OP_GT;
    else if (strcmp(op, ">=") == 0)
        out->op = OP_GTE;
    else if (strcmp(op, "<") == 0)
        out->op = OP_LT;
    else if (strcmp(op, "<=") == 0)
        out->op = OP_LTE;
    else if (strcmp(op, "=") == 0)
        out->op = OP_EQ;
    else
        return -1;

    /* Parse value */
    if (parse_int_field(json, "\"value\"", &out->value) != 0)
        return -1;

    return 0;
}