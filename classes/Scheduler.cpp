#include "Scheduler.h"
#include <iostream>      // for std::cout, etc.

Scheduler::Scheduler(const std::string& schedulerType, int quantum)
: SchedulerType(schedulerType), quantumCycles(quantum) {}

void Scheduler::addProcess(const Process& p) {
    std::lock_guard<std::mutex> lock(queueMutex);
    processes.push_back(p);
}


void Scheduler::checkIfComplete() {
    std::lock_guard<std::mutex> lock(queueMutex);
    if (!generatingProcesses && ready_queue.empty() && runningThreadsDone()) {
        schedulerRunning = false;
        queueCV.notify_all();
        std::cout << "All processes completed. Scheduler is shutting down.\n";
    }
}

void Scheduler::addProcess(const Process& process) {
    std::lock_guard<std::mutex> lock(queueMutex);
    processes.push_back(process);
}

void Scheduler::queueProcesses() {
    std::lock_guard<std::mutex> lock(queueMutex);
    
    for (auto& process : processes) {
        if (process.getState() == ProcessState::IDLE) {
            process.setState(ProcessState::WAITING);
            ready_queue.push(process);
            queueCV.notify_one(); 
        }
    }
}

void Scheduler::schedulerAlgo(int coreId) {
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
        current_mem_usage -= mem_per_proc;
        }
    }
    std::cout << "Worker thread for core "
                << coreId << " exiting.\n";
}


// void Scheduler::startScheduler(int num_cpu) {
// void startScheduler(int num_cpu) {
//     generatingProcesses = true;

//     std::cout << std::endl;
//     std::cout << "Starting Scheduler with " << num_cpu << " cores.\n";
//     std::cout << "Scheduler Type: " << SchedulerType << "\n";
//     std::cout << "Quantum Cycles: " << quantumCycles << "\n";
//     std::cout << "Scheduler Running: " << (schedulerRunning ? "Yes" : "No") << "\n";
//     std::cout << "Generating Processes: " << (generatingProcesses ? "Yes" : "No") << "\n\n\nƒ";

//     schedulerRunning = true;
//     generatingProcesses = true;
    
//     for (int coreId = 0; coreId < num_cpu; ++coreId) {
//         //directly bind to member function
//         // workerThreads.emplace_back(&Scheduler::schedulerAlgo, this, coreId);

//         workerThreads.emplace_back([this,coreId](){
//             std::cout << "Worker thread for core " << coreId << " started.\n";
//             this->schedulerAlgo(coreId);
//         });
//     }
// }

void Scheduler::startScheduler(int num_cpu) {
    generatingProcesses = true;

    if (SchedulerType == "fcfs") {
        double avgWait = FCFS();
        std::cout << "FCFS average waiting time: " << avgWait << "\n";
        finalizeScheduler();
        return;
    }

    // else RR:
    schedulerRunning = true;
    for (int coreId = 0; coreId < num_cpu; ++coreId) {
        workerThreads.emplace_back(
        [this,coreId](){ this->schedulerAlgo(coreId); });
    }
}


void Scheduler::stopGenerating() {
    generatingProcesses = false;
    queueCV.notify_all();   // wake up any workers waiting
}


void Scheduler::stopScheduler() {
    schedulerRunning = false;
    generatingProcesses = false;
    queueCV.notify_all();
    for (auto &t : workerThreads)
        if (t.joinable()) t.join();
    workerThreads.clear();
}

void Scheduler::finalizeScheduler() {
    schedulerRunning = false;
    for (auto& t : workerThreads) {
        if (t.joinable())
            t.join();
    }
    std::cout << "Scheduler fully shut down. All processes completed.\n";
}


bool Scheduler::isSchedulerRunning() const {
    return schedulerRunning;
}

bool Scheduler::isGeneratingProcesses() const {
    return generatingProcesses;
}

bool Scheduler::runningThreadsDone() {
    std::lock_guard<std::mutex> lock(queueMutex);
    return std::all_of(runningProcesses.begin(), runningProcesses.end(), [](const Process& p) {
        return p.getState() == ProcessState::FINISHED;
    });
}

void Scheduler::displayProcessList() {
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

struct CompareArrival {
    bool operator()(Process* a, Process* b) const {
        auto at_a = a->getArrivalTime();
        auto at_b = b->getArrivalTime();
        if (at_a == at_b)
            return a->getPid() > b->getPid();   // later PID => lower priority
        return at_a > at_b;                     // larger arrival_time => lower priority
    }
};

double Scheduler::FCFS() {
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

Process* Scheduler::findProcessByName(const std::string& name) {
    for (auto& p : processes) {
        if (p.getProcessName() == name) return &p;
    }
    return nullptr;
}

Scheduler::Status Scheduler::getStatus() const {
    Status s;
    s.totalCores = workerThreads.size();
    s.busyCores = // count threads currently executing 
    s.freeCores = s.totalCores - s.busyCores;
    s.cpuUtil   = 100.0 * s.busyCores / s.totalCores;

    // gather lists
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        for (auto& p : ready_queue)    s.readyList.push_back(p.getProcessName());
        for (auto& p : runningProcesses) s.runningList.push_back(p.getProcessName());
        for (auto& p : completedProcesses) s.finishedList.push_back(p.getProcessName());
    }
    return s;
}