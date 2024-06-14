#include <iostream>

#include <iomanip>

#include <string>

#include <sstream>

#include <fstream>

#include <cmath>

#include <bits/stdc++.h>

#include <algorithm>

#include <vector>

#include "Structs.h" // Include the Structs.h file to use the structs

#include "Globals.h" // Include the Globals.h header to access the global variables


using namespace std;

string::size_type sz;
long FileLineNum = 0;




CommandLineArgs cmdObj;


// Performance parameters
long l1Reads = 0;
long l1ReadMisses = 0;
long l1Writes = 0;
long l1WriteMisses = 0;
long l1MissRate = 0;
long l1Writebacks = 0;
long l2Reads = 0;
long l2ReadMisses = 0;
long l2Writes = 0;
long l2WriteMisses = 0;
long l2MissRate = 0;
long l2WriteBacks = 0;
long totalMemoryTraffic = 0;
long l1DirectWriteBacks = 0;



// CacheCounter Number counters
long L1CacheCounter = 1;
long L2CacheCounter = 1;

// L1 Cache identifier
long l1Sets;
block ** l1;

// L2 Cache identifier
long l2Sets;
block ** l2;


// THis is vector of trace file lines. Each line is one object.
// global vector
vector < fileContentStruct * > traceFileContent;




PhysicalAddr ExtractAddress(string hexAddress, int blockSize, int numSets) {
  // Beginning with the conversion of the hexadecimal address to its decimal counterpart.
  long decimalAddress = stol(hexAddress, & sz, 16);

  // Determining the necessary number of bits for representing block size and set count.
  int numBlockBits = ceil(log2(blockSize)); // Determining the minimum required bits for block size representation.
  int numSetBits = ceil(log2(numSets)); // Determining the bit count necessary for set representation.

  // Calculating the offset within the block by computing the remainder of the decimal address divided by 2 raised to the power of the number of block bits.
  long blockOffset = decimalAddress % long(pow(2, numBlockBits));

  // Ensuring alignment of the block's starting address by subtracting the block offset from the decimal address.
  long blockStartAddress = decimalAddress - blockOffset;

  // Deriving the tag address by right-shifting the decimal address to remove bits representing the block offset and index.
  long tagAddress = decimalAddress >> (numSetBits + numBlockBits);

  // Determining the index position within the set by masking the decimal address with appropriate bits.
  int indexPosition = (decimalAddress >> numBlockBits) & ((1 << numSetBits) - 1);

  // Creating an instance of the PhysicalAddress structure to encapsulate the tag, block, and index.
  PhysicalAddr newAddress;

  // Converting the tag address to hexadecimal format and storing it within the PhysicalAddress structure.
  std::stringstream tagStream;
  tagStream << std::hex << tagAddress;
  newAddress.tagAddr = tagStream.str();

  // Converting the block's starting address to hexadecimal and storing it within the PhysicalAddress structure.
  std::stringstream blockStream;
  blockStream << std::hex << blockStartAddress;
  newAddress.blockAddr = blockStream.str();

  // Storing the index position within the PhysicalAddress structure.
  newAddress.indexPos = indexPosition;

  // Returning the constructed PhysicalAddress structure.
  return newAddress;

}

// creating new struct object out of each line in trace.
// Memory will be allocated in heap.
// globally accessible.
/*
This function, getFileContentStruct, takes two arguments: ops and addr, both of type string. It returns a pointer to a dynamically allocated fileContentStruct.

Here's a breakdown of what the function does:

It creates a new fileContentStruct object dynamically using the new keyword.
It assigns the value of the addr parameter to the hexAdd member variable of the newly created fileContentStruct.
It assigns the value of the ops parameter to the ops member variable of the newly created fileContentStruct.
Finally, it returns a pointer to the newly created fileContentStruct object.
This function essentially creates a fileContentStruct object with the provided ops and addr values and returns a pointer to it.

*/

fileContentStruct * getFileContentStruct(string ops, string addr) {
  fileContentStruct * temp = new fileContentStruct();
  temp -> hexAdd = addr;
  temp -> ops = ops;
  return temp;
}

/*

In summary, this function reads a trace file containing pairs of strings (presumably representing operations and hexadecimal addresses) and populates a vector named traceFileContent with corresponding fileContentStruct objects created from these pairs using the getFileContentStruct function.
*/
// Read Whole trace file and make vector out of it.
void readTraceFile(string traceFileName) {
  ifstream infile(traceFileName);
  string op;
  string hexAdd;
  while (infile >> op >> hexAdd) {
    traceFileContent.push_back(getFileContentStruct(op, hexAdd));
  }
}


