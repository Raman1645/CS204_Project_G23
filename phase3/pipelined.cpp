#include <bits/stdc++.h>
#include "globals.h"
#include "structs.h"
#include "hazards.h"
#include "stats.h"
#include "utils.h"
#include "stack.h"
#include "pipelined.h"
#include "nonPipelined.h"

using namespace std;

// Forward declarations for pipeline stage functions
void pipelineIF();
void pipelineID();
void pipelineEX();
void pipelineMEM();
void pipelineWB();

// Main function to run the pipelined simulation
void runPipelinedSimulation()
{
    // Initialize all registers to zero
    for (int i = 0; i < 32; i++)
    {
        registerFile[i] = 0;
    }

    // Setup the execution environment
    initializeStack();
    initializeStats();
    branchPredictor = BranchPredictor();
    loadMC("input.mc");

    int clockCycle = 0;
    exitSimulator = false;

    cout << "Starting pipelined execution with "
         << (knob_data_forwarding ? "data forwarding enabled" : "data forwarding disabled")
         << endl;

    // Main simulation loop
    while (!exitSimulator)
    {
        cout << "\n================ Clock Cycle: " << clockCycle << " ================" << endl;

        // Check for hazards before executing the pipeline stages
        detectAndHandleHazards();

        // Execute pipeline stages in reverse to maintain data integrity
        // (WB must execute before MEM, MEM before EX, etc.)
        pipelineWB();
        pipelineMEM();
        pipelineEX();
        pipelineID();
        pipelineIF();

        // Update stats and counters
        updateStats();
        total_cycles++;

        // Handle debug output based on knob settings
        if (knob_print_registers)
        {
            printRegisterFile();
        }

        if (knob_print_pipeline_registers)
        {
            printPipelineRegisters();
        }

        if (knob_print_branch_predictor)
        {
            printBranchPredictorState();
        }

        if (knob_trace_instruction >= 0)
        {
            traceSpecificInstruction(knob_trace_instruction);
        }

        // Check termination conditions
        if (infLoop ||
            (mem_wb.decodedInst.name == "ADDI" &&
             mem_wb.decodedInst.rs1 == 0 &&
             mem_wb.decodedInst.rd == 0 &&
             mem_wb.decodedInst.imm == 1))
        {
            exitSimulator = true;
        }

        // Check if program execution is complete (all pipeline stages empty)
        if (pcMachineCode.find(currentPC) == pcMachineCode.end() &&
            if_id.instruction.empty() &&
            id_ex.decodedInst.type.empty() &&
            ex_mem.decodedInst.type.empty() &&
            mem_wb.decodedInst.type.empty())
        {
            exitSimulator = true;
        }

        clockCycle++;
    }

    // Output final statistics and results
    printStats();
    saveStatsToFile("pipeline_stats.txt");
    dumpMemoryToFile("output.mc");
}

// Instruction Fetch (IF) stage
void pipelineIF()
{
    // Skip if fetch stage is stalled
    if (stall_fetch)
    {
        cout << "IF Stage: Stalled" << endl;
        return;
    }

    // Check if current PC points to a valid instruction
    if (pcMachineCode.find(currentPC) != pcMachineCode.end())
    {
        cout << "IF Stage: Fetching instruction at PC=" << currentPC << endl;

        // Get instruction at current PC
        string machineCode = pcMachineCode[currentPC];

        // Update IF/ID pipeline register if not flushed
        if (!flush_fetch)
        {
            if_id.instruction = machineCode;
            if_id.pc = currentPC;
        }
        else
        {
            cout << "IF Stage: Flushed" << endl;
            if_id.instruction = "";
            if_id.pc = "";
        }

        // Handle branch prediction if enabled and not stalled/flushed
        if (!flush_fetch && !stall_decode)
        {
            // Convert machine code to binary for opcode extraction
            string binaryInst = hex2bin(machineCode);

            // Check if binary instruction is long enough to extract opcode
            if (binaryInst.length() >= 7)
            {
                string opcode = binaryInst.substr(binaryInst.length() - 7);

                // Handle branch instructions (opcode 1100011)
                if (opcode == "1100011")
                { 
                    bool prediction = branchPredictor.predict(currentPC);
                    string targetPC = branchPredictor.getTarget(currentPC);

                    if (prediction && !targetPC.empty())
                    {
                        // Branch predicted as taken with known target
                        predicted_branch = true;
                        predicted_pc = currentPC;
                        currentPC = targetPC;
                        cout << "IF Stage: Branch predicted taken, new PC=" << targetPC << endl;
                    }
                    else
                    {
                        // Branch predicted not taken or target unknown
                        predicted_branch = false;
                        predicted_pc = currentPC;

                        // Increment PC to next instruction
                        unsigned int pc_val = stoul(currentPC.substr(2), nullptr, 16);
                        pc_val += 4;
                        stringstream ss;
                        ss << hex << "0x" << pc_val;
                        currentPC = ss.str();
                    }
                }
                // Handle jump instructions (opcode 1101111 for JAL)
                else if (opcode == "1101111")
                {
                    // JAL instructions need target calculation in ID stage
                    // For now, proceed to next instruction
                    unsigned int pc_val = stoul(currentPC.substr(2), nullptr, 16);
                    pc_val += 4;
                    stringstream ss;
                    ss << hex << "0x" << pc_val;
                    currentPC = ss.str();
                }
                else
                {
                    // For non-branch/jump instructions, simply increment PC
                    unsigned int pc_val = stoul(currentPC.substr(2), nullptr, 16);
                    pc_val += 4;
                    stringstream ss;
                    ss << hex << "0x" << pc_val;
                    currentPC = ss.str();
                }
            }
        }
    }
    else
    {
        cout << "IF Stage: No valid instruction at PC=" << currentPC << endl;
        if_id.instruction = "";
        if_id.pc = "";
    }
    cout << "====================================================================================================================================" << endl;
}

