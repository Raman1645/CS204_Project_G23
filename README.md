# CS204 
# COMPUTER ARCHITECTURE

# GROUP PROJECT : RISC-V ASSEMBLER

# GROUP MEMBERS : 
Raman (2023CSB1152)
Divyansh Barodiya (2023CSB1119)
Aditya Kumar (2023CSB1096)


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
