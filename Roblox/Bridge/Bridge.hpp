#pragma once

#include <cstdint>
#include <fstream>

class Bridge {
    static Bridge* g_Singleton;

public:
    static Bridge* get_singleton() noexcept;

    void initialize();

    void execute_script(std::string source);

    void start();
};
