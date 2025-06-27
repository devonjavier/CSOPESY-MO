#include <iostream>
#include <string>
#include <cstdlib> // for system()
#include <thread>  // for future thread-safe tasks
#include <ctime>
#include "classes/screen.cpp"
#include "header.h"
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include "ProcessManager.cpp"
#include <fstream>
#include <sstream>


//initialization of variables
int num_cpu = 0;
std::string scheduler = "";
int quantumcycles = 0;
int batchprocess_freq = 0;
int min_ins = 0;
int max_ins = 0;
int delays_perexec = 0;

//initialization of flags
bool scheduler_running = false;

//initialization of queues
std::deque<Process> ready_queue;

//initialization of Screens and Processes Lists
ScreenSession *head = nullptr; // linked list head
ProcessManager* processManager = nullptr;

std::vector<Process> processes;
std::mutex process_mutex;


std::string formatTime(const std::chrono::time_point<std::chrono::system_clock>& tp) {
    if (tp.time_since_epoch().count() == 0) return "N/A";
    std::time_t tt = std::chrono::system_clock::to_time_t(tp);
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%d/%m/%Y %I:%M:%S%p", std::localtime(&tt));
    return std::string(buffer);
}


void run_fcfs_scheduler() {
    while (scheduler_running && !ready_queue.empty()) {
        Process proc = ready_queue.front();
        ready_queue.pop_front();

        std::thread([proc]() mutable {
            {
                std::lock_guard<std::mutex> lock(process_mutex);
                proc.setState(ProcessState::RUNNING);
                // proc.start_time = proc.getStartTime();
                // proc.thread_id = std::this_thread::get_id();
            }

            std::this_thread::sleep_for(std::chrono::seconds(2)); // simulate processing

            {
                std::lock_guard<std::mutex> lock(process_mutex);
                proc.setState(ProcessState::FINISHED);
                // proc.end_time = get_timestamp();
            }

        }).detach();
        std::this_thread::sleep_for(std::chrono::seconds(1)); // scheduler delay
    }
}


