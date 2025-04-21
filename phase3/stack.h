#ifndef STACK_H
#define STACK_H

#include "structs.h"

// Stack management function declarations
void initializeStack();
void pushToStack(int value);
int popFromStack();
void allocateStackSpace(unsigned int size);
void freeStackSpace(unsigned int size);
void createStackFrame(unsigned int frameSize);
void destroyStackFrame();
void executeStackOperations(Instruction instruction);

#endif // STACK_H
