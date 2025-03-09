#include <iostream>
#include<string>
#include<bits/stdc++.h>

using namespace std;  


#ifndef ASSIGN_H
#define ASSIGN_H

#endif


long long parseValue(string valueStr);
string determineOpcode (string mnemonic, string binaryStr);
string determineFunct3 (string mnemonic, string binaryStr);
string determineFunct7 (string mnemonic, string binaryStr);
string encodeImmediate(string valueStr, string binaryStr);
string encodeBranchImmediate(string valueStr, string binaryStr);
string encodeUpperImmediate(string valueStr, string binaryStr);

