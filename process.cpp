#include<process.h>
#include<assign.h>
#include<encode.h>
#include<map>
#include<vector>
#include<algorithm>
#include<sstream>

typedef long long LongInt;
using namespace std;

void processDataDirective(string line, LongInt memory[], LongInt &dataSize, LongInt MemoryStart, map<string, LongInt> &symbolTable) {
    istringstream tokenizer(line);
    string token, labelStr;
    
    // Extract label information
    tokenizer >> token;
    labelStr = token;

    // Handle label format with colon
    if (labelStr.back() == ':') {
        labelStr = labelStr.substr(0, labelStr.size() - 1);
    }
    
    // Register label in symbol table
    symbolTable[labelStr] = MemoryStart + dataSize;
    
    // Skip directive if label has no colon
    if (token.back() != ':') {
        tokenizer >> token;
    }

    // Get the directive type
    tokenizer >> token;
    
    // Process byte directive
    if (token == ".byte") {
        string valueToken;
        while (tokenizer >> valueToken) {
            if (valueToken != ",") {
                memory[dataSize++] = parseValue(valueToken);
            }
        }
    }
    // Process half-word directive
    else if (token == ".half") {
        string valueToken;
        while (tokenizer >> valueToken) {
            if (valueToken != ",") {
                LongInt value = parseValue(valueToken);
                
                // Store 16-bit value as 2 bytes (little-endian)
                for (int byteIndex = 0; byteIndex < 2; byteIndex++) {
                    memory[dataSize++] = value & 0xFF;
                    value >>= 8;
                }
            }
        }
    }
    // Process word directive
    else if (token == ".word") {
        string valueToken;
        while (tokenizer >> valueToken) {
            if (valueToken != ",") {
                LongInt value = parseValue(valueToken);
                
                // Store 32-bit value as 4 bytes (little-endian)
                for (int i = 0; i < 4; i++) {
                    memory[dataSize++] = value & 0xFF;
                    value >>= 8;
                }
            }
        }
    }
    // Process double-word directive
    else if (token == ".dword") {
        string valueToken;
        while (tokenizer >> valueToken) {
            if (valueToken != ",") {
                LongInt value = parseValue(valueToken);
                
                // Store 64-bit value as 8 bytes (little-endian)
                for (int byteIndex = 0; byteIndex < 8; byteIndex++) {
                    memory[dataSize++] = value & 0xFF;
                    value >>= 8;
                }
            }
        }
    }
    // Process null-terminated string directive
    else if (token == ".asciiz") {
        string valueToken;
        while (tokenizer >> valueToken) {
            if (valueToken != ",") {
                // Skip the quotes and store each character
                for (size_t charIndex = 1; charIndex < valueToken.size() - 1; charIndex++) {
                    memory[dataSize++] = static_cast<LongInt>(valueToken[charIndex]);
                }
            }
        }
    }
}

void processRType(string instruction, vector<string> &binaryInstructions, vector<string> &decodedInstructions, 
                 vector<string> &assemblyLines, vector<LongInt> &instructionAddresses, 
                 LongInt instructionPointer, unordered_map<string, string> &registerMap) {
    // Initialize fields array for R-type instruction components
    vector<string> fields(6, "");
    string binaryEncoding;
    
    // Parse instruction components
    istringstream tokenizer(instruction);
    string token, opcode;
    
    // Get operation code
    tokenizer >> opcode;
    
    // Set opcode field
    fields[0] = determineOpcode(opcode, fields[0]);
    
    // Extract destination register
    tokenizer >> token;
    fields[1] = encodeRegister(token, fields[1], registerMap);
    
    // Extract first source register
    tokenizer >> token;
    if (token == ",") tokenizer >> token;
    fields[3] = encodeRegister(token, fields[3], registerMap);
    
    // Extract second source register
    tokenizer >> token;
    if (token == ",") tokenizer >> token;
    fields[4] = encodeRegister(token, fields[4], registerMap);
    
    // Determine function codes
    fields[5] = determineFunct7(opcode, fields[5]);
    fields[2] = determineFunct3(opcode, fields[2]);
    
    // Construct binary encoding
    for (int fieldIndex = 5; fieldIndex >= 0; fieldIndex--) {
        if (fields[fieldIndex] == "error") {
            binaryInstructions.push_back("error");
            return;
        }
        binaryEncoding += fields[fieldIndex];
    }
    
    // Store results
    binaryInstructions.push_back(binaryEncoding);
    
    // Construct decoded representation
    string decoded = fields[0] + "-" + fields[2] + "-" + fields[5] + "-" + 
                    fields[1] + "-" + fields[3] + "-" + fields[4] + "-" + "NULL";
    
    // Save instruction information
    decodedInstructions.push_back(decoded);
    assemblyLines.push_back(instruction);
    instructionAddresses.push_back(instructionPointer);
}

