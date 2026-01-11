#ifndef ROWGROUP_H
#define ROWGROUP_H

#include "yodha.h"

#pragma pack(push, 1)

typedef struct {
    uint64_t row_group_id;
    uint32_t row_count;
    uint32_t column_count;
    uint64_t min_txn_id;
    uint64_t max_txn_id;
    uint64_t total_size;   // ✅ NEW: bytes of this row group
} RowGroupHeader;

typedef struct {
    uint32_t column_id;
    uint8_t  type;
    uint8_t  flags;
    uint16_t name_len;
} ColumnSchema;

typedef struct {
    uint32_t column_id;
    uint64_t offset;
    uint64_t size;

    /* Zone map (INT64 only for now) */
    int64_t  min_value;
    int64_t  max_value;
} ColumnChunkMeta;

#pragma pack(pop)

/* File helpers */
int yodha_init_file(FILE *fp);
int yodha_update_rowgroup_count(FILE *fp);

/* RowGroup writer / reader */
int yodha_write_rowgroup(
    FILE *fp,
    RowGroupHeader *hdr,
    ColumnSchema *schemas,
    const char **column_names,
    void **column_data,
    uint64_t *column_sizes
);

int yodha_read_column_meta(
    FILE *fp,
    uint64_t rowgroup_offset,
    uint32_t target_column_id,
    ColumnChunkMeta *out_meta
);

#endif