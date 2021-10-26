#ifndef META_COMMANDS_H_
#define META_COMMANDS_H_

#include <unordered_map>
#include <string>

class MetaCommands {
public:
    MetaCommands() : command2number_(nullptr) {
        init();
    }
    ~MetaCommands() {
        delete command2number_;
        command2number_ = nullptr;
    }
    // If the command is not found, return -1
    int command2number(std::string command) {
        if (command2number_->find(command) == command2number_->end()) {
            return -1;
        }
        return (*command2number_)[command];
    }
private:
    std::unordered_map<std::string, int> *command2number_;

private:
    void init() {
        command2number_ = new std::unordered_map<std::string, int>{
            {".exit", 0}
        };
    }
};

#endif
