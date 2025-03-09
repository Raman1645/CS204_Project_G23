#include<process.h>
#include<assign.h>
#include<encode.h>
#include <sstream>
#include<map>
#include<vector>
#include<algorithm>


#define LongInt long long
using namespace std;

void processDataDirective(string line, LongInt memory[], LongInt &dataSize, LongInt MemoryStart, map<string, LongInt> &symbolTable) {
    istringstream tokenizer(line);
    string token;
    // Get label
    tokenizer >> token;
    string labelStr = token;

    if (token[token.size() - 1] == ':') {
        labelStr = "";
        for (LongInt i = 0; i < token.size() - 1; i++)
            labelStr += token[i];
    }
    symbolTable[labelStr] = MemoryStart + dataSize; // Record label address
    
    if (token[token.size() - 1] != ':')
        tokenizer >> token; // Skip to directive

    tokenizer >> token; // Get directive type
    
    if (token == ".byte") {
        while (tokenizer >> token) {
            if (token == ",") continue;
            LongInt value = parseValue(token);
            memory[dataSize] = value;
            dataSize++;
        }
    }
    else if (token == ".word") {
        while (tokenizer >> token) {
            if (token == ",") continue;
            LongInt value = parseValue(token);

            // Split 32-bit word into 4 bytes
            LongInt mask = 255; 
            for (LongInt i = 0; i < 4; i++) {
                LongInt byteVal = mask & value;
                value >>= 8;
                memory[dataSize] = byteVal;
                dataSize++;
            }
        }
    }
    else if (token == ".asciiz") {
        while (tokenizer >> token) {
            if (token == ",") continue;
            // Store each character as a byte
            for (LongInt i = 1; i < token.size() - 1; i++) {
                memory[dataSize] = (LongInt)token[i];
                dataSize++;
            }
        }
    }
    else if (token == ".half") {
        while (tokenizer >> token) {
            if (token == ",") continue;
            LongInt value = parseValue(token);

            // Split 16-bit half-word into 2 bytes
            LongInt mask = 255;
            for (LongInt i = 0; i < 2; i++) {
                LongInt byteVal = mask & value;
                value >>= 8;
                memory[dataSize] = byteVal;
                dataSize++;
            }
        }
    }
    else if (token == ".dword") {
        while (tokenizer >> token) {
            if (token == ",") continue;
            LongInt value = parseValue(token);

            // Split 64-bit double-word into 8 bytes
            LongInt mask = 255;
            for (LongInt i = 0; i < 8; i++) {
                LongInt byteVal = mask & value;
                value >>= 8;
                memory[dataSize] = byteVal;
                dataSize++;
            }
        }
    }
}

void processRType(string instruction, vector<string> &binaryInstructions, vector<string> &decodedInstructions, vector<string> &assemblyLines, vector<LongInt> &instructionAddresses, LongInt instructionPointer, unordered_map<string, string> &registerMap) {
    string binaryEncoding;
    istringstream tokenizer(instruction);
    string token;
    tokenizer >> token;
    string opcode = token;
    vector<string> fields(6, "");
    
    fields[0] = determineOpcode(opcode, fields[0]);
    
    tokenizer >> token;
    fields[1] = encodeRegister(token, fields[1], registerMap);

    tokenizer >> token;
    if (token == ",")
        tokenizer >> token;
    fields[3] = encodeRegister(token, fields[3], registerMap);
    
    tokenizer >> token;
    if (token == ",")
        tokenizer >> token;
    fields[4] = encodeRegister(token, fields[4], registerMap);
    
    fields[5] = determineFunct7(opcode, fields[5]);
    fields[2] = determineFunct3(opcode, fields[2]);

    for (LongInt i = 5; i >= 0; i--) {
        binaryEncoding += fields[i];
        if (fields[i] == "error") {
            binaryInstructions.push_back("error");
            return;
        }
    }
    
    binaryInstructions.push_back(binaryEncoding);
    string decoded = fields[0] + "-" + fields[2] + "-" + fields[5] + "-" + 
                    fields[1] + "-" + fields[3] + "-" + fields[4] + "-" + "NULL";
    decodedInstructions.push_back(decoded);
    assemblyLines.push_back(instruction);
    instructionAddresses.push_back(instructionPointer);
}

