#include <spdlog/sinks/basic_file_sink.h>
#include <windows.h>
#include "UI.h"
#include "ItemData.h"

namespace logger = SKSE::log;

std::vector<GameItem> g_gameItems;

// powerofthree tweaks
std::string GetFormEditorIDViaPo3(std::uint32_t formID) {
    auto tweaks = GetModuleHandle(L"po3_Tweaks");
    if (!tweaks) {
        return "";
    }

    using _GetFormEditorID = const char* (*)(std::uint32_t);
    auto func = reinterpret_cast<_GetFormEditorID>(GetProcAddress(tweaks, "GetFormEditorID"));
    if (!func) {
        return "";
    }

    const char* editorIDPtr = func(formID);
    if (!editorIDPtr || strlen(editorIDPtr) == 0) {
        return "";
    }

    return std::string(editorIDPtr);
}

// Accessor function for UI
const std::vector<GameItem>& GetGameItems() {
    return g_gameItems;
}

std::string GetFormTypeName(RE::FormType formType) {
    switch (formType) {
        case RE::FormType::Armor: return "Armor";
        case RE::FormType::Weapon: return "Weapon";
        case RE::FormType::Ammo: return "Ammo";
        case RE::FormType::Book: return "Book";
        case RE::FormType::Ingredient: return "Ingredient";
        case RE::FormType::Light: return "Light";
        case RE::FormType::Misc: return "Misc";
        case RE::FormType::AlchemyItem: return "Potion";
        case RE::FormType::Scroll: return "Scroll";
        case RE::FormType::KeyMaster: return "Key";
        case RE::FormType::SoulGem: return "Soul Gem";
        default: return "Item";
    }
}

template<typename T>
void ScanItemType(RE::TESDataHandler* dataHandler, const std::string& typeName) {
    auto& items = dataHandler->GetFormArray<T>();
    logger::info("Scanning {} {}...", items.size(), typeName);

    for (auto* item : items) {
        if (!item) continue;

        // Get item name
        std::string itemName = item->GetName();
        if (itemName.empty()) {
            itemName = "[No Name]";
        }

        // get source plugin
        std::string primaryPlugin = "Unknown";
        if (item->sourceFiles.array && item->sourceFiles.array->size() > 0) {
            RE::TESFile** sourceFiles = item->sourceFiles.array->data();
            if (sourceFiles[0] && sourceFiles[0]->fileName) {
                primaryPlugin = sourceFiles[0]->fileName;
            }
        }

        // Get EditorID
        std::string editorID = GetFormEditorIDViaPo3(item->GetFormID());
        if (editorID.empty()) {
            editorID = "[Unknown]";
        }

        g_gameItems.emplace_back(item, itemName, primaryPlugin,
                                 editorID, item->GetFormID(), typeName);
    }
}

// Scan for all items in the game
void ScanForItems() {
    logger::info("Starting item scan...");

    auto* dataHandler = RE::TESDataHandler::GetSingleton();
    if (!dataHandler) {
        logger::error("Failed to get TESDataHandler");
        return;
    }

    g_gameItems.clear();

    ScanItemType<RE::TESObjectARMO>(dataHandler, "Armor");
    ScanItemType<RE::TESObjectWEAP>(dataHandler, "Weapon");
    ScanItemType<RE::TESAmmo>(dataHandler, "Ammo");
    ScanItemType<RE::TESObjectBOOK>(dataHandler, "Book");
    ScanItemType<RE::IngredientItem>(dataHandler, "Ingredient");
    ScanItemType<RE::TESObjectMISC>(dataHandler, "Misc");
    ScanItemType<RE::AlchemyItem>(dataHandler, "Potion");
    ScanItemType<RE::ScrollItem>(dataHandler, "Scroll");
    ScanItemType<RE::TESKey>(dataHandler, "Key");
    ScanItemType<RE::TESSoulGem>(dataHandler, "Soul Gem");

    logger::info("Item scan complete: {} total items loaded", g_gameItems.size());
}

namespace {
    void InitializeLog() {
        auto path = logger::log_directory();

        *path /= "SimpleSKSEItemCheatMenu.log"sv;
        auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);

        auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

        log->set_level(spdlog::level::info);
        log->flush_on(spdlog::level::info);

        spdlog::set_default_logger(std::move(log));
        spdlog::set_pattern("[%H:%M:%S] [%^%l%$] %v"s);

        logger::info("Simple SKSE Item Cheat Menu log initialized");
    }

    void MessageHandler(SKSE::MessagingInterface::Message* a_msg) {
        if (a_msg->type == SKSE::MessagingInterface::kDataLoaded) {
            logger::info("Data loaded event received");
            ScanForItems();
            UI::Register();
        }
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* skse) {
    InitializeLog();
    logger::info("Simple SKSE Item Cheat Menu v1.1 loading...");

    SKSE::Init(skse);

    logger::info("Game version: {}", REL::Module::get().version().string());

    auto messaging = SKSE::GetMessagingInterface();
    if (!messaging) {
        logger::critical("Failed to get messaging interface!");
        return false;
    }

    messaging->RegisterListener(MessageHandler);

    logger::info("Simple SKSE Item Cheat Menu loaded successfully");
    return true;
}
