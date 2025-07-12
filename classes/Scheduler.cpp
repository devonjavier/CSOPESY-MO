#include "process.cpp"
#include <queue>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <windows.h>

class Scheduler { 

    private:
    std::vector<Process> processes;
    std::queue<Process> ready_queue; 
    std::vector<Process> runningProcesses;
    std::vector<Process> completedProcesses;
    std::vector<std::thread> workerThreads;
    std::atomic<bool> schedulerRunning{false};
    std::atomic<bool> generatingProcesses{false};            //i think we only need one flag right??
    std::mutex queueMutex;
    std::condition_variable queueCV;
    std::string SchedulerType;
    int quantumCycles; 
    uint16_t programcounter = 0;


    public:
    Scheduler(const std::string& Scheduler, int quantum) 
        : SchedulerType(Scheduler), quantumCycles(quantum) {}


    void checkIfComplete() {
        std::lock_guard<std::mutex> lock(queueMutex);
        if (!generatingProcesses && ready_queue.empty() && runningThreadsDone()) {
            schedulerRunning = false;
            queueCV.notify_all();
            std::cout << "All processes completed. Scheduler is shutting down.\n";
        }
    }
    
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

    // //version 1
    // void schedulerAlgo(int coreId) {
    //     while (true) {
    //         Process current;
    //         {
    //             //dequeue
    //             std::unique_lock<std::mutex> lock(queueMutex);

    //             //wake on new work or shutdown
    //             queueCV.wait(lock, [this] {
    //                 return !ready_queue.empty() || !SchedulerRunning;
    //             });

    //             //no more work AND we shutting down --> exit
    //             if (!SchedulerRunning && ready_queue.empty()) break;

    //             //bobong fake awake or still no work? --> loop again
    //             if (ready_queue.empty()) continue;

    //             current = ready_queue.front();
    //             ready_queue.pop();
    //             // current.setCurrentCoreId(coreId); 
    //             runningProcesses.push_back(current);
    //         }

    //         current.setCurrentCoreId(coreId);
    //         current.setState(ProcessState::RUNNING);

    //         cout << "Running process ID: " << current.getPid() 
    //              << " on core ID: " << current.getCurrentCoreId() << "\n";
    //         //run the process instructions
    //         // current.runInstructions();

    //         //set state of process to FINISHED
    //         current.setState(ProcessState::FINISHED);
    //         cout << "Process ID: " << current.getPid() 
    //              << " has finished running on core ID: " << current.getCurrentCoreId() << "\n";

    //         {
    //             std::lock_guard<std::mutex> lock(queueMutex);

    //             //remove from running, push to completed
    //             auto it = std::find_if(runningProcesses.begin(), runningProcesses.end(),
    //                                 [&](const Process& p) { return p.getPid() == current.getPid(); });
    //             if (it != runningProcesses.end()) {
    //                 runningProcesses.erase(it);
    //             }
    //             completedProcesses.push_back(current);
    //         }
    //         checkIfComplete();
    //     }
    //     std::cout << "Worker thread for core " << coreId << " exiting.\n";
    // }
    //version 2
    void schedulerAlgo(int coreId) {
        while (generatingProcesses.load() || !ready_queue.empty()) {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCV.wait(lock, [this]{
            return !ready_queue.empty()
                || !generatingProcesses.load();
            });

            if (ready_queue.empty() && !generatingProcesses.load())
            break;

            // dequeue next process
            Process current = ready_queue.front();
            ready_queue.pop();
            lock.unlock();

            // ─── RUN YOUR ROUND-ROBIN TIME SLICE ───
            current.setState(ProcessState::RUNNING);
            unsigned slice = std::min<unsigned>(
                current.getRemainingBurst(),
                quantumCycles);
            // e.g. execute slice instructions (or cycles)
            for (unsigned i = 0; i < slice; ++i) {
            current.runInstructions();
            }
            current.setRemainingBurst(
            current.getRemainingBurst() - slice
            );

            if (current.getRemainingBurst() > 0) {
            // not done, re-enqueue
            current.setState(ProcessState::WAITING);
            std::lock_guard<std::mutex> reqlock(queueMutex);
            ready_queue.push(current);
            queueCV.notify_one();
            } else {
            // finished
            current.setState(ProcessState::FINISHED);
            std::lock_guard<std::mutex> complock(queueMutex);
            completedProcesses.push_back(current);
            }
        }
        std::cout << "Worker thread for core "
                    << coreId << " exiting.\n";
    }


