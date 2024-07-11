#pragma once

#include <cstdint>
#include <fstream>
#include "../../driver/Driver.h"

class DataModel {
    static DataModel* g_Singleton;

public:
    static DataModel* get_singleton() noexcept;

    std::uint64_t datamodel;
    bool ingame;
    DWORD main_thread_id;

    std::uint64_t get_datamodel();
};
