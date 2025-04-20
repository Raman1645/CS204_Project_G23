// structs.h
#ifndef STRUCTS_H
#define STRUCTS_H

#include <string>
#include <unordered_map>

struct Instruction
{
    std::string type;
    std::string name;
    int opcode;
    int rs1;
    int rs2;
    int rd;
    int fun3;
    int fun7;
    long long int imm;
};

struct IF_ID_Register
{
    std::string instruction;
    std::string pc;
};

struct ID_EX_Register
{
    std::string pc;
    Instruction decodedInst;
    int rs1_value;
    int rs2_value;
    bool isStall;
};

struct EX_MEM_Register
{
    std::string pc;
    Instruction decodedInst;
    long long int aluResult;
    int rs2_value;
    std::string branchTarget;
    bool branchTaken;
    unsigned int returnAddress;
};

struct MEM_WB_Register
{
    std::string pc;
    Instruction decodedInst;
    long long int aluResult;
    int memoryData;
    long long int writebackData;
};

struct BranchPredictor
{
    std::unordered_map<std::string, bool> pht;        // Pattern History Table
    std::unordered_map<std::string, std::string> btb; // Branch Target Buffer
    int predictions;                                  // Total predictions made
    int correct_predictions;                          // Correct predictions counter

    // Constructor to initialize counters
    BranchPredictor() : predictions(0), correct_predictions(0) {}

    bool predict(const std::string &pc);
    std::string getTarget(const std::string &pc);
    void update(const std::string &pc, bool taken, const std::string &target);
};

#endif // STRUCTS_H