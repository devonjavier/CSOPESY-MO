// MemoryManager.cpp
#include "MemoryManager.h"
#include "process.h"       // For the full Process definition
#include "Scheduler.cpp"     // For finding victim processes
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

    // Create the vector of Frame objects.
    physical_memory.resize(num_frames);

    // Populate the list of free frames. Initially, all frames are free.
    for (size_t i = 0; i < num_frames; ++i) {
        free_frames.push_back(i);
    }

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

    // WILL UTILIZE I/O, CHECK LATER
    // In a real system, this would involve file I/O.
    std::cout << "[MMU] Loading page " << page_number << " for PID " << pid 
              << " from backing store into frame " << frame_number << "." << std::endl;
    // Simulate work
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void MemoryManager::writePageToBackingStore(int pid, int page_number) {

    // // WILL UTILIZE I/O, CHECK LATER
    // In a real system, this would involve file I/O.
    std::cout << "[MMU] Writing dirty page " << page_number << " for PID " << pid 
              << " to backing store." << std::endl;
    // Simulate work
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void MemoryManager::handlePageFault(Process& process, int page_number) {
    std::lock_guard<std::mutex> lock(mmu_mutex); // Lock for thread safety

    // TODO: Implement this complex logic.
    // 1. Find a frame:
    //    - If 'free_frames' is not empty, get a frame from it.
    //    - If 'free_frames' is empty, call 'selectVictimFrame()'.
    // 2. If you selected a victim frame:
    //    - Get the victim's PID and page number from physical_memory[victim_frame_index].
    //    - Find the victim process object.
    //    - Check the victim page's dirty_bit in its PageTable.
    //    - If it's dirty, call 'writePageToBackingStore'.
    //    - Unmap the victim page in its PageTable.
    // 3. Load the new page:
    //    - Call 'loadPageFromBackingStore' for the faulting process/page.
    // 4. Update structures:
    //    - Call 'process.getPageTable()->mapPageToFrame(...)'.
    //    - Update the 'physical_memory[frame_index]' with the new owner info.
    //    - Push the new frame_index to the back of the 'fifo_queue'.
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