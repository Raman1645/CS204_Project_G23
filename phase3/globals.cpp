#include <bits/stdc++.h>
#include "globals.h"

using namespace std;

// Execution state tracking
map<string, string> pcMachineCode;
string currentPC = "0x0";
long long int result;
string currentInstruction;
int registerFile[32];
bool infLoop = false;

// Memory management variables
unordered_map<unsigned int, unsigned char> dataMemory;
unsigned int memoryBaseAddress = 0x10000000;
unsigned int stackBaseAddress = 0x7FFFFFFC;
unsigned int stackPointer = stackBaseAddress;
unsigned int framePointer = stackBaseAddress;

// Simulator control flags
bool exitSimulator = false;
bool knob_pipelining = true;
bool knob_data_forwarding = true;
bool knob_print_registers = false;
bool knob_print_pipeline_registers = false;
int knob_trace_instruction = -1;
bool knob_print_branch_predictor = false;

// Performance statistics
int total_cycles = 0;
int total_instructions = 0;
int data_transfer_instructions = 0;
int alu_instructions = 0;
int control_instructions = 0;
int pipeline_stalls = 0;
int data_hazards = 0;
int control_hazards = 0;
int branch_mispredictions = 0;
int stalls_data_hazards = 0;
int stalls_control_hazards = 0;

// Pipeline components
Instruction instruction;
IF_ID_Register if_id;
ID_EX_Register id_ex;
EX_MEM_Register ex_mem;
MEM_WB_Register mem_wb;
BranchPredictor branchPredictor;

// Memory model and branch prediction
unordered_map<int, int> memory;
bool predicted_branch;
string predicted_pc;

// File paths
string input_file = "input.mc";
string output_file = "output.mc";
string stats_file = "stats.txt";
