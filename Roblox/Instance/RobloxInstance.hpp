//
// Created by user on 5/8/2024.
//
#pragma once

#include <string>
#include <vector>
#include "../../offsets.hpp"
#include <optional>

class RobloxInstance {
public:
    std::uint64_t self;

    struct vector2_t final { float x, y; };
    struct vector3_t final { float x, y, z; };
    struct quaternion final { float x, y, z, w; };
    struct matrix4_t final { float data[16]; };

    std::string name();
    std::string class_name();
    std::vector<RobloxInstance> children();
    RobloxInstance find_first_child(std::string child);
    RobloxInstance FindFirstChildOfClass(std::string child);

    std::uintptr_t get_gameid();

    void spoof(RobloxInstance gyat);

    void SetBytecode(std::vector<uint8_t> bytes, int bytecode_size);
    
    std::string Bytecode();

    std::string get_string_value();

    void SetModuleBypass();

    long capabilities();

    RobloxInstance get_object_value();

    void set_scriptable();

    std::uint64_t get_class_descriptor();
};

