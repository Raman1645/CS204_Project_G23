#include <bits/stdc++.h>
#include "globals.h"
#include "structs.h"
#include "stats.h"

using namespace std;

// Initialize statistics variables
void initializeStats()
{
    total_cycles = 0;
    total_instructions = 0;
    data_transfer_instructions = 0;
    alu_instructions = 0;
    control_instructions = 0;
    pipeline_stalls = 0;
    data_hazards = 0;
    control_hazards = 0;
    branch_mispredictions = 0;
    stalls_data_hazards = 0;
    stalls_control_hazards = 0;
}

// Update statistics based on current instruction
void updateStats()
{
    // Check the instruction type in EX stage to categorize it
    if (id_ex.decodedInst.type != "")
    {
        // Categorize instruction type for statistics
        if (id_ex.decodedInst.type == "R-Type" ||
            id_ex.decodedInst.type == "I-Type" ||
            id_ex.decodedInst.type == "LUI_U-Type" ||
            id_ex.decodedInst.type == "AUIPC_U-Type")
        {
            alu_instructions++;
        }
        else if (id_ex.decodedInst.type == "Load_I-Type" ||
                 id_ex.decodedInst.type == "S-Type")
        {
            data_transfer_instructions++;
        }
        else if (id_ex.decodedInst.type == "SB-Type" ||
                 id_ex.decodedInst.type == "JAL_J-Type" ||
                 id_ex.decodedInst.type == "JALR_I-Type")
        {
            control_instructions++;
        }
    }
}

// Print register file state
void printRegisterFile()
{
    cout << "\n--- Register File State ---" << endl;
    for (int i = 0; i < 32; i++)
    {
        if (i % 4 == 0 && i > 0)
            cout << endl;
        cout << "R" << setw(2) << setfill('0') << i << ": " << setw(8) << setfill('0') << hex << registerFile[i] << "  ";
    }
    cout << dec << endl;
}

// Print statistics
void printStats()
{
    cout << "\n--- Pipeline Execution Statistics ---" << endl;
    cout << "Total cycles: " << total_cycles << endl;
    cout << "Total instructions executed: " << total_instructions << endl;
    cout << "CPI: " << (float)total_cycles / total_instructions << endl;
    cout << "Data transfer instructions: " << data_transfer_instructions << endl;
    cout << "ALU instructions: " << alu_instructions << endl;
    cout << "Control instructions: " << control_instructions << endl;
    cout << "Pipeline stalls: " << pipeline_stalls << endl;
    cout << "Data hazards: " << data_hazards << endl;
    cout << "Control hazards: " << control_hazards << endl;
    cout << "Branch mispredictions: " << branch_mispredictions << endl;
    cout << "Stalls due to data hazards: " << stalls_data_hazards << endl;
    cout << "Stalls due to control hazards: " << stalls_control_hazards << endl;
}

// Save statistics to file
void saveStatsToFile(const string &filename)
{
    ofstream outFile(filename);
    if (!outFile.is_open())
    {
        cerr << "Error opening file for statistics: " << filename << endl;
        return;
    }

    outFile << "--- Pipeline Execution Statistics ---" << endl;
    outFile << "Stat1: Total cycles: " << total_cycles << endl;
    outFile << "Stat2: Total instructions executed: " << total_instructions << endl;
    outFile << "Stat3: CPI: " << (float)total_cycles / total_instructions << endl;
    outFile << "Stat4: Data transfer instructions: " << data_transfer_instructions << endl;
    outFile << "Stat5: ALU instructions: " << alu_instructions << endl;
    outFile << "Stat6: Control instructions: " << control_instructions << endl;
    outFile << "Stat7: Pipeline stalls: " << pipeline_stalls << endl;
    outFile << "Stat8: Data hazards: " << data_hazards << endl;
    outFile << "Stat9: Control hazards: " << control_hazards << endl;
    outFile << "Stat10: Branch mispredictions: " << branch_mispredictions << endl;
    outFile << "Stat11: Stalls due to data hazards: " << stalls_data_hazards << endl;
    outFile << "Stat12: Stalls due to control hazards: " << stalls_control_hazards << endl;

    outFile.close();
    cout << "Statistics saved to " << filename << endl;
}