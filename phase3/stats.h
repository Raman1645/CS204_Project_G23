#ifndef STATS_H
#define STATS_H

// Performance monitoring and statistics reporting functions
void initializeStats();      // Reset all performance counters
void updateStats();          // Track instruction execution metrics
void printStats();           // Display current statistics to console
void saveStatsToFile(const std::string &filename);  // Export statistics to a file
void printRegisterFile();    // Display contents of all registers

#endif // STATS_H
