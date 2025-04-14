
# 🔄 Cache Simulator with Logical Cache Segmentation (LCS)

**Project Title:** Enhancement of Cache Performance using Adaptive Replacement Policy  
**Course:** CDA 5106: Computer Architecture, Spring 2024  
**University:** University of Central Florida  


---

## 📘 Overview

This project presents a custom-built cache simulator aimed at evaluating and improving cache performance using a novel replacement policy: **Logical Cache Segmentation (LCS)**. The simulator supports configurable parameters and compares LCS against traditional policies like **LRU**, **FIFO**, and **Optimal**.

The project is designed to:
- Simulate multi-level cache behavior
- Measure performance using SPEC-2000 traces
- Analyze the effects of varying cache configurations and policies

---

## 🚀 Key Features

- ✅ Supports multiple replacement policies: LCS, LRU, FIFO, Optimal  
- ✅ Configurable cache parameters: size, associativity, block size  
- ✅ Implements **Regions of Likeliness (ROL)**:  
  - MLR (0-1): Most Likely to Replace  
  - LR (2-5): Likely to Replace  
  - LLR (6-11): Less Likely to Replace  
  - LELR (12-15): Least Likely to Replace  
- ✅ Hit/miss tracking and dynamic counter adjustments  
- ✅ Graphs generated for hit rate, miss rate, and average access time (AAT)

---

## 🧠 How Logical Cache Segmentation (LCS) Works

- Every cache block has a **counter (0-15)**.
- **Hits** increase the block’s ROL (towards LELR), reinforcing its retention.
- **Misses** trigger:
  - Search for the lowest ROL block
  - Replacement if needed, resetting counter to 5 (LR region)
- Dynamically adjusts to input trace patterns for better performance.

---

## 📊 Performance Highlights

- **Higher hit rates** and **lower miss rates** than LRU/FIFO in most workloads.
- Excels with `Vortex`, `Compress`, `Perl`, and `GCC` SPEC-2000 traces.
- Demonstrated **lower AAT** in simulations with varied cache sizes and associativities.

---

## 📁 Directory Structure

```plaintext
AdaptiveCacheDesignSimulator/
├── Globals.h                  # Global configurations and constants
├── Makefile                   # Compilation script
├── README.md                  # Project documentation
├── SimulatorCacheDesign.cpp  # Main implementation file
├── SimulatorCacheDesign.o    # Object file (compiled)
├── Structs.h                  # Data structures used in simulation
├── checkDiffChecker.sh       # Shell script for result verification
├── compress_trace.txt        # SPEC-2000 trace file: Compress
├── gcc_trace.txt             # SPEC-2000 trace file: GCC
├── go_trace.txt              # SPEC-2000 trace file: Go
├── perl_trace.txt            # SPEC-2000 trace file: Perl
├── sim_cache                 # Compiled executable (simulator)
├── vortex_trace.txt          # SPEC-2000 trace file: Vortex
├── annotated.report         # View the report
```
---

## 📥 Getting Started

### Prerequisites
- C++ compiler (G++ or Clang)
- Python (for visualization scripts)
- GNUplot or Matplotlib (optional, for graphs)

### Compilation
g++ lcs_simulator.cpp -o cache_simulator

### Run
./cache_simulator trace_files/gcc.txt

---

## 📈 Sample Results

**Miss Rate (LCS vs LRU vs FIFO):**
- LCS: 12.3%
- LRU: 15.8%
- FIFO: 18.6%

**Average Access Time (ms):**
- LCS: 2.01 ms
- LRU: 2.45 ms
- FIFO: 2.89 ms

**Hit Rate (Higher is Better):**
- LCS: 87%
- LRU: 82%
- FIFO: 79%

> 📌 Detailed graphs are included in the `report.pdf` and `/results/` folder.

---

## 📄 Read the Research Paper

📘 Full Report (PDF): ./report.pdf

---

## ⚠️ Limitations

- Slightly higher computational complexity due to counter updates
- Requires parameter tuning per workload
- Adds minor storage overhead to maintain counters

---



---

## 📚 References

> Full list available in the paper

- Kharbutli et al., *Counter-Based Cache Replacement*, IEEE  
- Ghasemzadeh et al., *Modified Pseudo-LRU Algorithm*  
- The gem5 simulator project  

---

## 📬 Contact

For any questions or feedback, reach out to:  
📧 manasakaranam6199@gmail.com
