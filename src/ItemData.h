#pragma once
#include <vector>
#include <string>
#include <cstdint>

namespace RE {
    class TESBoundObject;
    class TESForm;
}

struct GameItem {
    RE::TESBoundObject* form;
    std::string name;
    std::string plugin;
    std::string editorID;
    uint32_t formID;
    std::string typeName;

    GameItem(RE::TESBoundObject* f, const std::string& n, const std::string& p,
             const std::string& eid, uint32_t fid, const std::string& tn)
        : form(f), name(n), plugin(p), editorID(eid), formID(fid), typeName(tn) {}
};
