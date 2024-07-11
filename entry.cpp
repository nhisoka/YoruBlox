#include <iostream>
#include "driver/Driver.h"
#include "Mapper/kdmapper.hpp"
#include "Mapper/utils.hpp"
#include "Mapper/driver.h"
#include <thread>
#include "Roblox/DataModel/DataModel.hpp"
#include "Roblox/Instance/RobloxInstance.hpp"
#include "Roblox/Bridge/Bridge.hpp"
#include "utils/utils.h"
#include "BytecodeUtils.h"

HANDLE iqvw64e_device_handle;
using namespace kdmapper;
using namespace intel_driver;

std::string random_string()
{
    srand((unsigned int)time((time_t*)0));
    std::string str = "QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm1234567890";
    std::string newstr;
    int pos;
    while (newstr.size() != 32)
    {
        pos = ((rand() % (str.size() + 1)));
        newstr += str.substr(pos, 1);
    }
    return newstr;
}

void TitleThread()
{
    while (true)
    {
        SetConsoleTitleA(random_string().c_str());
    }
}

std::thread Title(TitleThread);

int main()
{
    const auto pDatamodel{ DataModel::get_singleton() };
    const auto pBridge{ Bridge::get_singleton() };
    std::cout << "[~] Initializing..." << std::endl;

    HANDLE device_handler = Load();

    if (!device_handler || device_handler == INVALID_HANDLE_VALUE)
    {
        std::cout << "[-] Failed to initialize." << std::endl;
        std::cin.get();
        return -1;
    }

    MapDriverBytes(device_handler, RawData);

    Unload(device_handler);

    std::cout << "[+] Initialized" << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(3));

    std::cout << "[~] Finding Roblox..." << std::endl;
    const auto pDriver{ Driver::get_singleton() };

    std::wstring target = L"RobloxPlayerBeta.exe";
    DWORD pid = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hSnapshot, &pe32)) {
            do {
                if (target.compare(pe32.szExeFile) == 0) {
                    pid = pe32.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnapshot, &pe32));
        }
        CloseHandle(hSnapshot);
    }

    if (pid == 0) {
        std::cout << "[-] Couldn't find Roblox";
        std::cin.get();
        return 1;
    }

    std::cout << "[+] Found Roblox" << std::endl;

    pDriver->initialize(L"\\\\.\\{f751dd83-fcc5-43b5-aa0d-398fe67bc306}", pid);

    uintptr_t target_process_base_address = pDriver->get_base_address(L"RobloxPlayerBeta.exe");

    if (target_process_base_address == 0) {
        std::cout << "[-] Failed to get base address of roblox. This may be because of the driver not being loaded" << std::endl;
    }

    // TODO: Add Ingame injection

    auto target_process_hwnd = utils::get_hwnd_of_process_id(pid);
    auto thread_id = GetWindowThreadProcessId(target_process_hwnd, 0);

    pDatamodel->main_thread_id = thread_id;

    std::uint64_t datamodel = pDatamodel->get_datamodel();
    RobloxInstance game = static_cast<RobloxInstance>(datamodel);

    auto coregui = game.FindFirstChildOfClass("CoreGui");

    auto robloxgui = coregui.find_first_child("RobloxGui");

    auto Modules = robloxgui.find_first_child("Modules");

    auto Common = Modules.find_first_child("Common");

    auto policyservice = Common.find_first_child("PolicyService");

    std::cout << "[~] Attaching..." << std::endl;


    if (pDatamodel->ingame == false) {
        policyservice.SetBytecode(init_script_bytecode, init_script_size);
    }
    else {
        std::cout << "[-] Ingame Injection is not supported as of right now!" << std::endl;
    }

	std::cout << "[+] Attached!" << std::endl;


    pBridge->initialize();
    pBridge->start();
}