// This function finds the optimal blocks to evict from the cache based on the given trace file content.
void FindBlocksToEvictOptimally(vector<fileContentStruct*>& traceFileContent, int startIndex, vector<string>& blockAddresses, int blockSize, int numSets) {
    // Iterate over the trace file content
    for (int i = startIndex; i < traceFileContent.size(); i++) {
        // Check if there's only one block address left in the set
        if (blockAddresses.size() == 1)
            return;

        // Iterate over the block addresses in the set
        for (int j = 0; j < blockAddresses.size(); j++) {
            // Extract the physical address from the trace file content
            PhysicalAddr newAddress = ExtractAddress(traceFileContent[i]->hexAdd, blockSize, numSets);

            // Check if the current block address matches the one in the set
            if (blockAddresses[j] == newAddress.blockAddr) {
                // Remove the matching block address from the set
                blockAddresses.erase(blockAddresses.begin() + j);
                break;
            }
        }
    }
}

// This function looks up a block in the L1 cache based on the given address and invalidates it.
// If the block is dirty, it performs a writeback to the main memory.
bool purgeL1CacheBlock(string hexAddress) {
  // Extract tag and index position from the address
  PhysicalAddr newAddress = ExtractAddress(hexAddress, cmdObj.GlobalblockSize, l1Sets);

  // Iterate through the cache lines in the set
  for (int j = 0; j < cmdObj.GlobalL1Assoc; j++) {
    // Check if the block address matches
    if (l1[newAddress.indexPos][j].blockAdd == newAddress.blockAddr) {
      // Check if the cache line is valid
      if (l1[newAddress.indexPos][j].valid == true) {
        // Invalidate the cache line
        l1[newAddress.indexPos][j].valid = false;
        // If the block is dirty, perform a writeback
        if (l1[newAddress.indexPos][j].dirtyBit == 1) {
          l1DirectWriteBacks += 1;
          return true;
        }
        // If the block is not dirty, no writeback is needed
        if (l1[newAddress.indexPos][j].dirtyBit == 0) {
          return false;
        }
      }
    }
  }
  // If the block is not found, return false
  return false;
}


// Function to decrement LCS counters for other blocks
void decrementLCSCounters(block** cache,int indexPos,int assoc) {
  for (int i = 0; i < assoc; i++) {
    if (cache[indexPos][i].LCS_Counter >= 1) {
      cache[indexPos][i].LCS_Counter--; // Boundary condition
    }
  }
}