void run_rr_scheduler() {
    while (scheduler_running && !ready_queue.empty()) {
        Process proc = ready_queue.front();
        ready_queue.pop_front();

        std::thread([proc]() mutable {
            {
                std::lock_guard<std::mutex> lock(process_mutex);
                proc.setState(ProcessState::RUNNING);
                // proc.start_time = get_timestamp();
                // proc.thread_id = std::this_thread::get_id();
            }

            std::this_thread::sleep_for(std::chrono::seconds(quantumcycles));

            {
                std::lock_guard<std::mutex> lock(process_mutex);
                proc.setState(ProcessState::FINISHED);
                // proc.status = "Finished";
                // proc.end_time = get_timestamp();
            }

        }).detach();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void generate_random_processes() {
    static int next_id = 1;
    for (int i = 0; i < batchprocess_freq; ++i) {
        Process proc = Process(next_id++, "process" + std::to_string(next_id), int prio, int burst)
        // proc.id = next_id++;
        // proc.filename = "process" + std::to_string(proc.id);
        // proc.status = "Waiting";
        ready_queue.push_back(proc);
        processes.push_back(proc);
    }
}



void initialize() {
        // Gemini example:
        // Initialize GDT (or IDT) - architecture-specific!
        // gdt_init();
        // idt_init();

        // Set up a minimal stack (architecture-specific!)
        // stack_init();

        // Basic memory setup (e.g., identity mapping)
        // memory_init();

    std::ifstream config("config.txt");
    if (!config.is_open()) {
        std::cerr << "Error: Could not open config.txt" << std::endl;
        return;
    }

    std::string line;
    while (std::getline(config, line)) {
        std::istringstream iss(line);
        std::string key;
        if (!(iss >> key)) continue; // Skip empty lines

        if (key == "num-cpu") {
            iss >> num_cpu;
            if (num_cpu < 1 || num_cpu > 128) {
                std::cerr << "Invalid num-cpu value. Must be in [1,128]." << std::endl;
            }
        } else if (key == "scheduler") {
            std::string rest;
            std::getline(iss, rest);
            std::istringstream rest_iss(rest);
            rest_iss >> scheduler;

            if (scheduler != "fcfs" && scheduler != "rr") {
                std::cerr << "Invalid scheduler value. Must be 'fcfs' or 'rr'." << std::endl;
            }

            // Optional: print what's after "scheduler"
            std::string remaining_args;
            std::getline(rest_iss, remaining_args);
            if (!remaining_args.empty()) {
                std::cout << "Extra arguments after scheduler: " << remaining_args << std::endl;
            }

        } else if (key == "quantumcycles") {
            iss >> quantumcycles;
            if (quantumcycles < 1) {
                std::cerr << "Invalid quantumcycles value. Must be >=1." << std::endl;
            }
        } else if (key == "batchprocess-freq") {
            iss >> batchprocess_freq;
            if (batchprocess_freq < 1) {
                std::cerr << "Invalid batchprocess-freq value. Must be >=1." << std::endl;
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
        } else if (key == "delays-perexec") {
            iss >> delays_perexec;
        }
    }   

    processManager = new ProcessManager(scheduler, quantumcycles);

    config.close();
}

// 2. screen_init()
void screen_init() {


    // credits to https://patorjk.com/software/taag/#p=display&f=Slant%20Relief&t=CSOPESY for the unmodified ASCII art
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
    std::cout << "";
    std::cout << "Type 'exit' to quit, 'clear' to clear the screen, or 'help' for a list of commands.\n";
    std::cout << "Please enter a command:\n";
}

// 3. scheduler_start()
//    - set up a test environment for the scheduler
//    - create several test processes or threads
//    - according to the zoom kanina, each test process/thread will do some simple task
//
//    - starts the scheduler
void scheduler_start() {
    std::cout << "Starting scheduler test...\n";
    // file_count = 0;
    // std::thread background_task([](){
    //     start_file_generation();
    // });

    // background_task.detach();
    // std::cout << "File generation started in background.\n";

    scheduler_running = true;
    while (scheduler_running) {
        generate_random_processes();
        if (scheduler == "fcfs") {
            run_fcfs_scheduler();
        } else if (scheduler == "rr") {
            run_rr_scheduler();
        }
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}


// 4. scheduler_stop()
//    - stops scheduler
//        - preventing context switches
//        - disabling interrupts
//        - setting the CPU to a known state
void scheduler_stop() {
    scheduler_running = false;
    std::cout << "Scheduler stopped.\n";
}

// 5. report_util()
//    - report system information a d statistics
//        - Memory usage (total, used, free)
//        - CPU usage
//        - Running processes/threads
//        - System uptime
//        - Error logs

void report_util() {
    std::ofstream log("csopesy-log.txt", std::ios::app);
    log << "===== Report (" << get_timestamp() << ") =====\n";

    //write the same stats as screen-ls
    std::cout << "Memory Usage: __ / __ KB\n";
    std::cout << "CPU Usage: __%\n";

    log.close();
    std::cout << "System report saved to csopesy-log.txt\n";
}

// 6. clear_screen()
//    - clears the entire terminal screen
void clear_screen() {
    #ifdef _WIN32
        std::system("cls");
    #else
        std::system("clear");
    #endif
}

// 7. exit_os(int status)
//    - shut down the operating system
//        - stop scheduler
//        - unmount file systems (??idk)
//        - disable hardware devices (??idk)
//        - free memory
//        - close open files
//        - save system state
//        - log shutdown information
//        - notify other processes
//        - clean up resources
//        - stop all running processes
//        - disable interrupts
//        - set CPU to a known state
//        - return control to bootloader or firmware
//        - THEN halt CPU
//    - int status parameter to indicate exit status (egg. 0 for success, non-zero for an error).
void exit_os(int status) {
    // umount_all();     // Unmount file systems
    // disable_devices();
    // free_memory();   // Free allocated memory
    // close_open_files(); // Close open files
    // save_system_state(); // Save system state
    // log_shutdown_info(); // Log shutdown information
    // notify_processes(); // Notify other processes
    // clean_up_resources(); // Clean up resources
    // stop_all_processes(); // Stop all running processes
    // disable_interrupts(); // Disable interrupts
    // set_cpu_state(); // Set CPU to a known state
    // return_control_to_bootloader(); // Return control to bootloader or 
    scheduler_stop();
    std::exit(status);
}


std::string get_timestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%m/%d/%Y, %I:%M:%S %p", std::localtime(&now_time));
    return std::string(buffer);
}

// void generate_file(int core_id){
//     int current_file = file_count.fetch_add(1);
//     if (current_file >= num_processes) return;

//     std::string filename = "process_file_" + std::to_string(current_file) + "_id_" + std::to_string(core_id) + ".txt";
//     std::string start_time = get_timestamp();

//     {
//         std::lock_guard<std::mutex> lock(process_mutex);
//         processes.push_back({current_file, filename, "Running", std::this_thread::get_id(), core_id, start_time, ""});

//     }

//     std::ofstream outfile(filename);
//     for (int i = 0; i < 100; ++i) {
//         std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate work
//         std::string timestamp = get_timestamp();
//         outfile << "(" << timestamp << ") "
//                 << "Core: " << core_id << " - "
//                 << "\"Hello world from " << filename << "!\"\n";
//     }
//     outfile.close();

//     {
//         std::lock_guard<std::mutex> lock(process_mutex);
//         for (auto& p : processes) {
//             if (p.filename == filename) {
//                 p.status = "Finished";
//                 p.end_time = get_timestamp();
//                 break;
//             }
//         }
//     }
// }


// void start_file_generation() {
//     std::cout << "Generating files using " << num_cores << " cores...\n";
//     std::vector<std::thread> threads;

//     for (int i = 0; i < num_cores; ++i) {
//         threads.emplace_back([i]() {
//             while (true) {
//                 int current_file = file_count.load();
//                 if (current_file >= num_processes) break;
//                 generate_file(i); // core ID remains same
//             }
//         });
//     }

//     for (auto& t : threads) {
//         t.join();
//     }

//     std::cout << "All files generated.\n";
// }

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

        // // Simulate instruction progression
        // session.current_line = std::min(session.current_line + 1, session.total_lines);
    }
}


void new_screen(std::string name) {

    if (head == nullptr) {
        head = new ScreenSession(name, 1, 50, get_timestamp()); // placeholder values
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

            ScreenSession *new_screen = new ScreenSession(name, 1, 50, get_timestamp()); //placeholder values
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
        // initialize os env

        std::cout << std::endl << std::endl;

        std::cout << "Initialized configuration: \nCPU Cores: " << num_cpu << "\n";
        std::cout << "Scheduler: " << scheduler << "\n";
        std::cout << "Quantum Cycles: " << quantumcycles << "\n";
        std::cout << "Batch Process Frequency: " << batchprocess_freq << "\n";
        std::cout << "Min Instructions: " << min_ins << "\n";
        std::cout << "Max Instructions: " << max_ins << "\n";
        std::cout << "Delays per Execution: " << delays_perexec << "\n";
        
        initialize();
        if (current_screen) current_screen->current_line++;
        system("pause");
    } else if (choice == "scheduler-start") {
        std::cout << "Scheduler-test command recognized. Doing something.\n";
        scheduler_start();
        if (current_screen) current_screen->current_line++;
        system("pause");
    } else if (choice == "scheduler-stop") {
        std::cout << "Scheduler-stop command recognized. Doing something.\n";
      // debugging purposesl
        scheduler_stop();
        if (current_screen) current_screen->current_line++;
        system("pause");
    } else if (choice == "report-util") {
        std::cout << "Report-util command recognized. Doing something.\n";
        report_util();
        if (current_screen) current_screen->current_line++;
        system("pause");
    } else if (choice == "clear") {
        std::cout << "Clear command recognized. Doing something.\n";
        clear_screen();
        if (current_screen) current_screen->current_line++;
        system("pause");
    } else if (choice == "exit") {
        std::cout << "Exit command recognized. Exiting...\n";
        exit = true;
    } else if (choice == "help") {
        std::cout << "Help command recognized.\n";
        std::cout << "Available commands:\n";
        std::cout << "1. initialize - Initialize the OS environment.\n";
        std::cout << "2. screen - Initialize the screen.\n";
        std::cout << "3. scheduler-test - Start the scheduler test.\n";
        std::cout << "4. scheduler-stop - Stop the scheduler.\n";
        std::cout << "5. report-util - Report system information and statistics.\n";
        std::cout << "6. clear - Clear the screen.\n";
        std::cout << "7. exit - Exit the OS.\n";
        std::cout << "8. help - Show this help message.\n";
        if (current_screen) current_screen->current_line++;
        system("pause");

    } 

    else if (choice.rfind("screen -s ", 0) == 0) {
        std::string name = choice.substr(10);
        new_screen(name);
        if (current_screen) current_screen->current_line++;
    }
    else if (choice.rfind("screen -r ", 0) == 0) {
        std::string name = choice.substr(10);  // get <name>
        find_screen(name);
        if (current_screen) current_screen->current_line++;

    } else if(choice.rfind("screen -ls", 0) == 0) {
        std::cout << "\nBackground Processes:\n";
        std::lock_guard<std::mutex> lock(process_mutex);

        if (processes.empty()) {
            std::cout << "No background processes.\n";
        } else {
            for (const auto& proc : processes) {
                std::cout << "ID: " << proc.getPid() << "\n"
                        << "Name: " << proc.getProcessName() << "\n"
                        << "Priority: " << proc.getPriority() << "\n"
                        << "Status: " << processStateToString(proc.getState()) << "\n"
                        << "Thread/Core ID: " << proc.getCurrentCoreId() << "\n"
                        << "Started: " << formatTime(proc.getStartTime()) << "\n"
                        << "Ended: " << (proc.getState() == ProcessState::FINISHED ? formatTime(proc.getEndTime()) : "N/A") << "\n\n";
            }
        }

        if (current_screen) current_screen->current_line++;

        system("pause");

        
    } else if (choice == "^g") {
        std::thread(scheduler_start).detach();
    } else {
        std::cout << "Unknown command: " << choice << "\n";
    }
    clear_screen();
    return exit;
}


void menu(){
    std::string choice;

    while(true){
        screen_init();
        std:getline(std::cin, choice);
        bool exit = accept_input(choice, nullptr);
        if(exit == true){
            break;
        }
    }
}