// Instruction Decode (ID) stage
void pipelineID()
{
    // Skip if decode stage is stalled
    if (stall_decode)
    {
        cout << "ID Stage: Stalled" << endl;
        return;
    }

    // Check if there's a valid instruction to decode
    if (!if_id.instruction.empty())
    {
        cout << "ID Stage: Decoding instruction " << if_id.instruction << " from PC=" << if_id.pc << endl;

        Instruction decodedInst;
        string binInst = hex2bin(if_id.instruction);

        // Extract opcode (last 7 bits)
        int opcode = stoi(binInst.substr(25, 7), nullptr, 2);
        decodedInst.opcode = opcode;

        // Decode instruction based on opcode
        if (opcode == 0b0110011)
        {
            // R-Type instruction decoding
            decodedInst.type = "R-Type";
            decodedInst.rd = stoi(binInst.substr(20, 5), nullptr, 2);
            decodedInst.fun3 = stoi(binInst.substr(17, 3), nullptr, 2);
            decodedInst.rs1 = stoi(binInst.substr(12, 5), nullptr, 2);
            decodedInst.rs2 = stoi(binInst.substr(7, 5), nullptr, 2);
            decodedInst.fun7 = stoi(binInst.substr(0, 7), nullptr, 2);

            // Identify specific R-Type instruction
            if (decodedInst.fun3 == 0b000 && decodedInst.fun7 == 0b0000000)
            {
                decodedInst.name = "ADD";
            }
            else if (decodedInst.fun3 == 0b000 && decodedInst.fun7 == 0b0100000)
            {
                decodedInst.name = "SUB";
            }
            else if (decodedInst.fun3 == 0b111 && decodedInst.fun7 == 0b0000000)
            {
                decodedInst.name = "AND";
            }
            else if (decodedInst.fun3 == 0b110 && decodedInst.fun7 == 0b0000000)
            {
                decodedInst.name = "OR";
            }
            else if (decodedInst.fun3 == 0b001 && decodedInst.fun7 == 0b0000000)
            {
                decodedInst.name = "SLL";
            }
            else if (decodedInst.fun3 == 0b010 && decodedInst.fun7 == 0b0000000)
            {
                decodedInst.name = "SLT";
            }
            else if (decodedInst.fun3 == 0b101 && decodedInst.fun7 == 0b0100000)
            {
                decodedInst.name = "SRA";
            }
            else if (decodedInst.fun3 == 0b101 && decodedInst.fun7 == 0b0000000)
            {
                decodedInst.name = "SRL";
            }
            else if (decodedInst.fun3 == 0b100 && decodedInst.fun7 == 0b0000000)
            {
                decodedInst.name = "XOR";
            }
            else if (decodedInst.fun3 == 0b000 && decodedInst.fun7 == 0b0000001)
            {
                decodedInst.name = "MUL";
            }
            else if (decodedInst.fun3 == 0b100 && decodedInst.fun7 == 0b0000001)
            {
                decodedInst.name = "DIV";
            }
            else if (decodedInst.fun3 == 0b110 && decodedInst.fun7 == 0b0000001)
            {
                decodedInst.name = "REM";
            }
        }
        else if (opcode == 0b0010011)
        {
            // I-Type immediate arithmetic instructions
            decodedInst.type = "I-Type";
            decodedInst.rd = stoi(binInst.substr(20, 5), nullptr, 2);
            decodedInst.fun3 = stoi(binInst.substr(17, 3), nullptr, 2);
            decodedInst.rs1 = stoi(binInst.substr(12, 5), nullptr, 2);

            // Extract and sign-extend immediate value
            string imm_str = binInst.substr(0, 12);
            decodedInst.imm = stoi(imm_str, nullptr, 2);
            
            // Handle sign extension
            if (imm_str[0] == '1')
            {
                decodedInst.imm |= 0xFFFFF000; // Set upper 20 bits to 1
            }

            // Identify specific I-Type immediate instruction
            if (decodedInst.fun3 == 0b000)
            {
                decodedInst.name = "ADDI";
            }
            else if (decodedInst.fun3 == 0b111)
            {
                decodedInst.name = "ANDI";
            }
            else if (decodedInst.fun3 == 0b110)
            {
                decodedInst.name = "ORI";
            }
        }
        else if (opcode == 0b0000011)
        {
            // I-Type load instructions
            decodedInst.type = "Load_I-Type";
            decodedInst.rd = stoi(binInst.substr(20, 5), nullptr, 2);
            decodedInst.fun3 = stoi(binInst.substr(17, 3), nullptr, 2);
            decodedInst.rs1 = stoi(binInst.substr(12, 5), nullptr, 2);

            // Extract and sign-extend immediate value
            string imm_str = binInst.substr(0, 12);
            decodedInst.imm = stoi(imm_str, nullptr, 2);
            
            // Handle sign extension
            if (imm_str[0] == '1')
            {
                decodedInst.imm |= 0xFFFFF000; // Set upper 20 bits to 1
            }

            // Identify specific load instruction
            if (decodedInst.fun3 == 0b000)
            {
                decodedInst.name = "LB";
            }
            else if (decodedInst.fun3 == 0b001)
            {
                decodedInst.name = "LH";
            }
            else if (decodedInst.fun3 == 0b010)
            {
                decodedInst.name = "LW";
            }
            else if (decodedInst.fun3 == 0b011)
            {
                decodedInst.name = "LD";
            }
        }
        else if (opcode == 0b0100011)
        {
            // S-Type store instructions
            decodedInst.type = "S-Type";
            decodedInst.fun3 = stoi(binInst.substr(17, 3), nullptr, 2);
            decodedInst.rs1 = stoi(binInst.substr(12, 5), nullptr, 2);
            decodedInst.rs2 = stoi(binInst.substr(7, 5), nullptr, 2);

            // Extract and combine immediate parts
            string imm_upper = binInst.substr(0, 7);
            string imm_lower = binInst.substr(20, 5);
            string imm_str = imm_upper + imm_lower;
            decodedInst.imm = stoi(imm_str, nullptr, 2);
            
            // Handle sign extension
            if (imm_str[0] == '1')
            {
                decodedInst.imm |= 0xFFFFF000; // Set upper 20 bits to 1
            }

            // Identify specific store instruction
            if (decodedInst.fun3 == 0b000)
            {
                decodedInst.name = "SB";
            }
            else if (decodedInst.fun3 == 0b001)
            {
                decodedInst.name = "SH";
            }
            else if (decodedInst.fun3 == 0b010)
            {
                decodedInst.name = "SW";
            }
            else if (decodedInst.fun3 == 0b011)
            {
                decodedInst.name = "SD";
            }
        }
        else if (opcode == 0b1100011)
        {
            // SB-Type branch instructions
            decodedInst.type = "SB-Type";
            decodedInst.fun3 = stoi(binInst.substr(17, 3), nullptr, 2);
            decodedInst.rs1 = stoi(binInst.substr(12, 5), nullptr, 2);
            decodedInst.rs2 = stoi(binInst.substr(7, 5), nullptr, 2);

            // Extract and assemble immediate for branch target
            string imm_12 = binInst.substr(0, 1);                        
            string imm_10_5 = binInst.substr(1, 6);                      
            string imm_4_1 = binInst.substr(20, 4);                      
            string imm_11 = binInst.substr(24, 1);                       
            string imm_str = imm_12 + imm_11 + imm_10_5 + imm_4_1 + "0"; // Add implicit 0 bit
            decodedInst.imm = stoi(imm_str, nullptr, 2);
            
            // Handle sign extension
            if (imm_str[0] == '1')
            {
                decodedInst.imm |= 0xFFFFE000; // Set upper 19 bits to 1
            }

            // Identify specific branch instruction
            if (decodedInst.fun3 == 0b000)
            {
                decodedInst.name = "BEQ";
            }
            else if (decodedInst.fun3 == 0b001)
            {
                decodedInst.name = "BNE";
            }
            else if (decodedInst.fun3 == 0b101)
            {
                decodedInst.name = "BGE";
            }
            else if (decodedInst.fun3 == 0b100)
            {
                decodedInst.name = "BLT";
            }
        }
        else if (opcode == 0b0110111)
        {
            // U-Type LUI instruction
            decodedInst.type = "LUI_U-Type";
            decodedInst.rd = stoi(binInst.substr(20, 5), nullptr, 2);

            // Extract immediate (upper 20 bits)
            string imm_str = binInst.substr(0, 20);
            decodedInst.imm = stoi(imm_str, nullptr, 2) << 12;

            decodedInst.name = "LUI";
        }
        else if (opcode == 0b0010111)
        {
            // U-Type AUIPC instruction
            decodedInst.type = "AUIPC_U-Type";
            decodedInst.rd = stoi(binInst.substr(20, 5), nullptr, 2);

            // Extract immediate (upper 20 bits)
            string imm_str = binInst.substr(0, 20);
            decodedInst.imm = stoi(imm_str, nullptr, 2) << 12;

            decodedInst.name = "AUIPC";
        }
        else if (opcode == 0b1101111)
        {
            // UJ-Type JAL instruction
            decodedInst.type = "JAL_J-Type";
            decodedInst.rd = stoi(binInst.substr(20, 5), nullptr, 2);

            // Extract and assemble immediate for jump target
            string imm_20 = binInst.substr(0, 1);                         
            string imm_10_1 = binInst.substr(1, 10);                      
            string imm_11 = binInst.substr(11, 1);                        
            string imm_19_12 = binInst.substr(12, 8);                     
            string imm_str = imm_20 + imm_19_12 + imm_11 + imm_10_1 + "0"; 
            decodedInst.imm = stoi(imm_str, nullptr, 2);
            
            // Handle sign extension
            if (imm_str[0] == '1')
            {
                decodedInst.imm |= 0xFFF00000; // Set upper 12 bits to 1
            }

            decodedInst.name = "JAL";
        }
        else if (opcode == 0b1100111 && stoi(binInst.substr(17, 3), nullptr, 2) == 0b000)
        {
            // JALR instruction (I-Type format)
            decodedInst.type = "JALR_I-Type";
            decodedInst.rd = stoi(binInst.substr(20, 5), nullptr, 2);
            decodedInst.fun3 = stoi(binInst.substr(17, 3), nullptr, 2);
            decodedInst.rs1 = stoi(binInst.substr(12, 5), nullptr, 2);

            // Extract and sign-extend immediate
            string imm_str = binInst.substr(0, 12);
            decodedInst.imm = stoi(imm_str, nullptr, 2);
            
            // Handle sign extension
            if (imm_str[0] == '1')
            {
                decodedInst.imm |= 0xFFFFF000; // Set upper 20 bits to 1
            }

            decodedInst.name = "JALR";
        }
        else
        {
            // Unknown instruction
            decodedInst.type = "Unknown";
            decodedInst.name = "Unknown";
        }

        // Read register values for the next stage
        int rs1_value = 0;
        int rs2_value = 0;

        if (decodedInst.type != "Unknown")
        {
            // Read rs1 value for instructions that use it
            if (decodedInst.type == "R-Type" ||
                decodedInst.type == "I-Type" ||
                decodedInst.type == "Load_I-Type" ||
                decodedInst.type == "S-Type" ||
                decodedInst.type == "SB-Type" ||
                decodedInst.type == "JALR_I-Type")
            {
                rs1_value = registerFile[decodedInst.rs1];
            }

            // Read rs2 value for instructions that use it
            if (decodedInst.type == "R-Type" ||
                decodedInst.type == "S-Type" ||
                decodedInst.type == "SB-Type")
            {
                rs2_value = registerFile[decodedInst.rs2];
            }
        }

        // Update ID/EX pipeline register if not flushed
        if (!flush_decode)
        {
            id_ex.pc = if_id.pc;
            id_ex.decodedInst = decodedInst;
            id_ex.rs1_value = rs1_value;
            id_ex.rs2_value = rs2_value;
            id_ex.isStall = false;

            total_instructions++; // Increment instruction counter

            cout << "ID Stage: Decoded " << decodedInst.name << " instruction" << endl;
        }
        else
        {
            cout << "ID Stage: Flushed" << endl;
            id_ex.decodedInst.type = "";
            id_ex.pc = "";
        }
    }
    else
    {
        cout << "ID Stage: No instruction to decode" << endl;
        id_ex.decodedInst.type = "";
        id_ex.pc = "";
    }
    cout << "====================================================================================================================================" << endl;
}