long DetermineVictimCounter(block** cache , PhysicalAddr newAddress , int assoc) {
  // Initialize the victim serial number with the maximum possible value
  long victimSerial = INT_MAX;
  // Initialize the minimum LCS (Least Commonly Used) counter with the LCS counter of the first cache line
  int minLCSCounter = cache[newAddress.indexPos][0].LCS_Counter;

  // Determine the victim serial number based on the replacement policy
  if (cmdObj.ReplacementPolicy == 0 || cmdObj.ReplacementPolicy == 1) {
    for (int i = 0; i < assoc; i++) {
      if (!cache[newAddress.indexPos][i].valid) {
        // If the cache line is empty, assign its cache counter as the victim serial number
        victimSerial = cache[newAddress.indexPos][i].CacheCounter;
        break;
      } else {
        // Otherwise, find the minimum cache counter value among all cache lines
        victimSerial = min(victimSerial, cache[newAddress.indexPos][i].CacheCounter);
      }
    }
  }

  // If the replacement policy is Optimal
  if (cmdObj.ReplacementPolicy == 2) {
    // Iterate through the cache lines
    for (int j = 0; j < assoc; j++) {
      if (!cache[newAddress.indexPos][j].valid) {
        // If a vacant cache line is found, assign its cache counter as the victim serial number
        victimSerial = cache[newAddress.indexPos][j].CacheCounter;
        break;
      }
    }

    // If all cache lines are occupied
    if (victimSerial == INT_MAX) {
      // Prepare a list of block addresses in the set
      vector < string > blockAddresses;
      for (int j = 0; j < assoc; j++) {
        blockAddresses.push_back(cache[newAddress.indexPos][j].blockAdd);
      }

      // Find the block to evict based on the Optimal policy
      FindBlocksToEvictOptimally(traceFileContent, FileLineNum + 1, blockAddresses, cmdObj.GlobalblockSize, l1Sets);

      // Identify the block to evict by its block address
      string victimBlockAdd = blockAddresses[0];
      for (int j = 0; j < assoc; j++) {
        // Find the cache line associated with the victim block
        if (cache[newAddress.indexPos][j].blockAdd == victimBlockAdd) {
          // Assign its cache counter as the victim serial number
          victimSerial = cache[newAddress.indexPos][j].CacheCounter;
          break;
        }
      }
    }
  }

  // Determine the victim cache line using the LCS replacement policy
  if (cmdObj.ReplacementPolicy == 3) {
    // Iterate through the cache lines
    for (int i = 0; i < assoc; i++) {
      // If the cache line is empty, choose it as the victim
      if (!cache[newAddress.indexPos][i].valid) {
        victimSerial = cache[newAddress.indexPos][i].CacheCounter;
        break;
      } else {
        // Find the cache line with the minimum LCS counter value
        int minLCSCounter = cache[newAddress.indexPos][0].LCS_Counter;
        if (cache[newAddress.indexPos][i].LCS_Counter < minLCSCounter) {
          minLCSCounter = cache[newAddress.indexPos][i].LCS_Counter;
          // Assign its cache counter as the victim serial number
          victimSerial = cache[newAddress.indexPos][i].CacheCounter;
        }
      }
    }

    // Count the number of cache lines with the minimum LCS counter value
    int LCSMinimumCounterEntries = 0;
    for (int i = 0; i < assoc; i++) {
      if (cache[newAddress.indexPos][i].LCS_Counter == minLCSCounter) {
        LCSMinimumCounterEntries++;
      }
    }

    // If there are multiple cache lines with the minimum LCS counter value, choose the one with the minimum cache counter value
    if (LCSMinimumCounterEntries > 1) {
      for (int i = 0; i < assoc; i++) {
        if (cache[newAddress.indexPos][i].LCS_Counter == minLCSCounter) {
          // Find the cache line with the minimum cache counter value
          victimSerial = min(victimSerial, cache[newAddress.indexPos][i].CacheCounter);
        }
      }
    }

    // Decrement LCS counters for other cache lines in the set
    decrementLCSCounters(cache,newAddress.indexPos,assoc);
  }

  return victimSerial;
}

