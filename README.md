# CS-204 
## COURSE - COMPUTER ARCHITECTURE

## GROUP NO. 23

## PROJECT : RISC-V ASSEMBLER

## GROUP MEMBERS : 
- Raman (2023CSB1152)
- Divyansh Barodiya (2023CSB1119)
- Aditya Kumar (2023CSB1096)


# RISC-V Assembler

This is a C++ implementation of a RISC-V assembler that translates RISC-V assembly code into machine code. The assembler supports various RISC-V instruction formats and data directives.

## Features

- Supports multiple RISC-V instruction formats:
  - R-type (register-register) instructions
  - I-type (immediate) instructions
  - S-type (store) instructions
  - B-type (branch) instructions
  - U-type (upper immediate) instructions
  - J-type (jump) instructions
- Handles common RISC-V instructions including arithmetic, logical, control flow, and memory operations
- Processes data directives (.byte, .word, .asciiz, .half, .dword)
- Supports labels and symbolic references
- Manages both architectural register names (x0-x31) and ABI register names (zero, ra, sp, etc.)
- Generates a detailed output file with:
  - Encoded instructions in hexadecimal
  - Original assembly instructions
  - Decoded instruction fields for verification
  - Data segment contents

## Input Format

The assembler expects an input file named `input.asm` with the following structure:

```
.data
# Data directives here
label1: .word 10, 20, 30
str: .asciiz "Hello, world!"

.text
# Assembly instructions here
main:
    addi x1, x0, 5
    jal x1, label
```

## Supported Instructions

### R-Type Instructions
- Arithmetic: `add`, `sub`, `mul`, `div`, `rem`
- Logical: `and`, `or`, `xor`
- Shifts: `sll`, `srl`, `sra`
- Comparison: `slt`

### I-Type Instructions
- Immediate arithmetic: `addi`, `andi`, `ori`
- Loads: `lb`, `lh`, `lw`, `ld`
- Jump and link register: `jalr`

### S-Type Instructions
- Stores: `sb`, `sh`, `sw`, `sd`

### B-Type Instructions
- Branches: `beq`, `bne`, `blt`, `bge`

### U-Type Instructions
- Upper immediate: `lui`, `auipc`

### J-Type Instructions
- Jump and link: `jal`

## Data Directives

- `.byte`: 8-bit values
- `.half`: 16-bit values
- `.word`: 32-bit values
- `.dword`: 64-bit values
- `.asciiz`: Null-terminated strings

## Output Format

The assembler creates an output file named `output.mc` containing:

1. Machine code section:
   ```
   0x[address] , 0x[machine code] [assembly instruction] # [decoded fields]
   ```

2. Data segment section:
   ```
   0x[address]   [byte0] [byte1] [byte2] [byte3]
   ```

## Memory Layout

- Instructions start from address 0
- Data segment begins at address 2^28 (MemoryStart)

## Usage

1. Create an `input.asm` file with RISC-V assembly code
2. Compile the assembler: `g++ -o assembler main.cpp`
3. Run the assembler: `./assembler`
4. Check the output in `output.mc`

## Error Handling

The assembler performs basic validation and reports errors for issues like:
- Unknown instructions
- Invalid register names
- Immediate values that exceed size limitations
- Unaligned branch/jump targets

## Implementation Details

The assembler performs a two-pass process:
1. First pass: collects all labels and their addresses
2. Second pass: processes and encodes instructions using the label information

Each instruction type has its own dedicated processing function that handles the specific encoding requirements for that format.



# Phase 3: RISC-V Pipelined Simulator

## Overview
This phase implements a pipelined RISC-V processor simulator with hazard detection, data forwarding, and branch prediction capabilities. The simulator includes both pipelined and non-pipelined execution modes.

## Features

### Pipeline Implementation
- Five-stage pipeline: IF (Instruction Fetch), ID (Instruction Decode), EX (Execute), MEM (Memory), WB (Writeback)
- Pipeline register storage between stages
- Dynamic branch prediction with Branch Target Buffer (BTB)
- Data hazard detection and resolution
- Support for control hazards with pipeline flushing

### Hazard Handling
- Data Hazards (RAW, WAR, WAW)
- Control Hazards (Branch/Jump instructions)
- Structural Hazards
- Data Forwarding support
- Load-use hazard detection
- Branch misprediction recovery

