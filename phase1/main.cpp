#include<encode.h>
#include<assign.h>
#include<process.h>
#include<vector>
#include<map>
#include<sstream>
#include<fstream>

using namespace std;

#define LongInt long long
#define ERROR_VAL 1e18
LongInt MemoryStart = (1ll << 28);
map<string, LongInt> symbolTable;
LongInt instructionPointer = 0; // Program counter
LongInt dataSize = 0;
vector<string> binaryInstructions; // Store the binary format of instructions
vector<LongInt> instructionAddresses;
// Instruction formats:
// R format: funct7 | rs2 | rs1 | funct3 | rd | opcode
// I format: imm[11:0] | rs1 | funct3 | rd | opcode
// etc.
unordered_map<string, string> registerMap;
vector<string> decodedInstructions;
vector<string> assemblyLines;


string convertBinaryToHex(string binaryStr) {
    // Converts a 32-bit binary string to hexadecimal
    string hexOutput = "0x";
    for (LongInt i = 0; i < 32; i += 4) {
        string segment;
        LongInt value = 0;
        for (LongInt j = i; j < i + 4; j++)
            binaryStr[j] == '0' ? value *= 2 : value = value * 2 + 1;
        if (value >= 10)
            hexOutput += 'A' + value - 10;
        else
            hexOutput += '0' + value;
    }
    return hexOutput;
}

void processInstruction(string instruction) {
    // Process each instruction and encode it
    istringstream tokenizer(instruction);
    string mnemonic;
    tokenizer >> mnemonic;
    string token = mnemonic;
    if (token[token.size() - 1] != ':') {
        tokenizer >> mnemonic;
    }
    
    if (mnemonic == ":" || token[token.size() - 1] == ':') return;
    mnemonic = token;
    if (token == "add" || token == "sub" || token == "mul" || token == "div" || 
        token == "sll" || token == "srl" || token == "sra" || token == "slt" || 
        token == "xor" || token == "or" || token == "and" || token == "rem") {
        processRType(instruction, binaryInstructions, decodedInstructions, assemblyLines, instructionAddresses, instructionPointer, registerMap);
    }
    else if (token == "addi" || token == "andi" || token == "ori" || token == "jalr") {
        processIType(instruction, binaryInstructions, decodedInstructions, assemblyLines, instructionAddresses, instructionPointer, registerMap);
    }
    else if (token == "lb" || token == "ld" || token == "lw" || token == "lh") {
        processLoadType(instruction, binaryInstructions, decodedInstructions, assemblyLines, instructionAddresses, instructionPointer, registerMap);
    }
    else if (token == "sb" || token == "sh" || token == "sw" || token == "sd") {
        processStoreType(instruction, binaryInstructions, decodedInstructions, assemblyLines, instructionAddresses, instructionPointer, registerMap);
    }
    else if (token == "beq" || token == "bne" || token == "blt" || token == "bge") {
        processBranchType(instruction, binaryInstructions, decodedInstructions, assemblyLines, instructionAddresses, instructionPointer, symbolTable, registerMap);
    }
    else if (token == "auipc" || token == "lui") {
        processUpperImmediate(instruction, binaryInstructions, decodedInstructions, assemblyLines, instructionAddresses, instructionPointer, registerMap);
    }
    else if (token == "jal") {
        processJumpType(instruction, binaryInstructions, decodedInstructions, assemblyLines, instructionAddresses, instructionPointer, symbolTable, registerMap);
    }
    else {
        binaryInstructions.push_back("error");
        decodedInstructions.push_back("error");
        assemblyLines.push_back(instruction);
        instructionAddresses.push_back(instructionPointer);
    }
    instructionPointer += 4;
}

void readInstruction (string instruction) {
    istringstream tokenizer(instruction);
    string mnemonic;
    tokenizer >> mnemonic;
    string token = mnemonic;
    if (token[token.size() - 1] != ':') {
        tokenizer >> mnemonic;
    }
    LongInt size = token.size();
    if (mnemonic == ":" || token[token.size() - 1] == ':') {
        if (token[token.size() - 1] == ':') size--;
        string label;
        for (LongInt i = 0; i < size; i++) {
            label += token[i];
        }
        symbolTable[label] = instructionPointer;
        return;
    }
    instructionPointer += 4;
}

int main () {
    setupRegisterMapping(registerMap);
    ofstream op("output.mc");   
    ifstream file("input.asm");

    string instruction;
    LongInt memoryArray[204];
    for (LongInt i = 0; i < 204; i++)
        memoryArray[i] = 0;
    
    LongInt check = 0;
    LongInt memory = MemoryStart;

    while (getline(file, instruction)) {
        if (instruction == ".data") {
            while (getline(file, instruction)) {
                if (instruction == ".text") {
                    break;
                }
                processDataDirective(instruction, memoryArray, dataSize, memory, symbolTable);
            }
        } else if (instruction == ".text") continue;
        else readInstruction(instruction);
    }
    file.close();

    ifstream file2("input.asm");
    instructionPointer = 0;

    while (getline(file2, instruction)) {
        if (instruction == ".data") {
            while (getline(file2, instruction)) {
                if (instruction == ".text") {
                    break;
                }
            }
        } else if (instruction == ".text") continue;
        else processInstruction(instruction);
    }

    LongInt counter = 0;
    for (auto i = 0; i<binaryInstructions.size(); i++) {
        if (binaryInstructions[i] == "error") {
            cout << "Error in encoding instruction: " << assemblyLines[i] << endl;
            continue;
        }
        else if (binaryInstructions[i][0] == '0' || binaryInstructions[i][0] == '1') {
            op << "0x";
            op <<hex << instructionAddresses[counter] << " , ";
            op << convertBinaryToHex(binaryInstructions[i]) + " " + assemblyLines[i] + " " + "#" + " " + decodedInstructions[i] << endl;
        }
        else {
            op << binaryInstructions[i] <<endl;
        }
        counter++;
    }
    op << endl << endl ;
    op << "***********************************************************************************************" <<endl;
    op << "Data Segment" << endl;

    for (LongInt i =0 ; i<204; i += 4){
        op << "0x";
        op << hex << MemoryStart  << "   ";
        for (LongInt j = i; j < i+4; j++) {
            LongInt value = memoryArray[j];
            string result = "00";
            LongInt rem = value%16;
            if (rem > 9) result[1] = rem - 10 + 'A';
            else result[1] = rem + '0';
            value /= 16;
            rem = value % 16;
            if (rem > 9) result[0] = rem - 10 + 'A';
            else result[0] = rem + '0';
            op << result << " " ;
        }
        op<< endl;
        MemoryStart += 4;
    }

    op.close();
    file2.close();
    return 0;


}