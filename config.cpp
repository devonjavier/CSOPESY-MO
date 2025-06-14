// 
#include <fstream>
#include "src/json.hpp"
#include <iostream>
using json = nlohmann::json;

class config {
    private:
    int cores;
    int processes;

    struct configurations {
        int cores;
        int processes;
    };

    configurations read_json_file(const std::string &filename) {
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
            json config = json::parse(configContent);
            configurations configs;
            configs.cores = config.value("configurations", json::object()).value("cores", 0);
            configs.processes = config.value("configurations", json::object()).value("processes", 0);
            return configs; //return the configurations read from the file
        } catch (const json::parse_error& e) {
            std::cerr << "Error parsing JSON: " << e.what() << std::endl;
            return default_configs; //return default configurations if parsing fails
        } catch (const json::exception& e) {
            std::cerr << "Error accessing JSON data: " << e.what() << std::endl;
            return default_configs; //return default configurations if accessing fails
        }
    }

    public:
    config(const std::string &filename){
        configurations configurationsReceived = read_json_file(filename);
        cores = configurationsReceived.cores;
        processes = configurationsReceived.processes;
    }

    int getCores() const {
        return cores;
    }
    int getProcesses() const {
        return processes;
    }

    void displayConfig() const {
        std::cout << "Cores: " << cores << "\n"
        << "Processes: " << processes << std::endl;
    }
};