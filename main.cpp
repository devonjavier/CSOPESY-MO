#include "header_files/header.h"

std::queue<Cores*> cores; 
std::queue<Process*> processes;

std::vector<ProcessLogger> logger;

config week6Configurations("src/config.json");
int num_cores = week6Configurations.getCores();
int num_processes = week6Configurations.getProcesses();

void make_processes() {
    for (int i = 0; i < num_processes; ++i) {
        int pid = i + 1;
        std::string name = "Process_" + std::to_string(pid);
        int priority = 1;       //arbitrary priority for fcfs implementation
        int burst = 100;        //arbitrary burst time

        Process* newProcess = new Process(pid, name, priority, burst);
        processes.push(newProcess);
    }

    std::cout << "\nCreated Processes:\n";
    std::queue<Process*> processesCopy = processes; //copy to avoid modifying original queue
    while (!processesCopy.empty()) {
        Process* p = processesCopy.front();
        p->displayProcess();
        processesCopy.pop();
        // delete p; //free memory after displaying
    }
}

void make_cores() {
    for (int i = 0; i < num_cores; ++i) {
        Cores* newCore = new Cores(i);
        cores.push(newCore);
    }

    std::cout << "\nCreated Cores:\n";
    std::queue<Cores*> coresCopy = cores; //copy to avoid modifying original queue 
    while (!cores.empty()) {
        std::cout << coresCopy.front() << std::endl;
        coresCopy.pop();
    }
    
}

void write_to_file() {
    // Option 1: Open and write, then close explicitly
    std::ofstream myFile;
    myFile.open("example1.txt"); // Creates or overwrites example1.txt

    if (myFile.is_open()) {
        myFile << "This is the second line." << std::endl; // std::endl also flushes the buffer and adds a newline
        myFile << "A number: " << 42 << std::endl;
        myFile.close();
        std::cout << "Data written to example1.txt" << std::endl;
    } else {
        std::cerr << "Unable to open example1.txt" << std::endl;
    }
}

void print_process_log() {
    std::cout << "-----------------------------\n" << std::endl;
    std::cout << "\n--- Running Processes: ---" << std::endl;
    std::vector<ProcessLogEntry> runningLogs = logger.getLogsByState(ProcessState::RUNNING);
    if (runningLogs.empty()) {
        std::cout << "No finished processes logged." << std::endl;
    } else {
        for (const auto& entry : runningLogs) {
            entry.print();
        }
    }
    std::cout << "\n\n--- Finished Processes: ---" << std::endl;
    std::vector<ProcessLogEntry> finishedLogs = logger.getLogsByState(ProcessState::FINISHED);
    if (finishedLogs.empty()) {
        std::cout << "No finished processes logged." << std::endl;
    } else {
        for (const auto& entry : finishedLogs) {
            entry.print();
        }
    }
    std::cout << "------------------------------\n" << std::endl;
}

void first_come_first_served_scheduling_algorithm(std::queue<Process*> currentProcessQueue, std::queue<Cores*> coresReadyQueue) {
    // while (!currentProcessQueue.empty()) {
    //     Process* currentProcess = processes.front();
    //     currentProcess->displayProcess();
    //     processes.pop();

    //     if (coresReadyQueue.empty()) {
    //         std::cerr << "No cores available for scheduling." << std::endl;
    //         return;
    //     }
    //     ProcessLog log(currentProcess->getPid(), 0, currentProcess->getProcessName());
    //     log.displayLog();
    // }
}

int main() {
    // make_processes();
    make_cores();

    std::queue<Process*> currentProcessQueue = processes;
    std::queue<Cores*> coresReadyQueue = cores;

    


    // write_to_file();

    // ProcessLog log1(1234, 0, "SystemService");
    // log1.displayLog();

    return 0;
}