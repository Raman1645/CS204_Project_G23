#include<bits/stdc++.h>

using namespace std;

map<string, string> pcMachineCode;
string currentPC = "0x0";
long long int result;
string currentInstruction;
int registerFile[32];
bool infLoop = false;
// Add these declarations at the beginning of your file, with your other global variables
unordered_map<unsigned int, unsigned char> dataMemory; // Byte-addressable memory
unsigned int memoryBaseAddress = 0x10000000; // Starting address for data memory


struct Instruction {
    string type;
    string name;
    int opcode;
    int rs1;
    int rs2;
    int rd;
    int fun3;
    int fun7;
    long long int imm;
};
Instruction instruction;

void loadMC(const string &filename) {
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    string line;
    bool dataSegment = false;

    while (getline(file, line)) {
        // Check if we've reached the data segment marker
        if (line.find("Data Segment") != string::npos) {
            dataSegment = true;
            continue;
        }

        // Process based on whether we're in the instruction or data segment
        if (!dataSegment) {
            // Skip empty lines or lines without commas (not instruction lines)
            if (line.empty() || line.find(',') == string::npos) {
                continue;
            }

            size_t commaPos = line.find(',');
            
            // Ensure the line has the expected format with PC and machine code separated by a comma
            if (commaPos != string::npos) {
                string pc = line.substr(0, commaPos-1);
                pc = pc.substr(0, pc.find(' ')); // Remove trailing spaces
                
                // Extract the machine code, removing leading/trailing spaces
                string machine_code = line.substr(commaPos + 2);
                size_t startPos = machine_code.find_first_not_of(' ');
                if (startPos != string::npos) {
                    machine_code = machine_code.substr(startPos);
                }
                
                size_t endPos = machine_code.find(' ');
                if (endPos != string::npos) {
                    machine_code = machine_code.substr(0, endPos);
                }
                
                pcMachineCode[pc] = machine_code;
            }
        } else {
            // Process data segment lines
            // Skip empty lines or marker lines
            if (line.empty() || line.find("****") != string::npos) {
                continue;
            }

            // Parse data memory line like: "0x10000000   61 62 63 64"
            stringstream ss(line);
            string addrStr;
            ss >> addrStr; // Read address
            
            if (addrStr.empty() || addrStr[0] != '0') {
                continue; // Skip invalid lines
            }
            
            // Convert hex address to integer
            unsigned int baseAddr;
            try {
                baseAddr = stoul(addrStr, nullptr, 16);
            } catch (...) {
                continue; // Skip if address conversion fails
            }
            
            // Read byte values
            string byteStr;
            unsigned int offset = 0;
            
            while (ss >> byteStr) {
                // Convert hex byte to integer
                try {
                    unsigned char byteVal = stoul(byteStr, nullptr, 16);
                    dataMemory[baseAddr + offset] = byteVal;
                    offset++;
                } catch (...) {
                    // Skip if byte conversion fails
                }
            }
        }
    }

    file.close();
    cout << "Loaded " << pcMachineCode.size() << " instructions and " 
         << dataMemory.size() << " bytes of data memory." << endl;
}

string hex2bin (string hexStr) {
    // Check for "0x" prefix and remove it
    if (hexStr.substr(0, 2) == "0x" || hexStr.substr(0, 2) == "0X") {
        hexStr = hexStr.substr(2);  // Remove "0x"
    }

    // Ensure valid length (8 characters for 32 bits)
    if (hexStr.length() >= 8) {
        cout << "Invalid input. Please enter exactly 8 hexadecimal digits." << endl;
        return "";
    }

    // Convert hex string to unsigned int
    unsigned int num;
    stringstream ss;
    ss << hex << hexStr;
    ss >> num;

    // Convert to 32-bit binary representation
    bitset<32> binary(num);
    return binary.to_string();
}

string bin2hex (string binStr) {
    // Ensure valid length (32 characters for 32 bits)
    if (binStr.length() >= 32) {
        cout << "Invalid input. Please enter exactly 32 binary digits." << endl;
        return "";
    }

    // Convert binary string to unsigned int
    bitset<32> binary(binStr);
    unsigned int num = binary.to_ulong();

    // Convert to hex string
    stringstream ss;
    ss << hex << num;
    return ss.str();
}