### Memory System
- Instruction memory
- Data memory
- Register file (32 registers)
- Stack memory implementation

### Performance Monitoring
- Cycle-accurate simulation
- Pipeline stall tracking
- Branch prediction statistics
- Instruction execution counts
- Hazard occurrence tracking

## Files Structure
- `pipelined.cpp/h`: Main pipeline implementation
- `hazards.cpp/h`: Hazard detection and handling
- `stats.cpp/h`: Performance statistics tracking
- `globals.cpp/h`: Global variables and constants
- `structs.cpp/h`: Data structures for pipeline stages
- `stack.cpp/h`: Stack memory implementation
- `utils.cpp/h`: Utility functions
- `nonPipelined.cpp/h`: Non-pipelined execution mode

## Usage

### Compilation
```bash
g++ -o simulator *.cpp
```

### Running
```bash
./simulator
```

### Input Format
The simulator accepts machine code in hexadecimal format:
```
0x00500113    # addi x2, x0, 5
0x00310233    # add x4, x2, x3
```

## Configuration Options
- Enable/disable pipelining
- Enable/disable data forwarding
- Enable/disable branch prediction
- Configure performance monitoring options

## Statistics Output
The simulator provides detailed statistics including:
- Total cycles executed
- Number of instructions completed
- Pipeline stalls count
- Branch prediction accuracy
- Data hazard occurrences
- Control hazard occurrences

## Example Output
```
======= Cycle 10 =======
IF Stage: 0x00500113
ID Stage: 0x00310233
EX Stage: Stalled
MEM Stage: 0x00128293
WB Stage: 0x00100193
```

# 🚀 CS204 Phase 3: GUI Implementation 

This repository contains both **Python** (with GUI) and **C++** implementations of a five-stage pipelined 32-bit RISC-V processor simulator, developed as part of **Phase 3** of the CS204 - Computer Architecture course project.

---

## 📜 Objective

To simulate a **RISC-V pipelined processor** with:
- Support for all standard RISC-V 32-bit instructions
- Pipeline registers between stages (IF, ID, EX, MEM, WB)
- Data hazard resolution via **stalling** and **forwarding**
- Control hazard resolution using **1-bit dynamic branch prediction**
- Separate **text and data segments**
- Multiple debug knobs for tracing and analysis
- Final execution statistics and memory dump
- **Bonus GUI** (Python/Streamlit)

---

## 📁 File Structure

### Python (GUI version)
| File          | Description |
|---------------|-------------|
| `app.py`      | Streamlit GUI frontend with control knobs and visualization |
| `memory.py`   | Memory module handling text and data memory |
| `pipeline.py` | Pipeline implementation with registers, forwarding, stalling |
| `processor.py`| Processor controller managing stats, knobs, and cycles |

### C++ (CLI version)
| File             | Description |
|------------------|-------------|
| `pipelined.cpp`  | Core pipelined simulator logic and stage implementations |
| `hazards.cpp`    | Hazard detection, data forwarding, flushing logic |
| `structs.cpp`    | Branch predictor and related structures |
| `utils.cpp`      | Utility functions for hex/bin conversion and memory loading |

---

## 🕹️ Execution Knobs

The simulator supports the following **debugging and tracing knobs**:

| Knob # | Description |
|--------|-------------|
| Knob1  | Enable/Disable pipelining |
| Knob2  | Enable/Disable data forwarding |
| Knob3  | Print full register file each cycle |
| Knob4  | Print pipeline register contents per cycle |
| Knob5  | Trace pipeline stages for a specific instruction |
| Knob6  | Print Branch Prediction Unit (BTB & PHT) status |

---

## 📊 Output Statistics

At the end of the simulation, the following stats are generated and saved in `pipeline_stats.txt`:

- Total clock cycles
- Total instructions executed
- CPI (Cycles Per Instruction)
- Number of:
  - Data-transfer instructions (load/store)
  - ALU instructions
  - Control instructions
  - Pipeline stalls
  - Data hazards
  - Control hazards
  - Branch mispredictions
  - Stalls due to data hazards
  - Stalls due to control hazards

---

