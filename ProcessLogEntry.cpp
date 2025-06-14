#include "ProcessLogEntry.h" // Include your own header first!


ProcessLogEntry::ProcessLogEntry(const Process& p, ProcessState state, int coreId) :
    pid(p.getPid()),
    process_name(p.getProcessName()),
    state_at_log(state),
    core_id_at_log(coreId),
    burst_time_snapshot(p.getBurstTime()),
    remaining_burst_time_snapshot(p.getRemainingBurstTime()),
    waiting_time_snapshot(p.getWaitingTime()),
    turnaround_time_snapshot(p.getTurnaroundTime())
{
    log_timestamp = std::chrono::system_clock::now();
}

// print() method definition
void ProcessLogEntry::print() const {
    std::time_t tt = std::chrono::system_clock::to_time_t(log_timestamp);
    std::cout << "Time: " << std::put_time(std::localtime(&tt), "%Y-%m-%d %H:%M:%S")
              << " | PID: " << pid
              << " | Name: " << process_name
              << " | State: " << processStateToString(state_at_log);
    if (core_id_at_log != -1) {
        std::cout << " | Core: " << core_id_at_log;
    }
    std::cout << " | BT: " << burst_time_snapshot
              << " | RBT: " << remaining_burst_time_snapshot
              << " | WT: " << waiting_time_snapshot
              << " | TAT: " << turnaround_time_snapshot << std::endl;
}