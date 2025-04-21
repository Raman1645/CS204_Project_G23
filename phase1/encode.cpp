#include<encode.h>

using namespace std;

void setupRegisterMapping(unordered_map<string, string> &registerMap) {
    pair<string, string> regPairs[] = {
        make_pair("x0", "x0"),
        make_pair("x1", "x1"),
        make_pair("x2", "x2"),
        make_pair("x3", "x3"),
        make_pair("x4", "x4"),
        make_pair("x5", "x5"),
        make_pair("x6", "x6"),
        make_pair("x7", "x7"),
        make_pair("x8", "x8"),
        make_pair("x9", "x9"),
        make_pair("x10", "x10"),
        make_pair("x11", "x11"),
        make_pair("x12", "x12"),
        make_pair("x13", "x13"),
        make_pair("x14", "x14"),
        make_pair("x15", "x15"),
        make_pair("x16", "x16"),
        make_pair("x17", "x17"),
        make_pair("x18", "x18"),
        make_pair("x19", "x19"),
        make_pair("x20", "x20"),
        make_pair("x21", "x21"),
        make_pair("x22", "x22"),
        make_pair("x23", "x23"),
        make_pair("x24", "x24"),
        make_pair("x25", "x25"),
        make_pair("x26", "x26"),
        make_pair("x27", "x27"),
        make_pair("x28", "x28"),
        make_pair("x29", "x29"),
        make_pair("x30", "x30"),
        make_pair("x31", "x31"),
        make_pair("zero", "x0"),
        make_pair("ra", "x1"),
        make_pair("sp", "x2"),
        make_pair("gp", "x3"),
        make_pair("tp", "x4"),
        make_pair("t0", "x5"),
        make_pair("t1", "x6"),
        make_pair("t2", "x7"),
        make_pair("s0", "x8"),
        make_pair("s1", "x9"),
        make_pair("a0", "x10"),
        make_pair("a1", "x11"),
        make_pair("a2", "x12"),
        make_pair("a3", "x13"),
        make_pair("a4", "x14"),
        make_pair("a5", "x15"),
        make_pair("a6", "x16"),
        make_pair("a7", "x17"),
        make_pair("s2", "x18"),
        make_pair("s3", "x19"),
        make_pair("s4", "x20"),
        make_pair("s5", "x21"),
        make_pair("s6", "x22"),
        make_pair("s7", "x23"),
        make_pair("s8", "x24"),
        make_pair("s9", "x25"),
        make_pair("s10", "x26"),
        make_pair("s11", "x27"),
        make_pair("t3", "x28"),
        make_pair("t4", "x29"),
        make_pair("t5", "x30"),
        make_pair("t6", "x31"),
    };

    int mapSize = (sizeof(regPairs) / sizeof(regPairs[0]));
    unordered_map<string, string> tmpMap(regPairs, regPairs + mapSize);
    registerMap = tmpMap;
}

string encodeRegister(string reg, string binaryStr, unordered_map<string, string> &registerMap) {
    if (registerMap.find(reg) == registerMap.end()) {
        return "error";
    }
    reg = registerMap[reg];
    
    if (reg[0] != 'x') {
        return "error";
    }

    string numPart;
    long long length = reg.size();
    if (reg[reg.size() - 1] == ',')
        length = reg.size() - 1;
    else
        length = reg.size();

    for (long long i = 1; i < length; i++)
        numPart += reg[i];
        
    // Validate register number
    bool validDigits = true;
    for (long long i = 0; i < numPart.size(); i++) {
        long long digit = numPart[i] - '0';
        if (digit > 9 || digit < 0) {
            validDigits = false;
            break;
        }
    }
    
    if (!validDigits) {
        return "error";
    }
    
    long long regNum = stoi(numPart);
    if (regNum >= 32 || regNum < 0) {
        return "error";
    }

    // Encode the 5-bit register value
    for (long long i = 4; i >= 0; i--) {
        if (regNum & (1ll << i))
            binaryStr += '1';
        else
            binaryStr += '0';
    }
    
    return binaryStr;
}