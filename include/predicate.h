#ifndef YODHA_PREDICATE_H
#define YODHA_PREDICATE_H

#include <stdint.h>

typedef enum {
    OP_EQ,
    OP_NE,
    OP_GT,
    OP_GTE,
    OP_LT,
    OP_LTE
} YodhaOp;

typedef struct {
    uint32_t column_id;
    YodhaOp  op;
    int64_t  value;
} YodhaPredicate;

#endif