void processIType(string instruction, vector<string> &binaryInstructions, vector<string> &decodedInstructions, 
                 vector<string> &assemblyLines, vector<LongInt> &instructionAddresses, 
                 LongInt instructionPointer, unordered_map<string, string> &registerMap) {
    // Parse the I-type instruction
    istringstream tokenizer(instruction);
    string token, opcode;
    
    // Extract operation
    tokenizer >> opcode;
    
    // Initialize fields for I-type format
    vector<string> fields(5, "");
    fields[0] = determineOpcode(opcode, fields[0]);
    
    // Get destination register
    tokenizer >> token;
    fields[1] = encodeRegister(token, fields[1], registerMap);
    
    // Get source register
    tokenizer >> token;
    if (token == ",") tokenizer >> token;
    fields[3] = encodeRegister(token, fields[3], registerMap);
    
    // Get immediate value
    tokenizer >> token;
    if (token == ",") tokenizer >> token;
    fields[4] = encodeImmediate(token, fields[4]);
    
    // Set function code
    fields[2] = determineFunct3(opcode, fields[2]);
    
    // Build binary encoding
    string binaryEncoding;
    bool hasError = false;
    
    for (int i = 4; i >= 0; i--) {
        if (fields[i] == "error") {
            hasError = true;
            break;
        }
        binaryEncoding += fields[i];
    }
    
    if (hasError) {
        binaryInstructions.push_back("error");
        return;
    }
    
    // Store results
    binaryInstructions.push_back(binaryEncoding);
    
    // Create decoded representation
    string decoded = fields[0] + "-" + fields[2] + "-" + "NULL" + "-" + 
                    fields[1] + "-" + fields[3] + "-" + "NULL" + "-" + fields[4];
                    
    decodedInstructions.push_back(decoded);
    assemblyLines.push_back(instruction);
    instructionAddresses.push_back(instructionPointer);
}

void processLoadType(string instruction, vector<string> &binaryInstructions, vector<string> &decodedInstructions, 
                    vector<string> &assemblyLines, vector<LongInt> &instructionAddresses, 
                    LongInt instructionPointer, unordered_map<string, string> &registerMap) {
    // Initialize components
    vector<string> fields(6, "");
    istringstream tokenizer(instruction);
    string token, opcode;
    
    // Get opcode
    tokenizer >> opcode;
    fields[0] = determineOpcode(opcode, fields[0]);
    
    // Get destination register
    tokenizer >> token;
    if (token == ",") tokenizer >> token;
    fields[1] = encodeRegister(token, fields[1], registerMap);
    
    // Set function code
    fields[2] = determineFunct3(opcode, fields[2]);
    
    // Get memory address expression
    tokenizer >> token;
    if (token == ",") tokenizer >> token;
    
    // Parse offset(register) format
    string offset, reg;
    size_t openParenPos = token.find('(');
    offset = token.substr(0, openParenPos);
    
    // Extract register between parentheses
    size_t closeParenPos = token.find(')');
    reg = token.substr(openParenPos + 1, closeParenPos - openParenPos - 1);
    
    // Encode base register and offset
    fields[3] = encodeRegister(reg, fields[3], registerMap);
    fields[4] = encodeImmediate(offset, fields[4]);
    
    // Construct binary encoding
    string binaryEncoding;
    for (int i = 4; i >= 0; i--) {
        if (fields[i] == "error") {
            binaryInstructions.push_back("error");
            return;
        }
        binaryEncoding += fields[i];
    }
    
    // Save instruction information
    binaryInstructions.push_back(binaryEncoding);
    
    // Create decoded representation
    string decoded = fields[0] + "-" + fields[2] + "-" + "NULL" + "-" + 
                    fields[1] + "-" + fields[3] + "-" + "NULL" + "-" + fields[4];
                    
    decodedInstructions.push_back(decoded);
    assemblyLines.push_back(instruction);
    instructionAddresses.push_back(instructionPointer);
}

