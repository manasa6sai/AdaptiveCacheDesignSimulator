#ifndef GLOBALS_H
#define GLOBALS_H

#include <string>
#include <vector>
#include "Structs.h" // Include the Structs.h file to use the CommandLineArgs struct

extern std::string::size_type sz;
extern long FileLineNum;

extern CommandLineArgs cmdObj;

// Performance parameters
extern long l1Reads;
extern long l1ReadMisses;
extern long l1Writes;
extern long l1WriteMisses;
extern long l1MissRate;
extern long l1Writebacks;
extern long l2Reads;
extern long l2ReadMisses;
extern long l2Writes;
extern long l2WriteMisses;
extern long l2MissRate;
extern long l2WriteBacks;
extern long totalMemoryTraffic;
extern long l1DirectWriteBacks;

// CacheCounter Number counters
extern long L1CacheCounter;
extern long L2CacheCounter;

// L1 Cache identifier
extern long l1Sets;
extern block **l1;

// L2 Cache identifier
extern long l2Sets;
extern block **l2;

// Vector of trace file lines
extern std::vector<fileContentStruct *> traceFileContent;

#endif // GLOBALS_H
