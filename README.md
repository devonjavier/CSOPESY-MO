# CSOPESY-MO

# 💻 CSOPESY: Command-Line Operating System Simulator

Welcome to the **CSOPESY Destroyers'** Command-Line Operating System Simulator!  
This project simulates an operating system environment with multithreaded process scheduling, screen sessions, and interactive CLI utilities.

---

## 📂 Table of Contents

- [Features](#features)
- [Installation](#installation)
- [Configuration](#configuration)
- [Usage](#usage)
- [Command List](#command-list)
- [Function Overview](#function-overview)
- [Credits](#credits)

---

## ✨ Features

- FCFS and Round-Robin scheduling
- Multithreaded process execution
- Simulated screen sessions with background task management
- Dynamic process queue
- Interactive command-line interface
- Clear, modular architecture

---

## 🛠 Installation

1. **Clone the repository**:
   ```bash
   git clone https://github.com/devonjavier/CSOPESY-MO.git
   cd csopesy-os

2. Compile the project (requires a C++17+ compiler):
    g++ main.cpp -o main

3. Prepare your configuration file:
    Create a config.txt in the root directory with the following format with values such as:
    num-cpu 8
    scheduler rr
    quantumcycles 10
    batchprocess-freq 5
    min-ins 100
    max-ins 500
    delays-perexec 0

4. Run the OS simulator:
    ./main

5. Command List
    | Command            | Description                                     |
    | ------------------ | ----------------------------------------------- |
    | `initialize`       | Loads configuration and sets up the environment |
    | `scheduler-start`  | Begins the scheduler with random processes      |
    | `scheduler-stop`   | Halts the scheduler loop                        |
    | `report-util`      | (Planned) Displays system statistics            |
    | `clear`            | Clears the terminal screen                      |
    | `exit`             | Exits the OS                                    |
    | `help`             | Shows all available commands                    |
    | `screen -s <name>` | Starts a new screen session                     |
    | `screen -r <name>` | Resumes an existing screen session              |
    | `screen -ls`       | Lists all running background processes          |
    | `^g`               | Starts scheduler in a detached thread           |


6. Function Overview

Utility Functions
formatTime() — Format a time_point to readable string
get_timestamp() — Get current timestamp as string
Initialization
initialize() — Read config.txt and set up system
Scheduling
run_fcfs_scheduler() — First-Come First-Serve scheduling logic
run_rr_scheduler() — Round-Robin scheduling using quantumcycles
generate_random_processes() — Create random dummy processes
Scheduler Control
scheduler_start() — Starts process generation and dispatching
scheduler_stop() — Stops the process scheduler
System Utilities
report_util() — Display system usage info (stub)
clear_screen() — Clear screen cross-platform
exit_os(int status) — Graceful system shutdown
Screen Sessions
screen_init() — Welcome banner and startup prompt
screen_session() — Session view for an individual screen
new_screen(name) — Create a new screen session
find_screen(name) — Resume an existing screen session
Input Handling
accept_input() — Parses CLI commands
Main Loop
menu() — Repeated prompt waiting for user input

7. Credits
    This project was created by the CSOPESY team.
    ASCII art generated using patorjk.com/software/taag.

8. Notes
    This is a simulated OS environment; no actual memory or file system manipulation occurs.
    Multithreaded process simulations use C++11 threads and mutexes.
    You can extend the project with real file generation, memory tracking, and error logging.