void processStoreType(string instruction, vector<string> &binaryInstructions, vector<string> &decodedInstructions, 
                     vector<string> &assemblyLines, vector<LongInt> &instructionAddresses, 
                     LongInt instructionPointer, unordered_map<string, string> &registerMap) {
    // Setup instruction components
    vector<string> fields(6, "");
    string token, opcode;
    istringstream tokenizer(instruction);
    
    // Get operation
    tokenizer >> opcode;
    
    // Set opcode and function fields
    fields[0] = determineOpcode(opcode, fields[0]);
    fields[2] = determineFunct3(opcode, fields[2]);
    
    // Get source register (value to store)
    tokenizer >> token;
    if (token == ",") tokenizer >> token;
    fields[4] = encodeRegister(token, fields[4], registerMap);
    
    // Get memory address expression
    tokenizer >> token;
    if (token == ",") tokenizer >> token;
    
    // Extract offset and base register
    string offset, reg;
    size_t idx = 0;
    
    // Find opening parenthesis
    while (idx < token.size() && token[idx] != '(') {
        offset += token[idx++];
    }
    
    // Skip opening parenthesis
    idx++;
    
    // Extract register name
    while (idx < token.size() && token[idx] != ')') {
        reg += token[idx++];
    }
    
    // Encode base register and offset
    fields[3] = encodeRegister(reg, fields[3], registerMap);
    fields[5] = encodeImmediate(offset, fields[5]);
    
    // Handle S-type immediate split (7+5 bits)
    string immediateBits = fields[5];
    fields[5] = immediateBits.substr(0, 7); // Upper 7 bits
    fields[1] = immediateBits.substr(7, 5); // Lower 5 bits
    
    // Construct binary encoding
    string binaryEncoding;
    bool hasError = false;
    
    for (int i = 5; i >= 0; i--) {
        if (fields[i] == "error") {
            hasError = true;
            break;
        }
        binaryEncoding += fields[i];
    }
    
    if (hasError) {
        binaryInstructions.push_back("error");
        return;
    }
    
    // Store instruction information
    binaryInstructions.push_back(binaryEncoding);
    
    // Create decoded representation
    string fullImmediate = fields[5] + fields[1];
    string decoded = fields[0] + "-" + fields[2] + "-" + "NULL" + "-" + 
                    "NULL" + "-" + fields[3] + "-" + fields[4] + "-" + fullImmediate;
                    
    decodedInstructions.push_back(decoded);
    assemblyLines.push_back(instruction);
    instructionAddresses.push_back(instructionPointer);
}