// L2 write
/*

It first checks if the L2 cache size is zero. If so, it returns immediately, indicating that there's no L2 cache to perform the operation on.

It calls the ExtractAddress function to obtain the corresponding PhysicalAddr structure (newAdd) for the given hexadecimal PhysicalAddr.

If it's a write operation (write == 1), it increments the l2Writes counter to keep track of the number of write operations.

It then checks if the PhysicalAddr is already present in the cache. If it is, it updates the LRU (Least Recently Used) or FIFO (First In, First Out) information depending on the replacement policy (cmdObj.ReplacementPolicy) and sets the dirty bit to indicate that the block has been modified.

If the PhysicalAddr is not present in the cache, it handles the cache miss by selecting a victim block for replacement. The replacement policy (cmdObj.ReplacementPolicy) determines how the victim block is chosen:

For LRU and FIFO, it selects the least recently used or the oldest block, respectively.
For the optimal policy, it calls the FindBlocksToEvictOptimally function to determine the blocks to delete and selects one based on the optimal replacement strategy.
Once a victim block is chosen, it updates the cache with the new PhysicalAddr, updating the block PhysicalAddr, tagAddr, CacheCounter number, and setting the dirty bit accordingly.

If the replacement involves evicting a block with a dirty bit set, it handles writebacks to lower-level caches or main memory based on the inclusion policy (cmdObj.InclusionPolicy).

Finally, it breaks out of the loop after inserting the new block into the cache.

This function essentially handles write operations in the L2 cache, ensuring data integrity and proper cache management.

*/
void L2CacheWriteOperation(string hexAddress, bool isWrite) {
  if (cmdObj.GlobalL2Size == 0) {
    return;
  }

  // Extract tag and index position from the address
  PhysicalAddr newAddress = ExtractAddress(hexAddress, cmdObj.GlobalblockSize, l2Sets);

  // If it's an explicit write operation
  if (isWrite == 1) {
    l2Writes += 1;

    // Check if the physical address exists in the cache
    for (int i = 0; i < cmdObj.GlobalL2Assoc; i++) {
      if (l2[newAddress.indexPos][i].tag == newAddress.tagAddr && l2[newAddress.indexPos][i].valid == true) {
        // Increment CacheCounter based on the replacement policy
        if (cmdObj.ReplacementPolicy != 1) {
          l2[newAddress.indexPos][i].CacheCounter = L2CacheCounter++;
        }
        // Set dirty bit and return
        l2[newAddress.indexPos][i].dirtyBit = 1;
        return;
      }
    }
    // Increment write misses if address not found in cache
    l2WriteMisses += 1;
  }

  // Find the victim cache line based on the replacement policy
  long victimCounter = DetermineVictimCounter(l2, newAddress, cmdObj.GlobalL2Assoc);

  // Insert the data into the cache line
  for (int i = 0; i < cmdObj.GlobalL2Assoc; i++) {
    if (l2[newAddress.indexPos][i].CacheCounter == victimCounter) {
      // Check if the cache line is valid
      if (l2[newAddress.indexPos][i].valid == true) {
        // Handle dirty bit based on inclusion policy
        if (l2[newAddress.indexPos][i].dirtyBit == true) {
          if (cmdObj.InclusionPolicy == 1) {
            bool isInvalidateSuccessful = purgeL1CacheBlock(l2[newAddress.indexPos][i].hexAdd);
            if (!isInvalidateSuccessful) {
              l2WriteBacks += 1;
            } else {
              l2WriteBacks += 1;
            }
          } else {
            l2WriteBacks += 1;
          }
        } else {
          if (cmdObj.InclusionPolicy == 1) {
            bool isInvalidateSuccessful = purgeL1CacheBlock(l2[newAddress.indexPos][i].hexAdd);
          }
        }
      } else {
      }
      // Update cache line with new data
      l2[newAddress.indexPos][i].blockAdd = newAddress.blockAddr;
      l2[newAddress.indexPos][i].indexPos = newAddress.indexPos;
      l2[newAddress.indexPos][i].hexAdd = hexAddress;
      l2[newAddress.indexPos][i].tag = newAddress.tagAddr;
      l2[newAddress.indexPos][i].CacheCounter = L2CacheCounter++;
      l2[newAddress.indexPos][i].valid = 1;
      // Set dirty bit based on operation type
      if (isWrite == 1) {
        l2[newAddress.indexPos][i].dirtyBit = 1;
      } else {
        l2[newAddress.indexPos][i].dirtyBit = 0;
      }
      break;
    }
  }
}


/*
This l2Read function is responsible for handling read operations in the L2 cache. It takes a hexadecimal PhysicalAddr (hexAdd) as input.

Here's a breakdown of the function:

It first checks if the L2 cache size is zero. If so, it returns immediately, indicating that there's no L2 cache to perform the operation on.

It calls the ExtractAddress function to obtain the corresponding PhysicalAddr structure (newAdd) for the given hexadecimal PhysicalAddr.

It increments the l2Reads counter to keep track of the number of read operations.

It then iterates over each way (entry) in the L2 cache at the set determined by newAdd.indexPos.

For each way, it checks if the tagAddr matches the tagAddr in the cache entry and if the entry is valid. If both conditions are true, it indicates a cache hit.

If it's a cache hit, it updates the LRU (Least Recently Used) or optimal information depending on the replacement policy (cmdObj.ReplacementPolicy).
It then returns, indicating that the read operation is complete.
If no cache hit occurs (i.e., the loop completes without finding a matching entry), it indicates a cache miss by incrementing the l2ReadMisses counter.

In case of a cache miss, it fetches the block into the L2 cache by calling the L2CacheWriteOperation function with the write flag set to 0, indicating a read operation.
*/
// L2 read function
void l2Read(string hexAdd) {
  if (cmdObj.GlobalL2Size == 0) {
    return;
  }

  // Get the tagAddr and indexPos
  PhysicalAddr newAdd = ExtractAddress(hexAdd, cmdObj.GlobalblockSize, l2Sets);

  // Attempt L2 Read
  l2Reads += 1;
  for (int i = 0; i < cmdObj.GlobalL2Assoc; i++) {
    if (l2[newAdd.indexPos][i].tag == newAdd.tagAddr && l2[newAdd.indexPos][i].valid == true) {

      if (cmdObj.ReplacementPolicy == 0 || cmdObj.ReplacementPolicy ==2 ) {
        l2[newAdd.indexPos][i].CacheCounter = L2CacheCounter++;
      }

      return;
    }
  }
  l2ReadMisses += 1;
  L2CacheWriteOperation(hexAdd, false);
}



