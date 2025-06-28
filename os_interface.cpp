#include "classes/Screen.cpp"
#include "classes/Scheduler.cpp"
#include "header.h"

#include <iostream>
#include <string>
#include <cstdlib>
#include <thread>
#include <ctime>
#include <vector>
#include <mutex>
#include <atomic>
#include <fstream>
#include <sstream>
#include <random>
#include <chrono>
#include <cmath>

// Initialization of variables
int num_cpu = 0;
std::string scheduler_type = "";
int quantumcycles = 0;
int batchprocess_freq = 0;
int min_ins = 0;
int max_ins = 0;
int delays_perexec = 0;

// Global process management
std::vector<Process> processes;
std::mutex process_mutex;
std::atomic<bool> scheduler_running{false};

// Initialization of Screens and Processes Lists
ScreenSession *head = nullptr;
Scheduler* os_scheduler = nullptr;

std::string formatTime(const std::chrono::time_point<std::chrono::system_clock>& tp) {
    if (tp.time_since_epoch().count() == 0) return "N/A";
    std::time_t tt = std::chrono::system_clock::to_time_t(tp);
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%d/%m/%Y %I:%M:%S%p", std::localtime(&tt));
    return std::string(buffer);
}

void generate_random_processes() {
    static int next_id = 1;

    for (int i = 0; i < batchprocess_freq; ++i) {
        std::default_random_engine generator(
            std::chrono::system_clock::now().time_since_epoch().count() + i
        );
        std::uniform_int_distribution<int> distribution(min_ins, max_ins);

        Process proc = Process(next_id, "process" + std::to_string(next_id), distribution(generator));
        next_id++;

        os_scheduler->addProcess(proc);
    }
}

void initialize() {
    std::ifstream config("config.txt");
    if (!config.is_open()) {
        std::cerr << "Error: Could not open config.txt" << std::endl;
        return;
    }

    std::string line;
    while (std::getline(config, line)) {
        std::istringstream iss(line);
        std::string key;
        if (!(iss >> key)) continue;

        if (key == "num-cpu") {
            iss >> num_cpu;
            if (num_cpu < 1 || num_cpu > 128) {
                std::cerr << "Invalid num-cpu value. Must be in [1,128]." << std::endl;
            }
        } else if (key == "scheduler") {
            std::string rest;
            std::getline(iss, rest);
            std::istringstream rest_iss(rest);
            rest_iss >> scheduler_type;

            if (scheduler_type != "fcfs" && scheduler_type != "rr") {
                std::cerr << "Invalid scheduler value. Must be 'fcfs' or 'rr'." << std::endl;
            }
        } else if (key == "quantum-cycles") {
            iss >> quantumcycles;
            if (quantumcycles < 1) {
                std::cerr << "Invalid quantum-cycles value. Must be >=1." << std::endl;
            }
        } else if (key == "batch-process-freq") {
            iss >> batchprocess_freq;
            if (batchprocess_freq < 1) {
                std::cerr << "Invalid batch-process-freq value. Must be >=1." << std::endl;
            }
        } else if (key == "min-ins") {
            iss >> min_ins;
            if (min_ins < 1) {
                std::cerr << "Invalid min-ins value. Must be >=1." << std::endl;
            }
        } else if (key == "max-ins") {
            iss >> max_ins;
            if (max_ins < 1) {
                std::cerr << "Invalid max-ins value. Must be >=1." << std::endl;
            }
        } else if (key == "delay-per-exec") {
            iss >> delays_perexec;
        }
    }

    os_scheduler = new Scheduler(scheduler_type, quantumcycles, num_cpu, delays_perexec);
    config.close();
}

