#ifndef HAZARDS_H
#define HAZARDS_H

#include "structs.h"

// Pipeline control flags for stalls and flushes
extern bool stall_fetch;
extern bool stall_decode;
extern bool stall_execute;
extern bool stall_memory;
extern bool stall_writeback;

extern bool flush_fetch;
extern bool flush_decode;
extern bool flush_execute;
extern bool flush_memory;

// Function declarations for hazard detection and handling
void detectAndHandleHazards();
bool detectDataHazard();
bool detectControlHazard();
void insertStall(int stageNum);
void handleDataForwarding();
bool checkForwardingPath(int source_reg, int dest_reg, int pipeline_stage);

#endif // HAZARDS_H
