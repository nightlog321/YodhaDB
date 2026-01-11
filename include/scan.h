#ifndef YODHA_SCAN_H
#define YODHA_SCAN_H

#include <stdint.h>
#include <stdio.h>
#include "rowgroup.h"
#include "predicate.h"

/* Callback for each row */
typedef void (*yodha_int64_cb)(
    uint64_t row_id,
    int64_t value,
    void *ctx
);

typedef void (*yodha_where_cb)(
    uint64_t row_id,
    int64_t value,
    void *ctx
);

/*
 * Scan an INT64 column in a row group
 *
 * fp              : open file pointer
 * rowgroup_offset : offset where row group starts
 * column_id       : column to scan
 * cb              : callback per row
 * ctx             : user context
 */
int yodha_scan_int64_column(
    FILE *fp,
    uint64_t rowgroup_offset,
    uint32_t column_id,
    yodha_int64_cb cb,
    void *ctx
);

int yodha_scan_int64_where(
    FILE *fp,
    uint64_t rowgroup_offset,
    YodhaPredicate *pred,
    yodha_where_cb cb,
    void *ctx
);


#endif