void screen_init() {
    std::cout << "________/\\\\\\\\\\\\\\\\\\_____/\\\\\\\\\\\\\\\\\\\\\\_________/\\\\\\\\\\_______/\\\\\\\\\\\\\\\\\\\\\\\\\\____/\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\_____/\\\\\\\\\\\\\\\\\\\\\\____/\\\\\\________/\\\\\\_\n";
    std::cout << " _____/\\\\\\////////____/\\\\\\/////////\\\\\\_____/\\\\\\///\\\\\\____\\/\\\\\\/////////\\\\\\_\\/\\\\\\///////////____/\\\\\\/////////\\\\\\_\\///\\\\\\____/\\\\\\/__\n";
    std::cout << "  ___/\\\\\\/____________\\//\\\\\\______\\///____/\\\\\\/__\\///\\\\\\__\\/\\\\\\_______\\/\\\\\\_\\/\\\\\\______________\\//\\\\\\______\\///____\\///\\\\\\/\\\\\\/____\n";
    std::cout << "   __/\\\\\\_______________\\////\\\\\\__________/\\\\\\______\\//\\\\\\_\\/\\\\\\\\\\\\\\\\\\\\\\\\\\/__\\/\\\\\\\\\\\\\\\\\\\\\\_______\\////\\\\\\_____________\\///\\\\\\/______\n";
    std::cout << "    _\\/\\\\\\__________________\\////\\\\\\______\\/\\\\\\_______\\/\\\\\\_\\/\\\\\\/////////____\\/\\\\\\///////___________\\////\\\\\\____________\\/\\\\\\_______\n";
    std::cout << "     _\\//\\\\\\____________________\\////\\\\\\___\\//\\\\\\______/\\\\\\__\\/\\\\\\_____________\\/\\\\\\_____________________\\////\\\\\\_________\\/\\\\\\_______\n";
    std::cout << "      __\\///\\\\\\___________/\\\\\\______\\//\\\\\\___\\///\\\\\\__/\\\\\\____\\/\\\\\\_____________\\/\\\\\\______________/\\\\\\______\\//\\\\\\________\\/\\\\\\_______\n";
    std::cout << "       ____\\////\\\\\\\\\\\\\\\\\\_\\///\\\\\\\\\\\\\\\\\\\\\\/______\\///\\\\\\\\\\/_____\\/\\\\\\_____________\\/\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\_\\///\\\\\\\\\\\\\\\\\\\\\\/_________\\/\\\\\\_______\n";
    std::cout << "        _______\\/////////____\\///////////__________\\/////_______\\///______________\\///////////////____\\///////////___________\\///________\n";
    std::cout << "\n\nHello! Welcome to the CSOPESY destroyers' command-line operating system!\n";
    std::cout << "Type 'exit' to quit, 'clear' to clear the screen, or 'help' for a list of commands.\n";
    std::cout << "Please enter a command:\n";
}

void scheduler_start() {
    std::cout << "Starting scheduler...\n";
    scheduler_running = true;
    
    // Start the scheduler with the specified number of CPU cores
    os_scheduler->startScheduler(num_cpu);
    
    // Background thread for continuous process generation
    std::thread process_generator([]() {
        while (scheduler_running) {
            generate_random_processes();
            std::this_thread::sleep_for(std::chrono::seconds(1)); // Generate every second
        }
    });
    
    // Background thread for scheduler execution
    std::thread scheduler_thread([]() {
        while (scheduler_running) {
            os_scheduler->scheduleProcesses();
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Check frequently
        }
    });
    
    process_generator.detach();
    scheduler_thread.detach();
    
    std::cout << "Scheduler started with " << num_cpu << " CPU cores.\n";
    std::cout << "Process generation frequency: " << batchprocess_freq << " processes per batch\n";
    std::cout << "Scheduler type: " << scheduler_type << "\n";
    if (scheduler_type == "rr") {
        std::cout << "Quantum cycles: " << quantumcycles << "\n";
    }
}

void scheduler_stop() {
    scheduler_running = false;
    if (os_scheduler) {
        os_scheduler->stopScheduler();
    }
    std::cout << "Scheduler stopped.\n";
}

void report_util() {
    std::ofstream log("csopesy-log.txt", std::ios::app);
    // log << "===== Report (" << get_timestamp() << ") =====\n";
    
    // Get current statistics from scheduler
    if (os_scheduler) {
        int running_processes = 0;
        int waiting_processes = 0;
        int finished_processes = 0;
        
        {
            std::lock_guard<std::mutex> lock(process_mutex);
            for (const auto& proc : processes) {
                switch (proc.getState()) {
                    case ProcessState::RUNNING:
                        running_processes++;
                        break;
                    case ProcessState::WAITING:
                        waiting_processes++;
                        break;
                    case ProcessState::FINISHED:
                        finished_processes++;
                        break;
                }
            }
        }
        
        std::cout << "CPU Cores: " << num_cpu << "\n";
        std::cout << "Running Processes: " << running_processes << "\n";
        std::cout << "Waiting Processes: " << waiting_processes << "\n";
        std::cout << "Finished Processes: " << finished_processes << "\n";
        std::cout << "Total Processes: " << processes.size() << "\n";
        
        log << "CPU Cores: " << num_cpu << "\n";
        log << "Running Processes: " << running_processes << "\n";
        log << "Waiting Processes: " << waiting_processes << "\n";
        log << "Finished Processes: " << finished_processes << "\n";
        log << "Total Processes: " << processes.size() << "\n";
    }
    
    log.close();
    std::cout << "System report saved to csopesy-log.txt\n";
}

void clear_screen() {
    #ifdef _WIN32
        std::system("cls");
    #else
        std::system("clear");
    #endif
}

void exit_os(int status) {
    scheduler_stop();
    if (os_scheduler) {
        delete os_scheduler;
    }
    std::exit(status);
}

std::string get_timestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%m/%d/%Y, %I:%M:%S %p", std::localtime(&now_time));
    return std::string(buffer);
}

void screen_session(ScreenSession& session) {
    std::string command;
    while (true) {
        #ifdef _WIN32
            std::system("cls");
        #else
            std::system("clear");
        #endif
        std::cout << "--- Process: " << session.name << " ---\n";
        std::cout << "Instruction: " << session.current_line << " / " << session.total_lines << "\n";
        std::cout << "Created: " << session.timestamp << "\n";
        std::cout << "\nType 'exit' to return to main menu\n> ";
        std::getline(std::cin, command);
        bool exit = accept_input(command, &session);

        if(exit == true){
            break;
        }
    }
}

