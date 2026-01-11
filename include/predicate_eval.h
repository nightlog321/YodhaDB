#ifndef YODHA_PREDICATE_EVAL_H
#define YODHA_PREDICATE_EVAL_H

#include "predicate.h"

static inline int eval_pred(int64_t v, YodhaPredicate *p) {
    switch (p->op) {
        case OP_EQ:  return v == p->value;
        case OP_NE:  return v != p->value;
        case OP_GT:  return v >  p->value;
        case OP_GTE: return v >= p->value;
        case OP_LT:  return v <  p->value;
        case OP_LTE: return v <= p->value;
        default:     return 0;
    }
}

#endif