# 6. CPU Architecture

## 6.1 Overview

The simulator models a **32-bit RV32I processor** based on the classic **five-stage in-order pipeline**. The objective of the design is to faithfully reproduce the behavior of a real processor while remaining modular, configurable, and easy to visualize.

The simulated processor consists of the following major architectural components:

- Program Counter (PC)
- Instruction Memory
- Register File
- Immediate Generator
- Control Unit
- Arithmetic Logic Unit (ALU)
- Data Memory
- Pipeline Registers
- Forwarding Unit
- Branch Prediction Unit
- Cache Hierarchy

Although the architecture follows the conventional five-stage RISC pipeline, several components have been designed specifically to improve configurability and visualization within the simulator. These implementation decisions are discussed throughout this chapter.

---

### Complete Processor Datapath

[![CPU Datapath](assets/figures/cpu_datapath.svg)](assets/figures/cpu_datapath.svg)
> Figure 6.1: Complete Processor Datapath

---

## 6.2 Processor Datapath

Figure 6.1 illustrates the complete processor datapath implemented by the simulator. The datapath represents the hardware components responsible for executing every instruction and the connections between them.

At a high level, an instruction travels through the following sequence:

```text
Instruction Fetch
        ↓
Instruction Decode
        ↓
Execute
        ↓
Memory Access
        ↓
Write Back
```

Each stage performs a specific task before passing the required data and control information to the next stage through pipeline registers. Multiple instructions may occupy different stages simultaneously, allowing the processor to exploit instruction-level parallelism.

The datapath also contains several auxiliary components such as forwarding paths, hazard detection logic, branch prediction hardware, and cache interfaces. These components improve processor performance without altering the architectural behavior of the ISA.

The following sections describe each major component of the processor individually.

---

# 6.3 Major Architectural Components

Each hardware block shown in the datapath serves a specific purpose in instruction execution. This section introduces the role of every component, describes how it operates architecturally, and explains the design decisions made while implementing it in the simulator.

---

## 6.3.1 Program Counter (PC)

### Purpose

The Program Counter (PC) stores the address of the next instruction to be fetched. During normal execution, the PC advances sequentially, while branches and jumps modify its value to redirect control flow.

### Architectural Operation

- Stores the address of the next instruction.
- Normally increments by four bytes.
- Updated after branches, jumps, exceptions (future work), or resets.

### Simulator Implementation

The Program Counter is implemented as a **uint32_t** data type.

Why use uint32_t for registers ?
>See 6.5 Design Decisions 

---

## 6.3.2 Instruction Memory

### Purpose

Instruction memory stores the executable program that is fetched by the processor.

### Architectural Operation

- Supplies one instruction every fetch cycle.
- Read-only during program execution.
- Accessed using the current Program Counter.

### Simulator Implementation

The simulator models instruction memory as a separate read-only memory region loaded directly by the assembler or executable loader. Keeping instructions separate from writable data simplifies memory management. The code however, remains in one unified address space and conforms to **Von Neumann Architecture**.

---

## 6.3.3 Register File

### Purpose

The register file provides fast storage for operands and instruction results.

### Architectural Operation

The RV32I architecture defines thirty-two general-purpose registers together with a program counter. Register **r0** is permanently hardwired to zero, while the remaining registers may be used by software according to the calling convention.

The register file provides:

- Two simultaneous read ports
- One write port
- One write-back operation per cycle

### Simulator Implementation

The register file is implemented as a fixed-size array of 32 **uint32_t** data objects. Writes to r0 are ignored, ensuring architectural correctness while keeping the implementation straightforward.

Why uint32_t for registers ?
>See 6.5 Design Decisions

---

## 6.3.4 Immediate Generator

### Purpose

Many instructions contain immediate values encoded in different instruction formats.

### Architectural Operation

The Immediate Generator extracts, sign-extends and left shifts (if required by the format) immediates for the supported instruction formats:

- I-Type
- S-Type
- B-Type
- U-Type
- J-Type

