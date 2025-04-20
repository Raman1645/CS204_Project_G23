#ifndef STATS_H
#define STATS_H

// Statistics functions
void initializeStats();
void updateStats();
void printStats();
void saveStatsToFile(const std::string &filename);
void printRegisterFile();

#endif // STATS_H