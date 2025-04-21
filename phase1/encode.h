#include<iostream>
#include<string>
#include<unordered_map>
#include<bits/stdc++.h>

#ifndef ENCODE_H
#define ENCODE_H
using namespace std;

#endif

void setupRegisterMapping (unordered_map<string, string> &registerMap);
string encodeRegister(string reg, string binaryStr, unordered_map<string, string> &registerMap);