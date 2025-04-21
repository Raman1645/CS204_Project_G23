#include <bits/stdc++.h>
#include "globals.h"
#include "structs.h"
#include "hazards.h"
#include "pipelined.h"
#include "stats.h"
#include "utils.h"

using namespace std;

// Pipeline control flags
bool stall_fetch = false;
bool stall_decode = false;
bool stall_execute = false;
bool stall_memory = false;
bool stall_writeback = false;

bool flush_fetch = false;
bool flush_decode = false;
bool flush_execute = false;
bool flush_memory = false;

// Forward declaration for pipeline flush functionality
void flushPipeline(int fromStage);

// Main function to identify and resolve hazards in the pipeline
void detectAndHandleHazards()
{
    // Reset all control flags at the beginning of each cycle
    stall_fetch = stall_decode = stall_execute = stall_memory = stall_writeback = false;
    flush_fetch = flush_decode = flush_execute = flush_memory = false;

    // First check for data dependencies between instructions
    bool dataHazardDetected = detectDataHazard();

    // Handle data hazards differently based on forwarding configuration
    if (dataHazardDetected)
    {
        if (knob_data_forwarding)
        {
            cout << "Data hazard detected but handling with forwarding" << endl;
            handleDataForwarding();

            // Special case: Load-use hazard
            // We can't forward from memory until the MEM stage completes
            if (ex_mem.decodedInst.type == "Load_I-Type" &&
                id_ex.decodedInst.type != "" &&
                (id_ex.decodedInst.rs1 == ex_mem.decodedInst.rd ||
                 id_ex.decodedInst.rs2 == ex_mem.decodedInst.rd))
            {
                cout << "Load-use hazard detected, must stall even with forwarding enabled" << endl;
                insertStall(2); // Stall at ID stage
                data_hazards++;
            }
        }
        else
        {
            // Without forwarding, we need to stall the pipeline
            cout << "Data hazard detected and forwarding disabled, inserting stall" << endl;
            insertStall(2); // Stall at ID stage
            data_hazards++;
        }
    }

    // Now check for control flow hazards
    bool controlHazardDetected = detectControlHazard();

    // Update hazard statistics
    if (controlHazardDetected)
        control_hazards++;
}

// Detect Read-After-Write (RAW) data hazards in the pipeline
bool detectDataHazard()
{
    // Nothing to check if the decode stage is empty
    if (if_id.instruction.empty())
    {
        return false;
    }

    // Extract register dependencies from the binary instruction
    if (!if_id.instruction.empty())
    {
        string binInst = hex2bin(if_id.instruction);
        int opcode = stoi(binInst.substr(25, 7), nullptr, 2);

        // Determine which source registers are used by this instruction
        int rs1 = -1, rs2 = -1;

        // Most instructions use rs1 except LUI, AUIPC, JAL
        if (opcode != 0b0110111 && opcode != 0b0010111 && opcode != 0b1101111)
        { 
            rs1 = stoi(binInst.substr(12, 5), nullptr, 2);
        }

        // R-type, S-type, and B-type instructions use rs2
        if (opcode == 0b0110011 || opcode == 0b0100011 || opcode == 0b1100011)
        {
            rs2 = stoi(binInst.substr(7, 5), nullptr, 2);
        }

        // Register x0 is hardwired to zero, so no hazard possible
        if (rs1 == 0 && rs2 == 0)
        {
            return false;
        }

        // Check for hazards with the instruction in EX stage
        if (id_ex.decodedInst.type != "" && id_ex.decodedInst.rd != 0)
        {
            if ((rs1 != -1 && rs1 == id_ex.decodedInst.rd) ||
                (rs2 != -1 && rs2 == id_ex.decodedInst.rd))
            {
                cout << "RAW hazard detected: instruction in ID needs register ";
                if (rs1 != -1 && rs1 == id_ex.decodedInst.rd)
                    cout << "r" << rs1;
                if (rs2 != -1 && rs2 == id_ex.decodedInst.rd)
                    cout << "r" << rs2;
                cout << " being written by instruction in EX" << endl;
                return true;
            }
        }

        // Check for hazards with the instruction in MEM stage
        if (ex_mem.decodedInst.type != "" && ex_mem.decodedInst.rd != 0)
        {
            if ((rs1 != -1 && rs1 == ex_mem.decodedInst.rd) ||
                (rs2 != -1 && rs2 == ex_mem.decodedInst.rd))
            {
                cout << "RAW hazard detected: instruction in ID needs register ";
                if (rs1 != -1 && rs1 == ex_mem.decodedInst.rd)
                    cout << "r" << rs1;
                if (rs2 != -1 && rs2 == ex_mem.decodedInst.rd)
                    cout << "r" << rs2;
                cout << " being written by instruction in MEM" << endl;
                return true;
            }
        }

        // Check for hazards with the instruction in WB stage
        // This is usually resolved by forwarding but included for completeness
        if (mem_wb.decodedInst.type != "" && mem_wb.decodedInst.rd != 0)
        {
            if ((rs1 != -1 && rs1 == mem_wb.decodedInst.rd) ||
                (rs2 != -1 && rs2 == mem_wb.decodedInst.rd))
            {
                cout << "RAW hazard detected: instruction in ID needs register ";
                if (rs1 != -1 && rs1 == mem_wb.decodedInst.rd)
                    cout << "r" << rs1;
                if (rs2 != -1 && rs2 == mem_wb.decodedInst.rd)
                    cout << "r" << rs2;
                cout << " being written by instruction in WB" << endl;
                return true;
            }
        }
    }
    return false;
}

