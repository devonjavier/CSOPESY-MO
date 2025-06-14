#ifndef PROCESS_LOG_ENTRY_H
#define PROCESS_LOG_ENTRY_H

#include <string>           // For std::string
#include <chrono>           // For std::chrono::time_point and std::chrono::system_clock
#include <iostream>         // For std::cout (used in print method)
#include <iomanip>          // For std::put_time (used in print method)
#include <ctime>            // For std::localtime and std::time_t (used with std::put_time)

// Assuming these headers exist and define the Process class and ProcessState enum,
// along with the processStateToString function.
#include "Process.h"
#include "ProcessState.h"

// Forward declaration of ProcessStateToString if it's not in ProcessState.h
// extern std::string processStateToString(ProcessState state);


class ProcessLogEntry {
public:
    // Member variables to store the state of a process at a specific log timestamp
    std::chrono::time_point<std::chrono::system_clock> log_timestamp;
    int pid;
    std::string process_name;
    ProcessState state_at_log;
    int core_id_at_log;
    
    // Snapshots of process metrics at the time of logging
    int burst_time_snapshot;
    int remaining_burst_time_snapshot;
    int waiting_time_snapshot;
    int turnaround_time_snapshot;

    /**
     * @brief Constructor for ProcessLogEntry.
     * Initializes a log entry with the current state of a given Process object.
     * @param p The Process object to log.
     * @param state The current state of the process (e.g., RUNNING, READY, TERMINATED).
     * @param coreId The ID of the core the process is on, or -1 if not on a core.
     */
    ProcessLogEntry(const Process& p, ProcessState state, int coreId) :
        pid(p.getPid()),                               // Get PID from the Process object
        process_name(p.getProcessName()),             // Get process name from the Process object
        state_at_log(state),                          // Set the state at the time of logging
        core_id_at_log(coreId),                       // Set the core ID
        burst_time_snapshot(p.getBurstTime()),        // Snapshot of total burst time
        remaining_burst_time_snapshot(p.getRemainingBurstTime()), // Snapshot of remaining burst time
        waiting_time_snapshot(p.getWaitingTime()),    // Snapshot of waiting time
        turnaround_time_snapshot(p.getTurnaroundTime()) // Snapshot of turnaround time
    {
        // Record the exact time this log entry was created
        log_timestamp = std::chrono::system_clock::now();
    }

    /**
     * @brief Prints the details of the process log entry to standard output.
     * The timestamp is formatted, and all snapshots are displayed.
     */
    void print() const {
        // Convert the chrono timestamp to a time_t for use with std::localtime and std::put_time
        std::time_t tt = std::chrono::system_clock::to_time_t(log_timestamp);
        
        // Using std::put_time for formatted time output (e.g., "YYYY-MM-DD HH:MM:SS")
        std::cout << "Time: " << std::put_time(std::localtime(&tt), "%Y-%m-%d %H:%M:%S")
                  << " | PID: " << pid
                  << " | Name: " << process_name
                  << " | State: " << processStateToString(state_at_log); // Assumes processStateToString is available
        
        // Only print core ID if it's a valid core (not -1)
        if (core_id_at_log != -1) {
            std::cout << " | Core: " << core_id_at_log;
        }
        
        // Print all the snapshot metrics
        std::cout << " | BT: " << burst_time_snapshot
                  << " | RBT: " << remaining_burst_time_snapshot
                  << " | WT: " << waiting_time_snapshot
                  << " | TAT: " << turnaround_time_snapshot << std::endl;
    }
};

#endif // PROCESS_LOG_ENTRY_H