void fetchInstruction () {
    string programCounter = currentPC;
    currentInstruction = pcMachineCode[programCounter];
    cout<<"Fetching Instruction : "<<currentInstruction<< ", PC : "<<programCounter<<endl;
}

void decodeInstruction () {
    unsigned int ins = stoul(currentInstruction, nullptr, 16);
    cout<<"Decoding Instruction : "<<currentInstruction;
    
    instruction.opcode = ins & 0x7F;
    switch (instruction.opcode) {
        case 0x33:
            instruction.type = "R-Type";
            instruction.rd = (ins >> 7) & 0x1F;
            instruction.fun3 = (ins >> 12) & 0x7;
            instruction.rs1 = (ins >> 15) & 0x1F;
            instruction.rs2 = (ins >> 20) & 0x1F;
            instruction.fun7 = (ins >> 25) & 0x7F;
            switch (instruction.fun3) {
                case 0x0:
                    if (instruction.fun7 == 0x00) {
                        instruction.name = "ADD";
                    } else if (instruction.fun7 == 0x20) {
                        instruction.name = "SUB";
                    } else if (instruction.fun7 == 0x01) {
                        instruction.name = "MUL";
                    }
                    break;
                case 0x1:
                    instruction.name = "SLL";
                    break;
                case 0x2:
                    instruction.name = "SLT";
                    break;
                case 0x4:
                    if (instruction.fun7 == 0x00) {
                        instruction.name = "XOR";
                    } else if (instruction.fun7 == 0x01) {
                        instruction.name = "DIV";
                    }
                    break;
                case 0x5:
                    if (instruction.fun7 == 0x00) {
                        instruction.name = "SRL";
                    } else if (instruction.fun7 == 0x20) {
                        instruction.name = "SRA";
                    }
                    break;
                case 0x6:
                    if (instruction.fun7 == 0x00) {
                        instruction.name = "OR";
                    } else if (instruction.fun7 == 0x01) {
                        instruction.name = "REM";
                    }
                    break;
                case 0x7:
                    instruction.name = "AND";
                    break;
                default:
                    cout<<"Invalid R-Type Instruction"<<endl;    
            }
            cout<<", operation : "<<instruction.name<<", RS1 : "<<instruction.rs1<<", RS2 : "<<instruction.rs2<<", RD : "<<instruction.rd<<endl;
            break;
        case 0x13:
            instruction.type = "I-Type";
            instruction.rd = (ins >> 7) & 0x1F;
            instruction.fun3 = (ins >> 12) & 0x7;
            instruction.rs1 = (ins >> 15) & 0x1F;
            // Extract 12-bit immediate and sign-extend it
            instruction.imm = (ins >> 20) & 0xFFF;
            // Sign extension: if the most significant bit is 1, extend with 1s
            if (instruction.imm & 0x800) {
                instruction.imm |= 0xFFFFF000; // Sign extend
            }
            switch (instruction.fun3) {
                case 0x0:
                    instruction.name = "ADDI";
                    break;
                case 0x6:
                    instruction.name = "ORI";
                    break;
                case 0x7:
                    instruction.name = "ANDI";
                    break;
                default:
                    cout<<"Invalid I-Type Instruction"<<endl;
            }
            cout<<", operation : "<<instruction.name<<", RS1 : "<<instruction.rs1<<", IMM : "<<instruction.imm<<", RD : "<<instruction.rd<<endl;
            break;
        case 0x3:
            instruction.type = "Load_I-Type";
            instruction.rd = (ins >> 7) & 0x1F;
            instruction.fun3 = (ins >> 12) & 0x7;
            instruction.rs1 = (ins >> 15) & 0x1F;
            // Extract 12-bit immediate and sign-extend it
            instruction.imm = (ins >> 20) & 0xFFF;
            // Sign extension
            if (instruction.imm & 0x800) {
                instruction.imm |= 0xFFFFF000; // Sign extend
            }
            switch (instruction.fun3) {
                case 0x0:
                    instruction.name = "LB";
                    break;
                case 0x1:
                    instruction.name = "LH";
                    break;
                case 0x2:
                    instruction.name = "LW";
                    break;
                case 0x3:
                    instruction.name = "LD";
                    break;
                default:
                    cout<<"Invalid Load Instruction"<<endl;
            }
            cout<<", operation : "<<instruction.name<<", RS1 : "<<instruction.rs1<<", IMM : "<<instruction.imm<<", RD : "<<instruction.rd<<endl;
            break;
        case 0x23:
            instruction.type = "S-Type";
            instruction.rs1 = (ins >> 15) & 0x1F;
            instruction.rs2 = (ins >> 20) & 0x1F;
            instruction.fun3 = (ins >> 12) & 0x7;
            // Extract and combine immediate bits for S-type
            instruction.imm = (((ins >> 25) & 0x7F) << 5) | ((ins >> 7) & 0x1F);
            // Sign extension
            if (instruction.imm & 0x800) {
                instruction.imm |= 0xFFFFF000; // Sign extend
            }
            switch (instruction.fun3) {
                case 0x0:
                    instruction.name = "SB";
                    break;
                case 0x1:
                    instruction.name = "SH";
                    break;
                case 0x2:
                    instruction.name = "SW";
                    break;
                case 0x3:
                    instruction.name = "SD";
                    break;
                default:
                    cout<<"Invalid Store Instruction"<<endl;
            }
            cout<<", operation : "<<instruction.name<<", RS1 : "<<instruction.rs1<<", RS2 : "<<instruction.rs2<<", IMM : "<<instruction.imm<<endl;
            break;
        case 0x63:
            instruction.type = "SB-Type";
            instruction.rs1 = (ins >> 15) & 0x1F;
            instruction.rs2 = (ins >> 20) & 0x1F;
            instruction.fun3 = (ins >> 12) & 0x7;
            // Extract and combine immediate bits for SB-type (branch)
            instruction.imm = (((ins >> 31) & 0x1) << 12) | (((ins >> 7) & 0x1) << 11) | 
                             (((ins >> 25) & 0x3F) << 5) | (((ins >> 8) & 0xF) << 1);
            // Sign extension
            if (instruction.imm & 0x1000) {
                instruction.imm |= 0xFFFFE000; // Sign extend for 13-bit immediate
            }
            switch (instruction.fun3) {
                case 0x0:
                    instruction.name = "BEQ";
                    break;
                case 0x1:
                    instruction.name = "BNE";
                    break;
                case 0x4:
                    instruction.name = "BLT";
                    break;
                case 0x5:
                    instruction.name = "BGE";
                    break;
                default:
                    cout<<"Invalid SB-Type Instruction"<<endl;
            }
            cout<<", operation : "<<instruction.name<<", RS1 : "<<instruction.rs1<<", RS2 : "<<instruction.rs2<<", IMM : "<<instruction.imm<<endl;
            break;
        case 0x67:
            instruction.type = "JALR_I-Type";
            instruction.rd = (ins >> 7) & 0x1F;
            instruction.fun3 = (ins >> 12) & 0x7;
            instruction.rs1 = (ins >> 15) & 0x1F;
            // Extract 12-bit immediate and sign-extend it
            instruction.imm = (ins >> 20) & 0xFFF;
            // Sign extension
            if (instruction.imm & 0x800) {
                instruction.imm |= 0xFFFFF000; // Sign extend
            }
            instruction.name = "JALR";
            cout<<", operation : "<<instruction.name<<", RS1 : "<<instruction.rs1<<", IMM : "<<instruction.imm<<", RD : "<<instruction.rd<<endl;
            break;
        case 0x6F:
            instruction.type = "JAL_J-Type";
            instruction.rd = (ins >> 7) & 0x1F;
            // Extract and combine immediate bits for J-type (JAL)
            instruction.imm = (((ins >> 31) & 0x1) << 20) | (((ins >> 12) & 0xFF) << 12) | 
                             (((ins >> 20) & 0x1) << 11) | (((ins >> 21) & 0x3FF) << 1);
            // Sign extension
            if (instruction.imm & 0x100000) {
                instruction.imm |= 0xFFF00000; // Sign extend for 21-bit immediate
            }
            instruction.name = "JAL";
            cout<<", operation : "<<instruction.name<<", IMM : "<<instruction.imm<<", RD : "<<instruction.rd<<endl;
            break;
        case 0x17:
            instruction.type = "LUI_U-Type";
            instruction.rd = (ins >> 7) & 0x1F;
            // For U-type instructions, the immediate is already in the upper 20 bits
            // No sign extension needed for LUI
            instruction.imm = (ins >> 12) & 0xFFFFF;
            instruction.name = "LUI";
            cout<<", operation : "<<instruction.name<<", IMM : "<<instruction.imm<<", RD : "<<instruction.rd<<endl;
            break;
        case 0x37:
            instruction.type = "AUIPC_U-Type";
            instruction.rd = (ins >> 7) & 0x1F;
            // For U-type instructions, the immediate is already in the upper 20 bits
            // No sign extension needed for AUIPC
            instruction.imm = (ins >> 12) & 0xFFFFF;
            instruction.name = "AUIPC";
            cout<<", operation : "<<instruction.name<<", IMM : "<<instruction.imm<<", RD : "<<instruction.rd<<endl;
            break;
        default:
            cout<<"Invalid Instruction"<<endl;
    }
}