void processBranchType(string instruction, vector<string> &binaryInstructions, vector<string> &decodedInstructions, 
                      vector<string> &assemblyLines, vector<LongInt> &instructionAddresses, 
                      LongInt instructionPointer, map<string, LongInt> &symbolTable, 
                      unordered_map<string, string> &registerMap) {
    // Initialize fields for branch instruction
    vector<string> fields(6, "");
    string token, opcode;
    istringstream tokenizer(instruction);
    
    // Extract operation
    tokenizer >> opcode;
    
    // Set opcode and function code
    fields[0] = determineOpcode(opcode, fields[0]);
    fields[2] = determineFunct3(opcode, fields[2]);
    
    // Get first source register
    tokenizer >> token;
    fields[3] = encodeRegister(token, fields[3], registerMap);
    
    // Get second source register
    tokenizer >> token;
    if (token == ",") tokenizer >> token;
    fields[4] = encodeRegister(token, fields[4], registerMap);
    
    // Get branch target
    tokenizer >> token;
    if (token == ",") tokenizer >> token;
    
    // Check if target is valid
    bool isNumericTarget = all_of(token.begin(), token.end(), ::isdigit);
    bool isSymbolTarget = symbolTable.find(token) != symbolTable.end();
    
    if (!isNumericTarget && !isSymbolTarget) {
        binaryInstructions.push_back("error");
        return;
    }
    
    // Calculate branch offset
    LongInt immediate;
    if (isSymbolTarget) {
        immediate = symbolTable[token] - instructionPointer;
    } else {
        immediate = stoi(token);
    }
    
    // Verify 4-byte alignment
    if (immediate % 4 != 0) {
        binaryInstructions.push_back("error");
        return;
    }
    
    // Encode branch immediate
    string offsetStr = to_string(immediate);
    fields[5] = encodeBranchImmediate(offsetStr, fields[5]);
    
    // Rearrange immediate bits for SB-type format
    string immediateEncoding = fields[5];
    
    // Format: imm[12|10:5] rs2 rs1 funct3 imm[4:1|11] opcode
    fields[5] = immediateEncoding.substr(0, 1);          // imm[12]
    fields[5] += immediateEncoding.substr(2, 6);         // imm[10:5]
    
    fields[1] = immediateEncoding.substr(8, 4);          // imm[4:1]
    fields[1] += immediateEncoding.substr(1, 1);         // imm[11]
    
    // Construct binary encoding
    string binaryEncoding;
    for (int i = 5; i >= 0; i--) {
        if (fields[i] == "error") {
            binaryInstructions.push_back("error");
            return;
        }
        binaryEncoding += fields[i];
    }
    
    // Store instruction information
    binaryInstructions.push_back(binaryEncoding);
    
    // Construct full immediate for decoded form
    string fullImmediate = fields[5].substr(0, 1) + fields[1].substr(4, 1) + 
                           fields[5].substr(1) + fields[1].substr(0, 4);
                           
    // Create decoded representation
    string decoded = fields[0] + "-" + fields[2] + "-" + "NULL" + "-" + 
                    "NULL" + "-" + fields[3] + "-" + fields[4] + "-" + fullImmediate;
                    
    decodedInstructions.push_back(decoded);
    assemblyLines.push_back(instruction);
    instructionAddresses.push_back(instructionPointer);
}

void processUpperImmediate(string instruction, vector<string> &binaryInstructions, vector<string> &decodedInstructions, 
                          vector<string> &assemblyLines, vector<LongInt> &instructionAddresses, 
                          LongInt instructionPointer, unordered_map<string, string> &registerMap) {
    // U-type instruction handling (LUI, AUIPC)
    vector<string> fields(3, "");
    string token, opcode;
    istringstream tokenizer(instruction);
    
    // Extract operation
    tokenizer >> opcode;
    fields[0] = determineOpcode(opcode, fields[0]);
    
    // Get destination register
    tokenizer >> token;
    if (token == ",") tokenizer >> token;
    fields[1] = encodeRegister(token, fields[1], registerMap);
    
    // Get upper immediate value
    tokenizer >> token;
    if (token == ",") tokenizer >> token;
    fields[2] = encodeUpperImmediate(token, fields[2]);
    
    // Construct binary encoding
    string binaryEncoding;
    bool hasError = false;
    
    for (int i = 2; i >= 0; i--) {
        if (fields[i] == "error") {
            hasError = true;
            break;
        }
        binaryEncoding += fields[i];
    }
    
    if (hasError) {
        binaryInstructions.push_back("error");
        return;
    }
    
    // Store instruction information
    binaryInstructions.push_back(binaryEncoding);
    
    // Create decoded representation
    string decoded = fields[0] + "-" + "NULL" + "-" + "NULL" + "-" + 
                    fields[1] + "-" + "NULL" + "-" + "NULL" + "-" + fields[2];
                    
    decodedInstructions.push_back(decoded);
    assemblyLines.push_back(instruction);
    instructionAddresses.push_back(instructionPointer);
}

