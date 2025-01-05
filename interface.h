#ifndef INTERFACE_H
#define INTERFACE_H

#include <string>
#include <vector>
#include <stdexcept>
#include "llama.h"

class Interface {
public:
    Interface(const std::string& modelPath);
    ~Interface();

    void share(const std::string& text);
    std::string collect();

private:
    llama_context* ctx;
    llama_model* model;
    std::vector<llama_token> tokens;
};

#endif // INTERFACE_H
