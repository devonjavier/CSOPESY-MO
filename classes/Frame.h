#pragma once

struct Frame {
    bool is_free = true;
    int owner_pid = -1;      
    int page_number = -1;  
    
    void assign(int pid, int page_num) {
        is_free = false;
        owner_pid = pid;
        page_number = page_num;
    }

    void reset() {
        is_free = true;
        owner_pid = -1;
        page_number = -1;
    }
};