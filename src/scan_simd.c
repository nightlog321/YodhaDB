#include <immintrin.h>
#include "scan.h"
#include "predicate_eval.h"

static inline int supports_avx2() {
#if defined(__x86_64__)
    return __builtin_cpu_supports("avx2");
#else
    return 0;
#endif
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

int yodha_scan_int64_where_simd(
    FILE *fp,
    uint64_t rowgroup_offset,
    YodhaPredicate *pred,
    yodha_where_cb cb,
    void *ctx
) {
    if (!supports_avx2())
        return yodha_scan_int64_where(fp, rowgroup_offset, pred, cb, ctx);

    RowGroupHeader hdr;
    fseek(fp, rowgroup_offset, SEEK_SET);
    fread(&hdr, sizeof(hdr), 1, fp);

    ColumnChunkMeta meta;
    if (yodha_read_column_meta(fp, rowgroup_offset,
                               pred->column_id, &meta) != 0)
        return -1;

    if (!zone_map_allows(&meta, pred)) {
    return 0;
}

    fseek(fp, meta.offset, SEEK_SET);

    uint32_t n = hdr.row_count;
    uint32_t i = 0;

    __m256i cmp_val = _mm256_set1_epi64x(pred->value);

    while (i + 4 <= n) {
        int64_t buf[4];
        fread(buf, sizeof(int64_t), 4, fp);

        __m256i data = _mm256_loadu_si256((__m256i*)buf);
        __m256i mask;

        switch (pred->op) {
            case OP_GT:
                mask = _mm256_cmpgt_epi64(data, cmp_val);
                break;
            case OP_LT:
                mask = _mm256_cmpgt_epi64(cmp_val, data);
                break;
            case OP_EQ:
                mask = _mm256_cmpeq_epi64(data, cmp_val);
                break;
            default:
                /* Fallback for unsupported ops */
                fseek(fp, -4 * sizeof(int64_t), SEEK_CUR);
                return yodha_scan_int64_where(fp, rowgroup_offset,
                                              pred, cb, ctx);
        }

        int m = _mm256_movemask_pd((__m256d)mask);

        for (int j = 0; j < 4; j++) {
            if (m & (1 << j)) {
                cb(i + j, buf[j], ctx);
            }
        }
        i += 4;
    }

    /* Scalar tail */
    for (; i < n; i++) {
        int64_t v;
        fread(&v, sizeof(int64_t), 1, fp);
        if (eval_pred(v, pred)) {
            cb(i, v, ctx);
        }
    }

    return 0;
}