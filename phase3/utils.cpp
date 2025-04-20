#include <bits/stdc++.h>
#include "globals.h"
#include "structs.h"
using namespace std;

void loadMC(const string &filename)
{
    ifstream file(filename);

    if (!file.is_open())
    {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    string line;
    bool dataSegment = false;

    while (getline(file, line))
    {
        // Check if we've reached the data segment marker
        if (line.find("Data Segment") != string::npos)
        {
            dataSegment = true;
            continue;
        }

        // Process based on whether we're in the instruction or data segment
        if (!dataSegment)
        {
            // Skip empty lines or lines without commas (not instruction lines)
            if (line.empty() || line.find(',') == string::npos)
            {
                continue;
            }

            size_t commaPos = line.find(',');

            // Ensure the line has the expected format with PC and machine code separated by a comma
            if (commaPos != string::npos)
            {
                string pc = line.substr(0, commaPos - 1);
                pc = pc.substr(0, pc.find(' ')); // Remove trailing spaces

                // Extract the machine code, removing leading/trailing spaces
                string machine_code = line.substr(commaPos + 2);
                size_t startPos = machine_code.find_first_not_of(' ');
                if (startPos != string::npos)
                {
                    machine_code = machine_code.substr(startPos);
                }

                size_t endPos = machine_code.find(' ');
                if (endPos != string::npos)
                {
                    machine_code = machine_code.substr(0, endPos);
                }

                pcMachineCode[pc] = machine_code;
            }
        }
        else
        {
            // Process data segment lines
            // Skip empty lines or marker lines
            if (line.empty() || line.find("****") != string::npos)
            {
                continue;
            }

            // Parse data memory line like: "0x10000000   61 62 63 64"
            stringstream ss(line);
            string addrStr;
            ss >> addrStr; // Read address

            if (addrStr.empty() || addrStr[0] != '0')
            {
                continue; // Skip invalid lines
            }

            // Convert hex address to integer
            unsigned int baseAddr;
            try
            {
                baseAddr = stoul(addrStr, nullptr, 16);
            }
            catch (...)
            {
                continue; // Skip if address conversion fails
            }

            // Read byte values
            string byteStr;
            unsigned int offset = 0;

            while (ss >> byteStr)
            {
                // Convert hex byte to integer
                try
                {
                    unsigned char byteVal = stoul(byteStr, nullptr, 16);
                    dataMemory[baseAddr + offset] = byteVal;
                    offset++;
                }
                catch (...)
                {
                    // Skip if byte conversion fails
                }
            }
        }
    }

    file.close();
    cout << "Loaded " << pcMachineCode.size() << " instructions and "
         << dataMemory.size() << " bytes of data memory." << endl;
}

string hex2bin(string hexStr)
{
    // Check for "0x" prefix and remove it
    if (hexStr.substr(0, 2) == "0x" || hexStr.substr(0, 2) == "0X")
    {
        hexStr = hexStr.substr(2); // Remove "0x"
    }

    // Ensure valid length (8 characters for 32 bits)
    if (hexStr.length() > 8)
    {
        cout << "Invalid instruction. Please enter exactly 8 hexadecimal digits." << endl;
        return "";
    }

    // Convert hex string to unsigned int
    unsigned int num;
    stringstream ss;
    ss << hex << hexStr;
    ss >> num;

    // Convert to 32-bit binary representation
    bitset<32> binary(num);
    return binary.to_string();
}

string bin2hex(string binStr)
{
    // Ensure valid length (32 characters for 32 bits)
    if (binStr.length() > 32)
    {
        cout << "Invalid input. Please enter exactly 32 binary digits." << endl;
        return "";
    }

    // Convert binary string to unsigned int
    bitset<32> binary(binStr);
    unsigned int num = binary.to_ulong();

    // Convert to hex string
    stringstream ss;
    ss << hex << num;
    return ss.str();
}