// Function to update cache line with new data
void updateCacheLine(int indexPos, int i, string hexAdd, PhysicalAddr newAdd, bool IsWrite) {
  l1[indexPos][i].valid = true;
  l1[indexPos][i].hexAdd = hexAdd;
  l1[indexPos][i].blockAdd = newAdd.blockAddr;
  l1[indexPos][i].tag = newAdd.tagAddr;
  l1[indexPos][i].indexPos = newAdd.indexPos;
  l1[indexPos][i].CacheCounter = L1CacheCounter++;
  l1[indexPos][i].LCS_Counter = 5; // Initial value for LCS Counter

}



/*
The L1CacheWriteOperation function handles write operations in the L1 cache. It takes a hexadecimal PhysicalAddr (hexAdd) and a flag (write) indicating whether it's a write operation or not.

Here's a breakdown of the function:

It calls the ExtractAddress function to obtain the corresponding PhysicalAddr structure (newAdd) for the given hexadecimal PhysicalAddr.

If it's an explicit write operation (write == 1), it increments the l1Writes counter to keep track of the number of write operations.

It then checks if the PhysicalAddr is already present in the cache. If it is, it indicates a cache hit by setting the dirty bit and updating the LRU (Least Recently Used) or optimal information depending on the replacement policy (cmdObj.ReplacementPolicy).

If no cache hit occurs, it indicates a cache miss by incrementing the l1WriteMisses counter. It then determines the victim block for replacement based on the replacement policy.

If the victim block has a dirty bit set, indicating that it has been modified, it sends a writeback request to the next-level cache (L2 cache).

It then issues a read request to the L2 cache to fetch the data into the L1 cache.

Finally, it updates the cache with the new PhysicalAddr and appropriate metadata, including setting the dirty bit if it's a write operation.

This function ensures that write operations in the L1 cache are handled appropriately, maintaining cache coherence and efficiency by fetching data from the L2 cache or memory when necessary.


*/
// L1 write
void L1CacheWriteOperation(string hexAdd, bool IsWrite) {

  // Extract the physical address from the hexadecimal string
  PhysicalAddr newAdd = ExtractAddress(hexAdd, cmdObj.GlobalblockSize, l1Sets);

  // Process the write operation
  if (IsWrite) {
    // Increment the write operation count
    l1Writes++;

    // Check for a cache hit
    for (int i = 0; i < cmdObj.GlobalL1Assoc; i++) {
      if (l1[newAdd.indexPos][i].tag == newAdd.tagAddr && l1[newAdd.indexPos][i].valid) {
        // Update the cache counter according to the replacement policy
        if (cmdObj.ReplacementPolicy == 0 || cmdObj.ReplacementPolicy == 2 || cmdObj.ReplacementPolicy == 3) {
          l1[newAdd.indexPos][i].CacheCounter = L1CacheCounter++;
        }

        // Set the LCS counter to 5 for LCS replacement policy
        if (cmdObj.ReplacementPolicy == 3) {
          l1[newAdd.indexPos][i].LCS_Counter = 5;
        }

        // Set the dirty bit and exit on cache hit
        l1[newAdd.indexPos][i].dirtyBit = 1;
        return;
      }
    }

    // Increment the write miss count on cache miss
    l1WriteMisses++;
  }

  long victimSerial = DetermineVictimCounter(l1,newAdd,cmdObj.GlobalL1Assoc);

  for (int i = 0; i < cmdObj.GlobalL1Assoc; i++) {
    if (l1[newAdd.indexPos][i].CacheCounter == victimSerial) {
      if (l1[newAdd.indexPos][i].valid == false) {} else {
        if (l1[newAdd.indexPos][i].dirtyBit == 1) {
          l1[newAdd.indexPos][i].valid = false;
          l1Writebacks += 1;
          L2CacheWriteOperation(l1[newAdd.indexPos][i].hexAdd, true);
          //testing
          //l1DirectWriteBacks+=1;
        } else {
          l1[newAdd.indexPos][i].valid = false;
        }
      }

      l2Read(hexAdd);
      updateCacheLine(newAdd.indexPos, i, hexAdd, newAdd, IsWrite);
      if (IsWrite == true) {
        l1[newAdd.indexPos][i].dirtyBit = 1;
      } else {
        l1[newAdd.indexPos][i].dirtyBit = 0;
      }
      break;
    }
  }
}

