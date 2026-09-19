#include <thread>
#include <chrono>
#include "TeleportHandler.hpp"
#include "../Roblox/Offsets/Offsets.h"
#include "../Roblox/Offsets/Funcs.h"
#include "../Roblox/Environment/Environment.h"
#include "../Roblox/Environment/Instances.h"
#include "Execution/Execution.h"

template<typename T>
static bool safe_read(uintptr_t address, uintptr_t offset, T* out) {
    if (!address) return false;
    __try {
        *out = *reinterpret_cast<T*>(address + offset);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

void connection(lua_State* L);
void flagfix();

TeleportHandler::TeleportHandler() 
{
    isloaded = false;
    LoadedDatamodel = 0;
}

void TeleportHandler::load() 
{
    std::thread(&TeleportHandler::loop, this).detach();
}

void TeleportHandler::loop() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));

        uintptr_t fakeDataModel = 0;
        if (!safe_read(Main::Miscellaneous::FakeDatamodelPOINTER, 0, &fakeDataModel) || !fakeDataModel) {
            if (isloaded) {
                isloaded = false;
                LoadedDatamodel = 0;
            }
            continue;
        }

        uintptr_t DataModel = 0;
        if (!safe_read(fakeDataModel, Main::Offsets::DataModel, &DataModel) || !DataModel) {
            if (isloaded) {
                isloaded = false;
                LoadedDatamodel = 0;
            }
            continue;
        }

        if (DataModel != LoadedDatamodel) {
            isloaded = false;
            LoadedDatamodel = DataModel;
        }

        int State = 0;
        if (!safe_read(DataModel, Main::Offsets::GameLoaded, &State)) continue;

        if (State == 31) {
            if (!isloaded) {
                uintptr_t children = 0;
                if (!safe_read(DataModel, Main::Offsets::Children, &children) || !children) continue;

                uintptr_t firstChild = 0;
                if (!safe_read(children, 0, &firstChild) || !firstChild) continue;

                uintptr_t scriptContext = 0;
                if (!safe_read(firstChild, Main::Offsets::ScriptContext, &scriptContext) || !scriptContext) continue;

                lua_State* skidstate = laustate::GetLuasState(scriptContext);
                if (!skidstate) continue;

                Execution::skidsstate = skidstate;

                flagfix();
                env::Register(skidstate);

                lua_getglobal(skidstate, "print");
                lua_pushstring(skidstate, "(your shitsploit name) has handled teleporting sir");
                lua_pcall(skidstate, 1, 0, 0);

                std::thread(connection, skidstate).detach();

                isloaded = true;
            }
        }
        else {
            if (isloaded) {
                isloaded = false;
                LoadedDatamodel = 0;
            }
        }
    }
}
