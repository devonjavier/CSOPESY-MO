#include "Process.h" // Include its own header
#include <iostream>  // For std::cout in displayProcess

Process::Process(int id, const std::string& name, int prio, int burst)
    : pid(id), process_name(name), priority(prio), burst_time(burst),
      remaining_burst_time(burst), waiting_time(0), turnaround_time(0),
      current_core_id(-1), state(ProcessState::NONE) {
    start_time = std::chrono::system_clock::now();
}

int Process::getPid() const {
    return pid;
}
std::string Process::getProcessName() const {
    return process_name;
}
int Process::getPriority() const {
    return priority;
}
int Process::getBurstTime() const {
    return burst_time;
}
int Process::getRemainingBurstTime() const {
    return remaining_burst_time;
}
int Process::getWaitingTime() const {
    return waiting_time;
}
int Process::getTurnaroundTime() const {
    return turnaround_time;
}
std::chrono::time_point<std::chrono::system_clock> Process::getStartTime() const {
    return start_time;
}
int Process::getCurrentCoreId() const {
    return current_core_id;
}
ProcessState Process::getState() const {
    return state;
}

// std::string getFormattedStartDate() const {
//     struct tm temp;

//     #ifdef _WIN32
//         if (localtime_s(&temp, &process_start_date) != 0) {
//             return "Error formatting time (Windows)"; //error message
//         }
//     #else //macOS for andrei
//         struct tm* localTm = localtime(&process_start_date);
//         if (localTm == nullptr) {
//             return "Error formatting time (Unix)";
//         }
//         temp = *localTm;
//     #endif

//     std::ostringstream oss;
//     // %d: Day, %m: Month, %Y: Year, %I: Hour, %M: Minute, %S: Second, %p: AM/PM indicator
//     oss << std::put_time(&temp, "(%d/%m/%Y %I:%M:%S%p)");
//     return oss.str(); //return formatted date
// }

void Process::setState(ProcessState newState, int coreId) {
    this->state = newState;
    this->current_core_id = coreId;
    if (newState == ProcessState::RUNNING && coreId != -1 &&
        this->start_time.time_since_epoch().count() == 0) {
        this->start_time = std::chrono::system_clock::now();
    }
}

void Process::displayProcess() const {
    // std::cout << "Process ID: " << pid << "\n"
            //           << "Process Name: " << process_name << "\n"
            //           << "Priority: " << priority << "\n"
            //           << "Burst Time: " << burst_time << "\n"
            //           << "Waiting Time: " << waiting_time << "\n"
            //           << "Turnaround Time: " << turnaround_time 
            //           << "Process Start Date: " << getFormattedStartDate() << "\n"
            //           << "Current Core ID: " << current_core_id << "\n"
            //           << std::endl;
    std::cout << "Process ID: " << pid << "\n"
              << "Process Name: " << process_name << "\n"
              << "Priority: " << priority << "\n"
              << "Burst Time: " << burst_time << "\n"
              << "Remaining Burst Time: " << remaining_burst_time << "\n"
              << "Waiting Time: " << waiting_time << "\n"
              << "Turnaround Time: " << turnaround_time << "\n"
              << "Current Core ID: " << current_core_id << "\n"
              << "State: " << processStateToString(state) // Use helper function!
              << std::endl;
}