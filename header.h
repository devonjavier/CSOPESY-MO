#ifndef OS_INTERFACE_H
#define OS_INTERFACE_H

#include <string>

class Process;

void initialize();
void screen_init();
void scheduler_start();
void scheduler_stop();
void report_util();
void clear_screen();
bool accept_input(std::string, Process*);
void menu();

#endif