These immediates are used for arithmetic operations, address generation, branches, and jumps. Here is the [reference](references/RV32I.pdf#page=4).

### Simulator Implementation

The Immediate Generator is implemented as a part of the alu module rather than embedding immediate decoding inside the instruction decoder. This separation improves readability, simplifies debugging, and allows each instruction format to be tested independently.

---

## 6.3.5 Control Unit

### Purpose

The Control Unit interprets every instruction and generates the control signals required by each stage in the pipeline.

### Architectural Operation

The control logic determines:

- ALU operation
- Register write enable
- Memory read/write
- Branch control
- Multiplexer selections
- Write-back source

### Simulator Implementation

Instruction decoding is centralized inside a dedicated control module. Separating control generation from datapath logic reduces coupling between hardware components and makes the simulator easier to extend.

The Control Unit generates control signals with the signals relevant to each of the subsequent stages grouped together. 

See **Chapter 7 - Pipeline** for details on control signals.

---

## 6.3.6 Arithmetic Logic Unit (ALU)

### Purpose

The ALU performs arithmetic, logical, comparison, and address-generation operations.

### Architectural Operation

Supported operations include:

- Addition
- Subtraction
- Logical operations
- Comparisons
- Shift operations
- Effective address calculation

### Simulator Implementation

The ALU is implemented as a modular execution unit whose behavior is selected using the decoded control signals. This design allows additional operations to be incorporated with minimal changes to the surrounding datapath.

---

## 6.3.7 Pipeline Registers

### Purpose

Pipeline registers separate adjacent pipeline stages and preserve intermediate results between processor cycles.

### Architectural Operation

The processor contains the following pipeline registers:

- IF/ID
- ID/EX
- EX/MEM
- MEM/WB

Each register stores both data values and the corresponding control signals required by subsequent stages.

### Simulator Implementation

Each pipeline register is implemented as an independent structure containing all state required for the following stage. This organization closely matches hardware while making the internal pipeline state easy to inspect during simulation.

However, there are two extra registers wb_if and wb_recent which serve the purpose of visualzation and forwarding.

See **Chapter 7 - Pipeline** for more details.

---

## 6.3.8 Data Memory

### Purpose

Data memory stores program variables and supports all load and store operations.

### Architectural Operation

The processor accesses data memory through:

- Load Byte
- Load Byte Unsigned
- Load Halfword
- Load Halfword Unsigned
- Load Word
- Store Byte
- Store Halfword
- Store Word

Only load and store instructions access data memory, consistent with the load-store architecture defined by RV32I.

### Simulator Implementation

The simulator routes every memory request through the configurable cache hierarchy which determines the number of stalls required, before reaching main memory.
See **Chapter 10 - Cache Hierarchy** for more details.

---

## 6.3.9 Forwarding Unit

### Purpose

The Forwarding Unit reduces pipeline stalls by supplying recently computed values directly to dependent instructions.

### Simulator Implementation

Forwarding logic is implemented independently of the ALU so that operand selection and forwarding decisions remain clearly separated from arithmetic execution.

> **Forwarding logic is discussed in detail in Chapter 8.**

---

## 6.3.10 Branch Prediction Unit

### Purpose

The Branch Prediction Unit attempts to predict the outcome of branch instructions before they are resolved.

### Simulator Implementation

The simulator provides a configurable branch prediction module that operates independently of the instruction fetch stage. This modular design allows multiple prediction strategies to be implemented without modifying the remainder of the processor.

See **Chapter 9 - Branch Prediction** for more details.

---

## 6.3.11 Cache Interface

### Purpose

The processor communicates with the memory hierarchy through a dedicated cache interface responsible for instruction and data accesses.

### Simulator Implementation

Rather than directly accessing main memory, every instruction fetch and data access passes through the configurable cache hierarchy. The cache system is only responsible for the timings of the memory accesses. It is seperate from the pipeline and thus responsible only for the statistical acuracy of the system and not its computational accuracy. This modular design allows multiple cache policies to be implemented without modifying the remainder of the processor.

See **Chapter 10 - Cache Hierarchy** for more details.

---

# 6.4 Instruction Flow

The execution of every instruction follows the same high-level path through the processor.

For example, an arithmetic instruction proceeds through the following sequence:

```text
Program Counter
        ↓
Instruction Memory
        ↓
Instruction Decode
        ↓
Register File
        ↓
Immediate Generation
        ↓
ALU
        ↓
Write Back
```

For example, a load instruction proceeds through the following sequence:

```text
Program Counter
        ↓
Instruction Memory
        ↓
Instruction Decode
        ↓
Register File
        ↓
Immediate Generation
        ↓
ALU (address generation)
        ↓
Memory Read
        ↓
Write Back
```

During execution, control signals generated by the Control Unit travel alongside the instruction through the pipeline, ensuring that each stage performs the correct operation.

Although every instruction follows this general flow, different instruction classes utilize different subsets of the datapath. For example, arithmetic instructions bypass the data memory, whereas load and store instructions interact with the memory hierarchy.

See **Chapter 7 - Pipeline** for a more detailed explanation.

---

# 6.5 Design Decisions

Although the simulated processor closely follows the RV32I architecture, several implementation choices were made to improve modularity, extensibility, and educational value.


### Why uint32_t for all registers ?

> i) RV32I defines an XLEN of 32 bits.

> ii) This representation naturally models the modulo - 2<sup>32</sup> arithmetic performed by the hardware. \
Eg: 0xFFFFFFFF + 0x1 = 0x0

> iii) This representation also makes the behaviour of bitwise operations independent of the data. In essence, there is no sign extension and so on a right shift the left bits are filled with zeroes. This results in less error prone code.

In implementation, only a small subset of instructions like SLT, BLT, etc. interpret the values as signed. This can easily be done using type casting.

### Modular Components

> Major hardware blocks are implemented as independent modules rather than tightly coupling functionality. This organization simplifies testing, debugging, and future extensions.

### Separated Control Logic

> Instruction decoding and control generation are isolated from the datapath, reducing implementation complexity while improving readability.

### Independent Forwarding and Branch Prediction Units

> Forwarding and Brach Prediction are implemented as standalone modules to allow them to be upgraded independently in the future.

### Configurable Memory Hierarchy

> The processor interfaces with a configurable cache hierarchy rather than directly accessing main memory, allowing realistic evaluation of different cache organizations.

### Educational Visualization

> Several implementation decisions prioritize visibility of processor state over absolute execution speed, making the simulator suitable for architectural exploration and debugging.

---

# 6.6 Summary

This chapter introduced the overall architecture of the simulated RV32I processor and described the major hardware components that comprise the datapath. While this chapter focused on the structural organization of the processor, the following chapters examine the dynamic behavior of the pipeline, hazard handling mechanisms, branch prediction, cache hierarchy, and other architectural features in greater detail.
