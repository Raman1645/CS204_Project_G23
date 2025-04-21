#ifndef PIPELINED_H
#define PIPELINED_H

#include <string>
#include <map>
#include "structs.h"

// Main simulation function
void runPipelinedSimulation();

// Pipeline stage functions
void pipelineIF();
void pipelineID();
void pipelineEX();
void pipelineMEM();
void pipelineWB();

// Debug and visualization functions
void printPipelineRegisters();
void printBranchPredictorState();
void traceSpecificInstruction(int instructionNumber);


#endif // PIPELINED_H