void execute(Instruction instruction) {
    cout << "Executing instruction: " << instruction.name<<", ";
    
    // R-Type instructions
    if (instruction.name == "ADD") {
        result = registerFile[instruction.rs1] + registerFile[instruction.rs2];
        cout << "ADD operation: R" << instruction.rs1 << " (" << registerFile[instruction.rs1] 
             << ") + R" << instruction.rs2 << " (" << registerFile[instruction.rs2] 
             << ") = " << result << endl;
    } else if (instruction.name == "SUB") {
        result = registerFile[instruction.rs1] - registerFile[instruction.rs2];
        cout << "SUB operation: R" << instruction.rs1 << " (" << registerFile[instruction.rs1] 
             << ") - R" << instruction.rs2 << " (" << registerFile[instruction.rs2] 
             << ") = " << result << endl;
    } else if (instruction.name == "MUL") {
        result = registerFile[instruction.rs1] * registerFile[instruction.rs2];
        cout << "MUL operation: R" << instruction.rs1 << " (" << registerFile[instruction.rs1] 
             << ") * R" << instruction.rs2 << " (" << registerFile[instruction.rs2] 
             << ") = " << result << endl;
    } else if (instruction.name == "DIV") {
        if (registerFile[instruction.rs2] == 0) {
            cout << "Error: Division by zero!" << endl;
            result = -1; // Error value
        } else {
            result = registerFile[instruction.rs1] / registerFile[instruction.rs2];
            cout << "DIV operation: R" << instruction.rs1 << " (" << registerFile[instruction.rs1] 
                 << ") / R" << instruction.rs2 << " (" << registerFile[instruction.rs2] 
                 << ") = " << result << endl;
        }
    } else if (instruction.name == "REM") {
        if (registerFile[instruction.rs2] == 0) {
            cout << "Error: Modulo by zero!" << endl;
            result = -1; // Error value
        } else {
            result = registerFile[instruction.rs1] % registerFile[instruction.rs2];
            cout << "REM operation: R" << instruction.rs1 << " (" << registerFile[instruction.rs1] 
                 << ") % R" << instruction.rs2 << " (" << registerFile[instruction.rs2] 
                 << ") = " << result << endl;
        }
    } else if (instruction.name == "XOR") {
        result = registerFile[instruction.rs1] ^ registerFile[instruction.rs2];
        cout << "XOR operation: R" << instruction.rs1 << " (" << registerFile[instruction.rs1] 
             << ") ^ R" << instruction.rs2 << " (" << registerFile[instruction.rs2] 
             << ") = " << result << endl;
    } else if (instruction.name == "OR") {
        result = registerFile[instruction.rs1] | registerFile[instruction.rs2];
        cout << "OR operation: R" << instruction.rs1 << " (" << registerFile[instruction.rs1] 
             << ") | R" << instruction.rs2 << " (" << registerFile[instruction.rs2] 
             << ") = " << result << endl;
    } else if (instruction.name == "AND") {
        result = registerFile[instruction.rs1] & registerFile[instruction.rs2];
        cout << "AND operation: R" << instruction.rs1 << " (" << registerFile[instruction.rs1] 
             << ") & R" << instruction.rs2 << " (" << registerFile[instruction.rs2] 
             << ") = " << result << endl;
    } else if (instruction.name == "SLL") {
        result = registerFile[instruction.rs1] << registerFile[instruction.rs2];
        cout << "SLL operation: R" << instruction.rs1 << " (" << registerFile[instruction.rs1] 
             << ") << R" << instruction.rs2 << " (" << registerFile[instruction.rs2] 
             << ") = " << result << endl;
    } else if (instruction.name == "SRL") {
        result = (unsigned int)registerFile[instruction.rs1] >> registerFile[instruction.rs2];
        cout << "SRL operation: R" << instruction.rs1 << " (" << registerFile[instruction.rs1] 
             << ") >> R" << instruction.rs2 << " (" << registerFile[instruction.rs2] 
             << ") = " << result << endl;
    } else if (instruction.name == "SRA") {
        result = registerFile[instruction.rs1] >> registerFile[instruction.rs2]; // Arithmetic shift
        cout << "SRA operation: R" << instruction.rs1 << " (" << registerFile[instruction.rs1] 
             << ") >> R" << instruction.rs2 << " (" << registerFile[instruction.rs2] 
             << ") = " << result << endl;
    } else if (instruction.name == "SLT") {
        result = (registerFile[instruction.rs1] < registerFile[instruction.rs2]) ? 1 : 0;
        cout << "SLT operation: R" << instruction.rs1 << " (" << registerFile[instruction.rs1] 
             << ") < R" << instruction.rs2 << " (" << registerFile[instruction.rs2] 
             << ") = " << result << endl;
    } 
    
    // I-Type instructions
    else if (instruction.name == "ADDI") {
        result = registerFile[instruction.rs1] + instruction.imm;
        cout << "ADDI operation: R" << instruction.rs1 << " (" << registerFile[instruction.rs1] 
             << ") + " << instruction.imm << " = " << result << endl;
    } else if (instruction.name == "ORI") {
        result = registerFile[instruction.rs1] | instruction.imm;
        cout << "ORI operation: R" << instruction.rs1 << " (" << registerFile[instruction.rs1] 
             << ") | " << instruction.imm << " = " << result << endl;
    } else if (instruction.name == "ANDI") {
        result = registerFile[instruction.rs1] & instruction.imm;
        cout << "ANDI operation: R" << instruction.rs1 << " (" << registerFile[instruction.rs1] 
             << ") & " << instruction.imm << " = " << result << endl;
    } 
    
    // Load instructions - address calculation
    else if (instruction.name == "LB" || instruction.name == "LH" || 
             instruction.name == "LW" || instruction.name == "LD") {
        result = registerFile[instruction.rs1] + instruction.imm; // Calculate memory address
        cout << instruction.name << " operation: Address calculation - R" << instruction.rs1 
             << " (" << registerFile[instruction.rs1] << ") + " << instruction.imm 
             << " = " << result << endl;
    } 
    
    // Store instructions - address calculation
    else if (instruction.name == "SB" || instruction.name == "SH" || 
             instruction.name == "SW" || instruction.name == "SD") {
        result = registerFile[instruction.rs1] + instruction.imm; // Calculate memory address
        cout << instruction.name << " operation: Address calculation - R" << instruction.rs1 
             << " (" << registerFile[instruction.rs1] << ") + " << instruction.imm 
             << " = " << result << endl;
    } 
    
    // Branch instructions
    else if (instruction.name == "BEQ") {
        result = (registerFile[instruction.rs1] == registerFile[instruction.rs2]) ? 1 : 0;
        cout << "BEQ operation: Compare R" << instruction.rs1 << " (" << registerFile[instruction.rs1] 
             << ") == R" << instruction.rs2 << " (" << registerFile[instruction.rs2] 
             << ") = " << (result ? "True" : "False") << endl;
    } else if (instruction.name == "BNE") {
        result = (registerFile[instruction.rs1] != registerFile[instruction.rs2]) ? 1 : 0;
        cout << "BNE operation: Compare R" << instruction.rs1 << " (" << registerFile[instruction.rs1] 
             << ") != R" << instruction.rs2 << " (" << registerFile[instruction.rs2] 
             << ") = " << (result ? "True" : "False") << endl;
    } else if (instruction.name == "BLT") {
        result = (registerFile[instruction.rs1] < registerFile[instruction.rs2]) ? 1 : 0;
        cout << "BLT operation: Compare R" << instruction.rs1 << " (" << registerFile[instruction.rs1] 
             << ") < R" << instruction.rs2 << " (" << registerFile[instruction.rs2] 
             << ") = " << (result ? "True" : "False") << endl;
    } else if (instruction.name == "BGE") {
        result = (registerFile[instruction.rs1] >= registerFile[instruction.rs2]) ? 1 : 0;
        cout << "BGE operation: Compare R" << instruction.rs1 << " (" << registerFile[instruction.rs1] 
             << ") >= R" << instruction.rs2 << " (" << registerFile[instruction.rs2] 
             << ") = " << (result ? "True" : "False") << endl;
    } 
    
    // Jump instructions
    else if (instruction.name == "JAL") {
        // PC-relative jump
        result = stoi(currentPC, nullptr, 16) + 4; // Store return address (PC + 4)
        cout << "JAL operation: Return address calculation - PC (" << currentPC 
             << ") + 4 = " << "0x" << hex << result << dec << endl;
    } else if (instruction.name == "JALR") {
        // Jump to register + immediate
        result = stoi(currentPC, nullptr, 16) + 4; // Store return address (PC + 4)
        cout << "JALR operation: Return address calculation - PC (" << currentPC 
             << ") + 4 = " << "0x" << hex << result << dec << endl;
    } 
    
    // Upper immediate instructions
    else if (instruction.name == "LUI") {
        result = instruction.imm << 12; // Load upper immediate
        cout << "LUI operation: " << instruction.imm << " << 12 = " << result << endl;
    } else if (instruction.name == "AUIPC") {
        result = stoi(currentPC, nullptr, 16) + (instruction.imm << 12); // Add upper immediate to PC
        cout << "AUIPC operation: PC (" << currentPC << ") + (" << instruction.imm 
             << " << 12) = " << "0x" << hex << result << dec << endl;
    } else {
        cout << "Unknown instruction: " << instruction.name << endl;
    }
}

