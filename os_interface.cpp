#include <iostream>
#include <string>
#include <cstdlib> // for system()
#include <ctime>
#include "classes/screen.cpp"
#include "header.h"
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include "classes/Scheduler.cpp"
#include <fstream>
#include <sstream>
#include <random> // For random number generation
#include <chrono> // For random number seeding with time
#include <cmath> 

using namespace std;


//initialization of variables
int num_cpu = 0;
std::string scheduler_type = "";
int quantumcycles = 0;
int batchprocess_freq = 0;
int min_ins = 0;
int max_ins = 0;
int delays_perexec = 0;

// new parameters for OS emulator memory manager
int max_overall_mem = 0;
int mem_per_frame = 0;
int mem_per_proc = 0;

//initialization of Screens and Processes Lists
ScreenSession *head = nullptr; // linked list head
Scheduler* os_scheduler = nullptr;

std::string formatTime(const std::chrono::time_point<std::chrono::system_clock>& tp) {
    if (tp.time_since_epoch().count() == 0) return "N/A";
    std::time_t tt = std::chrono::system_clock::to_time_t(tp);
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%d/%m/%Y %I:%M:%S%p", std::localtime(&tt));
    return std::string(buffer);
}

std::string get_timestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%m/%d/%Y, %I:%M:%S %p", std::localtime(&now_time));
    return std::string(buffer);
}

ICommand* generateRandomInstruction() {
    // Randomly choose an instruction type
    int instruction_type = rand() % 6; // 0 to 4 for 5 different types

    switch (instruction_type) {
        case 0: // PRINT
            return new PRINT(); 
        case 1: // DECLARE
            return new DECLARE("var" + std::to_string(rand() % 100), rand() % 100);
        case 2: // SLEEP
            return new SLEEP(rand() % 50 + 1);
        case 3: {
            int loop_count = rand() % 3 + 1; 
            std::vector<std::unique_ptr<ICommand>> body;

            int body_instr_count = rand() % 3 + 1;
            for (int i = 0; i < body_instr_count; ++i) {
                body.push_back(std::unique_ptr<ICommand>(generateRandomInstruction()));
            }

            return new FOR(std::move(body), loop_count);
        }
        case 4: // SUBTRACT with random variables or values
            return new SUBTRACT("result", "var" + std::to_string(rand() % 100), "var" + std::to_string(rand() % 100));
        case 5:
            return new ADD("result", "var" + std::to_string(rand() % 100), "var" + std::to_string(rand() % 100));
        default:
            return new UNKNOWN;
    }
}