// Execute (EX) stage
void pipelineEX()
{
    // Skip if execute stage is stalled
    if (stall_execute)
    {
        cout << "EX Stage: Stalled" << endl;
        return;
    }

    // Check if there's a valid instruction to execute
    if (id_ex.decodedInst.type != "")
    {
        cout << "EX Stage: Executing " << id_ex.decodedInst.name << " instruction from PC=" << id_ex.pc << endl;

        long long int aluResult = 0;
        bool branchTaken = false;
        string branchTarget = "";
        unsigned int returnAddress = 0;

        // Execute based on instruction type
        if (id_ex.decodedInst.type == "R-Type")
        {
            // Handle R-Type ALU operations
            if (id_ex.decodedInst.name == "ADD")
            {
                aluResult = id_ex.rs1_value + id_ex.rs2_value;
            }
            else if (id_ex.decodedInst.name == "SUB")
            {
                aluResult = id_ex.rs1_value - id_ex.rs2_value;
            }
            else if (id_ex.decodedInst.name == "AND")
            {
                aluResult = id_ex.rs1_value & id_ex.rs2_value;
            }
            else if (id_ex.decodedInst.name == "OR")
            {
                aluResult = id_ex.rs1_value | id_ex.rs2_value;
            }
            else if (id_ex.decodedInst.name == "SLL")
            {
                aluResult = id_ex.rs1_value << (id_ex.rs2_value & 0x1F);
            }
            else if (id_ex.decodedInst.name == "SLT")
            {
                aluResult = (id_ex.rs1_value < id_ex.rs2_value) ? 1 : 0;
            }
            else if (id_ex.decodedInst.name == "SRA")
            {
                // Arithmetic shift right (preserve sign bit)
                aluResult = id_ex.rs1_value >> (id_ex.rs2_value & 0x1F);
                if ((id_ex.rs1_value & 0x80000000) && (id_ex.rs2_value & 0x1F) > 0)
                {
                    aluResult |= (~0U << (32 - (id_ex.rs2_value & 0x1F)));
                }
            }
            else if (id_ex.decodedInst.name == "SRL")
            {
                // Logical shift right (fill with zeros)
                aluResult = (unsigned int)id_ex.rs1_value >> (id_ex.rs2_value & 0x1F);
            }
            else if (id_ex.decodedInst.name == "XOR")
            {
                aluResult = id_ex.rs1_value ^ id_ex.rs2_value;
            }
            else if (id_ex.decodedInst.name == "MUL")
            {
                aluResult = id_ex.rs1_value * id_ex.rs2_value;
            }
            else if (id_ex.decodedInst.name == "DIV")
            {
                // Handle division by zero
                if (id_ex.rs2_value != 0)
                {
                    aluResult = id_ex.rs1_value / id_ex.rs2_value;
                }
                else
                {
                    aluResult = -1; // Division by zero error value
                }
            }
            else if (id_ex.decodedInst.name == "REM")
            {
                // Handle modulo by zero
                if (id_ex.rs2_value != 0)
                {
                    aluResult = id_ex.rs1_value % id_ex.rs2_value;
                }
                else
                {
                    aluResult = id_ex.rs1_value; // Remainder when dividing by zero is the dividend
                }
            }
        }
        else if (id_ex.decodedInst.type == "I-Type")
        {
            // Handle I-Type immediate operations
            if (id_ex.decodedInst.name == "ADDI")
            {
                aluResult = id_ex.rs1_value + id_ex.decodedInst.imm;
            }
            else if (id_ex.decodedInst.name == "ANDI")
            {
                aluResult = id_ex.rs1_value & id_ex.decodedInst.imm;
            }
            else if (id_ex.decodedInst.name == "ORI")
            {
                aluResult = id_ex.rs1_value | id_ex.decodedInst.imm;
            }
        }
        else if (id_ex.decodedInst.type == "Load_I-Type")
        {
            // Calculate memory address for load instructions
            aluResult = id_ex.rs1_value + id_ex.decodedInst.imm;
        }
        else if (id_ex.decodedInst.type == "S-Type")
        {
            // Calculate memory address for store instructions
            aluResult = id_ex.rs1_value + id_ex.decodedInst.imm;
        }
        else if (id_ex.decodedInst.type == "SB-Type")
        {
            // Calculate branch target address
            unsigned int pc_val = stoul(id_ex.pc.substr(2), nullptr, 16);
            unsigned int target_addr = pc_val + id_ex.decodedInst.imm;
            stringstream ss;
            ss << hex << "0x" << target_addr;
            branchTarget = ss.str();

            // Evaluate branch condition based on instruction type
            if (id_ex.decodedInst.name == "BEQ")
            {
                branchTaken = (id_ex.rs1_value == id_ex.rs2_value);
            }
            else if (id_ex.decodedInst.name == "BNE")
            {
                branchTaken = (id_ex.rs1_value != id_ex.rs2_value);
            }
            else if (id_ex.decodedInst.name == "BGE")
            {
                branchTaken = (id_ex.rs1_value >= id_ex.rs2_value);
            }
            else if (id_ex.decodedInst.name == "BLT")
            {
                branchTaken = (id_ex.rs1_value < id_ex.rs2_value);
            }

            // Update branch predictor with actual outcome
            branchPredictor.update(id_ex.pc, branchTaken, branchTarget);

            cout << "EX Stage: Branch condition " << (branchTaken ? "satisfied" : "not satisfied")
                 << ", target=" << branchTarget << endl;
        }
        else if (id_ex.decodedInst.type == "LUI_U-Type")
        {
            // Load Upper Immediate - just pass the immediate value
            aluResult = id_ex.decodedInst.imm;
        }
        else if (id_ex.decodedInst.type == "AUIPC_U-Type")
        {
            unsigned int pc_val = stoul(id_ex.pc.substr(2), nullptr, 16);
            aluResult = pc_val + id_ex.decodedInst.imm;
        }
        else if (id_ex.decodedInst.type == "JAL_J-Type")
        {
            // Calculate jump target address
            unsigned int pc_val = stoul(id_ex.pc.substr(2), nullptr, 16);
            unsigned int target_addr = pc_val + id_ex.decodedInst.imm;
            stringstream ss;
            ss << hex << "0x" << target_addr;
            branchTarget = ss.str();
            branchTaken = true; // JAL is always taken

            // Calculate return address (PC+4)
            returnAddress = pc_val + 4;
            aluResult = returnAddress; // JAL stores return address in rd

            cout << "EX Stage: JAL target=" << branchTarget << ", return address=" << returnAddress << endl;
        }
        else if (id_ex.decodedInst.type == "JALR_I-Type")
        {
            // Calculate jump target address
            unsigned int pc_val = stoul(id_ex.pc.substr(2), nullptr, 16);
            unsigned int target_addr = (id_ex.rs1_value + id_ex.decodedInst.imm) & ~1; // JALR must be even
            stringstream ss;
            ss << hex << "0x" << target_addr;
            branchTarget = ss.str();
            branchTaken = true; // JALR is always taken

            // Calculate return address (PC+4)
            returnAddress = pc_val + 4;
            aluResult = returnAddress; // JALR stores return address in rd

            cout << "EX Stage: JALR target=" << branchTarget << ", return address=" << returnAddress << endl;
        }
        else
        {
            cout << "EX Stage: Unknown instruction type" << endl;
        }

        // Update EX/MEM pipeline register if not flushed
        if (!flush_execute)
        {
            ex_mem.pc = id_ex.pc;
            ex_mem.decodedInst = id_ex.decodedInst;
            ex_mem.aluResult = aluResult;
            ex_mem.rs2_value = id_ex.rs2_value; // For store instructions
            ex_mem.branchTarget = branchTarget;
            ex_mem.branchTaken = branchTaken;
            ex_mem.returnAddress = returnAddress;

            cout << "EX Stage: ALU result = " << aluResult << endl;
        }
        else
        {
            cout << "EX Stage: Flushed" << endl;
            ex_mem.decodedInst.type = "";
            ex_mem.pc = "";
        }
    }
    else
    {
        cout << "EX Stage: No instruction to execute" << endl;
        ex_mem.decodedInst.type = "";
        ex_mem.pc = "";
    }
    cout << "====================================================================================================================================" << endl;
}

