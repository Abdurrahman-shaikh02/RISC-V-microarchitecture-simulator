```
cpu-sim/
│
├── src/
│   ├── core/              # CPU core logic
│   │   ├── cpu.c
│   │   ├── pipeline.c
│   │   ├── alu.c
│   │   ├── register_file.c
│   │   └── control.c
│   │
│   ├── stages/            # Pipeline stage implementations
│   │   ├── if_stage.c
│   │   ├── id_stage.c
│   │   ├── ex_stage.c
│   │   ├── mem_stage.c
│   │   └── wb_stage.c
│   │
│   ├── memory/            # Memory system
│   │   ├── memory.c
│   │   ├── cache.c
│   │   └── cache_policy.c
│   │
│   ├── branch/            # Branch prediction
│   │   ├── predictor.c
│   │   ├── btb.c          # optional later
│   │   └── branch_unit.c
│   │
│   ├── hazard/            # Hazard detection + forwarding
│   │   ├── hazard.c
│   │   └── forwarding.c
│   │
│   ├── isa/               # Instruction set definition
│   │   ├── instr.c
│   │   └── decoder.c
│   │
│   ├── stats/             # Performance tracking
│   │   └── stats.c
│   │
│   ├── utils/             # Debug + helpers
│   │   ├── logger.c
│   │   └── config.c
│   │
│   └── main.c             # Entry point
│
├── include/
│   ├── core/
│   ├── stages/
│   ├── memory/
│   ├── branch/
│   ├── hazard/
│   ├── isa/
│   ├── stats/
│   └── utils/
│
├── programs/              # Test programs (VERY IMPORTANT)
│   ├── simple_add.txt
│   ├── load_store.txt
│   ├── branch_test.txt
│   └── hazard_test.txt
│
├── build/
│
├── Makefile
│
└── README.md
```