// Increment the Least Common State (LCS) counter based on the previous value
// Region of Likeliness (ROL) based on the value of the counter.
// The ROLs include Least Likely to be Replaced (LeLR) constituting the counter values ranging from 12 to 15,
// Less Likely to be Replaced (LLR) constituting the counter values ranging from 6 to 11,
// Likely to be Replaced (LR) with counter values ranging from 2 to 5, and
// Most Likely to be Replaced (MLR) which includes the counter values 0 and 1.
// As we can observe from the table, the lower the counter value, the data is most likely to be replaced.
// The counter values get incremented on every repetition of the element. It is likely to stay in the cache longer, and therefore, it can be accessed very easily.
int IncrementLCSCounter(int previousValue) {
  int updatedValue;

  // Determine the updated value based on the previous LCS counter value
  if (previousValue == 0 || previousValue == 1) {
    updatedValue = 5; // Most Likely to be Replaced (MLR)
  } else if (previousValue >= 2 && previousValue <= 5) {
    updatedValue = 11; // Likely to be Replaced (LR)
  } else if (previousValue >= 6 && previousValue <= 15) {
    updatedValue = 15; // Less Likely to be Replaced (LLR)
  }

  return updatedValue;
}

/*The L1CacheReadOperation function is responsible for handling read operations in the L1 cache. It takes a hexadecimal PhysicalAddr (hexAdd) as input.

Here's a breakdown of the function:

It calls the ExtractAddress function to obtain the corresponding PhysicalAddr structure (newAdd) for the given hexadecimal PhysicalAddr.

It increments the l1Reads counter to keep track of the number of read operations.

It then iterates over each way (entry) in the L1 cache at the set determined by newAdd.indexPos.

For each way, it checks if the tagAddr matches the tagAddr in the cache entry and if the entry is valid. If both conditions are true, it indicates a cache hit.

If it's a cache hit, it updates the LRU (Least Recently Used) or optimal information depending on the replacement policy (cmdObj.ReplacementPolicy).
It then returns, indicating that the read operation is complete.
If no cache hit occurs (i.e., the loop completes without finding a matching entry), it indicates a cache miss by incrementing the l1ReadMisses counter.

In case of a cache miss, it fetches the block into the L1 cache by calling the L1CacheWriteOperation function with the write flag set to 0, indicating a read operation.

This function ensures that read operations in the L1 cache are handled appropriately, maintaining cache coherence and efficiency by fetching data from lower-level caches or memory when necessary.*/

// L1 read function
void L1CacheReadOperation(string hexAdd) {

  // Extract tag address and index position from the given hexadecimal address
  PhysicalAddr newAdd = ExtractAddress(hexAdd, cmdObj.GlobalblockSize, l1Sets);

  // Attempting L1 cache read
  l1Reads += 1;
  for (int i = 0; i < cmdObj.GlobalL1Assoc; i++) {
    // Check if the cache line matches the tag address and is valid
    if (l1[newAdd.indexPos][i].tag == newAdd.tagAddr && l1[newAdd.indexPos][i].valid) {
      // Update cache counters based on the replacement policy
      if (cmdObj.ReplacementPolicy == 0 || cmdObj.ReplacementPolicy == 2) { // LRU or Optimal
        l1[newAdd.indexPos][i].CacheCounter = L1CacheCounter++;
      }

      if (cmdObj.ReplacementPolicy == 3) { // LCS
        l1[newAdd.indexPos][i].LCS_Counter = IncrementLCSCounter(l1[newAdd.indexPos][i].LCS_Counter);
        l1[newAdd.indexPos][i].CacheCounter = L1CacheCounter++;
      }
      return; // Exiting loop on cache hit
    }
  }
  // Increment L1 read misses in case of cache miss
  l1ReadMisses += 1;

  // Perform L1 write operation to handle cache miss
  L1CacheWriteOperation(hexAdd, false);

}