void processIType(string instruction, vector<string> &binaryInstructions, vector<string> &decodedInstructions, vector<string> &assemblyLines, vector<LongInt> &instructionAddresses, LongInt instructionPointer, unordered_map<string, string> &registerMap) {
    string binaryEncoding;
    istringstream tokenizer(instruction);
    string token;
    tokenizer >> token;
    string opcode = token;
    vector<string> fields(5, "");
    
    fields[0] = determineOpcode(opcode, fields[0]);
    
    tokenizer >> token;
    fields[1] = encodeRegister(token, fields[1], registerMap);

    tokenizer >> token;
    if (token == ",")
        tokenizer >> token;
    fields[3] = encodeRegister(token, fields[3], registerMap);
    
    tokenizer >> token;
    if (token == ",")
        tokenizer >> token;
    fields[4] = encodeImmediate(token, fields[4]);
    
    fields[2] = determineFunct3(opcode, fields[2]);

    for (LongInt i = 4; i >= 0; i--) {
        binaryEncoding += fields[i];
        if (fields[i] == "error") {
            binaryInstructions.push_back("error");
            return;
        }
    }
    
    binaryInstructions.push_back(binaryEncoding);
    string decoded = fields[0] + "-" + fields[2] + "-" + "NULL" + "-" + 
                    fields[1] + "-" + fields[3] + "-" + "NULL" + "-" + fields[4];
    decodedInstructions.push_back(decoded);
    assemblyLines.push_back(instruction);
    instructionAddresses.push_back(instructionPointer);
}

void processLoadType(string instruction, vector<string> &binaryInstructions, vector<string> &decodedInstructions, vector<string> &assemblyLines, vector<LongInt> &instructionAddresses, LongInt instructionPointer, unordered_map<string, string> &registerMap) {
    string binaryEncoding;
    istringstream tokenizer(instruction);
    string token;
    tokenizer >> token;
    string opcode = token;
    vector<string> fields(6, "");
    
    fields[0] = determineOpcode(opcode, fields[0]);
    
    tokenizer >> token;
    if (token == ",")
        tokenizer >> token;
    fields[1] = encodeRegister(token, fields[1], registerMap);
    
    fields[2] = determineFunct3(opcode, fields[2]);
    
    tokenizer >> token;
    if (token == ",")
        tokenizer >> token;
        
    // Parse offset(register) format
    string offset, reg;
    LongInt idx = 0;
    while (token[idx] != '(') {
        offset += token[idx];
        idx++;
    }
    idx++;
    while (token[idx] != ')') {
        reg += token[idx];
        idx++;
    }

    fields[3] = encodeRegister(reg, fields[3], registerMap);
    fields[4] = encodeImmediate(offset, fields[4]);
    
    for (LongInt i = 4; i >= 0; i--) {
        binaryEncoding += fields[i];
        if (fields[i] == "error") {
            binaryInstructions.push_back("error");
            return;
        }
    }
    
    binaryInstructions.push_back(binaryEncoding);
    string decoded = fields[0] + "-" + fields[2] + "-" + "NULL" + "-" + 
                    fields[1] + "-" + fields[3] + "-" + "NULL" + "-" + fields[4];
    decodedInstructions.push_back(decoded);
    assemblyLines.push_back(instruction);
    instructionAddresses.push_back(instructionPointer);
}

