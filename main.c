#include <stdio.h>      // for standard I/O functions like printf
#include <stdbool.h>    // for bool data type
#include <stddef.h>     // for size_t and NULL
#include <stdint.h>     // for explicit width integer types like uint32_t
#include <string.h>     // for string manipulation functions like strcmp
#include <stdlib.h>     // for system() function to clear the screen

/*
    BALINGIT, JAVIER, RAMOS, <insert name here>

 * Task: Week 2 - Group Homework - Setting up your OS emulator
    * ✅ Create your first-ever main menu. Use only the C++ built-in libraries.
    * ✅ Provide your ASCII text header “CSOPESY” or a name for your command line emulator.
    * Set up template code that accepts the following commands: initialize, screen, scheduler-test, scheduler-stop, report-util, clear, and exit. Simply print “X command recognized. Doing something.”, where X is the command recognized.
    * For the clear command, once recognized, the screen clears, reprinting the header texts.
    * For the exit command, the application/CLI immediately closes.
 */

//TODO: Make everything thread safe

void initialize();
void screen_init();
void scheduler_test();
void scheduler_stop();
void report_util();
void clear_screen();
void exit_os(int status);

int main() {
    char choice[20];
    bool OSrunning = true;
    
    while (OSrunning) {
        screen_init();

        // to be replaced with thread safe input
        scanf("%s", choice);

        if (strcmp(choice, "initialize") == 0) {
            printf("Initialize command recognized. Doing something.\n");
            initialize();
        } else if (strcmp(choice, "screen") == 0) {
            printf("Screen command recognized. Doing something.\n");
            screen_init();
        } else if (strcmp(choice, "scheduler-test") == 0) {
            printf("Scheduler-test command recognized. Doing something.\n");
            scheduler_test();
        } else if (strcmp(choice, "scheduler-stop") == 0) {
            printf("Scheduler-stop command recognized. Doing something.\n");
            scheduler_stop();
        } else if (strcmp(choice, "report-util") == 0) {
            printf("Report-util command recognized. Doing something.\n");
            report_util();
        } else if (strcmp(choice, "clear") == 0) {
            printf("Clear command recognized. Doing something.\n");
            clear_screen();
        } else if (strcmp(choice, "exit") == 0) {
            printf("Exit command recognized. Exiting...\n");
            break;
        } else if (strcmp(choice, "help") == 0) {
            printf("Help command recognized. Exiting...\n");
            printf("Available commands:\n");
            printf("1. initialize - Initialize the OS environment.\n");
            printf("2. screen - Initialize the screen.\n");
            printf("3. scheduler-test - Start the scheduler test.\n");
            printf("4. scheduler-stop - Stop the scheduler.\n");
            printf("5. report-util - Report system information and statistics.\n");
            printf("6. clear - Clear the screen.\n");
            printf("7. exit - Exit the OS.\n");
            printf("8. help - Show this help message.\n");
            system("pause");
        } else {
            printf("Unknown command: %s\n", choice);
        }
    }
    
    exit_os(0); // exit the OS with a success status
    return 0;
}

// 1. initialize()
//  - sets up the basic hardware environment
//    - ie yung core core na pinakita ni sir neil
//
//    Gemini says:
//    - Essential initializations:
//        - Initializes the Global Descriptor Table (GDT) or Interrupt Descriptor Table (IDT) if needed for your architecture.
//        - Sets up basic memory management (e.g., identity mapping).
//        - Initializes the screen (in text mode or graphics mode).
//        - Sets up the initial stack.
//    - It does NOT handle full-fledged memory management or process scheduling.  Those come later.
void initialize() {
    // Gemini example:
        // Initialize GDT (or IDT) - architecture-specific!
        // gdt_init();
        // idt_init();

        // Set up a minimal stack (architecture-specific!)
        // stack_init();

        // Basic memory setup (e.g., identity mapping)
        // memory_init();
}

