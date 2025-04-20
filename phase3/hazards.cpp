#include <bits/stdc++.h>
#include "globals.h"
#include "structs.h"
#include "hazards.h"
#include "pipelined.h"
#include "stats.h"
#include "utils.h"

using namespace std;

// Pipeline stall and flush flags
bool stall_fetch = false;
bool stall_decode = false;
bool stall_execute = false;
bool stall_memory = false;
bool stall_writeback = false;

bool flush_fetch = false;
bool flush_decode = false;
bool flush_execute = false;
bool flush_memory = false;
void flushPipeline(int fromStage);
// Main hazard detection function
// Main hazard detection function
void detectAndHandleHazards()
{
    // Reset stall and flush flags
    stall_fetch = stall_decode = stall_execute = stall_memory = stall_writeback = false;
    flush_fetch = flush_decode = flush_execute = flush_memory = false;

    // Detect data hazards first
    bool dataHazardDetected = detectDataHazard();

    // If data forwarding is enabled, try to handle without stalling
    if (dataHazardDetected)
    {
        if (knob_data_forwarding)
        {
            cout << "Data hazard detected but handling with forwarding" << endl;
            handleDataForwarding();

            // Check if we still need to stall for load-use hazards
            if (ex_mem.decodedInst.type == "Load_I-Type" &&
                id_ex.decodedInst.type != "" &&
                (id_ex.decodedInst.rs1 == ex_mem.decodedInst.rd ||
                 id_ex.decodedInst.rs2 == ex_mem.decodedInst.rd))
            {
                // Can't forward from memory until MEM stage is complete
                cout << "Load-use hazard detected, must stall even with forwarding enabled" << endl;
                insertStall(2); // Stall ID stage
                data_hazards++;
            }
        }
        else
        {
            // Data forwarding disabled, must stall
            cout << "Data hazard detected and forwarding disabled, inserting stall" << endl;
            insertStall(2); // Stall ID stage
            data_hazards++;
        }
    }

    // Detect control hazards
    bool controlHazardDetected = detectControlHazard();

    // Update statistics for hazards
    if (controlHazardDetected)
        control_hazards++;
}

