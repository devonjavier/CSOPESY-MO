#pragma once

struct Frame {
    bool is_free = true;
    int owner_pid = -1;      // The PID of the process that owns the page in this frame.
    int page_number = -1;  // The virtual page number of the process's page.
    
    // Helper function to assign a process's page to this frame.
    void assign(int pid, int page_num) {
        is_free = false;
        owner_pid = pid;
        page_number = page_num;
    }

    // Helper function to mark this frame as free.
    void reset() {
        is_free = true;
        owner_pid = -1;
        page_number = -1;
    }
};