// Memory (MEM) stage
// Memory (MEM) stage
void pipelineMEM()
{
    // Skip if stalled
    if (stall_memory)
    {
        cout << "MEM Stage: Stalled" << endl;
        return;
    }

    // Check if there is a valid instruction to process
    if (ex_mem.decodedInst.type != "")
    {
        cout << "MEM Stage: Processing " << ex_mem.decodedInst.name << " instruction from PC=" << ex_mem.pc << endl;

        int memoryData = 0;
        unsigned int address = static_cast<unsigned int>(ex_mem.aluResult);
        
        // Check if this is a stack memory access
        bool isStackAccess = (address >= stackPointer && address <= stackBaseAddress);

        // Process based on instruction type
        if (ex_mem.decodedInst.type == "Load_I-Type")
        {
            // Load instruction - read from memory
            if (ex_mem.decodedInst.name == "LB")
            {
                // Load byte (8 bits) and sign extend
                memoryData = static_cast<int8_t>(dataMemory[address]);
                cout << (isStackAccess ? "STACK " : "") << "LB: Loading byte from address 0x" << hex << address << ": " << dec << memoryData << endl;
            }
            else if (ex_mem.decodedInst.name == "LH")
            {
                // Load half-word (16 bits) and sign extend
                int16_t value = 0;
                for (int i = 0; i < 2; i++)
                {
                    value |= (static_cast<int16_t>(dataMemory[address + i]) << (8 * i));
                }
                memoryData = value;
                cout << (isStackAccess ? "STACK " : "") << "LH: Loading half-word from address 0x" << hex << address << ": " << dec << memoryData << endl;
            }
            else if (ex_mem.decodedInst.name == "LW")
            {
                // Load word (32 bits)
                int32_t value = 0;
                for (int i = 0; i < 4; i++)
                {
                    value |= (static_cast<int32_t>(dataMemory[address + i]) << (8 * i));
                }
                memoryData = value;
                cout << (isStackAccess ? "STACK " : "") << "LW: Loading word from address 0x" << hex << address << ": " << dec << memoryData << endl;
            }
            else if (ex_mem.decodedInst.name == "LD")
            {
                // Load double-word (64 bits)
                int64_t value = 0;
                for (int i = 0; i < 8; i++)
                {
                    value |= (static_cast<int64_t>(dataMemory[address + i]) << (8 * i));
                }
                memoryData = value;
                cout << (isStackAccess ? "STACK " : "") << "LD: Loading double-word from address 0x" << hex << address << ": " << dec << memoryData << endl;
            }
        }
        else if (ex_mem.decodedInst.type == "S-Type")
        {
            // Store instruction - write to memory
            int storeData = ex_mem.rs2_value;

            if (ex_mem.decodedInst.name == "SB")
            {
                // Store byte (8 bits)
                dataMemory[address] = storeData & 0xFF;
                cout << (isStackAccess ? "STACK " : "") << "SB: Storing byte to address 0x" << hex << address << ": "
                     << (storeData & 0xFF) << dec << endl;
            }
            else if (ex_mem.decodedInst.name == "SH")
            {
                // Store half-word (16 bits)
                for (int i = 0; i < 2; i++)
                {
                    dataMemory[address + i] = (storeData >> (8 * i)) & 0xFF;
                }
                cout << (isStackAccess ? "STACK " : "") << "SH: Storing half-word to address 0x" << hex << address << ": "
                     << (storeData & 0xFFFF) << dec << endl;
            }
            else if (ex_mem.decodedInst.name == "SW")
            {
                // Store word (32 bits)
                for (int i = 0; i < 4; i++)
                {
                    dataMemory[address + i] = (storeData >> (8 * i)) & 0xFF;
                }
                cout << (isStackAccess ? "STACK " : "") << "SW: Storing word to address 0x" << hex << address << ": "
                     << storeData << dec << endl;
            }
            else if (ex_mem.decodedInst.name == "SD")
            {
                // Store double-word (64 bits)
                for (int i = 0; i < 8; i++)
                {
                    dataMemory[address + i] = (storeData >> (8 * i)) & 0xFF;
                }
                cout << (isStackAccess ? "STACK " : "") << "SD: Storing double-word to address 0x" << hex << address << ": "
                     << storeData << dec << endl;
            }
        }
        else
        {
            // Non-memory instruction, just pass ALU result through
            memoryData = ex_mem.aluResult;
            cout << "MEM Stage: No memory access needed for this instruction" << endl;
        }

        // Handle branch misprediction logic
        if ((ex_mem.decodedInst.type == "SB-Type" ||
             ex_mem.decodedInst.type == "JAL_J-Type" ||
             ex_mem.decodedInst.type == "JALR_I-Type") &&
            ex_mem.branchTaken)
        {
            // If branch is taken and we haven't already predicted it correctly
            if (predicted_pc != ex_mem.pc || !predicted_branch)
            {
                cout << "MEM Stage: Branch taken, but not predicted correctly" << endl;
                // This would be handled in detectControlHazard()
            }
        }

        // Update MEM/WB pipeline register if not flushed
        if (!flush_memory)
        {
            mem_wb.pc = ex_mem.pc;
            mem_wb.decodedInst = ex_mem.decodedInst;
            mem_wb.aluResult = ex_mem.aluResult;
            mem_wb.memoryData = memoryData;
            mem_wb.writebackData = (ex_mem.decodedInst.type == "Load_I-Type") ? memoryData : ex_mem.aluResult;
        }
        else
        {
            cout << "MEM Stage: Flushed" << endl;
            mem_wb.decodedInst.type = "";
            mem_wb.pc = "";
        }
    }
    else
    {
        cout << "MEM Stage: No instruction to process" << endl;
        mem_wb.decodedInst.type = "";
        mem_wb.pc = "";
    }
    cout << "====================================================================================================================================" << endl;
}