    // void Scheduler::startScheduler(int num_cpu) {
    void startScheduler(int num_cpu) {
        generatingProcesses = true;

        std::cout << endl;
        std::cout << "Starting Scheduler with " << num_cpu << " cores.\n";
        std::cout << "Scheduler Type: " << SchedulerType << "\n";
        std::cout << "Quantum Cycles: " << quantumCycles << "\n";
        std::cout << "Scheduler Running: " << (schedulerRunning ? "Yes" : "No") << "\n";
        std::cout << "Generating Processes: " << (generatingProcesses ? "Yes" : "No") << "\n\n\n";

        //version 1
        // // SchedulerRunning = true;
        // GeneratingProcesses = true;
        // while (SchedulerRunning) {
        //     //TODO: what coreID does the workerThread put the scheduler algorithm in????
        //     //-andrei
        //     //for now imma put it as the index of for loop
        //     std::cout << "to fix: startScheduler(), fix what coreID to put scheduler algorithm intto" << std::endl;
        //     workerThreads.emplace_back(&Scheduler::schedulerAlgo, this); //i variable here needs to be changed
        // }

        //version 2
        // SchedulerRunning.store(true);

        // for (int coreId = 0; coreId < num_cpu; ++coreId) {
        //     workerThreads.emplace_back([this, coreId]() {
        //         [this, coreId]() { this->schedulerAlgo(coreId); }
        //         std::cout << "Worker thread for core " << coreId << " started.\n";

        //         // // bind this thread to logical coreId
        //         // DWORD_PTR mask = (1ULL << coreId);
        //         // SetThreadAffinityMask(GetCurrentThread(), mask);

        //         // // now enter your scheduling loop
        //         // this->schedulerAlgo();
        //     });
        // }

        //version 3
        schedulerRunning = true;
        generatingProcesses = true;
        
        for (int coreId = 0; coreId < num_cpu; ++coreId) {
            //directly bind to member function
            // workerThreads.emplace_back(&Scheduler::schedulerAlgo, this, coreId);

            workerThreads.emplace_back([this,coreId](){
                std::cout << "Worker thread for core " << coreId << " started.\n";
                this->schedulerAlgo(coreId);
            });
        }
    }

    void stopGenerating() {
        generatingProcesses = false;
        queueCV.notify_all();   // wake up any workers waiting
    }


    void stopScheduler() {
        // GeneratingProcesses = false;
        // std::cout << "Scheduler stop signal received. Waiting for all processes to finish...\n";

        // {
        //     std::lock_guard<std::mutex> lock(queueMutex);
        //     SchedulerRunning.store(false);
        // }
        // queueCV.notify_all();

        // for (auto &t : workerThreads)
        //     if (t.joinable())
        //         t.join();
        // workerThreads.clear();

        schedulerRunning = false;
        generatingProcesses = false;
        queueCV.notify_all();
        for (auto &t : workerThreads)
            if (t.joinable()) t.join();
        workerThreads.clear();


    }

    void finalizeScheduler() {
        schedulerRunning = false;
            for (auto& t : workerThreads) {
                if (t.joinable())
                    t.join();
            }
            std::cout << "Scheduler fully shut down. All processes completed.\n";
    }


    bool isSchedulerRunning() const {
        return schedulerRunning;
    }

    bool isGeneratingProcesses() const {
        return generatingProcesses;
    }

    bool runningThreadsDone() {
        std::lock_guard<std::mutex> lock(queueMutex);
        return std::all_of(runningProcesses.begin(), runningProcesses.end(), [](const Process& p) {
            return p.getState() == ProcessState::FINISHED;
        });
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