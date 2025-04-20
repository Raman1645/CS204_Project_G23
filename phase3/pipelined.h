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

// Helper function for instruction execution
// void executeInPipeline();

// Hazard detection and handling
// void detectAndHandleHazards();
// void detectStructuralHazard();

// Data forwarding logic
// bool checkForwardingPath(int srcReg, int* forwardedValue);

#endif // PIPELINED_H