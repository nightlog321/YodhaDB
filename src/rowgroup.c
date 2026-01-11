#include <stdlib.h>
#include <string.h>
#include "rowgroup.h"

/* Initialize file header if empty */
int yodha_init_file(FILE *fp) {
    fseek(fp, 0, SEEK_END);
    if (ftell(fp) != 0) return 0;

    YodhaFileHeader hdr = {
        .magic = YODHA_MAGIC,
        .version = YODHA_VERSION,
        .row_group_count = 0
    };

    fseek(fp, 0, SEEK_SET);
    fwrite(&hdr, sizeof(hdr), 1, fp);
    fflush(fp);
    return 0;
}

int yodha_update_rowgroup_count(FILE *fp) {
    YodhaFileHeader hdr;
    fseek(fp, 0, SEEK_SET);
    fread(&hdr, sizeof(hdr), 1, fp);

    hdr.row_group_count++;

    fseek(fp, 0, SEEK_SET);
    fwrite(&hdr, sizeof(hdr), 1, fp);
    fflush(fp);
    return 0;
}

int yodha_write_rowgroup(
    FILE *fp,
    RowGroupHeader *hdr,
    ColumnSchema *schemas,
    const char **column_names,
    void **column_data,
    uint64_t *column_sizes
) {
    /* Ensure file header exists */
    yodha_init_file(fp);

    /* Move to append position */
    fseek(fp, 0, SEEK_END);
    uint64_t rowgroup_start = ftell(fp);

    /* Placeholder header (we fill total_size later) */
    uint64_t header_pos = rowgroup_start;
    fwrite(hdr, sizeof(RowGroupHeader), 1, fp);

    /* Schema section */
    for (uint32_t i = 0; i < hdr->column_count; i++) {
        fwrite(&schemas[i], sizeof(ColumnSchema), 1, fp);
        fwrite(column_names[i], schemas[i].name_len, 1, fp);
    }

    ColumnChunkMeta *metas =
        calloc(hdr->column_count, sizeof(ColumnChunkMeta));

    /* Column data */
    for (uint32_t i = 0; i < hdr->column_count; i++) {
        metas[i].column_id = schemas[i].column_id;
metas[i].offset    = ftell(fp);
metas[i].size      = column_sizes[i];

/* Compute zone map for INT64 */
if (schemas[i].type == Y_INT64) {
    int64_t *vals = (int64_t *)column_data[i];
    uint32_t n = hdr->row_count;

    int64_t minv = vals[0];
    int64_t maxv = vals[0];

    for (uint32_t r = 1; r < n; r++) {
        if (vals[r] < minv) minv = vals[r];
        if (vals[r] > maxv) maxv = vals[r];
    }

    metas[i].min_value = minv;
    metas[i].max_value = maxv;
}

fwrite(column_data[i], column_sizes[i], 1, fp);

    }

    /* Footer */
    uint64_t footer_offset = ftell(fp);
    fwrite(metas, sizeof(ColumnChunkMeta), hdr->column_count, fp);
    fwrite(&footer_offset, sizeof(uint64_t), 1, fp);

    uint64_t end_offset = ftell(fp);

    /* Patch total_size */
    hdr->total_size = end_offset - rowgroup_start;
    fseek(fp, header_pos, SEEK_SET);
    fwrite(hdr, sizeof(RowGroupHeader), 1, fp);

    /* Restore append position */
    fseek(fp, end_offset, SEEK_SET);
    free(metas);

    /* Update file header */
    yodha_update_rowgroup_count(fp);

    return 0;
}

int yodha_read_column_meta(
    FILE *fp,
    uint64_t rowgroup_offset,
    uint32_t target_column_id,
    ColumnChunkMeta *out_meta
) {
    RowGroupHeader hdr;
    fseek(fp, rowgroup_offset, SEEK_SET);
    fread(&hdr, sizeof(hdr), 1, fp);

    uint64_t footer_ptr =
        rowgroup_offset + hdr.total_size - sizeof(uint64_t);

    uint64_t footer_offset;
    fseek(fp, footer_ptr, SEEK_SET);
    fread(&footer_offset, sizeof(uint64_t), 1, fp);

    fseek(fp, footer_offset, SEEK_SET);

    ColumnChunkMeta meta;
    for (uint32_t i = 0; i < hdr.column_count; i++) {
        fread(&meta, sizeof(meta), 1, fp);
        if (meta.column_id == target_column_id) {
            *out_meta = meta;
            return 0;
        }
    }
    return -1;
}