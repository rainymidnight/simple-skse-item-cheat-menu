#pragma once
#include "SKSEMenuFramework.h"
#include "ItemData.h"
#include <string>
#include <vector>

const std::vector<GameItem>& GetGameItems();

namespace UI {
    void Register();

    namespace Main {
        void __stdcall Render();
    }

    namespace ItemSpawner {
        void __stdcall Render();
    }
}
