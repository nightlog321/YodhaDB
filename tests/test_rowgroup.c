#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "yodha.h"
#include "rowgroup.h"
#include "scan.h"
#include "predicate.h"

/* ---------- Callbacks ---------- */

void print_scan_cb(uint64_t row, int64_t value, void *ctx) {
    (void)ctx;
    printf("row %lu -> %ld\n", row, value);
}

void print_where_cb(uint64_t row, int64_t value, void *ctx) {
    (void)ctx;
    printf("MATCH row %lu -> %ld\n", row, value);
}

/* ---------- Main Test ---------- */

int main() {
    const char *fname = "test.ydb";

    /* ================================
       STEP 1: WRITE ROW GROUP
       ================================ */
    {
        FILE *fp = fopen(fname, "wb+");
        if (!fp) {
            perror("fopen");
            return 1;
        }

        RowGroupHeader hdr = {
            .row_group_id = 0,
            .row_count = 5,
            .column_count = 2,
            .min_txn_id = 1,
            .max_txn_id = 1,
            .total_size = 0  /* patched by writer */
        };

        ColumnSchema schemas[2] = {
            { .column_id = 0, .type = Y_INT64, .flags = COL_FLAG_PK, .name_len = 8 },
            { .column_id = 1, .type = Y_INT64, .flags = 0,           .name_len = 7 }
        };

        const char *names[2] = {
            "user_key",
            "user_age"
        };

        int64_t user_key[] = { 1, 2, 3, 4, 5 };
        int64_t user_age[] = { 24, 30, 28, 35, 22 };

        void *columns[2] = {
            user_key,
            user_age
        };

        uint64_t sizes[2] = {
            sizeof(user_key),
            sizeof(user_age)
        };

        yodha_write_rowgroup(fp, &hdr, schemas, names, columns, sizes);

        fclose(fp);
        printf("Row group written to %s\n", fname);
    }

    /* ================================
       STEP 2: READ + SCAN
       ================================ */
    {
        FILE *fp = fopen(fname, "rb");
        if (!fp) {
            perror("fopen");
            return 1;
        }

        /* Row group starts immediately after file header */
        uint64_t rowgroup_offset = sizeof(YodhaFileHeader);

        printf("\n--- Full scan: user_age ---\n");
        yodha_scan_int64_column(
            fp,
            rowgroup_offset,
            1,  /* user_age */
            print_scan_cb,
            NULL
        );

        /* ================================
           STEP 3: WHERE + SIMD
           ================================ */

        printf("\n--- WHERE user_age > 25 (SIMD if available) ---\n");

        YodhaPredicate pred = {
            .column_id = 1,   /* user_age */
            .op = OP_GT,
            .value = 100
        };

        yodha_scan_int64_where_simd(
            fp,
            rowgroup_offset,
            &pred,
            print_where_cb,
            NULL
        );

        fclose(fp);
    }

    return 0;
}