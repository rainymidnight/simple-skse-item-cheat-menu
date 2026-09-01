#include "UI.h"

#include <memory>

namespace logger = SKSE::log;

namespace UI {
    void Register() {

        if (!SKSEMenuFramework::IsInstalled()) {
            logger::info("SKSEMenuFramework not found");
            return;
        }

        ItemSpawner::Initialize();
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
        static std::unique_ptr<ImGuiMCP::ImGuiTextFilter> filter;
        static std::vector<const GameItem*> filteredItems;
        static bool needsFilterUpdate = true;
        static bool excludeEnchanted = false;

        void Initialize() {
            filter = std::make_unique<ImGuiMCP::ImGuiTextFilter>();
        }

        // Helper function to check if an item has an enchantment
        bool IsItemEnchanted(const GameItem* item) {
            if (!item || !item->form) return false;

            auto* enchantableForm = item->form->As<RE::TESEnchantableForm>();
            if (enchantableForm) {
                return enchantableForm->formEnchanting != nullptr ||
                       enchantableForm->amountofEnchantment > 0;
            }

            return false;
        }

        void __stdcall Render() {
            if (!filter) {
                ImGuiMCP::TextColored(ImGuiMCP::ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Menu integration is not initialized.");
                return;
            }

            const auto& gameItems = GetGameItems();

            if (gameItems.empty()) {
                ImGuiMCP::TextColored(ImGuiMCP::ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "No items found.");
                return;
            }

            if (needsFilterUpdate) {
                filteredItems.clear();
                for (const auto& gameItem : gameItems) {
                    bool passesTextFilter = filter->PassFilter(gameItem.name.c_str()) ||
                                           filter->PassFilter(gameItem.plugin.c_str()) ||
                                           filter->PassFilter(gameItem.editorID.c_str()) ||
                                           filter->PassFilter(gameItem.typeName.c_str()); // weapon, armor, ingredient, etc

                    bool passesEnchantmentFilter = !excludeEnchanted || !IsItemEnchanted(&gameItem);

                    if (passesTextFilter && passesEnchantmentFilter) {
                        filteredItems.push_back(&gameItem);
                    }
                }
                needsFilterUpdate = false;
            }

            if (filter->IsActive() || excludeEnchanted) {
                ImGuiMCP::Text("Total Items: %zu (Filtered: %zu)", gameItems.size(), filteredItems.size());
            } else {
                ImGuiMCP::Text("Total Items: %zu", gameItems.size());
            }
            ImGuiMCP::Separator();

            ImGuiMCP::SetNextItemWidth(ImGuiMCP::GetWindowWidth() * 0.7f);

            if (filter->Draw("Search##ArmorFilter")) {
                needsFilterUpdate = true;
            }
            ImGuiMCP::Separator();

            if (ImGuiMCP::Checkbox("Exclude Enchanted", &excludeEnchanted)) {
                needsFilterUpdate = true;
            }

            ImGuiMCP::SameLine();

            float windowWidth = ImGuiMCP::GetWindowWidth();
            float targetWidth = windowWidth * 0.70f; // 70% of window width
            float cursorX = ImGuiMCP::GetCursorPosX() - ImGuiMCP::GetStyle()->ItemSpacing.x; // 8.0f
            float inputWidth = targetWidth - cursorX;
            ImGuiMCP::SetNextItemWidth(inputWidth);

            ImGuiMCP::InputInt("Spawn Count", &itemCount, 1, 10);
            if (itemCount < 1) itemCount = 1;
            if (itemCount > 999) itemCount = 999;
            ImGuiMCP::Separator();

            if (ImGuiMCP::BeginChild("ArmorListRegion", ImGuiMCP::ImVec2(0, 0), true)) {
                ImGuiMCP::ImGuiListClipper* clipper = ImGuiMCP::ImGuiListClipperManager::Create(); // only render visible items
                ImGuiMCP::ImGuiListClipperManager::Begin(clipper, static_cast<int>(filteredItems.size()), -1.0f);

                while (ImGuiMCP::ImGuiListClipperManager::Step(clipper)) {
                    for (int i = clipper->DisplayStart; i < clipper->DisplayEnd; i++) {
                        const GameItem* gameItem = filteredItems[i];

                        ImGuiMCP::PushID(gameItem);

                        // button
                        std::string label = std::format("{} ({}) [{}]", gameItem->name, gameItem->plugin, gameItem->typeName);
                        if (ImGuiMCP::Selectable(label.c_str(), false)) {
                            auto player = RE::PlayerCharacter::GetSingleton();
                            if (player && gameItem->form) {
                                // container function, add item to player inventory container
                                player->AddObjectToContainer(gameItem->form, nullptr, itemCount, nullptr);
                                logger::info("Spawned {} x{} [FormID: {:08X}]", gameItem->name, itemCount, gameItem->formID);
                            }
                        }
                        ImGuiMCP::PopID();
                        ImGuiMCP::Separator();
                    }
                }
                ImGuiMCP::ImGuiListClipperManager::End(clipper);
                ImGuiMCP::ImGuiListClipperManager::Destroy(clipper);
            }
            ImGuiMCP::EndChild();
        }
    }
}
