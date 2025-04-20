#include <bits/stdc++.h>
#include "globals.h"
#include "structs.h"
using namespace std;

// Function to initialize stack and register state
void initializeStack()
{
    // Initialize stack pointer (x2 in RISC-V)
    registerFile[2] = stackBaseAddress;

    // Initialize frame pointer (x8 in RISC-V convention)
    registerFile[8] = stackBaseAddress;

    cout << "Stack initialized: SP=0x" << hex << registerFile[2] << ", FP=0x" << registerFile[8] << dec << endl;
}

// Function to push a value onto the stack
void pushToStack(int value)
{
    // Update stack pointer (grows downward)
    stackPointer -= 4;

    // Store the value at the new stack pointer location
    for (int i = 0; i < 4; i++)
    {
        dataMemory[stackPointer + i] = (value >> (8 * i)) & 0xFF;
    }

    // Update SP register
    registerFile[2] = stackPointer;

    cout << "Pushed value " << value << " to stack at address 0x" << hex << stackPointer << dec << endl;
}

// Function to pop a value from the stack
int popFromStack()
{
    // Read the value from the current stack pointer location
    int value = 0;
    for (int i = 0; i < 4; i++)
    {
        value |= (dataMemory[stackPointer + i] << (8 * i));
    }

    // Update stack pointer (grows downward)
    stackPointer += 4;

    // Update SP register
    registerFile[2] = stackPointer;

    cout << "Popped value " << value << " from stack at address 0x" << hex << stackPointer - 4 << dec << endl;

    return value;
}

// Function to allocate space on the stack (for local variables)
void allocateStackSpace(unsigned int size)
{
    // Ensure size is aligned to 4 bytes (word alignment)
    size = (size + 3) & ~3;

    // Update stack pointer
    stackPointer -= size;

    // Update SP register
    registerFile[2] = stackPointer;

    cout << "Allocated " << size << " bytes on stack. New SP=0x" << hex << stackPointer << dec << endl;
}

// Function to free space on the stack
void freeStackSpace(unsigned int size)
{
    // Ensure size is aligned to 4 bytes (word alignment)
    size = (size + 3) & ~3;

    // Update stack pointer
    stackPointer += size;

    // Update SP register
    registerFile[2] = stackPointer;

    cout << "Freed " << size << " bytes from stack. New SP=0x" << hex << stackPointer << dec << endl;
}

// Function to create a new stack frame
void createStackFrame(unsigned int frameSize)
{
    // Save old frame pointer
    pushToStack(framePointer);

    // Set new frame pointer to current stack pointer
    framePointer = stackPointer;
    registerFile[8] = framePointer;

    // Allocate space for local variables
    allocateStackSpace(frameSize);

    cout << "Created new stack frame with size " << frameSize << " bytes. FP=0x" << hex << framePointer << dec << endl;
}

// Function to destroy the current stack frame
void destroyStackFrame()
{
    // Restore stack pointer to frame pointer
    stackPointer = framePointer;
    registerFile[2] = stackPointer;

    // Restore previous frame pointer
    framePointer = popFromStack();
    registerFile[8] = framePointer;

    cout << "Destroyed stack frame. Restored FP=0x" << hex << framePointer << dec << endl;
}

// Modified execute function to handle stack operations
void executeStackOperations(Instruction instruction)
{
    if (instruction.name == "ADDI" && instruction.rd == 2 && instruction.rs1 == 2)
    {
        // Stack pointer adjustment (e.g., addi sp, sp, -16 to allocate space)
        if (instruction.imm < 0)
        {
            // Allocate stack space
            allocateStackSpace(-instruction.imm);
        }
        else
        {
            // Free stack space
            freeStackSpace(instruction.imm);
        }
    }
    else if ((instruction.name == "SD" || instruction.name == "SW") && instruction.rs1 == 2)
    {
        // Store to stack (e.g., sw t0, 4(sp))
        unsigned int address = registerFile[2] + instruction.imm;
        cout << "Stack store: Writing register R" << instruction.rs2 << " to stack at offset "
             << instruction.imm << " from SP" << endl;
    }
    else if ((instruction.name == "LD" || instruction.name == "LW") && instruction.rs1 == 2)
    {
        // Load from stack (e.g., lw t0, 4(sp))
        unsigned int address = registerFile[2] + instruction.imm;
        cout << "Stack load: Reading from stack at offset " << instruction.imm << " from SP into R"
             << instruction.rd << endl;
    }
}