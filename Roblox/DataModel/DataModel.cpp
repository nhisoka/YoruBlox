//
// Created by Yoru on 5/8/2024.
//

#include "DataModel.hpp"
#include <iostream>

DataModel* DataModel::g_Singleton = nullptr;

DataModel* DataModel::get_singleton() noexcept {
    if (g_Singleton == nullptr)
        g_Singleton = new DataModel();
    return g_Singleton;
}

std::uint64_t DataModel::get_datamodel()
{

	HANDLE main_thread = OpenThread(THREAD_ALL_ACCESS, NULL, this->main_thread_id);

	CONTEXT ctx = {};
	ctx.ContextFlags = CONTEXT_INTEGER;


	while (1) {
			GetThreadContext(main_thread, &ctx);

			std::uint64_t name_ptr = Driver::get_singleton()->read<std::uint64_t>(ctx.Rcx + 0x48);

			char name[999];

			Driver::get_singleton()->read_memory(name_ptr, (uintptr_t)&name, sizeof(name));

			if (strcmp(name, "Game") == 0 or strcmp(name, "App") == 0) {
				this->datamodel = ctx.Rcx;

				if (strcmp(name, "Game") == 0) {
					this->ingame = true;
				}
				else
				{
					this->ingame = false;
				}
				break;
		}
	}

    return this->datamodel;
}