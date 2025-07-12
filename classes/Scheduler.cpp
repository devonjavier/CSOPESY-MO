#include "process.cpp"
#include <queue>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <algorithm>

class Scheduler { 

    private:
    std::vector<Process> processes;
    std::queue<Process> ready_queue; 
    std::vector<Process> runningProcesses;
    std::vector<Process> completedProcesses;
    std::vector<std::thread> workerThreads;
    std::atomic<bool> SchedulerRunning{false};
    std::atomic<bool> GeneratingProcesses{false};
    std::mutex queueMutex;
    std::condition_variable queueCV;
    std::string SchedulerType; 
    int quantumCycles; 

    std::atomic<bool>       SchedulerRunning;
    std::mutex              queueMutex;
    std::condition_variable queueCV;
    std::queue<Process>     ready_queue;
    std::vector<Process>    runningProcesses;
    std::vector<Process>    completedProcesses;
    std::vector<std::thread> workerThreads;


    public:
    Scheduler(const std::string& Scheduler, int quantum) 
        : SchedulerType(Scheduler), quantumCycles(quantum) {}

    void addProcess(const Process& process) {
        std::lock_guard<std::mutex> lock(queueMutex);
        processes.push_back(process);
    }

    void queueProcesses() {
        std::lock_guard<std::mutex> lock(queueMutex);
        
        for (auto& process : processes) {
            if (process.getState() == ProcessState::IDLE) {
                process.setState(ProcessState::WAITING);
                ready_queue.push(process);
                queueCV.notify_one(); 
            }
        }
    }

    void schedulerAlgo() { //int coreId) {
        while (true) {
            Process current;

            {
                std::unique_lock<std::mutex> lock(queueMutex);
                queueCV.wait(lock, [this] {
                    return !ready_queue.empty() || !SchedulerRunning;
                });

                //no more work *and* we’re shutting down --> exit
                if (!SchedulerRunning && ready_queue.empty()) break;

                //spurious wake or still no work? loop again
                if (ready_queue.empty()) continue;

                current = ready_queue.front();
                ready_queue.pop();

                current.setState(ProcessState::RUNNING);
                // current.setCurrentCoreId(coreId); // ✅ Set here

                runningProcesses.push_back(current);
            }

            // current.runInstructions();
            current.setState(ProcessState::FINISHED);

            {
                std::lock_guard<std::mutex> lock(queueMutex);

                auto it = std::find_if(runningProcesses.begin(), runningProcesses.end(),
                                    [&](const Process& p) { return p.getPid() == current.getPid(); });
                if (it != runningProcesses.end()) {
                    runningProcesses.erase(it);
                }

                completedProcesses.push_back(current);
            }

            checkIfComplete();
        }

        // std::cout << "Worker thread for core " << coreId << " exiting.\n";
    }

    void startScheduler(int num_cpu) {
        // // SchedulerRunning = true;
        // GeneratingProcesses = true;
        // while (SchedulerRunning) {
        //     //TODO: what coreID does the workerThread put the scheduler algorithm in????
        //     //-andrei
        //     //for now imma put it as the index of for loop
        //     std::cout << "to fix: startScheduler(), fix what coreID to put scheduler algorithm intto" << std::endl;
        //     workerThreads.emplace_back(&Scheduler::schedulerAlgo, this); //i variable here needs to be changed
        // }
        SchedulerRunning.store(true);

        for (int coreId = 0; coreId < num_cpu; ++coreId) {
            workerThreads.emplace_back([this, coreId]() {
                // bind this thread to logical coreId
                DWORD_PTR mask = (1ULL << coreId);
                SetThreadAffinityMask(GetCurrentThread(), mask);

                // now enter your scheduling loop
                this->schedulerAlgo();
            });
        }

    }

    void stopScheduler() {
        // GeneratingProcesses = false;
        // std::cout << "Scheduler stop signal received. Waiting for all processes to finish...\n";
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            SchedulerRunning.store(false);
        }
        queueCV.notify_all();

