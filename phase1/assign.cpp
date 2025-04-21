#include<assign.h>

using namespace std;

#define LongInt long long
#define MAX_ERROR_VALUE 1e18

// Helper function to convert numeric strings to integers
LongInt parseValue(string valueStr) {
    LongInt result = 0;
    
    // Handle empty case
    if (valueStr.empty()) {
        return MAX_ERROR_VALUE;
    }
    
    // Handle zero
    if (valueStr == "0") {
        return 0;
    }
    
    // Handle negative numbers
    if (valueStr[0] == '-') {
        string numericPortion = valueStr.substr(1);
        try {
            result = -stoi(numericPortion);
            // Size constraint check
            if (abs(result) > (1ll << 32)) {
                return MAX_ERROR_VALUE;
            }
        } catch (...) {
            return MAX_ERROR_VALUE;
        }
        return result;
    }
    
    // Handle hexadecimal format (0x...)
    if (valueStr.size() >= 2 && valueStr[0] == '0' && valueStr[1] == 'x') {
        if (valueStr.size() > 10) {
            return MAX_ERROR_VALUE;
        }
        
        try {
            string hexPortion = valueStr.substr(2);
            result = stoi(hexPortion, 0, 16);
        } catch (...) {
            return MAX_ERROR_VALUE;
        }
        return result;
    }
    
    // Handle binary format (0...)
    if (valueStr.size() >= 1 && valueStr[0] == '0' && valueStr.size() > 1) {
        if (valueStr.size() > 33) {
            return MAX_ERROR_VALUE;
        }
        
        try {
            string binaryPortion = valueStr.substr(1);
            result = stoi(binaryPortion, 0, 2);
        } catch (...) {
            return MAX_ERROR_VALUE;
        }
        return result;
    }
    
    // Handle ASCII interpretation - non-numeric first character
    if (valueStr[0] < '0' || valueStr[0] > '9') {
        for (char c : valueStr) {
            result += static_cast<LongInt>(c);
        }
        return result;
    }
    
    // Handle regular integer
    try {
        result = stoi(valueStr);
    } catch (...) {
        return MAX_ERROR_VALUE;
    }
    
    return result;
}

// Maps from instruction name to 7-bit opcode
string determineOpcode(string mnemonic, string binaryStr) {
    // Create lookup table for opcodes
    const unordered_map<string, string> opcodeMap = {
        // R-type instructions
        {"add", "0110011"}, {"sub", "0110011"}, {"mul", "0110011"}, {"div", "0110011"},
        {"rem", "0110011"}, {"sll", "0110011"}, {"srl", "0110011"}, {"sra", "0110011"},
        {"slt", "0110011"}, {"xor", "0110011"}, {"or", "0110011"}, {"and", "0110011"},
        
        // I-type instructions
        {"addi", "0010011"}, {"andi", "0010011"}, {"ori", "0010011"},
        {"jalr", "1100111"},
        
        // Load instructions
        {"lb", "0000011"}, {"lh", "0000011"}, {"lw", "0000011"}, {"ld", "0000011"},
        
        // S-type instructions
        {"sb", "0100011"}, {"sh", "0100011"}, {"sw", "0100011"}, {"sd", "0100011"},
        
        // B-type instructions
        {"beq", "1100011"}, {"bne", "1100011"}, {"blt", "1100011"}, {"bge", "1100011"},
        
        // U-type instructions
        {"auipc", "0010111"}, {"lui", "0110111"},
        
        // J-type instruction
        {"jal", "1101111"}
    };
    
    auto it = opcodeMap.find(mnemonic);
    if (it != opcodeMap.end()) {
        return binaryStr + it->second;
    }
    
    return "error";
}

// Determine the appropriate funct7 field based on instruction type
string determineFunct7(string mnemonic, string binaryStr) {
    if (binaryStr == "error") {
        return binaryStr;
    }
    
    // Define groups of instructions that share funct7 values
    const unordered_map<string, string> funct7Map = {
        // Standard operations (0000000)
        {"add", "0000000"}, {"sll", "0000000"}, {"slt", "0000000"}, 
        {"xor", "0000000"}, {"srl", "0000000"}, {"or", "0000000"}, 
        {"and", "0000000"},
        
        // Alternate operations (0100000)
        {"sub", "0100000"}, {"sra", "0100000"},
        
        // M-extension operations (0000001)
        {"mul", "0000001"}, {"div", "0000001"}, {"rem", "0000001"}
    };
    
    auto it = funct7Map.find(mnemonic);
    if (it != funct7Map.end()) {
        return binaryStr + it->second;
    }
    
    return "error";
}