void generate_random_processes() {
    static int next_id = 1;

    for (int i = 0; i < batchprocess_freq; ++i) {
        cout << (i+1) << " out of " << batchprocess_freq << " processes being generated.\n";
        std::default_random_engine generator(
            std::chrono::system_clock::now().time_since_epoch().count()
        );
        // std::uniform_int_distribution<int> instructionDist(min_ins, max_ins);
        std::uniform_int_distribution<uint64_t> otherDist(min_ins, (1ULL << max_ins));
        uint64_t num_instructions = otherDist(generator);

        Process proc(next_id, "process" + std::to_string(next_id));

        std::cout << "  -> Creating Process ID " << next_id
                  << " with " << num_instructions << " instructions.\n";

        for (uint64_t i = 0; i < num_instructions; ++i) {
            ICommand* cmd = generateRandomInstruction(); // Your custom logic here
            proc.addInstruction(cmd);
        }

        os_scheduler->addProcess(proc);
        next_id++;
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
        } else if (key == "Scheduler") {
            std::string rest;
            std::getline(iss, rest);
            std::istringstream rest_iss(rest);
            rest_iss >> scheduler_type;

            if (scheduler_type != "fcfs" && scheduler_type != "rr") {
                std::cerr << "Invalid Scheduler value. Must be 'fcfs' or 'rr'." << std::endl;
            }

            // Optional: print what's after "Scheduler"
            std::string remaining_args;
            std::getline(rest_iss, remaining_args);
            if (!remaining_args.empty()) {
                std::cout << "Extra arguments after Scheduler: " << remaining_args << std::endl;
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
    };

    os_scheduler = new Scheduler(scheduler_type, quantumcycles);

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

// 3. Scheduler_start()
//    - set up a test environment for the Scheduler
//    - create several test processes or threads
//    - according to the zoom kanina, each test process/thread will do some simple task
//
//    - starts the Scheduler
void Scheduler_start() {
    std::cout << "Starting Scheduler test...\n";

    os_scheduler->startScheduler(num_cpu);

    std::thread process_generator([]() {
        while (os_scheduler->isGeneratingProcesses()) {
            std::cout << "[Process Generator] Generating new batch of processes...\n";
            generate_random_processes();
            os_scheduler->queueProcesses();
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    });

    os_scheduler->checkIfComplete();

    process_generator.detach();  
}


// 4. Scheduler_stop()
//    - stops Scheduler
//        - preventing context switches
//        - disabling interrupts
//        - setting the CPU to a known state
void Scheduler_stop() {
    // Scheduler_running = false;
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
//        - stop Scheduler
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
    Scheduler_stop();
    std::exit(status);
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

    //convert choice to lowercase for case-insensitive comparison
    for (char &c : choice) {
        c = std::tolower(static_cast<unsigned char>(c));
    }

    if (choice == "initialize") {
        // initialize os env

        initialize();

        std::cout << std::endl << std::endl;
        std::cout << "Initialized configuration: \nCPU Cores: " << num_cpu << "\n";
        std::cout << "Scheduler: " << scheduler_type << "\n";
        std::cout << "Quantum Cycles: " << quantumcycles << "\n";
        std::cout << "Batch Process Frequency: " << batchprocess_freq << "\n";
        std::cout << "Min Instructions: " << min_ins << "\n";
<<<<<<< HEAD
        std::cout << "Max Instructions: " << (1ULL << max_ins) << "\n";
        std::cout << "Delays per Execution: " << delays_perexec << "\n\n";
        std::cout << "Max Overall Memory: " << max_overall_mem << "\n";
        std::cout << "Memory per Frame: " << mem_per_frame << "\n";
        std::cout << "Memory per Process: " << mem_per_proc << "\n\n\n\n";

=======
        std::cout << "Max Instructions: " << max_ins << "\n";
        std::cout << "Delays per Execution: " << delays_perexec << "\n\n\n";
>>>>>>> 1de3a9529098009810675c4fdd40f0a93d370dc0
        
        if (current_screen) current_screen->current_line++;
        system("pause");
    } else if (choice == "scheduler-start") {
        std::cout << "Scheduler-test command recognized. Doing something.\n";
        Scheduler_start();
        if (current_screen) current_screen->current_line++;
<<<<<<< HEAD
        std::cout << "ending scheduler_start() function\n";
        // sleep(60);
        system("pause");
=======
        sleep(60);
>>>>>>> 1de3a9529098009810675c4fdd40f0a93d370dc0
    } else if (choice == "scheduler-stop") {
        std::cout << "Scheduler-stop command recognized. Doing something.\n";
      // debugging purposesl
        Scheduler_stop();
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
        std::cout << "3. Scheduler-test - Start the Scheduler test.\n";
        std::cout << "4. Scheduler-stop - Stop the Scheduler.\n";
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

    // } else if(choice.rfind("screen -ls", 0) == 0) {
    //     std::cout << "\nBackground Processes:\n";
    //     std::lock_guard<std::mutex> lock(process_mutex);

    //     if (processes.empty()) {
    //         std::cout << "No background processes.\n";
    //     } else {
    //         for (const auto& proc : processes) {
    //             std::cout << "ID: " << proc.getPid() << "\n"
    //                     << "Name: " << proc.getProcessName() << "\n"
    //                     // << "Priority: " << proc.getPriority() << "\n"
    //                     << "Status: " << processStateToString(proc.getState()) << "\n"
    //                     << "Thread/Core ID: " << proc.getCurrentCoreId() << "\n"
    //                     << "Started: " << formatTime(proc.getStartTime()) << "\n"
    //                     << "Ended: " << (proc.getState() == ProcessState::FINISHED ? formatTime(proc.getEndTime()) : "N/A") << "\n\n";
    //         }
    //     }

    //     if (current_screen) current_screen->current_line++;

    //     system("pause");
        
    } else if (choice == "^g") {
        std::thread(Scheduler_start).detach();
    } else {
        std::cout << "Unknown command: " << choice << "\n";
    }
    
    clear_screen();
    return exit;
}


void menu(){
    clear_screen();

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
