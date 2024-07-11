//
// Created by Yoru on 5/8/2024.
//

#include "Bridge.hpp"
#include <iostream>
#include "../../Dependencies/httplib.h"
#include <cpr/cpr.h>
#include <string>
#include "../../utils/json.hpp"
#include "../Instance/RobloxInstance.hpp"
#include <fstream>
#include "../DataModel/DataModel.hpp"
#include "../../utils/base64.h"
#include "../../utils/utils.h"

using namespace httplib;

Bridge* Bridge::g_Singleton = nullptr;

Bridge* Bridge::get_singleton() noexcept {
	if (g_Singleton == nullptr)
		g_Singleton = new Bridge();
	return g_Singleton;
}

Server svr;

/*
class : public Luau::BytecodeEncoder {
	std::uint8_t encodeOp(const std::uint8_t opcode) {
		return opcode * 227;
	}
} encoder{};
*/

/*
* IMPORTANT
* The filesystem library isnt secure at all!
* Also i would recommend switching to crow lib instead of this one
*/


void Bridge::initialize( ) {
	svr.Post("/bridge", [](const Request& req, Response& res) {


			auto json_body = nlohmann::json::parse(req.body);

			if (json_body.contains("action") && json_body.contains("script")) {
				auto action = json_body["action"].get<std::string>();

				if (action == "loadstring") {
					// just a attempt to make a faster and more reliable loadstring, ignore ugly code

					/*nlohmann::json response_json;
					RobloxInstance real_game = static_cast<RobloxInstance>(DataModel::get_singleton()->get_datamodel());
					auto coregui = real_game.FindFirstChildOfClass("CoreGui");

					auto module_script = coregui.find_first_child("YoruBlox");


					if (!module_script.self)
						return;

					module_script.SetModuleBypass();


					Luau::CompileOptions options{};
					options.coverageLevel = 0;
					options.debugLevel = 1;
					options.optimizationLevel = 1;
					std::string compiled = Luau::compile("return function(...) " + source + "\n end", options, {}, &encoder);

					std::vector<uint8_t> sigma_rizz(compiled.begin(), compiled.end());



					module_script.SetBytecode(sigma_rizz, compiled.size());
					response_json["status"] = "success";
					response_json["message"] = "loaded bytecode into script";
					res.set_content(response_json.dump(), "application/json");
				}
				else {
					res.status = 400;
					res.set_content("Invalid action", "text/plain");
				}
				*/
				}
				else {

					res.status = 400;
					res.set_content("Missing 'action' or 'script' field", "text/plain");
				}
			}
	});
	svr.Get("/bridge", [](const Request& req, Response& res) {
		auto action = req.get_param_value("action");
		auto arg1 = req.get_param_value("arg1");
		auto arg2 = req.get_param_value("arg2");

		if (action == "HttpGet") {
			auto result = cpr::Get(cpr::Url{ arg1 });

			nlohmann::json response_json;
			response_json["status"] = "success";
			response_json["response"] = base64_encode(result.text);
			response_json["message"] = "Done";

			res.set_content(response_json.dump(), "application/json");
		}
		else if (action == "isfolder") {
			nlohmann::json response_json;

			if (std::filesystem::is_directory("workspace\\" + arg1)) {
				response_json["status"] = "success";
				response_json["message"] = "True";
			}
			else {
				response_json["status"] = "success";
				response_json["message"] = "False";
			}

			res.set_content(response_json.dump(), "application/json");
		}
		else if (action == "fix_require") {
			nlohmann::json response_json;

			RobloxInstance real_game = static_cast<RobloxInstance>(DataModel::get_singleton()->get_datamodel());
			auto coregui = real_game.FindFirstChildOfClass("CoreGui");
			auto g = coregui.find_first_child("moduleholder_yorublox").get_object_value();

			if (!g.self) {
				response_json["status"] = "error";
				response_json["message"] = "Important files are missing";
			}

			g.SetModuleBypass();

			response_json["status"] = "success";
			response_json["message"] = "Bypassed";

			res.set_content(response_json.dump(), "application/json");
		}
		else if (action == "writefile") {
			nlohmann::json response_json;



				std::ofstream outfile("workspace\\" + arg1);

				if (outfile.is_open()) {
					outfile << arg2;
					outfile.close();

					response_json["status"] = "success";

					response_json["message"] = "Wrote content to file";

					res.set_content(response_json.dump(), "application/json");
				}
				else {
					response_json["status"] = "error";
					response_json["message"] = "Failed to write to file";


					res.set_content(response_json.dump(), "application/json");
				}

		}
		else if (action == "readfile") {
			std::ifstream inputf("workspace\\" + arg1);
			nlohmann::json response_json;

			if (inputf.is_open()) {
				std::string content;
				while (std::getline(inputf, content)) {}
				inputf.close();

				response_json["status"] = "success";


				response_json["message"] = content;

				res.set_content(response_json.dump(), "application/json");
			}
			else {
				response_json["status"] = "error";
				response_json["message"] = "Failed to read file";
			}
		}
		else if (action == "makefolder") {
			bool result = std::filesystem::create_directories("workspace\\" + arg1);

			nlohmann::json response_json;

			if (result) {
				response_json["status"] = "success";
				response_json["message"] = "Done easily";
			}
			else {
				response_json["status"] = "errhor";
				response_json["message"] = "Failed to create folders";
			}

			res.set_content(response_json.dump(), "application/json");
		}
		else if (action == "isfile") {
			nlohmann::json response_json;

			if (std::filesystem::is_regular_file("workspace\\" + arg1)) {
				response_json["status"] = "success";
				response_json["message"] = "True";
			}
			else {
				response_json["status"] = "success";
				response_json["message"] = "False";
			}

			res.set_content(response_json.dump(), "application/json");
		}
		else if (action == "delfile") {
			nlohmann::json response_json;

			bool result = std::filesystem::remove("workspace\\" + arg1);

			if (result) {
				response_json["status"] = "success";
				response_json["message"] = "Removed file";
			}
			else {
				response_json["status"] = "error";
				response_json["message"] = "Failed to remove file";
			}

			res.set_content(response_json.dump(), "application/json");
		}
		else if (action == "delfolder") {
			nlohmann::json response_json;

			bool result = std::filesystem::remove_all("workspace\\" + arg1);

			if (result) {
				response_json["status"] = "success";
				response_json["message"] = "Removed folder";
			}
			else {
				response_json["status"] = "error";
				response_json["message"] = "Folder doesnt exist";
			} 

			res.set_content(response_json.dump(), "application/json");
		}
		else if (action == "appendfile") {
			nlohmann::json response_json;



			std::ofstream outfile("workspace\\" + arg1, std::ios_base::app);

			if (outfile.is_open()) {
				outfile.write(arg2.c_str(), arg2.size());
				outfile.close();

				response_json["status"] = "success";

				response_json["message"] = "Appended content to file";

				res.set_content(response_json.dump(), "application/json");
			}
			else {
				response_json["status"] = "error";
				response_json["message"] = "Failed to append to file";


				res.set_content(response_json.dump(), "application/json");
			}

		}
		else if (action == "listfiles") {
			std::vector<std::string> files_paths = {};
			nlohmann::json response_json;

			if (std::filesystem::is_directory("workspace\\"+ arg1)) {
				for (const auto& entry : std::filesystem::directory_iterator("workspace\\"+arg1))
				{
					auto file_path = entry.path().string();
					file_path = utils::replace(file_path, "workspace\\", "");

					files_paths.push_back(file_path.c_str());
				}
				response_json["status"] = "success";
				response_json["message"] = nlohmann::json(files_paths);


				res.set_content(response_json.dump(), "application/json");
			}
			else {
				response_json["status"] = "error";
				response_json["message"] = "Failed to listfiles";


				res.set_content(response_json.dump(), "application/json");
			}
		}
		else if (action == "getscriptbytecode") {
			/*
			* Not working as of right now
			* TODO: FIX IT
			*/

			nlohmann::json response_json;
			RobloxInstance real_game = static_cast<RobloxInstance>(DataModel::get_singleton()->get_datamodel());
			auto coregui = real_game.FindFirstChildOfClass("CoreGui");

			auto holder = coregui.find_first_child("getscriptbytecode");

			auto script = holder.get_object_value();

			if (!script.self)
				response_json["status"] = "error";
		 	    response_json["message"] = "Failed to obtain the script to get bytecode of";


			auto script_bytecode = script.Bytecode();

			response_json["status"] = "success";
			response_json["message"] = base64_encode(script_bytecode);

			res.set_content(response_json.dump(), "application/json");
		}
	});
}


void Bridge::start() {
	svr.listen("localhost", 8000);
}