#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <sqlite3.h>

#include "yodha.h"
#include "rowgroup.h"
#include "scan.h"
#include "predicate.h"

/* ---------------- Timing ---------------- */

static inline uint64_t now_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

/* ---------------- Yodha ---------------- */

void noop_cb(uint64_t row, int64_t v, void *ctx) {
    (void)row; (void)v; (void)ctx;
}

void benchmark_yodha(int threshold) {
    FILE *fp = fopen("bench.ydb", "rb");
    uint64_t rg_offset = sizeof(YodhaFileHeader);

    YodhaPredicate pred = {
        .column_id = 1,
        .op = OP_GT,
        .value = threshold
    };

    uint64_t t0 = now_ns();
    yodha_scan_int64_where_simd(
        fp,
        rg_offset,
        &pred,
        noop_cb,
        NULL
    );
    uint64_t t1 = now_ns();

    fclose(fp);
    printf("Yodha  (age > %d): %lu ns\n", threshold, t1 - t0);
}

/* ---------------- SQLite ---------------- */

void benchmark_sqlite(int threshold) {
    sqlite3 *db;
    sqlite3_stmt *stmt;

    sqlite3_open("bench.sqlite", &db);

    const char *sql =
        "SELECT user_age FROM users WHERE user_age > ?;";

    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, threshold);

    uint64_t t0 = now_ns();
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        sqlite3_column_int(stmt, 0);
    }
    uint64_t t1 = now_ns();

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    printf("SQLite (age > %d): %lu ns\n", threshold, t1 - t0);
}

/* ---------------- Setup ---------------- */

void setup_sqlite() {
    sqlite3 *db;
    char *err = NULL;

    sqlite3_open("bench.sqlite", &db);

    sqlite3_exec(db,
        "DROP TABLE IF EXISTS users;"
        "CREATE TABLE users (user_key INTEGER, user_age INTEGER);",
        NULL, NULL, &err);

    sqlite3_exec(db, "BEGIN;", NULL, NULL, NULL);

    for (int i = 1; i <= 50000; i++) {
        char buf[128];
        int age = (i % 50) + 10;  /* ages 10–59 */
        snprintf(buf, sizeof(buf),
                 "INSERT INTO users VALUES (%d, %d);",
                 i, age);
        sqlite3_exec(db, buf, NULL, NULL, NULL);
    }

    sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);
    sqlite3_close(db);
}

void setup_yodha() {
    FILE *fp = fopen("bench.ydb", "wb+");

    RowGroupHeader hdr = {
        .row_group_id = 0,
        .row_count = 50000,
        .column_count = 2,
        .min_txn_id = 1,
        .max_txn_id = 1,
        .total_size = 0
    };

    ColumnSchema schemas[2] = {
        {0, Y_INT64, COL_FLAG_PK, 8},
        {1, Y_INT64, 0,           7}
    };

    const char *names[2] = { "user_key", "user_age" };

    int64_t *keys = malloc(sizeof(int64_t) * 50000);
    int64_t *ages = malloc(sizeof(int64_t) * 50000);

    for (int i = 0; i < 50000; i++) {
        keys[i] = i + 1;
        ages[i] = (i % 50) + 10;
    }

    void *cols[2] = { keys, ages };
    uint64_t sizes[2] = {
        sizeof(int64_t) * 50000,
        sizeof(int64_t) * 50000
    };

    yodha_write_rowgroup(fp, &hdr, schemas, names, cols, sizes);

    fclose(fp);
    free(keys);
    free(ages);
}

/* ---------------- Main ---------------- */

int main() {
    printf("Setting up data...\n");
    setup_sqlite();
    setup_yodha();

    printf("\n--- Benchmark: age > 25 (scan required) ---\n");
    benchmark_sqlite(25);
    benchmark_yodha(25);

    printf("\n--- Benchmark: age > 100 (zone-map prune) ---\n");
    benchmark_sqlite(100);
    benchmark_yodha(100);

    return 0;
}