// MemoryManager.cpp
#include "MemoryManager.h"
#include <fstream>
#include "process.h"       
#include "Scheduler.cpp"    
#include "PageTable.cpp"
#include <iostream>
#include <stdexcept>

extern Scheduler* os_scheduler;


MemoryManager::MemoryManager(size_t total_memory_size, size_t frame_size, size_t mem_per_proc) {
    if (frame_size == 0) {
        throw std::invalid_argument("Frame size cannot be zero.");
    }
    this->frame_size = frame_size;
    this->max_process_memory = mem_per_proc;
    size_t num_frames = total_memory_size / frame_size;
    physical_memory.resize(num_frames);
    for (size_t i = 0; i < num_frames; ++i) {
        free_frames.push_back(i);
    }

    std::ofstream backing_store(backing_store_filename, std::ios::trunc | std::ios::binary);
    if (!backing_store.is_open()) {
        std::cerr << "[MMU] WARNING: Could not clear backing store file on startup." << std::endl;
    }
    backing_store.close();

    std::cout << "[MMU] Memory Manager initialized with " << num_frames 
              << " frames of " << frame_size << " bytes each." << std::endl;
}

int MemoryManager::selectVictimFrame() {

    if (fifo_queue.empty()) {
        throw std::runtime_error("Attempted to select a victim frame, but FIFO queue is empty.");
    }

    int victim_frame_index = fifo_queue.front();
    
    fifo_queue.pop_front();

    return victim_frame_index;
}

void MemoryManager::loadPageFromBackingStore(int pid, int page_number, int frame_number) {

    std::fstream backing_store(backing_store_filename, std::ios::in | std::ios::binary);

    if (!backing_store.is_open()) {
        std::cerr << "[MMU] CRITICAL ERROR: Could not open backing store for reading." << std::endl;
        return;
    }

    long long offset = ((long long)(pid - 1) * max_process_memory) + ((long long)page_number * frame_size);
    backing_store.seekg(offset);
    std::vector<char> page_data(frame_size);
    backing_store.read(page_data.data(), frame_size);

    if (backing_store.gcount() == 0) {

        std::cout << "[MMU] Loaded NEW page " << page_number << " for PID " << pid 
                  << " into frame " << frame_number << "." << std::endl;
    } else {
        std::cout << "[MMU] Loaded EXISTING page " << page_number << " for PID " << pid 
                  << " from backing store into frame " << frame_number << "." << std::endl;
    }

    backing_store.close();

}

void MemoryManager::writePageToBackingStore(int pid, int page_number) {

    std::fstream backing_store(backing_store_filename, std::ios::in | std::ios::out | std::ios::binary | std::ios::ate);
    
    if (!backing_store.is_open()) {
        // If the file doesn't exist, create it.
        std::ofstream new_file(backing_store_filename, std::ios::binary);
        new_file.close();
        // And try opening it again.
        backing_store.open(backing_store_filename, std::ios::in | std::ios::out | std::ios::binary | std::ios::ate);
        if(!backing_store.is_open()){
            std::cerr << "[MMU] CRITICAL ERROR: Could not create or open backing store for writing." << std::endl;
            return;
        }
    }


    long long offset = ((long long)(pid - 1) * max_process_memory) + ((long long)page_number * frame_size);


    std::vector<char> page_data(frame_size);
    for(size_t i = 0; i < frame_size; ++i) {
        page_data[i] = (i % 2 == 0) ? (char)pid : (char)page_number;
    }

    backing_store.seekp(offset);
    backing_store.write(page_data.data(), frame_size);
    backing_store.close();
    
    std::cout << "[MMU] Wrote dirty page " << page_number << " for PID " << pid 
              << " to backing store." << std::endl;
}

void MemoryManager::handlePageFault(Process& faulting_process, int page_number) {
    std::lock_guard<std::mutex> lock(mmu_mutex);

    std::cout << "[MMU] Page fault for PID " << faulting_process.getPid() 
              << ", Page " << page_number << "." << std::endl;

    int target_frame_index = -1;


    if (!free_frames.empty()) {
        target_frame_index = free_frames.front();
        free_frames.pop_front();
        std::cout << "[MMU] Found free frame: " << target_frame_index << "." << std::endl;
    } else {
        target_frame_index = selectVictimFrame(); 
        std::cout << "[MMU] No free frames. Evicting victim from frame: " << target_frame_index << "." << std::endl;

        Frame& victim_frame = physical_memory[target_frame_index];
        int victim_pid = victim_frame.owner_pid;
        int victim_page_number = victim_frame.page_number;
        
        Process* victim_process = os_scheduler->findProcessByName("Process" + std::to_string(victim_pid)); 
        
        if (victim_process == nullptr) {
            throw std::runtime_error("MMU CRITICAL ERROR: Could not find victim process with PID " + std::to_string(victim_pid));
        }

        if (victim_process->getPageTable()->isDirty(victim_page_number)) {
            std::cout << "[MMU] Writing dirty page " << victim_page_number << " for PID " << victim_pid 
                      << " to backing store." << std::endl;
            writePageToBackingStore(victim_pid, victim_page_number);
        }

        victim_process->getPageTable()->unmapPage(victim_page_number);
        

        std::cout << "[MMU] Evicted Page " << victim_page_number << " from PID " << victim_pid << "." << std::endl;
    }

    loadPageFromBackingStore(faulting_process.getPid(), page_number, target_frame_index);
    faulting_process.getPageTable()->mapPageToFrame(page_number, target_frame_index);
    physical_memory[target_frame_index].assign(faulting_process.getPid(), page_number);

    fifo_queue.push_back(target_frame_index);
    
    std::cout << "[MMU] Page fault for PID " << faulting_process.getPid() << " resolved." << std::endl;
}

void MemoryManager::releaseProcessMemory(int pid) {
    std::lock_guard<std::mutex> lock(mmu_mutex);

    std::cout << "[MMU] Releasing all memory frames for terminated PID " << pid << "." << std::endl;

    for (size_t i = 0; i < physical_memory.size(); ++i) {
        if (physical_memory[i].owner_pid == pid) {

            physical_memory[i].reset();
            free_frames.push_back(i);
            fifo_queue.erase(std::remove(fifo_queue.begin(), fifo_queue.end(), i), fifo_queue.end());
        }
    }
}

size_t MemoryManager::getPageSize() const {
    return this->frame_size;
}


size_t MemoryManager::getTotalMemory() const {
    return physical_memory.size() * frame_size;
}

size_t MemoryManager::getFreeMemory() const {
    return free_frames.size() * frame_size;
}

size_t MemoryManager::getUsedMemory() const {
    return getTotalMemory() - getFreeMemory();
}