void processJumpType(string instruction, vector<string> &binaryInstructions, vector<string> &decodedInstructions, 
                    vector<string> &assemblyLines, vector<LongInt> &instructionAddresses, 
                    LongInt instructionPointer, map<string, LongInt> &symbolTable, 
                    unordered_map<string, string> &registerMap) {
    // Process JAL/JALR instructions
    vector<string> fields(3, "");
    string token, opcode;
    istringstream tokenizer(instruction);
    
    // Extract operation
    tokenizer >> opcode;
    fields[0] = determineOpcode(opcode, fields[0]);
    
    // Get destination register
    tokenizer >> token;
    if (token == ",") tokenizer >> token;
    fields[1] = encodeRegister(token, fields[1], registerMap);
    
    // Get jump target
    tokenizer >> token;
    if (token == ",") tokenizer >> token;
    
    // Validate jump target
    bool isNumericTarget = all_of(token.begin(), token.end(), ::isdigit);
    bool isSymbolTarget = symbolTable.find(token) != symbolTable.end();
    
    if (!isNumericTarget && !isSymbolTarget) {
        binaryInstructions.push_back("error");
        return;
    }
    
    // Calculate jump offset
    LongInt immediate;
    if (isSymbolTarget) {
        immediate = symbolTable[token] - instructionPointer;
    } else {
        immediate = stoi(token);
    }
    
    // Verify 4-byte alignment
    if (immediate % 4 != 0) {
        binaryInstructions.push_back("error");
        return;
    }
    
    // Generate 20-bit jump target encoding
    string immediateBits;
    for (int i = 20; i >= 1; i--) {
        immediateBits += ((immediate & (1LL << i)) ? '1' : '0');
    }
    
    // UJ-type format: imm[20|10:1|11|19:12] rd opcode
    // Reorder immediate bits per format specification
    fields[2] = immediateBits.substr(0, 1);          // imm[20]
    fields[2] += immediateBits.substr(10, 10);       // imm[10:1]
    fields[2] += immediateBits.substr(9, 1);         // imm[11]
    fields[2] += immediateBits.substr(1, 8);         // imm[19:12]
    
    // Construct binary encoding
    string binaryEncoding;
    for (int i = 2; i >= 0; i--) {
        if (fields[i] == "error") {
            binaryInstructions.push_back("error");
            return;
        }
        binaryEncoding += fields[i];
    }
    
    // Store instruction information
    binaryInstructions.push_back(binaryEncoding);
    
    // Restore original bit ordering for decoded form
    string fullImmediate = fields[2].substr(0, 1) + 
                           fields[2].substr(12) + 
                           fields[2].substr(11, 1) + 
                           fields[2].substr(1, 10);
                           
    // Create decoded representation
    string decoded = fields[0] + "-" + "NULL" + "-" + "NULL" + "-" + 
                    fields[1] + "-" + "NULL" + "-" + "NULL" + "-" + fullImmediate;
                    
    decodedInstructions.push_back(decoded);
    assemblyLines.push_back(instruction);
    instructionAddresses.push_back(instructionPointer);
}