// Writeback (WB) stage
void pipelineWB()
{
    // Skip if stalled
    if (stall_writeback)
    {
        cout << "WB Stage: Stalled" << endl;
        return;
    }

    // Check if there is a valid instruction to writeback
    if (mem_wb.decodedInst.type != "")
    {
        cout << "WB Stage: Writing back " << mem_wb.decodedInst.name << " instruction from PC=" << mem_wb.pc << endl;

        // Determine if this instruction writes to a register
        bool writesToRegister = false;

        if (mem_wb.decodedInst.type == "R-Type" ||
            mem_wb.decodedInst.type == "I-Type" ||
            mem_wb.decodedInst.type == "Load_I-Type" ||
            mem_wb.decodedInst.type == "LUI_U-Type" ||
            mem_wb.decodedInst.type == "AUIPC_U-Type" ||
            mem_wb.decodedInst.type == "JAL_J-Type" ||
            mem_wb.decodedInst.type == "JALR_I-Type")
        {
            writesToRegister = true;
        }

        // Write to register file if needed
        if (writesToRegister && mem_wb.decodedInst.rd != 0)
        {
            int writeValue = mem_wb.writebackData;

            // Update register file (never write to R0)
            if (mem_wb.decodedInst.rd != 0)
            {
                registerFile[mem_wb.decodedInst.rd] = writeValue;
                cout << "WB Stage: Written " << writeValue << " to register R" << mem_wb.decodedInst.rd << endl;
            }
        }
        else
        {
            cout << "WB Stage: No register writeback needed" << endl;
        }
    }
    else
    {
        cout << "WB Stage: No instruction to writeback" << endl;
    }
    cout << "====================================================================================================================================" << endl;
}