// Add this function to handle memory access
void memoryAccess(Instruction instruction) {
    cout << "Memory Access Stage for instruction: " << instruction.name << ", ";
    
    unsigned int address = static_cast<unsigned int>(result);
    
    // Load instructions
    if (instruction.name == "LB") {
        // Load byte (8 bits)
        result = static_cast<int8_t>(dataMemory[address]); // Sign extend
        cout << "LB: Loading byte from address 0x" << hex << address << ": " << dec << result << endl;
    } 
    else if (instruction.name == "LH") {
        // Load half-word (16 bits)
        int16_t value = 0;
        for (int i = 0; i < 2; i++) {
            value |= (static_cast<int16_t>(dataMemory[address + i]) << (8 * i));
        }
        result = value; // Sign extend
        cout << "LH: Loading half-word from address 0x" << hex << address << ": " << dec << result << endl;
    } 
    else if (instruction.name == "LW") {
        // Load word (32 bits)
        int32_t value = 0;
        for (int i = 0; i < 4; i++) {
            value |= (static_cast<int32_t>(dataMemory[address + i]) << (8 * i));
        }
        result = value;
        cout << "LW: Loading word from address 0x" << hex << address << ": " << dec << result << endl;
    } 
    else if (instruction.name == "LD") {
        // Load double-word (64 bits)
        int64_t value = 0;
        for (int i = 0; i < 8; i++) {
            value |= (static_cast<int64_t>(dataMemory[address + i]) << (8 * i));
        }
        result = value;
        cout << "LD: Loading double-word from address 0x" << hex << address << ": " << dec << result << endl;
    } 
    // Store instructions
    else if (instruction.name == "SB") {
        // Store byte (8 bits)
        dataMemory[address] = registerFile[instruction.rs2] & 0xFF;
        cout << "SB: Storing byte to address 0x" << hex << address << ": " 
             << (registerFile[instruction.rs2] & 0xFF) << dec << endl;
    } 
    else if (instruction.name == "SH") {
        // Store half-word (16 bits)
        for (int i = 0; i < 2; i++) {
            dataMemory[address + i] = (registerFile[instruction.rs2] >> (8 * i)) & 0xFF;
        }
        cout << "SH: Storing half-word to address 0x" << hex << address << ": " 
             << (registerFile[instruction.rs2] & 0xFFFF) << dec << endl;
    } 
    else if (instruction.name == "SW") {
        // Store word (32 bits)
        for (int i = 0; i < 4; i++) {
            dataMemory[address + i] = (registerFile[instruction.rs2] >> (8 * i)) & 0xFF;
        }
        cout << "SW: Storing word to address 0x" << hex << address << ": " 
             << registerFile[instruction.rs2] << dec << endl;
    } 
    else if (instruction.name == "SD") {
        // Store double-word (64 bits)
        for (int i = 0; i < 8; i++) {
            dataMemory[address + i] = (registerFile[instruction.rs2] >> (8 * i)) & 0xFF;
        }
        cout << "SD: Storing double-word to address 0x" << hex << address << ": " 
             << registerFile[instruction.rs2] << dec << endl;
    }
    // For non-memory instructions, this stage is a pass-through
    else {
        cout << "No memory access needed for this instruction" << endl;
    }
}

