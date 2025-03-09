#include<assign.h>

using namespace std;

#define LongInt long long
#define ERROR_VAL 1e18


LongInt parseValue(string valueStr) {
    // Parse different numeric formats: decimal, hex, binary
    string parsedStr;
    LongInt result = 0;
    
    if (valueStr == "0")
        result = 0;
    else if (valueStr[0] == '0' && valueStr[1] == 'x') {
        if (valueStr.size() > 10)
            return (LongInt)ERROR_VAL;
        for (LongInt i = 2; i < min((LongInt)valueStr.size(), 10ll); i++)
            parsedStr += valueStr[i];
        result = stoi(parsedStr, 0, 16);
    }
    else if (valueStr[0] == '0') {
        if (valueStr.size() > 33)
            return ERROR_VAL;
        for (LongInt i = 1; i < valueStr.size(); i++)
            parsedStr += valueStr[i];
        result = stoi(parsedStr, 0, 2);
    }
    else if (valueStr[0] == '-') {
        string numPart;
        for (LongInt i = 1; i < valueStr.size(); i++)
            numPart += valueStr[i];
        result = stoi(numPart);
        if (result > (1ll << 32))
            return ERROR_VAL;
        result = -result;
    }
    else if ((LongInt)(valueStr[0] - '0') < 0 || (LongInt)(valueStr[0] - '0') > 9) {
        // ASCII value
        for (LongInt i = 0; i < valueStr.size(); i++)
            result += (LongInt)valueStr[i];
    }
    else
        result = stoi(valueStr);
    return result;
}

string determineOpcode(string mnemonic, string binaryStr) {
    // Add the opcode for the instruction format
    if (mnemonic == "rem" || mnemonic == "sra" || mnemonic == "add" || 
        mnemonic == "sll" || mnemonic == "slt" || mnemonic == "xor" || 
        mnemonic == "srl" || mnemonic == "or" || mnemonic == "and" || 
        mnemonic == "sub" || mnemonic == "mul" || mnemonic == "div")
        binaryStr += "0110011";
    else if (mnemonic == "addi" || mnemonic == "andi" || mnemonic == "ori")
        binaryStr += "0010011";
    else if (mnemonic == "lb" || mnemonic == "ld" || mnemonic == "lw" || mnemonic == "lh")
        binaryStr += "0000011";
    else if (mnemonic == "jalr")
        binaryStr += "1100111";
    else if (mnemonic == "beq" || mnemonic == "bne" || mnemonic == "bge" || mnemonic == "blt")
        binaryStr += "1100011";
    else if (mnemonic == "sb" || mnemonic == "sh" || mnemonic == "sw" || mnemonic == "sd")
        binaryStr += "0100011";
    else if (mnemonic == "auipc")
        binaryStr += "0010111";
    else if (mnemonic == "lui")
        binaryStr += "0110111";
    else if (mnemonic == "jal")
        binaryStr += "1101111";
    else
        binaryStr = "error"; // Unknown opcode
    return binaryStr;
}

string determineFunct7(string mnemonic, string binaryStr) {
    if (binaryStr == "error")
        return binaryStr;
        
    if (mnemonic == "sub" || mnemonic == "sra")
        binaryStr += "0100000";
    else if (mnemonic == "add" || mnemonic == "sll" || mnemonic == "slt" || 
             mnemonic == "xor" || mnemonic == "srl" || mnemonic == "or" || 
             mnemonic == "and")
        binaryStr += "0000000";
    else if (mnemonic == "mul" || mnemonic == "div" || mnemonic == "rem")
        binaryStr += "0000001";
    else
        binaryStr = "error";
        
    return binaryStr;
}

string determineFunct3(string mnemonic, string binaryStr) {
    if (mnemonic == "add" || mnemonic == "sub" || mnemonic == "mul" || mnemonic == "sb") {
        binaryStr += "000";
    } else if (mnemonic == "xor" || mnemonic == "div") {
        binaryStr += "100";
    } else if (mnemonic == "srl") {
        binaryStr += "101";
    } else if (mnemonic == "sra") {
        binaryStr += "101";
    } else if (mnemonic == "sll" || mnemonic == "sh") {
        binaryStr += "001";
    } else if (mnemonic == "slt" || mnemonic == "sw") {
        binaryStr += "010";
    } else if (mnemonic == "or" || mnemonic == "rem") {
        binaryStr += "110";
    } else if (mnemonic == "and") {
        binaryStr += "111";
    } else if (mnemonic == "sd") {
        binaryStr += "011";
    } else if (mnemonic == "lb" || mnemonic == "jalr" || mnemonic == "addi") {
        binaryStr += "000";
    } else if (mnemonic == "lh") {
        binaryStr += "001";
    } else if (mnemonic == "lw") {
        binaryStr += "010";
    } else if (mnemonic == "ld") {
        binaryStr += "011";
    } else if (mnemonic == "ori") {
        binaryStr += "110";
    } else if (mnemonic == "andi") {
        binaryStr += "111";
    } else if (mnemonic == "beq") {
        binaryStr += "000";
    } else if (mnemonic == "bne") {
        binaryStr += "001";
    } else if (mnemonic == "blt") {
        binaryStr += "100";
    } else if (mnemonic == "bge") {
        binaryStr += "101";
    }
    return binaryStr;
}

string encodeImmediate(string valueStr, string binaryStr) {
    if (binaryStr == "error")
        return binaryStr;
        
    LongInt value = parseValue(valueStr);
    if (value == ERROR_VAL)
        return "error";

    if (value >= (1ll << 11) || value <= -(1ll << 11)) {
        return "error";
    }
    
    if (value < 0)
        value += (1ll << 12);
        
    // Convert value to 12-bit binary string
    for (LongInt i = 11; i >= 0; i--) {
        if (value & (1ll << i))
            binaryStr += '1';
        else
            binaryStr += '0';
    }
    return binaryStr;
}

string encodeBranchImmediate(string valueStr, string binaryStr) {
    if (binaryStr == "error")
        return binaryStr;
        
    LongInt value = parseValue(valueStr);
    if (value == ERROR_VAL)
        return "error";
        
    if (value < 0)
        value += (1ll << 13);
        
    if (value >= (1ll << 13) || value < 0) {
        return "error";
    }
    
    // Convert to 13-bit binary string for branch targets
    for (LongInt i = 12; i >= 0; i--) {
        if (value & (1ll << i))
            binaryStr += '1';
        else
            binaryStr += '0';
    }
    return binaryStr;
}

string encodeUpperImmediate(string valueStr, string binaryStr) {
    if (binaryStr == "error")
        return binaryStr;
        
    LongInt value = parseValue(valueStr);
    if (value == ERROR_VAL)
        return "error";
        
    if (value < 0)
        value += (1ll << 12);
        
    value <<= 12;
    
    // Convert to 20-bit upper immediate
    for (LongInt i = 31; i >= 12; i--) {
        if (value & (1ll << i))
            binaryStr += '1';
        else
            binaryStr += '0';
    }
    return binaryStr;
}