// 2. screen_init()
void screen_init() {
    clear_screen();

    // credits to https://patorjk.com/software/taag/#p=display&f=Slant%20Relief&t=CSOPESY for the unmodified ASCII art
    printf("________/\\\\\\\\\\\\\\\\\\_____/\\\\\\\\\\\\\\\\\\\\\\_________/\\\\\\\\\\_______/\\\\\\\\\\\\\\\\\\\\\\\\\\____/\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\_____/\\\\\\\\\\\\\\\\\\\\\\____/\\\\\\________/\\\\\\_        \n");
    printf(" _____/\\\\\\////////____/\\\\\\/////////\\\\\\_____/\\\\\\///\\\\\\____\\/\\\\\\/////////\\\\\\_\\/\\\\\\///////////____/\\\\\\/////////\\\\\\_\\///\\\\\\____/\\\\\\/__       \n");
    printf("  ___/\\\\\\/____________\\//\\\\\\______\\///____/\\\\\\/__\\///\\\\\\__\\/\\\\\\_______\\/\\\\\\_\\/\\\\\\______________\\//\\\\\\______\\///____\\///\\\\\\/\\\\\\/____      \n");
    printf("   __/\\\\\\_______________\\////\\\\\\__________/\\\\\\______\\//\\\\\\_\\/\\\\\\\\\\\\\\\\\\\\\\\\\\/__\\/\\\\\\\\\\\\\\\\\\\\\\_______\\////\\\\\\_____________\\///\\\\\\/______     \n");
    printf("    _\\/\\\\\\__________________\\////\\\\\\______\\/\\\\\\_______\\/\\\\\\_\\/\\\\\\/////////____\\/\\\\\\///////___________\\////\\\\\\____________\\/\\\\\\_______    \n");
    printf("     _\\//\\\\\\____________________\\////\\\\\\___\\//\\\\\\______/\\\\\\__\\/\\\\\\_____________\\/\\\\\\_____________________\\////\\\\\\_________\\/\\\\\\_______   \n");
    printf("      __\\///\\\\\\___________/\\\\\\______\\//\\\\\\___\\///\\\\\\__/\\\\\\____\\/\\\\\\_____________\\/\\\\\\______________/\\\\\\______\\//\\\\\\________\\/\\\\\\_______  \n");
    printf("       ____\\////\\\\\\\\\\\\\\\\\\_\\///\\\\\\\\\\\\\\\\\\\\\\/______\\///\\\\\\\\\\/_____\\/\\\\\\_____________\\/\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\_\\///\\\\\\\\\\\\\\\\\\\\\\/_________\\/\\\\\\_______ \n");
    printf("        _______\\/////////____\\///////////__________\\/////_______\\///______________\\///////////////____\\///////////___________\\///________\n");
    printf("\n\nHello! Welcome to the CSOPESY destroyers' command-line operating system!\n");
    printf("Type 'exit' to quit, 'clear' to clear the screen, or 'help' for a list of commands.\n");
    printf("Please enter a command:\n");
}

// 3. scheduler_test()
//    - set up a test environment for the scheduler
//    - create several test processes or threads
//    - according to the zoom kanina, each test process/thread will do some simple task
//
//    - starts the scheduler
//
//    - bruh kailangan kaya nito ng sleep() or something to mimic the hz frequency shit of the CPU
void scheduler_test() {
    // TODO: Need randomization of threads 
    // TODO: Process creation itself
        // create_process("test_process_1", test_process_1_function, PRIORITY_HIGH);
        // create_process("test_process_2", test_process_2_function, PRIORITY_LOW);
        // create_process("test_process_3", test_process_3_function, PRIORITY_MEDIUM);
        // start_scheduler();
}

// 4. scheduler_stop()
//    - stops scheduler
//        - preventing context switches
//        - disabling interrupts
//        - setting the CPU to a known state
void scheduler_stop() {
    // disable_interrupts();
    // stop_scheduling();
}

// 5. report_util()
//    - report system information a d statistics
//        - Memory usage (total, used, free)
//        - CPU usage
//        - Running processes/threads
//        - System uptime
//        - Error logs
//        - idr what else was in the thingy sir presented
void report_util() {
    printf("Memory Usage: __ / __ KB\\n"); // printf("Memory Usage: %u / %u KB\\n", get_used_memory(), get_total_memory());
    printf("CPU Usage: __\n"); // printf("CPU Usage: %f%%\\n", get_cpu_usage());
    printf("Running Processes:\n");
    // print_process_list();
}

// 6. clear_screen()
//    - clears the entire terminal screen
void clear_screen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
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
    scheduler_stop();     // Stop the scheduler

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
    // return_control_to_bootloader(); // Return control to bootloader or firmware
    if (status != 0) {
       // printf("OS Exiting with error status: %d\n", status);
    }
    printf("OS shutting down...\n");
}