void new_screen(std::string name) {
    if (head == nullptr) {
        head = new ScreenSession(name, 1, 50, get_timestamp());
        screen_session(*head);
        return;
    }

    ScreenSession *current_screen = head;
            
    while(current_screen != nullptr){
        if(current_screen->name == name){
            std::cout << "Screen session with name '" << name << "' already exists.\n";
            system("pause");
            return;
        } else if (current_screen->next == nullptr) {
            ScreenSession *new_screen = new ScreenSession(name, 1, 50, get_timestamp());
            current_screen->next = new_screen;
            screen_session(*new_screen);
            return;
        } else {
            current_screen = current_screen->next;
        }
    }
}

void find_screen(std::string name) {
    ScreenSession *current_screen = head;

    while(current_screen != nullptr && current_screen->name != name){
        current_screen = current_screen->next;
    }
    
    if(current_screen == nullptr){
        std::cout << "Screen session with name '" << name << "' not found.\n";
        system("pause");
        return;
    }

    screen_session(*current_screen);
}

bool accept_input(std::string choice, ScreenSession *current_screen){
    bool exit = false;
    if (choice == "initialize") {
        initialize();
        std::cout << "\nInitialized configuration:\n";
        std::cout << "CPU Cores: " << num_cpu << "\n";
        std::cout << "Scheduler: " << scheduler_type << "\n";
        std::cout << "Quantum Cycles: " << quantumcycles << "\n";
        std::cout << "Batch Process Frequency: " << batchprocess_freq << "\n";
        std::cout << "Min Instructions: " << min_ins << "\n";
        std::cout << "Max Instructions: " << max_ins << "\n";
        std::cout << "Delays per Execution: " << delays_perexec << "\n";
        
        if (current_screen) current_screen->current_line++;
        system("pause");
    } else if (choice == "scheduler-start") {
        std::cout << "Scheduler-start command recognized.\n";
        scheduler_start();
        if (current_screen) current_screen->current_line++;
        system("pause");
    } else if (choice == "scheduler-stop") {
        std::cout << "Scheduler-stop command recognized.\n";
        scheduler_stop();
        if (current_screen) current_screen->current_line++;
        system("pause");
    } else if (choice == "report-util") {
        std::cout << "Report-util command recognized.\n";
        report_util();
        if (current_screen) current_screen->current_line++;
        system("pause");
    } else if (choice == "clear") {
        clear_screen();
        if (current_screen) current_screen->current_line++;
    } else if (choice == "exit") {
        std::cout << "Exit command recognized. Exiting...\n";
        exit = true;
    } else if (choice == "help") {
        std::cout << "Available commands:\n";
        std::cout << "1. initialize - Initialize the OS environment.\n";
        std::cout << "2. scheduler-start - Start the scheduler.\n";
        std::cout << "3. scheduler-stop - Stop the scheduler.\n";
        std::cout << "4. report-util - Report system information and statistics.\n";
        std::cout << "5. screen -s <name> - Create new screen session.\n";
        std::cout << "6. screen -r <name> - Resume screen session.\n";
        std::cout << "7. screen -ls - List all processes.\n";
        std::cout << "8. clear - Clear the screen.\n";
        std::cout << "9. exit - Exit the OS.\n";
        std::cout << "10. help - Show this help message.\n";
        if (current_screen) current_screen->current_line++;
        system("pause");
    } else if (choice.rfind("screen -s ", 0) == 0) {
        std::string name = choice.substr(10);
        new_screen(name);
        if (current_screen) current_screen->current_line++;
    } else if (choice.rfind("screen -r ", 0) == 0) {
        std::string name = choice.substr(10);
        find_screen(name);
        if (current_screen) current_screen->current_line++;
    } else if(choice.rfind("screen -ls", 0) == 0) {
        std::cout << "\nProcesses:\n";
        std::lock_guard<std::mutex> lock(process_mutex);

        if (processes.empty()) {
            std::cout << "No processes found.\n";
        } else {
            for (const auto& proc : processes) {
                std::cout << "ID: " << proc.getPid() << "\n"
                        << "Name: " << proc.getProcessName() << "\n"
                        << "Status: " << processStateToString(proc.getState()) << "\n"
                        << "Core ID: " << proc.getCurrentCoreId() << "\n"
                        << "Started: " << formatTime(proc.getStartTime()) << "\n"
                        << "Ended: " << (proc.getState() == ProcessState::FINISHED ? formatTime(proc.getEndTime()) : "N/A") << "\n\n";
            }
        }

        if (current_screen) current_screen->current_line++;
        system("pause");
    } else {
        std::cout << "Unknown command: " << choice << "\n";
        system("pause");
    }
    
    if (choice != "clear") {
        clear_screen();
    }
    return exit;
}

void menu(){
    std::string choice;

    while(true){
        screen_init();
        std::getline(std::cin, choice);
        bool exit = accept_input(choice, nullptr);
        if(exit == true){
            break;
        }
    }
}