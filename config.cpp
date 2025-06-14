#include <fstream>      //std::ifstream

#include "header_files/config.h"


config::configurations config::read_json_file(const std::string &filename) {
    std::ifstream configFile(filename);

    //default
    configurations default_configs = {0, 0};

    //check if file opened successfully
    if (!configFile.is_open()) {
        std::cerr << "Error: Could not open config file: " << filename << std::endl;
        return default_configs; //return default configurations if file cannot be opened
    }

    //parse file into a string
    std::string configContent((std::istreambuf_iterator<char>(configFile)), std::istreambuf_iterator<char>());

    //close file
    configFile.close();

    //parse JSON
    try {
        json config_json = json::parse(configContent); // Renamed to avoid conflict with class name
        configurations configs;
        configs.cores = config_json.value("configurations", json::object()).value("cores", 0);
        configs.processes = config_json.value("configurations", json::object()).value("processes", 0);
        return configs; //return the configurations read from the file
    } catch (const json::parse_error& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        return default_configs; //return default configurations if parsing fails
    } catch (const json::exception& e) {
        std::cerr << "Error accessing JSON data: " << e.what() << std::endl;
        return default_configs; //return default configurations if accessing fails
    }
}

config::config(const std::string &filename){
    configurations configurationsReceived = read_json_file(filename);
    cores = configurationsReceived.cores;
    processes = configurationsReceived.processes;
}

int config::getCores() const {
    return cores;
}
int config::getProcesses() const {
    return processes;
}

void config::displayConfig() const {
    std::cout << "Cores: " << cores << "\n"
              << "Processes: " << processes << std::endl;
}