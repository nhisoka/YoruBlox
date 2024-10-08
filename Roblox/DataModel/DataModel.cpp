//
// Created by Yoru on 5/8/2024.
//

#include "DataModel.hpp"
#include <iostream>
#include "../Instance/RobloxInstance.hpp"

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

			auto possible_datamodel_instance = static_cast<RobloxInstance>(ctx.Rcx);

			if (possible_datamodel_instance.class_name() == "DataModel") {
				this->datamodel = ctx.Rcx;

				if (strcmp(possible_datamodel_instance.name().c_str(), "Game") == 0) {
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