// Detect and handle control flow hazards from branches and jumps
bool detectControlHazard()
{
    // Check if the execute stage contains a branch or jump instruction
    if (id_ex.decodedInst.type == "SB-Type" ||
        id_ex.decodedInst.type == "JAL_J-Type" ||
        id_ex.decodedInst.type == "JALR_I-Type")
    {
        // Control hazard found
        return true;
    }

    // Verify branch prediction accuracy (checked in MEM stage)
    if (ex_mem.decodedInst.type == "SB-Type")
    {
        bool actualBranchTaken = ex_mem.branchTaken;
        string predictedPC = predicted_pc;

        if (predicted_pc == ex_mem.pc)
        {
            // This branch was predicted - check if correctly
            if ((predicted_branch && !actualBranchTaken) ||
                (!predicted_branch && actualBranchTaken))
            {
                // Branch was mispredicted
                branch_mispredictions++;

                // Recover by flushing pipeline and redirecting to correct path
                if (actualBranchTaken)
                {
                    currentPC = ex_mem.branchTarget;
                }
                else
                {
                    // Branch not taken, go to next sequential instruction
                    unsigned int pc_val = stoul(ex_mem.pc.substr(2), nullptr, 16);
                    pc_val += 4;
                    stringstream ss;
                    ss << hex << pc_val;
                    currentPC = "0x" + ss.str();
                }

                // Clear instructions from wrong path
                flushPipeline(2); // Flush IF through EX stages

                return true;
            }
        }
    }

    return false;
}

// Implement register value forwarding to resolve data hazards
void handleDataForwarding()
{
    // Only process if there's a valid instruction in EX stage
    if (id_ex.decodedInst.type != "")
    {
        int rs1 = id_ex.decodedInst.rs1;
        int rs2 = id_ex.decodedInst.rs2;

        // Check if values can be forwarded from MEM stage
        if (ex_mem.decodedInst.type != "" && ex_mem.decodedInst.rd != 0)
        {
            if (rs1 == ex_mem.decodedInst.rd)
            {
                // Forward MEM result to first ALU input
                id_ex.rs1_value = ex_mem.aluResult;
                cout << "Forwarding EX/MEM result to RS1 in EX stage" << endl;
            }
            if (rs2 == ex_mem.decodedInst.rd)
            {
                // Forward MEM result to second ALU input
                id_ex.rs2_value = ex_mem.aluResult;
                cout << "Forwarding EX/MEM result to RS2 in EX stage" << endl;
            }
        }

        // Check if values can be forwarded from WB stage
        // Lower priority than MEM forwarding
        if (mem_wb.decodedInst.type != "" && mem_wb.decodedInst.rd != 0)
        {
            // Only forward from WB if no forwarding from MEM for this register
            if (rs1 == mem_wb.decodedInst.rd &&
                !(ex_mem.decodedInst.type != "" && ex_mem.decodedInst.rd == rs1))
            {
                id_ex.rs1_value = mem_wb.writebackData;
                cout << "Forwarding MEM/WB result to RS1 in EX stage" << endl;
            }
            if (rs2 == mem_wb.decodedInst.rd &&
                !(ex_mem.decodedInst.type != "" && ex_mem.decodedInst.rd == rs2))
            {
                id_ex.rs2_value = mem_wb.writebackData;
                cout << "Forwarding MEM/WB result to RS2 in EX stage" << endl;
            }
        }
    }
}