// Determine the 3-bit function code for the instruction
string determineFunct3(string mnemonic, string binaryStr) {
    // Group instructions by their funct3 values
    const unordered_map<string, string> funct3Map = {
        // 000 group
        {"add", "000"}, {"sub", "000"}, {"mul", "000"}, {"lb", "000"},
        {"sb", "000"}, {"jalr", "000"}, {"addi", "000"}, {"beq", "000"},
        
        // 001 group
        {"sll", "001"}, {"sh", "001"}, {"lh", "001"}, {"bne", "001"},
        
        // 010 group
        {"slt", "010"}, {"sw", "010"}, {"lw", "010"},
        
        // 011 group
        {"sd", "011"}, {"ld", "011"},
        
        // 100 group
        {"xor", "100"}, {"div", "100"}, {"blt", "100"},
        
        // 101 group
        {"srl", "101"}, {"sra", "101"}, {"bge", "101"},
        
        // 110 group
        {"or", "110"}, {"rem", "110"}, {"ori", "110"},
        
        // 111 group
        {"and", "111"}, {"andi", "111"}
    };
    
    auto it = funct3Map.find(mnemonic);
    if (it != funct3Map.end()) {
        return binaryStr + it->second;
    }
    
    return binaryStr;
}

// Encode standard immediate values (12-bit)
string encodeImmediate(string valueStr, string binaryStr) {
    if (binaryStr == "error") {
        return binaryStr;
    }
    
    LongInt value = parseValue(valueStr);
    if (value == MAX_ERROR_VALUE) {
        return "error";
    }
    
    // Check range
    const LongInt maxImm = (1ll << 11) - 1;
    const LongInt minImm = -(1ll << 11);
    
    if (value > maxImm || value < minImm) {
        return "error";
    }
    
    // Handle negative values
    if (value < 0) {
        value += (1ll << 12);
    }
    
    // Convert to binary representation
    string immBits;
    for (int i = 0; i < 12; i++) {
        immBits = ((value & 1) ? "1" : "0") + immBits;
        value >>= 1;
    }
    
    return binaryStr + immBits;
}

// Encode branch immediate values (13-bit)
string encodeBranchImmediate(string valueStr, string binaryStr) {
    if (binaryStr == "error") {
        return binaryStr;
    }
    
    LongInt value = parseValue(valueStr);
    if (value == MAX_ERROR_VALUE) {
        return "error";
    }
    
    // Check range (13-bit signed immediate)
    const LongInt maxBranchImm = (1ll << 12) - 1;
    const LongInt minBranchImm = -(1ll << 12);
    
    if (value > maxBranchImm || value < minBranchImm) {
        return "error";
    }
    
    // Handle negative values
    if (value < 0) {
        value += (1ll << 13);
    }
    
    // Convert to binary string
    string branchBits;
    for (int i = 0; i < 13; i++) {
        branchBits = ((value & 1) ? "1" : "0") + branchBits;
        value >>= 1;
    }
    
    return binaryStr + branchBits;
}

// Encode upper immediate values (20-bit shifted)
string encodeUpperImmediate(string valueStr, string binaryStr) {
    if (binaryStr == "error") {
        return binaryStr;
    }
    
    LongInt value = parseValue(valueStr);
    if (value == MAX_ERROR_VALUE) {
        return "error";
    }
    
    // Check valid range for upper immediate (20-bit value)
    const LongInt maxUpperImm = (1ll << 20) - 1;
    
    if (value < 0 || value > maxUpperImm) {
        if (value < 0) {
            value += (1ll << 12);
        } else if (value > maxUpperImm) {
            return "error";
        }
    }
    
    // Shift for upper immediate format
    value <<= 12;
    
    // Generate binary string for upper 20 bits
    string upperImmBits;
    for (int i = 31; i >= 12; i--) {
        upperImmBits += ((value & (1ll << i)) ? "1" : "0");
    }
    
    return binaryStr + upperImmBits;
}
