// PageTable.h
#pragma once

#include <vector>
#include <cstddef> // for size_t

/**
 * @class PageTable
 * @brief Manages the mapping of a single process's virtual pages to physical frames.
 * Each process will own one instance of this class.
 */
class PageTable {
public:
    // Represents a single entry in the page table.
    struct PageTableEntry {
        bool present_bit = false; // Is the page currently in a physical frame?
        bool dirty_bit = false;   // Has the page been modified since being loaded?
        int frame_number = -1;  // The physical frame number where the page is located.
    };

private:
    std::vector<PageTableEntry> entries;
    size_t num_pages;
    size_t page_size;

public:
    // Constructor: Calculates the number of pages needed for the process.
    PageTable(size_t process_memory_size, size_t page_size);
    
    // Gets the frame number for a given page, returns -1 if not present.
    int getFrameNumber(int page_number) const;

    // Checks if a page is currently in physical memory.
    bool isPresent(int page_number) const;
    
    // Checks if a page has been modified.
    bool isDirty(int page_number) const;
    
    // Sets the dirty bit for a page.
    void setDirty(int page_number, bool is_dirty);

    // Updates the entry when a page is loaded into a frame.
    void mapPageToFrame(int page_number, int frame_number);

    // Updates the entry when a page is evicted from a frame.
    void unmapPage(int page_number);

    size_t getPageSize() const; // Returns the size of a page in bytes
};