void processStoreType(string instruction, vector<string> &binaryInstructions, vector<string> &decodedInstructions, vector<string> &assemblyLines, vector<LongInt> &instructionAddresses, LongInt instructionPointer, unordered_map<string, string> &registerMap) {
    string binaryEncoding;
    istringstream tokenizer(instruction);
    string token;
    tokenizer >> token;
    string opcode = token;
    vector<string> fields(6, "");
    
    fields[0] = determineOpcode(opcode, fields[0]);
    fields[2] = determineFunct3(opcode, fields[2]);
    
    tokenizer >> token;
    if (token == ",")
        tokenizer >> token;
    fields[4] = encodeRegister(token, fields[4], registerMap);

    tokenizer >> token;
    if (token == ",")
        tokenizer >> token;
        
    // Parse offset(register) format
    string offset, reg;
    LongInt idx = 0;
    while (token[idx] != '(') {
        offset += token[idx];
        idx++;
    }
    idx++;
    while (token[idx] != ')') {
        reg += token[idx];
        idx++;
    }

    fields[3] = encodeRegister(reg, fields[3], registerMap);
    fields[5] = encodeImmediate(offset, fields[5]);
    
    // Split immediate field for S-type instruction format
    string temp = fields[5];
    fields[5] = "";
    for (LongInt i = 0; i < 7; i++)
        fields[5] += temp[i];
    for (LongInt i = 7; i <= 11; i++)
        fields[1] += temp[i];

    for (LongInt i = 5; i >= 0; i--) {
        binaryEncoding += fields[i];
        if (fields[i] == "error") {
            binaryInstructions.push_back("error");
            return;
        }
    }
    
    binaryInstructions.push_back(binaryEncoding);
    string decoded = fields[0] + "-" + fields[2] + "-" + "NULL" + "-" + 
                    "NULL" + "-" + fields[3] + "-" + fields[4] + "-" + 
                    fields[5] + fields[1];
    decodedInstructions.push_back(decoded);
    assemblyLines.push_back(instruction);
    instructionAddresses.push_back(instructionPointer);
}

void processBranchType(string instruction, vector<string> &binaryInstructions, vector<string> &decodedInstructions, vector<string> &assemblyLines, vector<LongInt> &instructionAddresses, LongInt instructionPointer, map<string, LongInt> &symbolTable, unordered_map<string, string> &registerMap) {
    istringstream tokenizer(instruction);
    string token;
    tokenizer >> token;
    string opcode = token;
    vector<string> fields(6, "");
    
    fields[0] = determineOpcode(opcode, fields[0]);
    fields[2] = determineFunct3(opcode, fields[2]);
    
    tokenizer >> token;
    fields[3] = encodeRegister(token, fields[3], registerMap);
    
    tokenizer >> token;
    if (token == ",")
        tokenizer >> token;
    fields[4] = encodeRegister(token, fields[4], registerMap);
    
    tokenizer >> token;
    if (token == ",")
        tokenizer >> token;
        
    string binaryEncoding;
    bool isNumeric = all_of(token.begin(), token.end(), ::isdigit);
    
    if (symbolTable.find(token) == symbolTable.end() && !isNumeric) {
        binaryInstructions.push_back("error");
        return;
    }
    
    LongInt immediate;
    if (symbolTable.find(token) != symbolTable.end())
        immediate = symbolTable[token] - instructionPointer;
    else
        immediate = stoi(token);
        
    if (immediate % 4 != 0) {
        binaryInstructions.push_back("error");
        return;
    }
    
    // Encode branch offset
    string offsetStr = to_string(immediate);
    fields[5] = encodeBranchImmediate(offsetStr, fields[5]);
    
    // SB-type format requires splitting immediate bits
    string temp = fields[5];
    fields[5] = "";
    fields[5] += temp[0];
    for (LongInt i = 2; i <= 7; i++)
        fields[5] += temp[i];
    for (LongInt i = 8; i < 12; i++)
        fields[1] += temp[i];
    fields[1] += temp[1];
    
    for (LongInt i = 5; i >= 0; i--) {
        binaryEncoding += fields[i];
        if (fields[i] == "error") {
            binaryInstructions.push_back("error");
            return;
        }
    }
    
    binaryInstructions.push_back(binaryEncoding);
    string decoded = fields[0] + "-" + fields[2] + "-" + "NULL" + "-" + 
                    "NULL" + "-" + fields[3] + "-" + fields[4] + "-" + 
                    temp;
    decodedInstructions.push_back(decoded);
    assemblyLines.push_back(instruction);
    instructionAddresses.push_back(instructionPointer);
}

