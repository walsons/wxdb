#ifndef META_COMMAND_H_
#define META_COMMAND_H_

#include <unordered_map>
#include <string>

class MetaCommand {
public:
    MetaCommand() : command2number_(nullptr) {
        init();
    }
    ~MetaCommand() {
        delete command2number_;
        command2number_ = nullptr;
    }
    bool parse(const std::string &user_input) const {
        bool flag = true;
        if (command2number(user_input) == -1) {
            std::cout << "unknown command" << std::endl;
            return flag;
        }
        switch(command2number(user_input)) {
            case 0: 
                flag = false;
                break;
            default:
                break;
        }
        return flag;
    }
private:
    std::unordered_map<std::string, int> *command2number_;

private:
    void init() {
        command2number_ = new std::unordered_map<std::string, int>{
            {".exit", 0}
        };
    }
    // If the command is not found, return -1
    int command2number(const std::string &command) const {
        if (command2number_->find(command) == command2number_->end()) {
            return -1;
        }
        return (*command2number_)[command];
    }
};

#endif
