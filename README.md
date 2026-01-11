
A research-oriented columnar storage engine written in C, featuring SIMD-accelerated scans,
zone-map pruning, and a local AI-assisted query interface.

Defined a new file format:
For Example,consider a User table with 2 columns:user_key,user_age.In this file format,in persistent storage it would be stored as
```text
test.ydb
│
├── FileHeader
│
└── RowGroup 0
    ├── RowGroupHeader
    ├── Schema
    │   ├── user_key  (INT64, PK)
    │   └── user_age  (INT64)
    │
    ├── Column Data
    │   ├── user_key  → [1, 2, 3, 4, 5]
    │   └── user_age  → [24, 30, 28, 35, 22]
    │
    └── Footer (column offsets)
```
    
**Zone-Mapping:**
Each row group:
1.Has its own schema
2.Has its own column offsets
3.Has its own size
4.Can be skipped or scanned independently.

**For each column chunk in a row group, store:**
min_value
max_value
```text
RowGroup 0
├── user_key: min = 1,  max = 5
└── user_age: min = 22, max = 35
```
 These min,max can be used to skip entire row group entirely.

**Features**
- Custom columnar file format
- SIMD-accelerated predicate evaluation
- Row-group level zone-map pruning
- SQLite benchmark comparisons
- Local BitNet-inspired neural intent model (CPU-only)
- Deterministic C execution engine

**How to Run:**
make clean
make all
./test_rg
./bench

python3 python/bitnet_intent_model.py "age equal to 30" | ./ai_cli

<img width="1128" height="116" alt="image" src="https://github.com/user-attachments/assets/42072f27-b44d-49b3-bb5b-77079fc7dc66" />

## Benchmark Results

The following benchmarks compare YodhaDB against SQLite on a 50,000-row dataset
using equivalent predicates and no secondary indexes.
<img width="1013" height="224" alt="image" src="https://github.com/user-attachments/assets/c8ba254a-a7d4-4a51-afd8-9554118bf28f" />

These results demonstrate the benefits of columnar execution and zone-map
pruning for analytical workloads. Measurements were taken on a single machine
and are intended for qualitative comparison rather than absolute performance claims.


**Future Work**

 **Adaptive and Learned Metadata**  
  While YodhaDB currently uses static zone-map statistics, future work could
  explore learned or adaptive metadata (e.g., workload-aware statistics or
  learned indexes) to further reduce scan overhead.

- **Execution Planning Beyond Single Predicates**  
  Query execution is currently limited to single-predicate scans. Extending
  the engine with a lightweight planning layer for compound predicates would
  enable more expressive analytical queries without introducing full SQL
  parsing complexity.

- **Memory and Cache-Aware Layouts**  
  Additional exploration of cache-line alignment, prefetching strategies, and
  vectorized execution paths could further improve performance on modern CPUs.

- **Stronger AI–Systems Integration**  
  The AI-assisted query layer currently focuses on intent-to-predicate mapping.
  Future work could investigate ML-assisted execution decisions, such as
  dynamically choosing scan strategies based on observed workloads.