// Print the contents of all pipeline registers
void printPipelineRegisters()
{
    cout << "\n--- Pipeline Registers State ---" << endl;
    cout << "====================================================================================================================================" << endl;
    // IF/ID Register
    cout << "IF/ID Register:" << endl;
    cout << "  PC: " << if_id.pc << endl;
    cout << "  Instruction: " << if_id.instruction << endl;
    cout << "====================================================================================================================================" << endl;
    // ID/EX Register
    cout << "ID/EX Register:" << endl;
    cout << "  PC: " << id_ex.pc << endl;
    cout << "  Instruction: " << id_ex.decodedInst.name << endl;
    cout << "  Type: " << id_ex.decodedInst.type << endl;
    cout << "  RS1: " << id_ex.decodedInst.rs1 << " (Value: " << id_ex.rs1_value << ")" << endl;
    cout << "  RS2: " << id_ex.decodedInst.rs2 << " (Value: " << id_ex.rs2_value << ")" << endl;
    cout << "  RD: " << id_ex.decodedInst.rd << endl;
    cout << "  Immediate: " << id_ex.decodedInst.imm << endl;
    cout << "====================================================================================================================================" << endl;
    // EX/MEM Register
    cout << "EX/MEM Register:" << endl;
    cout << "  PC: " << ex_mem.pc << endl;
    cout << "  Instruction: " << ex_mem.decodedInst.name << endl;
    cout << "  Type: " << ex_mem.decodedInst.type << endl;
    cout << "  ALU Result: " << ex_mem.aluResult << endl;
    cout << "  RS2 Value: " << ex_mem.rs2_value << endl;
    cout << "  Branch Target: " << ex_mem.branchTarget << endl;
    cout << "  Branch Taken: " << (ex_mem.branchTaken ? "Yes" : "No") << endl;
    cout << "====================================================================================================================================" << endl;
    // MEM/WB Register
    cout << "MEM/WB Register:" << endl;
    cout << "  PC: " << mem_wb.pc << endl;
    cout << "  Instruction: " << mem_wb.decodedInst.name << endl;
    cout << "  Type: " << mem_wb.decodedInst.type << endl;
    cout << "  ALU Result: " << mem_wb.aluResult << endl;
    cout << "  Memory Data: " << mem_wb.memoryData << endl;
    cout << "  Writeback Data: " << mem_wb.writebackData << endl;
    cout << "====================================================================================================================================" << endl;
}

