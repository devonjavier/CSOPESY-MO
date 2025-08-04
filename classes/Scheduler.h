#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <queue>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <algorithm>

// forward‐declare Process and its enum
#include "Process.h"

struct Status {
  int totalCores, busyCores, freeCores;
  double cpuUtil;                        // in percent
  std::vector<std::string> readyList,
                           runningList,
                           finishedList;
};
Status getStatus() const;


class Scheduler {
private:
    std::vector<Process> processes;
    std::queue<Process> ready_queue; 
    std::vector<Process> runningProcesses;
    std::vector<Process> completedProcesses;
    std::vector<std::thread> workerThreads;
    std::atomic<bool> schedulerRunning{false};
    std::atomic<bool> generatingProcesses{false};
    std::mutex queueMutex;
    std::condition_variable queueCV;
    std::string SchedulerType;
    int quantumCycles; 
    uint16_t programcounter = 0;

    void checkIfComplete();
    void schedulerAlgo(int coreId);

public:
    Scheduler(const std::string& schedulerType, int quantum);

    void addProcess(const Process& process);
    void queueProcesses();
    void startScheduler(int num_cpu);
    void stopGenerating();
    void stopScheduler();
    void finalizeScheduler();

    bool isSchedulerRunning() const;
    bool isGeneratingProcesses() const;
    bool runningThreadsDone();

    void displayProcessList();

    double FCFS();

    Process* findProcessByName(const std::string& name);
};
#endif // SCHEDULER_H
