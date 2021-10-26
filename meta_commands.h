#ifndef META_COMMANDS_H_
#define META_COMMANDS_H_

#include <unordered_map>
#include <string>

class MetaCommands {
public:
    MetaCommands() : data_(nullptr) {
        init();
    }
    void init() {
        data_ = new std::unordered_map<std::string, int>{
            {".exit", 0}
        };
    }
    ~MetaCommands() {
        delete data_;
        data_ = nullptr;
    }
    std::unordered_map<std::string, int> *data_;
};

#endif
