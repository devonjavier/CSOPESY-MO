#include "Process.cpp"
#include <queue>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <chrono>
#include <algorithm> // for std::clamp

class Scheduler { 
private:
    std::vector<std::shared_ptr<Process>> processes;
    std::queue<std::shared_ptr<Process>> ready_queue; 
    std::vector<std::shared_ptr<Process>> runningProcesses;
    std::vector<std::shared_ptr<Process>> completedProcesses;
    std::vector<std::thread> workerThreads;
    std::atomic<bool> SchedulerRunning{false};
    mutable std::mutex queueMutex;
    mutable std::mutex processesMutex;
    std::condition_variable queueCV;
    std::string SchedulerType; 
    int quantumCycles;
    int numCores;
    int delays_perexec;

    void SchedulerLoop(int coreId) {
        while (SchedulerRunning) {
            std::shared_ptr<Process> proc = nullptr;
            
            // Get next process from ready queue
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                queueCV.wait(lock, [this]{ return !ready_queue.empty() || !SchedulerRunning; });
                
                if (!SchedulerRunning) break;
                
                if (!ready_queue.empty()) {
                    proc = ready_queue.front();
                    ready_queue.pop();
                }
            }
            
            if (proc) {
                // Assign core and execute process
                proc->setCurrentCoreId(coreId);
                proc->setState(ProcessState::RUNNING);
                
                // Add to running processes
                {
                    std::lock_guard<std::mutex> lock(processesMutex);
                    runningProcesses.push_back(proc);
                }
                
                executeProcess(proc, coreId);
                
                // Remove from running processes
                {
                    std::lock_guard<std::mutex> lock(processesMutex);
                    auto it = std::find(runningProcesses.begin(), runningProcesses.end(), proc);
                    if (it != runningProcesses.end()) {
                        runningProcesses.erase(it);
                    }
                }
            }
        }
    }

    void executeProcess(std::shared_ptr<Process> proc, int coreId) {
        if (SchedulerType == "fcfs") {
            executeFCFS(proc, coreId);
        } else if (SchedulerType == "rr") {
            executeRoundRobin(proc, coreId);
        }
    }

    void executeFCFS(std::shared_ptr<Process> proc, int coreId) {
        // Execute until completion
        while (proc->getState() == ProcessState::RUNNING && SchedulerRunning) {
            if (proc->executeNextInstruction()) {
                // Process completed
                proc->setState(ProcessState::FINISHED);
                proc->setEndTime(std::chrono::system_clock::now());
                
                {
                    std::lock_guard<std::mutex> lock(processesMutex);
                    completedProcesses.push_back(proc);
                }
                break;
            }
            
            // Add delay per execution if configured
            if (delays_perexec > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(delays_perexec));
            }
        }
    }

    void executeRoundRobin(std::shared_ptr<Process> proc, int coreId) {
        int cyclesExecuted = 0;
        
        while (proc->getState() == ProcessState::RUNNING && 
               cyclesExecuted < quantumCycles && 
               SchedulerRunning) {
            
            if (proc->executeNextInstruction()) {
                // Process completed
                proc->setState(ProcessState::FINISHED);
                proc->setEndTime(std::chrono::system_clock::now());
                
                {
                    std::lock_guard<std::mutex> lock(processesMutex);
                    completedProcesses.push_back(proc);
                }
                return;
            }
            
            cyclesExecuted++;
            
            // Add delay per execution if configured
            if (delays_perexec > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(delays_perexec));
            }
        }
        
        // If process didn't complete, put it back in ready queue
        if (proc->getState() == ProcessState::RUNNING) {
            proc->setState(ProcessState::WAITING);
            proc->setCurrentCoreId(-1); // Unassign core
            
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                ready_queue.push(proc);
                queueCV.notify_one();
            }
        }
    }

public:
    Scheduler(const std::string& scheduler, int quantum, int numCores, int delays_perexec) 
        : SchedulerType(scheduler), quantumCycles(quantum), numCores(numCores), delays_perexec(delays_perexec) {}

    ~Scheduler() {
        stopScheduler();
    }

    void addProcess(const Process& process) {
        auto sharedProc = std::make_shared<Process>(process);
        sharedProc->setState(ProcessState::WAITING);
        sharedProc->setStartTime(std::chrono::system_clock::now());
        
        {
            std::lock_guard<std::mutex> lock(processesMutex);
            processes.push_back(sharedProc);
        }
        
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            ready_queue.push(sharedProc);
            queueCV.notify_one();
        }
    }

    void scheduleProcesses() {
        // This method can be used for additional scheduling logic if needed
        // The main scheduling is handled by the worker threads
    }

    void startScheduler(int num_cpu) {
        if (SchedulerRunning) return;
        
        numCores = num_cpu;
        SchedulerRunning = true;
        
        // Create worker threads for each CPU core
        for (int i = 0; i < num_cpu; ++i) {
            workerThreads.emplace_back(&Scheduler::SchedulerLoop, this, i);
        }
    }

    void stopScheduler() {
        if (!SchedulerRunning) return;
        
        SchedulerRunning = false;
        queueCV.notify_all();
        
        // Wait for all worker threads to finish
        for (auto& thread : workerThreads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        
        workerThreads.clear();
    }

    bool isSchedulerRunning() const {
        return SchedulerRunning;
    }

    // Statistics methods
    int getRunningProcessCount() const {
        std::lock_guard<std::mutex> lock(processesMutex);
        return runningProcesses.size();
    }

    int getWaitingProcessCount() const {
        std::lock_guard<std::mutex> lock(queueMutex);
        return ready_queue.size();
    }

    int getCompletedProcessCount() const {
        std::lock_guard<std::mutex> lock(processesMutex);
        return completedProcesses.size();
    }

    int getTotalProcessCount() const {
        std::lock_guard<std::mutex> lock(processesMutex);
        return processes.size();
    }

    std::vector<std::shared_ptr<Process>> getAllProcesses() const {
        std::lock_guard<std::mutex> lock(processesMutex);
        return processes;
    }
};