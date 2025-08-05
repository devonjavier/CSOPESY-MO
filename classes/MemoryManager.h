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
    std::deque<int> free_frames; 
    std::deque<int> fifo_queue;  
    std::mutex mmu_mutex;        
    size_t frame_size;
    size_t max_process_memory; 
    const std::string backing_store_filename = "csopesy-backing-store.txt";
    mutable std::mutex mmu_mutex; 
    
    int selectVictimFrame();
    void releaseProcessMemory(int pid);
    void loadPageFromBackingStore(int pid, int page_number, int frame_number);
    void writePageToBackingStore(int pid, int page_number);

public:
    MemoryManager(size_t total_memory_size, size_t frame_size, size_t mem_per_proc);


    void handlePageFault(Process& process, int page_number);
    size_t getPageSize() const;
    size_t getTotalMemory() const;
    size_t getFreeMemory() const;
    size_t getUsedMemory() const;

};