// Detect RAW (Read After Write) data hazards
bool detectDataHazard()
{
    // Check if the instruction in ID stage needs a register that is
    // being written to in EX, MEM, or WB stages

    // Only check if there's an instruction in ID stage that uses registers
    if (if_id.instruction.empty())
    {
        return false;
    }

    // Need to check what registers the instruction in ID will need
    // Use the binary instruction to get opcode and registers
    if (!if_id.instruction.empty())
    {
        string binInst = hex2bin(if_id.instruction);
        int opcode = stoi(binInst.substr(25, 7), nullptr, 2);

        // Get source registers based on instruction format
        int rs1 = -1, rs2 = -1;

        // Most instruction formats have rs1 in the same place
        if (opcode != 0b0110111 && opcode != 0b0010111 && opcode != 0b1101111)
        { // Not LUI, AUIPC, JAL
            rs1 = stoi(binInst.substr(12, 5), nullptr, 2);
        }

        // R-type, S-type, and B-type have rs2
        if (opcode == 0b0110011 || opcode == 0b0100011 || opcode == 0b1100011)
        {
            rs2 = stoi(binInst.substr(7, 5), nullptr, 2);
        }

        // Skip if both source registers are 0 (x0 is hardwired to 0)
        if (rs1 == 0 && rs2 == 0)
        {
            return false;
        }

        // Check hazard with EX stage
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

        // Check hazard with MEM stage
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

        // Check hazard with WB stage - normally this can be resolved by forwarding
        // but including for completeness
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

// Detect control hazards (branches and jumps)
bool detectControlHazard()
{
    // Implementation of control hazard detection logic

    // Check if EX stage has a branch or jump instruction
    if (id_ex.decodedInst.type == "SB-Type" ||
        id_ex.decodedInst.type == "JAL_J-Type" ||
        id_ex.decodedInst.type == "JALR_I-Type")
    {

        // Control hazard detected
        return true;
    }

    // Check if branch prediction was incorrect (in MEM stage)
    if (ex_mem.decodedInst.type == "SB-Type")
    {
        bool actualBranchTaken = ex_mem.branchTaken;
        string predictedPC = predicted_pc;

        if (predicted_pc == ex_mem.pc)
        {
            // This was a predicted branch, check if prediction was correct
            if ((predicted_branch && !actualBranchTaken) ||
                (!predicted_branch && actualBranchTaken))
            {
                // Misprediction detected
                branch_mispredictions++;

                // Flush pipeline and restart from correct target
                if (actualBranchTaken)
                {
                    currentPC = ex_mem.branchTarget;
                }
                else
                {
                    // Convert PC to int, add 4, convert back to hex string
                    unsigned int pc_val = stoul(ex_mem.pc.substr(2), nullptr, 16);
                    pc_val += 4;
                    stringstream ss;
                    ss << hex << pc_val;
                    currentPC = "0x" + ss.str();
                }

                // Flush wrong path instructions
                flushPipeline(2); // Flush from IF to EX stages

                return true;
            }
        }
    }

    return false;
}

// Handle data forwarding when enabled
void handleDataForwarding()
{
    // Implementation of data forwarding logic

    // Example: Forward from MEM to EX stage if needed
    if (id_ex.decodedInst.type != "")
    {
        int rs1 = id_ex.decodedInst.rs1;
        int rs2 = id_ex.decodedInst.rs2;

        // Forward from EX/MEM pipeline register if needed
        if (ex_mem.decodedInst.type != "" && ex_mem.decodedInst.rd != 0)
        {
            if (rs1 == ex_mem.decodedInst.rd)
            {
                // Forward result from EX/MEM to rs1 input of ALU
                id_ex.rs1_value = ex_mem.aluResult;
                cout << "Forwarding EX/MEM result to RS1 in EX stage" << endl;
            }
            if (rs2 == ex_mem.decodedInst.rd)
            {
                // Forward result from EX/MEM to rs2 input of ALU
                id_ex.rs2_value = ex_mem.aluResult;
                cout << "Forwarding EX/MEM result to RS2 in EX stage" << endl;
            }
        }

        // Forward from MEM/WB pipeline register if needed
        if (mem_wb.decodedInst.type != "" && mem_wb.decodedInst.rd != 0)
        {
            if (rs1 == mem_wb.decodedInst.rd &&
                !(ex_mem.decodedInst.type != "" && ex_mem.decodedInst.rd == rs1))
            {
                // Forward result from MEM/WB to rs1 input of ALU
                id_ex.rs1_value = mem_wb.writebackData;
                cout << "Forwarding MEM/WB result to RS1 in EX stage" << endl;
            }
            if (rs2 == mem_wb.decodedInst.rd &&
                !(ex_mem.decodedInst.type != "" && ex_mem.decodedInst.rd == rs2))
            {
                // Forward result from MEM/WB to rs2 input of ALU
                id_ex.rs2_value = mem_wb.writebackData;
                cout << "Forwarding MEM/WB result to RS2 in EX stage" << endl;
            }
        }
    }
}

// Insert a stall in the pipeline
// Insert a stall in the pipeline
void insertStall(int stageNum)
{
    // Stall appropriate stages based on where the stall is needed
    switch (stageNum)
    {
    case 1: // Stall Fetch
        stall_fetch = true;
        pipeline_stalls++;
        cout << "Inserting stall at Fetch stage" << endl;
        break;
    case 2: // Stall Decode
        stall_fetch = true;
        stall_decode = true;
        // Insert a bubble into the execute stage (NOP)
        id_ex.decodedInst.type = "";
        id_ex.decodedInst.name = "NOP";
        pipeline_stalls++;
        cout << "Inserting stall at Decode stage, bubbling the pipeline" << endl;
        break;
    case 3: // Stall Execute
        stall_fetch = true;
        stall_decode = true;
        stall_execute = true;
        // Insert a bubble into the memory stage
        ex_mem.decodedInst.type = "";
        ex_mem.decodedInst.name = "NOP";
        pipeline_stalls++;
        cout << "Inserting stall at Execute stage, bubbling the pipeline" << endl;
        break;
    case 4: // Stall Memory
        stall_fetch = true;
        stall_decode = true;
        stall_execute = true;
        stall_memory = true;
        // Insert a bubble into the writeback stage
        mem_wb.decodedInst.type = "";
        mem_wb.decodedInst.name = "NOP";
        pipeline_stalls++;
        cout << "Inserting stall at Memory stage, bubbling the pipeline" << endl;
        break;
    }
}

// Flush the pipeline from a given stage
void flushPipeline(int fromStage)
{

    // Flush appropriate stages based on where the flush is needed
    switch (fromStage)
    {
    case 1: // Flush Fetch
        flush_fetch = true;
        if_id = IF_ID_Register();
        break;
    case 2: // Flush Decode
        flush_fetch = true;
        flush_decode = true;
        if_id = IF_ID_Register();
        id_ex = ID_EX_Register();
        break;
    case 3: // Flush Execute
        flush_fetch = true;
        flush_decode = true;
        flush_execute = true;
        if_id = IF_ID_Register();
        id_ex = ID_EX_Register();
        ex_mem = EX_MEM_Register();
        break;
    }
}

// Check if a forwarding path exists for a specific register
// Returns true if forwarding is possible and updates forwardedValue
bool checkForwardingPath(int srcReg, int *forwardedValue)
{
    // Skip register 0 which is hardwired to 0
    if (srcReg == 0)
    {
        *forwardedValue = 0;
        return true;
    }

    // Check EX/MEM forwarding path (highest priority)
    if (ex_mem.decodedInst.type != "" && ex_mem.decodedInst.rd != 0 && ex_mem.decodedInst.rd == srcReg)
    {
        // Forward ALU result from EX/MEM
        *forwardedValue = ex_mem.aluResult;
        return true;
    }

    // Check MEM/WB forwarding path (lower priority)
    if (mem_wb.decodedInst.type != "" && mem_wb.decodedInst.rd != 0 && mem_wb.decodedInst.rd == srcReg)
    {
        // Forward writeback value from MEM/WB
        *forwardedValue = mem_wb.writebackData;
        return true;
    }

    // No forwarding path available
    return false;
}