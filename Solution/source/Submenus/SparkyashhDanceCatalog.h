// SPDX-License-Identifier: GPL-3.0-or-later
// SPARKYASHH dance catalogue integration for Menyoo.
#pragma once
#include <pugixml/src/pugixml.hpp>
#include <string>
#include <vector>
#include <set>
#include <cmath>
#include <utility>
namespace Sparkyashh {
struct Dance { std::string name, dict, clip; bool loop; double seconds; };
inline bool Load(const char* path, std::vector<Dance>& out, std::string& error) {
    pugi::xml_document doc;
    auto result = doc.load_file(path);
    auto fail = [&](const std::string& why) { error = why; return false; };
    if (!result) return fail(std::string("Cannot read catalogue: ") + result.description());
    auto root = doc.child("SparkyashhDances");
    if (!root || std::string(root.attribute("version").value()) != "1") return fail("Unsupported catalogue version");
    std::vector<Dance> parsed;
    std::set<std::pair<std::string,std::string>> keys;
    for (auto node : root.children()) {
        if (std::string(node.name()) != "Dance") return fail("Unexpected catalogue element");
        Dance d{node.attribute("name").value(),node.attribute("dict").value(),node.attribute("clip").value(),false,0};
        const std::string mode = node.attribute("loop").value();
        if (d.name.empty() || d.name.size()>128 || d.dict.empty() || d.dict.size()>128 || d.clip.empty() || d.clip.size()>128) return fail("Missing or oversized animation name");
        if (mode != "true" && mode != "false") return fail("Loop must be true or false");
        const auto allowed = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_@-";
        if (d.dict.find_first_not_of(allowed)!=std::string::npos || d.clip.find_first_not_of(allowed)!=std::string::npos) return fail("Invalid dictionary or clip name");
        std::string seconds = node.attribute("durationSeconds").value();
        try { size_t used=0; d.seconds=std::stod(seconds,&used); if(used!=seconds.size()) return fail("Invalid duration"); }
        catch (...) { return fail("Invalid duration"); }
        if (!std::isfinite(d.seconds) || d.seconds<=0 || d.seconds>3600) return fail("Invalid duration");
        d.loop=mode=="true";
        if (!keys.emplace(d.dict,d.clip).second) return fail("Duplicate animation");
        parsed.push_back(std::move(d));
        if (parsed.size()>256) return fail("Too many animations");
    }
    if (parsed.empty()) return fail("Catalogue is empty");
    out=std::move(parsed); error.clear(); return true;
}
}
