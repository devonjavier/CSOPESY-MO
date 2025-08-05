
// PageTable.cpp
#include "PageTable.h"
#include <stdexcept> // For error handling

// need to pass memory size, need checker in process ? 
PageTable::PageTable(size_t process_memory_size, size_t page_size) {
if (page_size == 0) {
        throw std::invalid_argument("Page size cannot be zero.");
    }

    this->num_pages = (process_memory_size + page_size - 1) / page_size;

    entries.resize(this->num_pages);
}

int PageTable::getFrameNumber(int page_number) const {
    if (page_number < 0 || page_number >= this->num_pages) {
        throw std::out_of_range("Page number is out of the valid range for this process.");
    }

    // check if present in memory
    if (entries[page_number].present_bit) {
        return entries[page_number].frame_number;
    }

    return -1;
}

bool PageTable::isPresent(int page_number) const {
    if (page_number < 0 || page_number >= this->num_pages) {
        throw std::out_of_range("Page number is out of the valid range for this process.");
    }

    return entries[page_number].present_bit;
}

bool PageTable::isDirty(int page_number) const {
    if (page_number < 0 || page_number >= this->num_pages) {
        throw std::out_of_range("Page number is out of the valid range for this process.");
    }

    return entries[page_number].dirty_bit;
}

void PageTable::setDirty(int page_number, bool is_dirty) {
    if (page_number < 0 || page_number >= this->num_pages) {
        throw std::out_of_range("Page number is out of the valid range for this process.");
    }
    
    entries[page_number].dirty_bit = is_dirty;
}

void PageTable::mapPageToFrame(int page_number, int frame_number) {
    if (page_number < 0 || page_number >= this->num_pages) {
        throw std::out_of_range("Page number is out of the valid range for this process.");
    }


    entries[page_number].present_bit = true;
    entries[page_number].frame_number = frame_number;
    

    entries[page_number].dirty_bit = false; 
}

void PageTable::unmapPage(int page_number) {
    if (page_number < 0 || page_number >= this->num_pages) {
        throw std::out_of_range("Page number is out of the valid range for this process.");
    }

    entries[page_number].present_bit = false;
    entries[page_number].frame_number = -1;
}