void processUpperImmediate(string instruction, vector<string> &binaryInstructions, vector<string> &decodedInstructions, vector<string> &assemblyLines, vector<LongInt> &instructionAddresses, LongInt instructionPointer, unordered_map<string, string> &registerMap) {
    istringstream tokenizer(instruction);
    string token;
    tokenizer >> token;
    string opcode = token;
    string binaryEncoding;
    vector<string> fields(3, "");
    
    fields[0] = determineOpcode(token, fields[0]);
    
    tokenizer >> token;
    if (token == ",")
        tokenizer >> token;
    fields[1] = encodeRegister(token, fields[1], registerMap);
    
    tokenizer >> token;
    if (token == ",")
        tokenizer >> token;
    fields[2] = encodeUpperImmediate(token, fields[2]);
    
    for (LongInt i = 2; i >= 0; i--) {
        binaryEncoding += fields[i];
        if (fields[i] == "error") {
            binaryInstructions.push_back("error");
            return;
        }
    }
    
    binaryInstructions.push_back(binaryEncoding);
    string decoded = fields[0] + "-" + "NULL" + "-" + "NULL" + "-" + 
                    fields[1] + "-" + "NULL" + "-" + "NULL" + "-" + fields[2];
    decodedInstructions.push_back(decoded);
    assemblyLines.push_back(instruction);
    instructionAddresses.push_back(instructionPointer);
}

void processJumpType(string instruction, vector<string> &binaryInstructions, vector<string> &decodedInstructions, vector<string> &assemblyLines, vector<LongInt> &instructionAddresses, LongInt instructionPointer, map<string, LongInt> &symbolTable, unordered_map<string, string> &registerMap) {
    istringstream tokenizer(instruction);
    string token;
    tokenizer >> token;
    string opcode = token;
    string binaryEncoding;
    vector<string> fields(3, "");
    
    fields[0] = determineOpcode(token, fields[0]);
    
    tokenizer >> token;
    if (token == ",")
        tokenizer >> token;
    fields[1] = encodeRegister(token, fields[1], registerMap);
    
    tokenizer >> token;
    if (token == ",")
        tokenizer >> token;
        
    bool isNumeric = all_of(token.begin(), token.end(), ::isdigit);
    if (symbolTable.find(token) == symbolTable.end() && !isNumeric) {
        binaryInstructions.push_back("error");
        return;
    }
    
    LongInt immediate;
    if (symbolTable.find(token) != symbolTable.end())
        immediate = symbolTable[token] - instructionPointer;
    else
        immediate = stoi(token);
        
    if (immediate % 4 != 0) {
        binaryInstructions.push_back("error");
        return;
    }
    
    LongInt jumpOffset = symbolTable[token] - instructionPointer;
    
    // Encode 20-bit jump target
    for (LongInt i = 20; i >= 1; i--) {
        if (jumpOffset & (1ll << i))
            fields[2] += '1';
        else
            fields[2] += '0';
    }
    string temp = fields[2];
    fields[2] = "";
    fields[2] += temp[0];
    for (LongInt i = 10; i <= 19; i++)
        fields[2] += temp[i];
    
    fields[2] += temp[9];
    for (LongInt i = 1; i <= 8; i++)
        fields[2] += temp[i];
    for (LongInt i = 2; i>= 0; i--){
        binaryEncoding += fields[i];
        if (fields[i] == "error") {
            binaryInstructions.push_back("error");
            return;
        }
    }

    binaryInstructions.push_back(binaryEncoding);
    string decoded = fields[0] + "-" + "NULL" + "-" + "NULL" + "-" + 
                    fields[1] + "-" + "NULL" + "-" + "NULL" + "-" + temp;
    decodedInstructions.push_back(decoded);
    assemblyLines.push_back(instruction);
    instructionAddresses.push_back(instructionPointer);
}