// Print the current state of the branch predictor
void printBranchPredictorState()
{
    cout << "\n--- Branch Predictor State ---" << endl;
    cout << "Pattern History Table (PHT):" << endl;

    for (const auto &entry : branchPredictor.pht)
    {
        cout << "  PC: " << entry.first << " -> Prediction: " << (entry.second ? "Taken" : "Not Taken") << endl;
    }

    cout << "Branch Target Buffer (BTB):" << endl;
    for (const auto &entry : branchPredictor.btb)
    {
        cout << "  PC: " << entry.first << " -> Target: " << entry.second << endl;
    }

    cout << "Total branch predictions: " << branchPredictor.predictions << endl;
    cout << "Correct predictions: " << branchPredictor.correct_predictions << endl;
    cout << "Accuracy: " << (branchPredictor.predictions > 0 ? 100.0 * branchPredictor.correct_predictions / branchPredictor.predictions : 0)
         << "%" << endl;
}

// Trace the execution of a specific instruction through the pipeline
void traceSpecificInstruction(int instructionNumber)
{
    static int instructionCounter = 0;
    static map<string, int> instructionIDs;

    // Assign IDs to instructions as they enter the pipeline (in IF stage)
    if (!if_id.instruction.empty() && if_id.pc != "")
    {
        if (instructionIDs.find(if_id.pc) == instructionIDs.end())
        {
            instructionIDs[if_id.pc] = instructionCounter++;
        }
    }

    cout << "\n--- Tracing Instruction #" << instructionNumber << " ---" << endl;

    // Check each pipeline stage for the specific instruction
    if (instructionIDs.find(if_id.pc) != instructionIDs.end() &&
        instructionIDs[if_id.pc] == instructionNumber)
    {
        cout << "Currently in IF/ID stage" << endl;
    }

    if (instructionIDs.find(id_ex.pc) != instructionIDs.end() &&
        instructionIDs[id_ex.pc] == instructionNumber)
    {
        cout << "Currently in ID/EX stage" << endl;
        cout << "  Instruction: " << id_ex.decodedInst.name << endl;
        cout << "  RS1 Value: " << id_ex.rs1_value << endl;
        cout << "  RS2 Value: " << id_ex.rs2_value << endl;
    }

    if (instructionIDs.find(ex_mem.pc) != instructionIDs.end() &&
        instructionIDs[ex_mem.pc] == instructionNumber)
    {
        cout << "Currently in EX/MEM stage" << endl;
        cout << "  Instruction: " << ex_mem.decodedInst.name << endl;
        cout << "  ALU Result: " << ex_mem.aluResult << endl;
        if (ex_mem.decodedInst.type == "SB-Type")
        {
            cout << "  Branch Taken: " << (ex_mem.branchTaken ? "Yes" : "No") << endl;
            cout << "  Branch Target: " << ex_mem.branchTarget << endl;
        }
    }

    if (instructionIDs.find(mem_wb.pc) != instructionIDs.end() &&
        instructionIDs[mem_wb.pc] == instructionNumber)
    {
        cout << "Currently in MEM/WB stage" << endl;
        cout << "  Instruction: " << mem_wb.decodedInst.name << endl;
        cout << "  Writeback Data: " << mem_wb.writebackData << endl;
        cout << "  Destination Register: R" << mem_wb.decodedInst.rd << endl;
    }
}
