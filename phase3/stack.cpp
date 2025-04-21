#include <bits/stdc++.h>
#include "globals.h"
#include "structs.h"
using namespace std;

// Sets up initial stack and register states
void initializeStack()
{
    // Set stack pointer (x2 in RISC-V)
    registerFile[2] = stackBaseAddress;

    // Set frame pointer (x8 in RISC-V convention)
    registerFile[8] = stackBaseAddress;

    cout << "Stack initialized: SP=0x" << hex << registerFile[2] << ", FP=0x" << registerFile[8] << dec << endl;
}

// Adds a value to the stack
void pushToStack(int value)
{
    // Move stack pointer down (stack grows downward)
    stackPointer -= 4;

    // Write value to memory at the stack location
    for (int i = 0; i < 4; i++)
    {
        dataMemory[stackPointer + i] = (value >> (8 * i)) & 0xFF;
    }

    // Update SP register value
    registerFile[2] = stackPointer;

    cout << "Pushed value " << value << " to stack at address 0x" << hex << stackPointer << dec << endl;
}

// Retrieves and removes a value from the stack
int popFromStack()
{
    // Extract value from current stack position
    int value = 0;
    for (int i = 0; i < 4; i++)
    {
        value |= (dataMemory[stackPointer + i] << (8 * i));
    }

    // Move stack pointer up
    stackPointer += 4;

    // Update SP register value
    registerFile[2] = stackPointer;

    cout << "Popped value " << value << " from stack at address 0x" << hex << stackPointer - 4 << dec << endl;

    return value;
}

// Reserves memory on the stack for local variables
void allocateStackSpace(unsigned int size)
{
    // Word-align the size (multiple of 4 bytes)
    size = (size + 3) & ~3;

    // Move stack pointer down to allocate space
    stackPointer -= size;

    // Update SP register
    registerFile[2] = stackPointer;

    cout << "Allocated " << size << " bytes on stack. New SP=0x" << hex << stackPointer << dec << endl;
}

// Releases previously allocated stack memory
void freeStackSpace(unsigned int size)
{
    // Word-align the size (multiple of 4 bytes)
    size = (size + 3) & ~3;

    // Move stack pointer up to free space
    stackPointer += size;

    // Update SP register
    registerFile[2] = stackPointer;

    cout << "Freed " << size << " bytes from stack. New SP=0x" << hex << stackPointer << dec << endl;
}

// Sets up a new function's stack frame
void createStackFrame(unsigned int frameSize)
{
    // Store current frame pointer on stack
    pushToStack(framePointer);

    // Update frame pointer to current stack position
    framePointer = stackPointer;
    registerFile[8] = framePointer;

    // Make room for local variables
    allocateStackSpace(frameSize);

    cout << "Created new stack frame with size " << frameSize << " bytes. FP=0x" << hex << framePointer << dec << endl;
}

// Cleans up the current function's stack frame
void destroyStackFrame()
{
    // Restore stack pointer to current frame base
    stackPointer = framePointer;
    registerFile[2] = stackPointer;

    // Retrieve previous frame pointer
    framePointer = popFromStack();
    registerFile[8] = framePointer;

    cout << "Destroyed stack frame. Restored FP=0x" << hex << framePointer << dec << endl;
}

// Handles stack-related processor instructions
void executeStackOperations(Instruction instruction)
{
    if (instruction.name == "ADDI" && instruction.rd == 2 && instruction.rs1 == 2)
    {
        // Stack pointer adjustment instruction
        if (instruction.imm < 0)
        {
            // Negative immediate means allocate space
            allocateStackSpace(-instruction.imm);
        }
        else
        {
            // Positive immediate means free space
            freeStackSpace(instruction.imm);
        }
    }
    else if ((instruction.name == "SD" || instruction.name == "SW") && instruction.rs1 == 2)
    {
        // Store to stack operation
        unsigned int address = registerFile[2] + instruction.imm;
        cout << "Stack store: Writing register R" << instruction.rs2 << " to stack at offset "
             << instruction.imm << " from SP" << endl;
    }
    else if ((instruction.name == "LD" || instruction.name == "LW") && instruction.rs1 == 2)
    {
        // Load from stack operation
        unsigned int address = registerFile[2] + instruction.imm;
        cout << "Stack load: Reading from stack at offset " << instruction.imm << " from SP into R"
             << instruction.rd << endl;
    }
}
