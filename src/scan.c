#include <stdlib.h>
#include "scan.h"
#include "predicate_eval.h"

int yodha_scan_int64_column(
    FILE *fp,
    uint64_t rowgroup_offset,
    uint32_t column_id,
    yodha_int64_cb cb,
    void *ctx
) {
    /* Read row group header */
    RowGroupHeader hdr;
    fseek(fp, rowgroup_offset, SEEK_SET);
    fread(&hdr, sizeof(hdr), 1, fp);

    /* Locate column metadata */
    ColumnChunkMeta meta;
    if (yodha_read_column_meta(fp, rowgroup_offset, column_id, &meta) != 0) {
        return -1;
    }

    /* Seek to column data */
    fseek(fp, meta.offset, SEEK_SET);

    uint32_t row_count = hdr.row_count;

    for (uint32_t i = 0; i < row_count; i++) {
        int64_t value;
        fread(&value, sizeof(int64_t), 1, fp);
        cb(i, value, ctx);
    }

    return 0;
}

static inline int zone_map_allows(
    ColumnChunkMeta *m,
    YodhaPredicate *p
) {
    switch (p->op) {
        case OP_GT:  return m->max_value >  p->value;
        case OP_GTE: return m->max_value >= p->value;
        case OP_LT:  return m->min_value <  p->value;
        case OP_LTE: return m->min_value <= p->value;
        case OP_EQ:  return (p->value >= m->min_value &&
                             p->value <= m->max_value);
        default:     return 1;
    }
}


int yodha_scan_int64_where(
    FILE *fp,
    uint64_t rowgroup_offset,
    YodhaPredicate *pred,
    yodha_where_cb cb,
    void *ctx
) {
    RowGroupHeader hdr;
    fseek(fp, rowgroup_offset, SEEK_SET);
    fread(&hdr, sizeof(hdr), 1, fp);

    ColumnChunkMeta meta;
    if (yodha_read_column_meta(fp, rowgroup_offset,
                               pred->column_id, &meta) != 0)
        return -1;

    if (!zone_map_allows(&meta, pred)) {
    /* Entire row group does not satisfy predicate */
    return 0;
}

    fseek(fp, meta.offset, SEEK_SET);

    for (uint32_t i = 0; i < hdr.row_count; i++) {
        int64_t v;
        fread(&v, sizeof(int64_t), 1, fp);
        if (eval_pred(v, pred)) {
            cb(i, v, ctx);
        }
    }
    return 0;
}


