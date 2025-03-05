#include<bits/stdc++.h>

using namespace std;
class Instruction {
};

unordered_map<string, Instruction> instructionMap = {
    {"add", {"0110011", "000", "0000000"}},
    {"sub", {"0110011", "000", "0100000"}},
    {"and", {"0110011", "111", "0000000"}},
    {"or", {"0110011", "110", "0000000"}},
    {"xor", {"0110011", "100", "0000000"}},
    {"sll", {"0110011", "001", "0000000"}},
    {"slt", {"0110011", "010", "0000000"}},
    {"sra", {"0110011", "101", "0100000"}},
    {"srl", {"0110011", "101", "0000000"}},
    {"mul", {"0110011", "000", "0000001"}},
    {"div", {"0110011", "100", "0000001"}},
    {"rem", {"0110011", "110", "0000001"}},
    {"addi", {"0010011", "000", ""}},
    {"andi", {"0010011", "111", ""}},
    {"ori", {"0010011", "110", ""}},
    {"lb", {"0000011", "000", ""}},
    {"ld", {"0000011", "011", ""}},
    {"lh", {"0000011", "001", ""}},
    {"lw", {"0000011", "010", ""}},
    {"jalr", {"1100111", "000", ""}},
    {"sb", {"0100011", "000", ""}},
    {"sw", {"0100011", "010", ""}},
    {"sd", {"0100011", "011", ""}},
    {"sh", {"0100011", "001", ""}},
    {"beq", {"1100011", "000", ""}},
    {"bne", {"1100011", "001", ""}},
    {"bge", {"1100011", "101", ""}},
    {"blt", {"1100011", "100", ""}},
    {"auipc", {"0010111", "", ""}},
    {"lui", {"0110111", "", ""}},
    {"jal", {"1101111", "", ""}}
};
int main(){
    
}
