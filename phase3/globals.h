// globals.h
#ifndef GLOBALS_H
#define GLOBALS_H

#include <string>
#include <map>
#include <unordered_map>
#include "structs.h"

// Program counter and instruction tracking
extern std::map<std::string, std::string> pcMachineCode;
extern std::string currentPC;
extern long long int result;
extern std::string currentInstruction;
extern int registerFile[32];
extern bool infLoop;

// Memory management
extern std::unordered_map<unsigned int, unsigned char> dataMemory;
extern unsigned int memoryBaseAddress;
extern unsigned int stackBaseAddress;
extern unsigned int stackPointer;
extern unsigned int framePointer;

// Simulator control flags
extern bool exitSimulator;
extern bool knob_pipelining;
extern bool knob_data_forwarding;
extern bool knob_print_registers;
extern bool knob_print_pipeline_registers;
extern int knob_trace_instruction;
extern bool knob_print_branch_predictor;

// Performance metrics
extern int total_cycles;
extern int total_instructions;
extern int data_transfer_instructions;
extern int alu_instructions;
extern int control_instructions;
extern int pipeline_stalls;
extern int data_hazards;
extern int control_hazards;
extern int branch_mispredictions;
extern int stalls_data_hazards;
extern int stalls_control_hazards;

// Pipeline components
extern Instruction instruction;
extern IF_ID_Register if_id;
extern ID_EX_Register id_ex;
extern EX_MEM_Register ex_mem;
extern MEM_WB_Register mem_wb;
extern BranchPredictor branchPredictor;

// Memory model and branch prediction
extern std::unordered_map<int, int> memory;
extern bool predicted_branch;
extern std::string predicted_pc;

// File paths
extern std::string input_file;
extern std::string output_file;
extern std::string stats_file;

#endif // GLOBALS_H
