#include <stdio.h>
#include <stdlib.h>

#include "ai_shim.h"
#include "scan.h"

void print_cb(uint64_t row, int64_t value, void *ctx) {
    printf("row %lu -> %ld\n", row, value);
}

int main() {
    char json[256];

    if (!fgets(json, sizeof(json), stdin)) {
        fprintf(stderr, "Failed to read AI JSON\n");
        return 1;
    }
    printf("RAW JSON INPUT:\n[%s]\n", json);

    YodhaPredicate pred;
    if (yodha_parse_ai_json(json, &pred) != 0) {
        fprintf(stderr, "Invalid AI predicate\n");
        return 1;
    }

    FILE *fp = fopen("test.ydb", "rb");
    uint64_t rg_offset = sizeof(YodhaFileHeader);

    yodha_scan_int64_where_simd(
        fp,
        rg_offset,
        &pred,
        print_cb,
        NULL
    );

    fclose(fp);
    return 0;
}