// Function to create L1 Cache
block ** createCache(int sets, int assoc) {

  // Allocate memory for L1/L2 cache
  block ** cache = new(std::nothrow) block * [sets];
  if (!cache) {
    std::cerr << "Failed to allocate memory for cache sets" << std::endl;
    return 0;
  }

  // Initialize L1 cache with default values
  for (int i = 0; i < sets; i++) {
    cache[i] = new(std::nothrow) block[assoc];
    if (!cache[i]) {
      std::cerr << "Failed to allocate memory for cache blocks" << std::endl;
      // Free previously allocated memory
      for (int j = 0; j < i; j++) {
        delete[] cache[j];
      }
      delete[] cache;
      return 0;
    }
    for (int j = 0; j < assoc; j++) {
      cache[i][j].blockAdd = "";
      cache[i][j].hexAdd = "";
      cache[i][j].indexPos = i;
      cache[i][j].tag = "";
      cache[i][j].valid = false;
      cache[i][j].CacheCounter = 0;
      cache[i][j].dirtyBit = 0;
    }
  }

  // Return the created L1/L2 cache 
  return cache;
}

void PrintStatements() {

  // Print the initial configuration
  cout << "===== Simulator configuration =====" <<
    "\n";
  cout << "BLOCKSIZE:             " << cmdObj.GlobalblockSize << "\n";
  cout << "L1_SIZE:               " << cmdObj.GlobalL1Size << "\n";
  cout << "L1_ASSOC:              " << cmdObj.GlobalL1Assoc << "\n";
  cout << "L2_SIZE:               " << cmdObj.GlobalL2Size << "\n";
  cout << "L2_ASSOC:              " << cmdObj.GlobalL2Assoc << "\n";
  if (cmdObj.ReplacementPolicy == 0) {
    cout << "REPLACEMENT POLICY:    " <<
      "LRU" <<
      "\n";
  } else if (cmdObj.ReplacementPolicy == 1) {
    cout << "REPLACEMENT POLICY:    " <<
      "FIFO" <<
      "\n";
  } else if (cmdObj.ReplacementPolicy == 2) {
    cout << "REPLACEMENT POLICY:    " <<
      "optimal" <<
      "\n";
  } else if (cmdObj.ReplacementPolicy == 3) {
    cout << "REPLACEMENT POLICY:    " <<
      "LCS-logical Cache Segmentation" <<
      "\n";
  }

  if (cmdObj.InclusionPolicy == 0) {
    cout << "INCLUSION PROPERTY:    " <<
      "non-inclusive" <<
      "\n";
  } else if (cmdObj.InclusionPolicy == 1) {
    cout << "INCLUSION PROPERTY:    " <<
      "inclusive" <<
      "\n";
  }

  cout << "trace_file:            " << cmdObj.TraceFileName << "\n";

};

void PerformReadWriteOperations() {
  for (FileLineNum = 0; FileLineNum < traceFileContent.size(); FileLineNum++) {
    string Operation = traceFileContent[FileLineNum] -> ops;
    string HexAddr = traceFileContent[FileLineNum] -> hexAdd;

    if (Operation == "w") {

      L1CacheWriteOperation(HexAddr, true);
    }

    if (Operation == "r") {
      L1CacheReadOperation(HexAddr);
    }

  }

}

void PrintCacheStructure(block ** cache, int sets, int assoc) {

  for (int i = 0; i < sets; i++) {
    cout << "Set" <<
      "\t" << i << ":" <<
      "\t";

    for (int j = 0; j < assoc; j++) {
      cout << cache[i][j].tag;
      if (cache[i][j].dirtyBit == 1) {
        cout << " " <<
          "D";
      }
      cout << "\t";
    }
    cout << endl;
  }

}