// Add this function to handle the write-back stage
void writeBack(Instruction instruction) {
    cout << "Register Write-Back Stage for instruction: " << instruction.name << ", ";
    
    // Instructions that write to a register
    if (instruction.type == "R-Type" || 
        instruction.type == "I-Type" || 
        instruction.type == "Load_I-Type" || 
        instruction.type == "JALR_I-Type" || 
        instruction.type == "JAL_J-Type" || 
        instruction.type == "LUI_U-Type" || 
        instruction.type == "AUIPC_U-Type") {
        
        // Don't write to register 0 (hardwired to 0 in RISC-V)
        if (instruction.rd != 0) {
            registerFile[instruction.rd] = result;
            cout << "Writing " << result << " to R" << instruction.rd << endl;
        } else {
            cout << "Skipping write to R0 (hardwired to 0)" << endl;
        }
    } else {
        cout << "No register write-back needed for this instruction" << endl;
    }
}

// Add this function to handle PC update
void updatePC(Instruction instruction) {
    unsigned int pc_val = stoul(currentPC.substr(2), nullptr, 16);
    string nextPC;
    
    if ((instruction.name == "BEQ" && result == 1) ||
        (instruction.name == "BNE" && result == 1) ||
        (instruction.name == "BLT" && result == 1) ||
        (instruction.name == "BGE" && result == 1)) {
        int offset = instruction.imm;
        if ((offset >> 11) & 1) {
            offset |= 0xFFFFF000;
        }
        pc_val += offset;
        stringstream ss;
        ss << hex << pc_val;
        nextPC = "0x" + ss.str();
        cout << "Branch taken! New PC: " << nextPC << endl;
    } else if (instruction.name == "JAL") {
        int offset = instruction.imm;
        if ((offset >> 20) & 1) {
            offset |= 0xFFE00000;
        }
        pc_val += offset;
        stringstream ss;
        ss << hex << pc_val;
        nextPC = "0x" + ss.str();
        cout << "JAL jump! New PC: " << nextPC << endl;
    } else if (instruction.name == "JALR") {
        int offset = instruction.imm;
        if ((offset >> 11) & 1) {
            offset |= 0xFFFFF000;
        }
        pc_val = (registerFile[instruction.rs1] + offset) & ~1;
        stringstream ss;
        ss << hex << pc_val;
        nextPC = "0x" + ss.str();
        cout << "JALR jump! New PC: " << nextPC << endl;
    } else {
        pc_val += 4;
        stringstream ss;
        ss << hex << pc_val;
        nextPC = "0x" + ss.str();
    }
    if (nextPC == currentPC) infLoop = true;
    currentPC = nextPC;
    cout << "Updated PC to: " << currentPC << endl;
}
// Add this function to dump memory to a file
void dumpMemoryToFile(const string &filename) {
    ofstream outFile(filename);
    if (!outFile.is_open()) {
        cerr << "Error opening file for memory dump: " << filename << endl;
        return;
    }

    vector<pair<unsigned int, unsigned char>> memoryEntries;
    for (const auto &entry : dataMemory) {
        memoryEntries.push_back(entry);
    }

    // Sort by address
    sort(memoryEntries.begin(), memoryEntries.end());

    // Group into 4-byte words for output
    unsigned int currentAddr = 0;
    unsigned int word = 0;
    int byteCount = 0;

    for (const auto &entry : memoryEntries) {
        unsigned int addr = entry.first;
        
        // If we're starting a new word
        if (byteCount == 0 || addr != currentAddr + byteCount) {
            // Output previous word if we had one
            if (byteCount > 0) {
                outFile << "0x" << hex << setw(8) << setfill('0') << currentAddr << ", 0x" 
                       << setw(8) << setfill('0') << word << endl;
            }
            
            // Start a new word
            currentAddr = addr - (addr % 4); // Align to 4-byte boundary
            word = 0;
            byteCount = 0;
        }
        
        // Add this byte to the word
        int bytePosition = addr % 4;
        word |= (static_cast<unsigned int>(entry.second) << (8 * bytePosition));
        byteCount++;
        
        // If we've collected 4 bytes, output the word
        if (byteCount == 4) {
            outFile << "0x" << hex << setw(8) << setfill('0') << currentAddr << ", 0x" 
                   << setw(8) << setfill('0') << word << endl;
            byteCount = 0;
        }
    }
    
    // Output final word if incomplete
    if (byteCount > 0) {
        outFile << "0x" << hex << setw(8) << setfill('0') << currentAddr << ", 0x" 
               << setw(8) << setfill('0') << word << endl;
    }
    
    outFile.close();
    cout << "Memory dumped to " << filename << endl;
}

