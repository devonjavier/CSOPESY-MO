// PageTable.h
#pragma once

#include <vector>
#include <cstddef> // for size_t

class PageTable {
public:
    
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
    PageTable(size_t process_memory_size, size_t page_size);

    int getFrameNumber(int page_number) const;
    bool isPresent(int page_number) const;
    bool isDirty(int page_number) const;
    void setDirty(int page_number, bool is_dirty);
    void mapPageToFrame(int page_number, int frame_number);
    void unmapPage(int page_number);

    size_t getPageSize() const; 
};