        for (auto &t : workerThreads)
            if (t.joinable())
                t.join();
        workerThreads.clear();
    }

    void finalizeScheduler() {
    SchedulerRunning = false;
        for (auto& t : workerThreads) {
            if (t.joinable())
                t.join();
        }
        std::cout << "Scheduler fully shut down. All processes completed.\n";
    }


    bool isSchedulerRunning() const {
        return SchedulerRunning;
    }

    bool isGeneratingProcesses() const {
        return GeneratingProcesses;
    }

    bool runningThreadsDone() {
        std::lock_guard<std::mutex> lock(queueMutex);
        return std::all_of(runningProcesses.begin(), runningProcesses.end(), [](const Process& p) {
            return p.getState() == ProcessState::FINISHED;
        });
    }


    void checkIfComplete() {
        std::lock_guard<std::mutex> lock(queueMutex);
        if (!GeneratingProcesses && ready_queue.empty() && runningThreadsDone()) {
            SchedulerRunning = false;
            queueCV.notify_all();
            std::cout << "All processes completed. Scheduler is shutting down.\n";
        }
    }

    void displayProcessList() {
        for (Process proc : processes) {
            std::cout << "Process ID: " << proc.getPid() << "\n"
                << "Process Name: " << proc.getProcessName() << "\n"
                << "Current Core ID: " << proc.getCurrentCoreId() << "\n"
                << "Arrival Time: " << proc.getArrivalTime() << "\n"
                << "Burst Time: " << proc.getBurstTime() << "\n"
                << "Remaining Burst: " << proc.getRemainingBurst() << "\n"
                << "Waiting Time: " << proc.getWaitingTime() << "\n"
                << "Run Count: " << proc.getRunCount() << "\n"
                << "State: " << processStateToString(proc.getState()) << "\n"
                << "\n"
                << std::endl;
            
            proc.displayInstructionList();
        }
    }

    // Runs FCFS on the given list of processes.
    //
    // For each process, this will:
    // 1) record its start/end times via appendStartTime/appendEndTime
    // 2) set remaining_burst to 0
    // 3) set waiting_time
    // and finally return the overall average waiting time.
    struct CompareArrival {
        bool operator()(Process* a, Process* b) const {
            auto at_a = a->getArrivalTime();
            auto at_b = b->getArrivalTime();
            if (at_a == at_b)
                return a->getPid() > b->getPid();   // later PID => lower priority
            return at_a > at_b;                     // larger arrival_time => lower priority
        }
    };

    double FCFS() {
        if (processes.empty()) 
            return 0.0;

        // Build a min‐heap of Process* by arrival time
        std::priority_queue<
            Process*,
            std::vector<Process*>,
            CompareArrival
        > arrivalQ;

        for (auto& proc : processes) {
            arrivalQ.push(&proc);
        }

        // Pop the very first process
        Process* prev = arrivalQ.top();
        arrivalQ.pop();

        // First process: starts at its arrival, no waiting
        prev->setStartTime(prev->getArrivalTime());
        prev->setEndTime(prev->getStartTime(0) + prev->getBurstTime());
        prev->setRemainingBurst(0);
        prev->setWaitingTime    (0);

        double aveWait = 0.0;
        size_t count  = 1;  // we’ll use this as the “i” in computeStreamAve

        // Now handle the rest in arrival order
        while (!arrivalQ.empty()) {
            Process* curr = arrivalQ.top();
            arrivalQ.pop();

            uint64_t start = prev->getEndTime(0);
            curr->setStartTime(start);

            uint64_t finish = start + curr->getBurstTime();
            curr->setEndTime(finish);

            curr->setRemainingBurst(0);

            uint64_t wait = finish
                        - curr->getArrivalTime()
                        - curr->getBurstTime();
            curr->setWaitingTime(wait);

            // update running average
            aveWait = updateRunningAverage(aveWait, wait, count);
            ++count;

            prev = curr;
        }

        return aveWait;
    }
};