int main() {
    for (int i = 0; i < 32; i++) {
        registerFile[i] = 0;
    }
    
    // Load machine code
    loadMC("input.mc");
    int clockCycle = 0;
    bool exitSimulator = false;
    // Main execution loop
    while (!infLoop && !exitSimulator && pcMachineCode.find(currentPC) != pcMachineCode.end()) {
        cout << "\n================ Clock Cycle: " << clockCycle << " ================" << endl;
        cout<<pcMachineCode[currentPC]<<endl;
        // Stage 1: Fetch
        fetchInstruction();
        
        // Stage 2: Decode
        decodeInstruction();
        
        // Check for custom exit instruction (you can define a specific instruction or pattern)
        if (instruction.name == "ADDI" && instruction.rs1 == 0 && instruction.rd == 0 && instruction.imm == 1) {
            cout << "Exit instruction detected. Terminating simulation." << endl;
            exitSimulator = true;
            break;
        }
        
        // Stage 3: Execute
        execute(instruction);
        
        // Stage 4: Memory Access
        memoryAccess(instruction);
        
        // Stage 5: Write Back
        writeBack(instruction);
        
        // Update PC for next instruction
        updatePC(instruction);
        
        clockCycle++;
    }

    for (int i = 0; i < 32; i++) {
        cout << "R" << i << ": " << registerFile[i] << endl;
    }
    
    cout << "\nSimulation completed in " << clockCycle << " clock cycles." << endl;
    
    // Dump data memory to file
    dumpMemoryToFile("output.mc");
    
    return 0;
}




