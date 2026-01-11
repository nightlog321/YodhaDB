#ifndef YODHA_H
#define YODHA_H

#include <stdint.h>
#include <stdio.h>

#define YODHA_MAGIC "YODHADB"
#define YODHA_VERSION 1

typedef enum {
    Y_INT64  = 1,
    Y_DOUBLE = 2,
    Y_BOOL   = 3,
    Y_STRING = 4
} YodhaType;

#define COL_FLAG_PK       0x01
#define COL_FLAG_NULLABLE 0x02

#pragma pack(push, 1)
typedef struct {
    char     magic[8];          // "YODHADB"
    uint32_t version;           // 1
    uint64_t row_group_count;
} YodhaFileHeader;
#pragma pack(pop)

#endif