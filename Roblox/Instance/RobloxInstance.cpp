//
// Created by Yoru on 5/8/2024.
//

#include <iostream>
#include "RobloxInstance.hpp"
#include "../../driver/Driver.h"
#include "xxhash.h"
#include "zstd.h"

const auto pDriver{ Driver::get_singleton() };

std::string readstring(std::uint64_t address)
{
    std::string string;
    char character = 0;
    int char_size = sizeof(character);
    int offset = 0;

    string.reserve(204);

    while (offset < 200)
    {
        character = pDriver->read<char>(address + offset);

        if (character == 0)
            break;

        offset += char_size;
        string.push_back(character);
    }

    return string;
}


std::string readstring2(std::uint64_t string)
{
    const auto length = pDriver->read<int>(string + 0x18);

    if (length >= 16u)
    {
        const auto New = pDriver->read<std::uint64_t>(string);
        return readstring(New);
    }
    else
    {
        const auto Name = readstring(string);
        return Name;
    }
}

std::string Decompress(const std::string& source) noexcept {
    const uint8_t kBytecodeMagic[4] = { 'R', 'S', 'B', '1' };
    const int kBytecodeHashMultiplier = 41;
    const int kBytecodeHashSeed = 42;

    try {
        std::vector<uint8_t> ss(source.begin(), source.end());
        std::vector<uint8_t> hb(4);

        for (size_t i = 0; i < 4; ++i) {
            hb[i] = ss[i] ^ kBytecodeMagic[i];
            hb[i] = (hb[i] - i * kBytecodeHashMultiplier) % 256;
        }

        for (size_t i = 0; i < ss.size(); ++i) {
            ss[i] ^= (hb[i % 4] + i * kBytecodeHashMultiplier) % 256;
        }

        uint32_t hash_value = 0;
        for (size_t i = 0; i < 4; ++i) {
            hash_value |= hb[i] << (i * 8);
        }

        uint32_t rehash = XXH32(ss.data(), ss.size(), kBytecodeHashSeed);
        if (rehash != hash_value) {
            throw std::runtime_error("Failed to decompress bytecode. (1)");
        }

        uint32_t decompressed_size = 0;
        for (size_t i = 4; i < 8; ++i) {
            decompressed_size |= ss[i] << ((i - 4) * 8);
        }

        std::vector<uint8_t> compressed_data(ss.begin() + 8, ss.end());
        std::vector<uint8_t> decompressed(decompressed_size);

        size_t const decompressed_size_actual = ZSTD_decompress(decompressed.data(), decompressed_size, compressed_data.data(), compressed_data.size());
        if (ZSTD_isError(decompressed_size_actual)) {
            throw std::runtime_error("Failed to decompress bytecode. (2)");
        }

        decompressed.resize(decompressed_size_actual);
        return std::string(decompressed.begin(), decompressed.end());

    }
    catch (const std::exception& e) {
        return "failed to decompress bytecode";
    }
}


std::string RobloxInstance::name()
{
    const auto ptr = pDriver->read<std::uint64_t>(this->self + offsets::name);

    if (ptr)
        return readstring2(ptr);

    return "???";
}

std::string RobloxInstance::class_name()
{
    const auto ptr = pDriver->read<std::uint64_t>(this->self + offsets::classname);

    if (ptr)
        return readstring(pDriver->read<std::uint64_t>(ptr + 0x8)); // return readstring2(ptr + 0x8)

    return "???_classname";
}

std::uint64_t RobloxInstance::get_class_descriptor()
{
    return pDriver->read<std::uint64_t>(this->self + offsets::classname);
}


std::vector<RobloxInstance> RobloxInstance::children()
{
    std::vector<RobloxInstance> container;

    if (!this->self)
        return container;

    auto start = pDriver->read<std::uint64_t>(this->self + offsets::children);

    if (!start)
        return container;

    auto end = pDriver->read<std::uint64_t>(start + offsets::size);

    for (auto instances = pDriver->read<std::uint64_t>(start); instances != end; instances += 16)
        container.emplace_back(pDriver->read<RobloxInstance>(instances));

    return container;
}

RobloxInstance RobloxInstance::find_first_child(std::string child)
{
    RobloxInstance ret;

    for (auto& object : this->children())
    {
        if (object.name() == child)
        {
            ret = static_cast<RobloxInstance>(object);
            break;
        }
    }

    return ret;
}

RobloxInstance RobloxInstance::FindFirstChildOfClass(std::string class_name)
{
    RobloxInstance ret;

    for (auto& object : this->children())
    {

        if (object.class_name() == class_name)
        {
            ret = static_cast<RobloxInstance>(object);
            break;
        }
    }

    return ret;
}


RobloxInstance RobloxInstance::get_object_value()
{
    auto ptr = pDriver->read<std::uint64_t>(this->self + 0xC0);

    auto ptr_to_instance = static_cast<RobloxInstance>(ptr);

    return ptr_to_instance;
}

void RobloxInstance::set_scriptable()
{
    auto class_descriptor = this->get_class_descriptor();

    if (!class_descriptor)
        return;

    pDriver->write<unsigned long>(class_descriptor + 0x40, 63);
}

void RobloxInstance::spoof(RobloxInstance gyat)
{
    pDriver->write<unsigned long long>(this->self + 0x8, gyat.self);
}






std::string RobloxInstance::get_string_value()
{
    auto ptr = pDriver->read < std::uint64_t >(this->self + 0xC0);

    return readstring2(ptr);
}

void RobloxInstance::SetModuleBypass()
{
    pDriver->write<unsigned long long>(this->self + offsets::ModuleFlags, 0x100000000);
}

void RobloxInstance::SetBytecode(std::vector<uint8_t> bytes, int bytecode_size)
{
    auto old_bytecode_ptr = pDriver->read<long long>(this->self + offsets::ModuleScript);

    auto protected_str_ptr = (long long)pDriver->allocate_virtual_memory(bytecode_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE); //  size of my dick
    /* Allocated nice */

    pDriver->write_memory(protected_str_ptr, (uintptr_t)bytes.data(), bytes.size());
    pDriver->write<unsigned long long>(old_bytecode_ptr + 0x10, protected_str_ptr);
    pDriver->write<unsigned long>(old_bytecode_ptr + 0x20, bytecode_size);


}


std::string RobloxInstance::Bytecode()
{
    auto class_name = this->class_name();

    int offset = 0x0;

    if (class_name == "ModuleScript") {
        offset = offsets::ModuleScript;
    }
    else if (class_name == "LocalScript") {
        offset = offsets::LocalScript;
    }
    else {
        throw std::exception("Invalid Script");
    }


    std::uint64_t bytecode_ptr = pDriver->read<std::uint64_t>(this->self + offset);
    unsigned long long bytecode_ptr_str = pDriver->read<unsigned long long>(bytecode_ptr + 0x10);

    unsigned long bytecode_size = pDriver->read<unsigned long>(bytecode_ptr + 0x20);
    std::vector<uint8_t> bytecode(bytecode_size);
    pDriver->read_memory(bytecode_ptr_str, (uintptr_t)bytecode.data(), bytecode_size);

    std::cout << "[~] Bytecode size: " << bytecode_size << std::endl;

    return "";
}


std::uintptr_t RobloxInstance::get_gameid()
{
    auto gameid = pDriver->read<std::uintptr_t>(this->self + offsets::gameid);
    return gameid;
}
