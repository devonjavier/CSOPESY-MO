// MemoryManager.h
#pragma once

#include <vector>
#include <deque>
#include <mutex>
#include "Frame.h"

class Process; 

class MemoryManager {
private:
    std::vector<Frame> physical_memory;
    std::deque<int> free_frames; // A queue of indices for available frames.
    std::deque<int> fifo_queue;  // A queue of frame indices for the FIFO replacement algorithm.
    std::mutex mmu_mutex;        // To ensure thread-safe access to memory structures.
    size_t frame_size;
    size_t max_process_memory; // Will store mem_per_proc
    const std::string backing_store_filename = "csopesy-backing-store.txt";


    int selectVictimFrame();
    void loadPageFromBackingStore(int pid, int page_number, int frame_number);
    void writePageToBackingStore(int pid, int page_number);

public:
    MemoryManager(size_t total_memory_size, size_t frame_size, size_t mem_per_proc);


    void handlePageFault(Process& process, int page_number);

    size_t getTotalMemory() const;
    size_t getFreeMemory() const;
    size_t getUsedMemory() const;

};