#include<iostream>
#include<string>
#include<bits/stdc++.h>

#ifndef PROCESS_H
#define PROCESS_H
#define LongInt long long
using namespace std;

#endif

void processDataDirective(string line, LongInt memory[], LongInt &dataSize, LongInt MemoryStart, map<string, LongInt> &symbolTable);
void processRType(string instruction, vector<string> &binaryInstructions, vector<string> &decodedInstructions, vector<string> &assemblyLines, vector<LongInt> &instructionAddresses, LongInt instructionPointer, unordered_map<string, string> &registerMap);
void processIType(string instruction, vector<string> &binaryInstructions, vector<string> &decodedInstructions, vector<string> &assemblyLines, vector<LongInt> &instructionAddresses, LongInt instructionPointer, unordered_map<string, string> &registerMap);
void processLoadType(string instruction, vector<string> &binaryInstructions, vector<string> &decodedInstructions, vector<string> &assemblyLines, vector<LongInt> &instructionAddresses, LongInt instructionPointer, unordered_map<string, string> &registerMap);
void processStoreType(string instruction, vector<string> &binaryInstructions, vector<string> &decodedInstructions, vector<string> &assemblyLines, vector<LongInt> &instructionAddresses, LongInt instructionPointer, unordered_map<string, string> &registerMap);
void processBranchType(string instruction, vector<string> &binaryInstructions, vector<string> &decodedInstructions, vector<string> &assemblyLines, vector<LongInt> &instructionAddresses, LongInt instructionPointer, map<string, LongInt> &symbolTable, unordered_map<string, string> &registerMap);
void processUpperImmediate(string instruction, vector<string> &binaryInstructions, vector<string> &decodedInstructions, vector<string> &assemblyLines, vector<LongInt> &instructionAddresses, LongInt instructionPointer, unordered_map<string, string> &registerMap);
void processJumpType(string instruction, vector<string> &binaryInstructions, vector<string> &decodedInstructions, vector<string> &assemblyLines, vector<LongInt> &instructionAddresses, LongInt instructionPointer, map<string, LongInt> &symbolTable, unordered_map<string, string> &registerMap);