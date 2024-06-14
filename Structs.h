// Structs.h

#ifndef STRUCTS_H
#define STRUCTS_H

#include <string>
#include <vector>

// Define the structure for the conversion of hex address to tag and indexPos
struct PhysicalAddr {
  std::string blockAddr;
  std::string tagAddr;
  int indexPos;
};

// Define the structure for command line arguments
struct CommandLineArgs {
  long GlobalblockSize;
  long GlobalL1Size;
  long GlobalL1Assoc;
  long GlobalL2Size;
  long GlobalL2Assoc;
  long ReplacementPolicy;
  long InclusionPolicy;
  std::string TraceFileName;
};

// Define the structure for block parameters
struct block {
  std::string blockAdd;
  std::string hexAdd;
  std::string tag;
  int indexPos;
  bool valid;
  long CacheCounter;
  long dirtyBit;
  int LCS_Counter;
};

// Define the structure for the input content
struct fileContentStruct {
  std::string ops;
  std::string hexAdd;
};

#endif
