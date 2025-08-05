// MemoryManager.cpp
#include "MemoryManager.h"
#include <fstream>
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
    // Open the backing store for binary reading.
    std::fstream backing_store(backing_store_filename, std::ios::in | std::ios::binary);

    if (!backing_store.is_open()) {
        std::cerr << "[MMU] CRITICAL ERROR: Could not open backing store for reading." << std::endl;
        return;
    }

    // --- Calculate the exact position of the page in the file ---
    long long offset = ((long long)(pid - 1) * max_process_memory) + ((long long)page_number * frame_size);

    // Seek to that position.
    backing_store.seekg(offset);

    // Create a buffer to hold the page data.
    std::vector<char> page_data(frame_size);

    // Read a chunk of 'frame_size' bytes from the file into our buffer.
    backing_store.read(page_data.data(), frame_size);

    // Check if the read was successful. gcount() tells us how many bytes were actually read.
    if (backing_store.gcount() == 0) {
        // This means we are reading a page for the first time (it doesn't exist on disk yet).
        // This is normal for demand paging. In a real OS, this would be a page of zeros.
        std::cout << "[MMU] Loaded NEW page " << page_number << " for PID " << pid 
                  << " into frame " << frame_number << "." << std::endl;
    } else {
        std::cout << "[MMU] Loaded EXISTING page " << page_number << " for PID " << pid 
                  << " from backing store into frame " << frame_number << "." << std::endl;
    }

    backing_store.close();
    
    // NOTE: For this simulation, we don't need to do anything with 'page_data'.
    // The act of reading it from the file successfully simulates loading it into memory.
    // The physical_memory vector in MemoryManager is just for tracking metadata (Frames).
}

void MemoryManager::writePageToBackingStore(int pid, int page_number) {
    // Open the backing store for binary reading AND writing.
    // std::ios::ate places the cursor at the end initially, but seekp will move it.
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

    // --- Calculate the exact position to write the page ---
    long long offset = ((long long)(pid - 1) * max_process_memory) + ((long long)page_number * frame_size);

    // Create some dummy data to write. In a real OS, this would be the actual
    // contents of the RAM frame. Here, we just create a pattern to prove it works.
    std::vector<char> page_data(frame_size);
    for(size_t i = 0; i < frame_size; ++i) {
        // Fill with a pattern like "PID X, Page Y" for easy debugging in a hex editor.
        page_data[i] = (i % 2 == 0) ? (char)pid : (char)page_number;
    }

    // Seek to the correct position to write.
    backing_store.seekp(offset);

    // Write the contents of our buffer to the file.
    backing_store.write(page_data.data(), frame_size);

    backing_store.close();
    
    std::cout << "[MMU] Wrote dirty page " << page_number << " for PID " << pid 
              << " to backing store." << std::endl;
}

void MemoryManager::handlePageFault(Process& faulting_process, int page_number) {
    // Lock the mutex for the entire duration of the fault handling
    // to ensure the MMU's state is consistent and thread-safe.
    std::lock_guard<std::mutex> lock(mmu_mutex);

    std::cout << "[MMU] Page fault for PID " << faulting_process.getPid() 
              << ", Page " << page_number << "." << std::endl;

    int target_frame_index = -1;

    // --- STEP 1: FIND AN AVAILABLE PHYSICAL FRAME ---
    if (!free_frames.empty()) {
        // CASE A: We have free memory. This is the easy path.
        target_frame_index = free_frames.front();
        free_frames.pop_front();
        std::cout << "[MMU] Found free frame: " << target_frame_index << "." << std::endl;
    } else {
        // CASE B: No free frames. We must perform page replacement.
        target_frame_index = selectVictimFrame(); // Implements FIFO
        std::cout << "[MMU] No free frames. Evicting victim from frame: " << target_frame_index << "." << std::endl;

        // --- STEP 2: HANDLE THE VICTIM PROCESS (if one was chosen) ---
        // Get metadata about who owns the victim frame.
        Frame& victim_frame = physical_memory[target_frame_index];
        int victim_pid = victim_frame.owner_pid;
        int victim_page_number = victim_frame.page_number;
        
        // We must find the actual Process object for the victim to access its page table.
        // This is why we need access to the scheduler.
        Process* victim_process = os_scheduler->findProcessByName("Process" + std::to_string(victim_pid)); // Adjust name format if yours is different
        
        if (victim_process == nullptr) {
            // This is a critical error. It means our MMU's state is out of sync with the scheduler's.
            throw std::runtime_error("MMU CRITICAL ERROR: Could not find victim process with PID " + std::to_string(victim_pid));
        }

        // Check the victim page's dirty bit in its own PageTable.
        if (victim_process->getPageTable()->isDirty(victim_page_number)) {
            // If the page was modified, we MUST write its contents back to the
            // backing store to save the changes before we overwrite the frame.
            writePageToBackingStore(victim_pid, victim_page_number);
        }

        // Unmap the victim page in its PageTable. This sets its 'present_bit' to false.
        // From the victim process's perspective, this page is no longer in memory.
        victim_process->getPageTable()->unmapPage(victim_page_number);
        
        std::cout << "[MMU] Evicted Page " << victim_page_number << " from PID " << victim_pid << "." << std::endl;
    }

    // --- STEP 3: LOAD THE REQUIRED PAGE INTO THE TARGET FRAME ---
    // Now that we have a guaranteed free frame (target_frame_index), we can load the page
    // that the faulting process originally requested.
    loadPageFromBackingStore(faulting_process.getPid(), page_number, target_frame_index);

    // --- STEP 4: UPDATE ALL DATA STRUCTURES FOR THE NEW PAGE ---
    // A) Update the faulting process's page table: map the logical page to the physical frame.
    faulting_process.getPageTable()->mapPageToFrame(page_number, target_frame_index);

    // B) Update the physical frame's metadata to reflect its new owner.
    physical_memory[target_frame_index].assign(faulting_process.getPid(), page_number);

    // C) For FIFO, push the frame index to the back of the queue. It is now the "newest" frame in memory.
    fifo_queue.push_back(target_frame_index);
    
    std::cout << "[MMU] Page fault for PID " << faulting_process.getPid() << " resolved." << std::endl;
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