// Insert a pipeline stall at the specified stage
void insertStall(int stageNum)
{
    // Apply appropriate stalls based on which stage needs to be frozen
    switch (stageNum)
    {
    case 1: // Stall at Fetch
        stall_fetch = true;
        pipeline_stalls++;
        cout << "Inserting stall at Fetch stage" << endl;
        break;
    case 2: // Stall at Decode
        stall_fetch = true;
        stall_decode = true;
        // Create a bubble (NOP) in the execute stage
        id_ex.decodedInst.type = "";
        id_ex.decodedInst.name = "NOP";
        pipeline_stalls++;
        cout << "Inserting stall at Decode stage, bubbling the pipeline" << endl;
        break;
    case 3: // Stall at Execute
        stall_fetch = true;
        stall_decode = true;
        stall_execute = true;
        // Create a bubble in the memory stage
        ex_mem.decodedInst.type = "";
        ex_mem.decodedInst.name = "NOP";
        pipeline_stalls++;
        cout << "Inserting stall at Execute stage, bubbling the pipeline" << endl;
        break;
    case 4: // Stall at Memory
        stall_fetch = true;
        stall_decode = true;
        stall_execute = true;
        stall_memory = true;
        // Create a bubble in the writeback stage
        mem_wb.decodedInst.type = "";
        mem_wb.decodedInst.name = "NOP";
        pipeline_stalls++;
        cout << "Inserting stall at Memory stage, bubbling the pipeline" << endl;
        break;
    }
}

// Flush pipeline stages to clear incorrect speculative execution
void flushPipeline(int fromStage)
{
    // Apply flushes to clear the appropriate stages
    switch (fromStage)
    {
    case 1: // Flush from Fetch
        flush_fetch = true;
        if_id = IF_ID_Register(); // Reset IF/ID register
        break;
    case 2: // Flush from Decode
        flush_fetch = true;
        flush_decode = true;
        if_id = IF_ID_Register(); // Reset IF/ID register
        id_ex = ID_EX_Register(); // Reset ID/EX register
        break;
    case 3: // Flush from Execute
        flush_fetch = true;
        flush_decode = true;
        flush_execute = true;
        if_id = IF_ID_Register(); // Reset IF/ID register
        id_ex = ID_EX_Register(); // Reset ID/EX register
        ex_mem = EX_MEM_Register(); // Reset EX/MEM register
        break;
    }
}

// Check if a value can be forwarded for a specific register
// Returns true if forwarding is possible and updates forwardedValue
bool checkForwardingPath(int srcReg, int *forwardedValue)
{
    // Register x0 is hardwired to 0
    if (srcReg == 0)
    {
        *forwardedValue = 0;
        return true;
    }

    // First priority: Forward from MEM stage
    if (ex_mem.decodedInst.type != "" && ex_mem.decodedInst.rd != 0 && ex_mem.decodedInst.rd == srcReg)
    {
        *forwardedValue = ex_mem.aluResult;
        return true;
    }

    // Second priority: Forward from WB stage
    if (mem_wb.decodedInst.type != "" && mem_wb.decodedInst.rd != 0 && mem_wb.decodedInst.rd == srcReg)
    {
        *forwardedValue = mem_wb.writebackData;
        return true;
    }

    // No forwarding path available
    return false;
}
