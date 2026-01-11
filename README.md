 YodhaDB

A research-oriented columnar storage engine written in C, featuring SIMD-accelerated scans,
zone-map pruning, and a local AI-assisted query interface.

Features
- Custom columnar file format
- SIMD-accelerated predicate evaluation
- Row-group level zone-map pruning
- SQLite benchmark comparisons
- Local BitNet-inspired neural intent model (CPU-only)
- Deterministic C execution engine

How to Run:


python3 python/bitnet_intent_model.py "age equal to 30" | ./ai_cli
