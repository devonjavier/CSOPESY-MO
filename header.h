#ifndef OS_INTERFACE_H
#define OS_INTERFACE_H

#include <string>

void initialize();
void screen_init();
void scheduler_test();
void scheduler_stop();
void report_util();
void clear_screen();
void exit_os(int status);
bool accept_input(std::string, ScreenSession*);
void menu();

#endif