#ifndef NONPIPELINED_H
#define NONPIPELINED_H

#include "structs.h"
#include <string>

// Instruction pipeline stages
void fetchInstruction();
void decodeInstruction();
void execute(Instruction instruction);
void memoryAccess(Instruction instruction);
void writeBack(Instruction instruction);
void updatePC(Instruction instruction);

// Memory and stack dumping functions
void dumpMemoryToFile(const std::string &filename);
void dumpStackToFile(const std::string &filename);

#endif // NONPIPELINED_H