void PrintCalculatedResults() {

  // Calculate L1 miss rate
  float l1MissRate = static_cast < float > (l1ReadMisses + l1WriteMisses) / static_cast < float > (l1Reads + l1Writes);

  // Initialize L2 miss rate
  float l2MissRate = 0;

  // Calculate L2 miss rate if L2 cache is enabled
  if (cmdObj.GlobalL2Size > 0) {
    l2MissRate = static_cast < float > (l2ReadMisses) / static_cast < float > (l2Reads);
  }

  // Calculate total memory traffic based on cache configurations
  int totalMemoryTraffic = 0;
  if (cmdObj.GlobalL2Size == 0) {
    totalMemoryTraffic = l1ReadMisses + l1WriteMisses + l1Writebacks;
  } else if (cmdObj.GlobalL2Size > 0 && cmdObj.InclusionPolicy == 0) {
    totalMemoryTraffic = l2ReadMisses + l2WriteMisses + l2WriteBacks;
  } else if (cmdObj.GlobalL2Size > 0 && cmdObj.InclusionPolicy == 1) {
    totalMemoryTraffic = l2ReadMisses + l2WriteMisses + l2WriteBacks + l1DirectWriteBacks;
  }

  // Display simulation results
  cout << "===== Simulation results (raw) =====" << endl;
  cout << "a. Number of L1 reads:         " << l1Reads << endl;
  cout << "b. Number of L1 read misses:   " << l1ReadMisses << endl;
  cout << "c. Number of L1 writes:        " << l1Writes << endl;
  cout << "d. Number of L1 write misses:  " << l1WriteMisses << endl;
  cout << "e. L1 miss rate:               " << fixed << setprecision(6) << l1MissRate << endl;
  cout << "f. Number of L1 writebacks:    " << l1Writebacks << endl;
  cout << "g. Number of L2 reads:         " << l2Reads << endl;
  cout << "h. Number of L2 read misses:   " << l2ReadMisses << endl;
  cout << "i. Number of L2 writes:        " << l2Writes << endl;
  cout << "j. Number of L2 write misses:  " << l2WriteMisses << endl;
  if (cmdObj.GlobalL2Size > 0) {
    cout << "k. L2 miss rate:               " << fixed << setprecision(6) << l2MissRate << endl;
  } else {
    cout << "k. L2 miss rate:               " << static_cast < int > (l2MissRate) << endl;
  }
  cout << "l. Number of L2 writebacks:    " << l2WriteBacks << endl;
  cout << "m. Total memory traffic:       " << totalMemoryTraffic << endl;

}

int InitialiseCommandLineArgsIntoStruct(int argc, char * argv[]) {

  if (argc < 8) {
    std::cerr << "Error: Insufficient number of arguments." << std::endl;
    std::cerr << "Expected format:  <block size> <L1 size><associativity> <L2 size> <L2 associativity><Replacement Policy> <Inclusion Policy> <Trace File>" << std::endl;
    std::cerr << "Example: ./program 16 1024 1 0 0 0 perl_trace.txt" << std::endl;
    return 1; // Returning non-zero value to indicate failure
  }

  cmdObj.GlobalblockSize = stol(argv[1], & sz);
  cmdObj.GlobalL1Size = stol(argv[2], & sz);
  cmdObj.GlobalL1Assoc = stol(argv[3], & sz);
  cmdObj.GlobalL2Size = stol(argv[4], & sz);
  cmdObj.GlobalL2Assoc = stol(argv[5], & sz);
  cmdObj.ReplacementPolicy = stol(argv[6], & sz);
  cmdObj.InclusionPolicy = stol(argv[7], & sz);
  cmdObj.TraceFileName = argv[8];
  return 0;

}

int main(int argc, char * argv[]) {

  if (InitialiseCommandLineArgsIntoStruct(argc, argv) == 1) {
    std::cerr << "Error: Exiting.....!" << std::endl;
  }

  PrintStatements();

  // Create L1 Cache
  l1Sets = cmdObj.GlobalL1Size / (cmdObj.GlobalblockSize * cmdObj.GlobalL1Assoc);
  l1 = createCache(l1Sets, cmdObj.GlobalL1Assoc);

  // Create L2 Cache
  if (cmdObj.GlobalL2Size > 0) {
    l2Sets = cmdObj.GlobalL2Size / (cmdObj.GlobalblockSize * cmdObj.GlobalL2Assoc);
    l2 = createCache(l2Sets, cmdObj.GlobalL2Assoc);

  }

  // Start the timer
  std::chrono::time_point < std::chrono::high_resolution_clock > start = std::chrono::high_resolution_clock::now();

  // Open and read trace file
  readTraceFile(cmdObj.TraceFileName);
  PerformReadWriteOperations();

  // Stop the timer
  std::chrono::time_point < std::chrono::high_resolution_clock > end = std::chrono::high_resolution_clock::now();

  // Print L1 Cache
  cout << "===== L1 contents =====" << endl;
  PrintCacheStructure(l1, l1Sets, cmdObj.GlobalL1Assoc);

  // Print L2 Cache
  if (cmdObj.GlobalL2Size > 0) {
    cout << "===== L2 contents =====" << endl;
    PrintCacheStructure(l2, l2Sets, cmdObj.GlobalL2Assoc);

  }

  // Calculations
  PrintCalculatedResults();

  // Calculate the duration in milliseconds
  std::chrono::duration < double, std::milli > duration = end - start;

  // Return the execution time in milliseconds
  //cout << "DURATION in milli seconds" << duration.count();
}