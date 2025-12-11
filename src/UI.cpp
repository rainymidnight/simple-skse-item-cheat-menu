#include "UI.h"

namespace logger = SKSE::log;

namespace UI {
    void Register() {

        if (!SKSEMenuFramework::IsInstalled()) {
            logger::info("SKSEMenuFramework not found");
            return;
        }

        SKSEMenuFramework::SetSection("Simple SKSE Item Cheat Menu");
        SKSEMenuFramework::AddSectionItem("Simple SKSE Item Cheat Menu", Main::Render);

        logger::info("UI menus registered successfully");
    }

    namespace Main {
        void __stdcall Render() {
            ItemSpawner::Render();
        }
    }

    namespace ItemSpawner {
        static char searchBuffer[256] = "";
        static int itemCount = 1;
        static ImGuiTextFilter filter;
        static std::vector<const GameItem*> filteredItems;
        static bool needsFilterUpdate = true;

        void __stdcall Render() {
            const auto& gameItems = GetGameItems();

            if (gameItems.empty()) {
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "No items found.");
                return;
            }

            if (needsFilterUpdate) {
                filteredItems.clear();
                for (const auto& gameItem : gameItems) {
                    if (filter.PassFilter(gameItem.name.c_str()) ||
                        filter.PassFilter(gameItem.plugin.c_str()) ||
                        filter.PassFilter(gameItem.editorID.c_str()) ||
                        filter.PassFilter(gameItem.typeName.c_str())) { // weapon, armor, ingredient, etc
                        filteredItems.push_back(&gameItem);
                    }
                }
                needsFilterUpdate = false;
            }

            // Display count with filtered count on same line if active
            if (filter.IsActive()) {
                ImGui::Text("Total Items: %zu (Filtered: %zu)", gameItems.size(), filteredItems.size());
            } else {
                ImGui::Text("Total Items: %zu", gameItems.size());
            }
            ImGui::Separator();

            if (filter.Draw("Search##ArmorFilter")) {
                needsFilterUpdate = true;
            }
            ImGui::Separator();

            ImGui::InputInt("Spawn Count", &itemCount, 1, 10);
            if (itemCount < 1) itemCount = 1;
            if (itemCount > 999) itemCount = 999;
            ImGui::Separator();

            if (ImGui::BeginChild("ArmorListRegion", ImVec2(0, 0), true)) {
                ImGuiListClipper* clipper = ImGui::ImGuiListClipperManager::Create(); // only render visible items
                ImGui::ImGuiListClipperManager::Begin(clipper, static_cast<int>(filteredItems.size()), -1.0f);

                while (ImGui::ImGuiListClipperManager::Step(clipper)) {
                    for (int i = clipper->DisplayStart; i < clipper->DisplayEnd; i++) {
                        const GameItem* gameItem = filteredItems[i];

                        ImGui::PushID(gameItem);

                        // button
                        std::string label = std::format("{} ({}) [{}]", gameItem->name, gameItem->plugin, gameItem->typeName);
                        if (ImGui::Selectable(label.c_str(), false)) {
                            auto player = RE::PlayerCharacter::GetSingleton();
                            if (player && gameItem->form) {
                                // container function, add item to player inventory container
                                player->AddObjectToContainer(gameItem->form, nullptr, itemCount, nullptr);
                                logger::info("Spawned {} x{} [FormID: {:08X}]", gameItem->name, itemCount, gameItem->formID);
                            }
                        }
                        ImGui::PopID();
                        ImGui::Separator();
                    }
                }
                ImGui::ImGuiListClipperManager::End(clipper);
                ImGui::ImGuiListClipperManager::Destroy(clipper);
            }
            ImGui::EndChild();
        }
    }
}
