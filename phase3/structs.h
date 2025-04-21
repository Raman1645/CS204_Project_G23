// structs.h
#ifndef STRUCTS_H
#define STRUCTS_H

#include <string>
#include <unordered_map>

// Decoded instruction representation
struct Instruction
{
    std::string type;        // Instruction format type (R-type, I-type, etc.)
    std::string name;        // Mnemonic representation
    int opcode;              // Primary operation code
    int rs1;                 // First source register
    int rs2;                 // Second source register
    int rd;                  // Destination register
    int fun3;                // Function code 3
    int fun7;                // Function code 7
    long long int imm;       // Immediate value
};

// Fetch-Decode pipeline register
struct IF_ID_Register
{
    std::string instruction; // Raw instruction bits
    std::string pc;          // Program counter value
};

// Decode-Execute pipeline register
struct ID_EX_Register
{
    std::string pc;          // Program counter value
    Instruction decodedInst; // Decoded instruction data
    int rs1_value;           // Value read from first source register
    int rs2_value;           // Value read from second source register
    bool isStall;            // Indicates if this stage is stalled
};

// Execute-Memory pipeline register
struct EX_MEM_Register
{
    std::string pc;              // Program counter value
    Instruction decodedInst;     // Decoded instruction data
    long long int aluResult;     // Result from ALU operation
    int rs2_value;               // Value from second source register (for stores)
    std::string branchTarget;    // Target address for branch instructions
    bool branchTaken;            // Whether branch condition was true
    unsigned int returnAddress;  // Return address for jumps
};

// Memory-Writeback pipeline register
struct MEM_WB_Register
{
    std::string pc;              // Program counter value
    Instruction decodedInst;     // Decoded instruction data
    long long int aluResult;     // Result from ALU operation
    int memoryData;              // Data loaded from memory
    long long int writebackData; // Final data to write back to register
};

// Branch prediction unit
struct BranchPredictor
{
    std::unordered_map<std::string, bool> pht;        // Pattern History Table - tracks taken/not taken
    std::unordered_map<std::string, std::string> btb; // Branch Target Buffer - stores target addresses
    int predictions;                                  // Count of total predictions made
    int correct_predictions;                          // Count of accurate predictions

    // Initialize with zero predictions
    BranchPredictor() : predictions(0), correct_predictions(0) {}

    // Predict whether a branch at given PC will be taken
    bool predict(const std::string &pc);
    
    // Get predicted target address for a branch
    std::string getTarget(const std::string &pc);
    
    // Update prediction tables with actual branch outcome
    void update(const std::string &pc, bool taken, const std::string &target);
};

#endif // STRUCTS_H
