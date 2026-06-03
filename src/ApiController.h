/**
 * ApiController.h - REST API Controller
 *
 * Maps HTTP endpoints to game actions
 * Returns JSON responses for frontend consumption
 */

#pragma once

#include <string>
#include <sstream>
#include <algorithm>
#include <cstdlib>

#include "HttpServer.h"
#include "GameState.h"
#include "Exceptions.h"

// ============================================================
// Helper: Parse JSON body (simple parser)
// ============================================================
namespace JsonParser {

inline std::string get_string(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return "";

    pos = json.find(':', pos);
    if (pos == std::string::npos) return "";

    pos = json.find('"', pos + 1);
    if (pos == std::string::npos) return "";

    size_t end = json.find('"', pos + 1);
    return json.substr(pos + 1, end - pos - 1);
}

inline int get_int(const std::string& json, const std::string& key, int default_val = 0) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return default_val;

    pos = json.find(':', pos);
    if (pos == std::string::npos) return default_val;

    // Skip colon and whitespace
    ++pos;
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) pos++;

    // Find end of number
    size_t end = pos;
    while (end < json.size() && (isdigit(json[end]) || json[end] == '-')) end++;

    if (end > pos) {
        return std::atoi(json.substr(pos, end - pos).c_str());
    }
    return default_val;
}

} // namespace JsonParser

// ============================================================
// API Controller
// ============================================================
class ApiController {
private:
    GameState& m_game;

    // Helper to create JSON error response
    static HttpResponse error(int code, const std::string& msg) {
        return HttpResponse::json(code, "Error",
            "{\"success\":false,\"error\":" + json::quote(msg) + "}");
    }

    static HttpResponse success(const std::string& data) {
        return HttpResponse::ok("{\"success\":true," + data + "}");
    }

    // Extract index from path like "/api/pets/0"
    static int extract_index(const std::string& path, const std::string& prefix) {
        std::string remainder = path.substr(prefix.size());
        // Remove trailing slashes
        while (!remainder.empty() && remainder.back() == '/') {
            remainder.pop_back();
        }
        if (remainder.empty()) return -1;
        return std::atoi(remainder.c_str()) - 1; // Convert to 0-based
    }

public:
    explicit ApiController(GameState& game) : m_game(game) {}

    // ============================================================
    // GET /api/state - Get complete game state
    // ============================================================
    HttpResponse get_state(const HttpRequest& req) {
        return HttpResponse::ok(m_game.to_json());
    }

    // ============================================================
    // GET /api/pets - Get all pets
    // ============================================================
    HttpResponse get_pets(const HttpRequest& req) {
        return HttpResponse::ok(m_game.pets_to_json());
    }

    // ============================================================
    // GET /api/pets/:id - Get single pet
    // ============================================================
    HttpResponse get_pet(const HttpRequest& req) {
        int index = extract_index(req.path, "/api/pets/");
        if (index < 0 || index >= m_game.get_pet_count()) {
            return error(404, "Pet not found");
        }
        auto pet = m_game.get_pet(index);
        return HttpResponse::ok(pet->to_json());
    }

    // ============================================================
    // POST /api/pets - Create new pet
    // Body: {"type":"Fire|Water|Mech","name":"PetName"}
    // ============================================================
    HttpResponse create_pet(const HttpRequest& req) {
        try {
            std::string type = JsonParser::get_string(req.body, "type");
            std::string name = JsonParser::get_string(req.body, "name");

            if (type.empty() || name.empty()) {
                return error(400, "Missing 'type' or 'name' field");
            }

            if (m_game.get_pet_count() >= m_game.get_max_pets()) {
                return error(400, "Maximum pets reached");
            }

            std::string result = m_game.create_pet(type, name);
            return HttpResponse::created("{\"pet\":" + result + "}");

        } catch (const std::exception& e) {
            return error(400, e.what());
        }
    }

    // ============================================================
    // DELETE /api/pets/:id - Delete pet
    // ============================================================
    HttpResponse delete_pet(const HttpRequest& req) {
        int index = -1;
        // Support both /api/pets/:id and /api/pet/:id paths
        if (req.path.find("/api/pets/") != std::string::npos) {
            index = extract_index(req.path, "/api/pets/");
        } else if (req.path.find("/api/pet/") != std::string::npos) {
            index = extract_index(req.path, "/api/pet/");
        }

        if (index < 0 || index >= m_game.get_pet_count()) {
            return error(404, "Pet not found");
        }

        if (m_game.delete_pet(index)) {
            return success("\"message\":\"Pet deleted\"");
        }
        return error(500, "Failed to delete pet");
    }

    // ============================================================
    // POST /api/pets/:id/train - Train pet
    // ============================================================
    HttpResponse train_pet(const HttpRequest& req) {
        int index = extract_index(req.path, "/api/pets/");
        if (index < 0 || index >= m_game.get_pet_count()) {
            return error(404, "Pet not found");
        }

        try {
            std::string result = m_game.train_pet(index);
            return success("\"pet\":" + result);
        } catch (const std::exception& e) {
            return error(400, e.what());
        }
    }

    // ============================================================
    // POST /api/pets/:id/heal - Heal pet with item
    // Body: {"itemIndex":0}
    // ============================================================
    HttpResponse heal_pet(const HttpRequest& req) {
        int petIndex = extract_index(req.path, "/api/pets/");
        if (petIndex < 0 || petIndex >= m_game.get_pet_count()) {
            return error(404, "Pet not found");
        }

        try {
            int itemIndex = JsonParser::get_int(req.body, "itemIndex", -1);
            if (itemIndex < 0) {
                return error(400, "Missing 'itemIndex' field");
            }

            std::string result = m_game.heal_pet(petIndex, itemIndex);
            return success("\"pet\":" + result);
        } catch (const std::exception& e) {
            return error(400, e.what());
        }
    }

    // ============================================================
    // POST /api/pets/:id/enhance - Use enhancement
    // Body: {"itemIndex":0}
    // ============================================================
    HttpResponse enhance_pet(const HttpRequest& req) {
        int petIndex = extract_index(req.path, "/api/pets/");
        if (petIndex < 0 || petIndex >= m_game.get_pet_count()) {
            return error(404, "Pet not found");
        }

        try {
            int itemIndex = JsonParser::get_int(req.body, "itemIndex", -1);
            if (itemIndex < 0) {
                return error(400, "Missing 'itemIndex' field");
            }

            std::string result = m_game.use_enhancement(petIndex, itemIndex);
            return success("\"pet\":" + result);
        } catch (const std::exception& e) {
            return error(400, e.what());
        }
    }

    // ============================================================
    // GET /api/backpack - Get backpack contents
    // ============================================================
    HttpResponse get_backpack(const HttpRequest& req) {
        return success("\"items\":" + m_game.get_backpack().to_json());
    }

    // ============================================================
    // POST /api/backpack - Add item to backpack
    // Body: {"name":"ItemName","type":"Potion|Enhancement","value":30}
    // ============================================================
    HttpResponse add_item(const HttpRequest& req) {
        try {
            std::string name = JsonParser::get_string(req.body, "name");
            std::string type = JsonParser::get_string(req.body, "type");
            int value = JsonParser::get_int(req.body, "value", 0);

            if (name.empty() || type.empty() || value <= 0) {
                return error(400, "Missing or invalid fields");
            }

            m_game.add_item(name, type, value);
            return success("\"message\":\"Item added to backpack\"");

        } catch (const std::exception& e) {
            return error(400, e.what());
        }
    }

    // ============================================================
    // DELETE /api/backpack/:id - Remove item from backpack
    // ============================================================
    HttpResponse remove_item(const HttpRequest& req) {
        int index = extract_index(req.path, "/api/backpack/");
        if (index < 0) {
            return error(400, "Invalid item index");
        }

        if (m_game.remove_item(index)) {
            return success("\"message\":\"Item removed\"");
        }
        return error(404, "Item not found");
    }

    // ============================================================
    // POST /api/backpack/sort - Sort backpack by value
    // ============================================================
    HttpResponse sort_backpack(const HttpRequest& req) {
        m_game.sort_backpack();
        return success("\"message\":\"Backpack sorted\"");
    }

    // ============================================================
    // POST /api/battle - Start battle
    // Body: {"attacker":0,"defender":1}
    // ============================================================
    HttpResponse start_battle(const HttpRequest& req) {
        try {
            int attacker = JsonParser::get_int(req.body, "attacker", -1);
            int defender = JsonParser::get_int(req.body, "defender", -1);

            if (attacker < 0 || defender < 0) {
                return error(400, "Missing attacker or defender");
            }

            std::string result = m_game.start_battle(attacker, defender);
            return HttpResponse::ok("{\"success\":true,\"battle\":" + result + "}");

        } catch (const std::exception& e) {
            return error(400, e.what());
        }
    }

    // ============================================================
    // POST /api/compare - Compare two pets
    // Body: {"petA":0,"petB":1}
    // ============================================================
    HttpResponse compare_pets(const HttpRequest& req) {
        try {
            int petA = JsonParser::get_int(req.body, "petA", -1);
            int petB = JsonParser::get_int(req.body, "petB", -1);

            if (petA < 0 || petB < 0) {
                return error(400, "Missing petA or petB");
            }

            std::string result = m_game.compare_pets(petA, petB);
            return HttpResponse::ok("{\"success\":true,\"comparison\":" + result + "}");

        } catch (const std::exception& e) {
            return error(400, e.what());
        }
    }

    // ============================================================
    // POST /api/monsters/random - Get random monster
    // Query: ?difficulty=N (1-10, default varies)
    // ============================================================
    HttpResponse get_random_monster(const HttpRequest& req) {
        // Parse difficulty from query string
        int difficulty = 1;
        if (!req.query.empty()) {
            std::string search = "difficulty=";
            size_t pos = req.query.find(search);
            if (pos != std::string::npos) {
                pos += search.size();
                size_t end = pos;
                while (end < req.query.size() && req.query[end] != '&') end++;
                difficulty = std::atoi(req.query.substr(pos, end - pos).c_str());
            }
        }
        int randDiff = difficulty + (rand() % 5) - 2;  // difficulty-2 to difficulty+2
        if (randDiff < 1) randDiff = 1;
        if (randDiff > 10) randDiff = 10;

        static const char* types[] = {"fire","water","grass","electric","rock","wind","ice","mech"};
        static const char* names[] = {"火焰","水波","草叶","雷电","岩石","旋风","冰霜","机械"};
        int ti = rand() % 8;

        int hp = 50 + randDiff * 10;
        int atk = 10 + randDiff * 2;

        std::ostringstream oss;
        oss << "{";
        oss << "\"id\":" << (rand() % 1000) << ",";
        oss << "\"name\":\"野生" << names[ti] << "兽\",";
        oss << "\"type\":\"" << types[ti] << "\",";
        oss << "\"level\":" << randDiff << ",";
        oss << "\"difficulty\":" << randDiff << ",";
        oss << "\"hp\":" << hp << ",";
        oss << "\"maxHp\":" << hp << ",";
        oss << "\"attack\":" << atk;
        oss << "}";
        return HttpResponse::ok("{\"success\":true,\"monster\":" + oss.str() + "}");
    }

    // ============================================================
    // POST /api/monsters/battle - Battle with monster
    // Body: {"petIndex":0,"monsterType":"fire","monsterLevel":3,"monsterName":"Wild"}
    // ============================================================
    HttpResponse start_monster_battle(const HttpRequest& req) {
        try {
            int petIndex = JsonParser::get_int(req.body, "petIndex", -1);
            std::string monsterName = JsonParser::get_string(req.body, "monsterName");
            std::string monsterType = JsonParser::get_string(req.body, "monsterType");
            int monsterLevel = JsonParser::get_int(req.body, "monsterLevel", 1);

            if (petIndex < 0 || petIndex >= m_game.get_pet_count()) {
                return error(404, "Pet not found");
            }
            if (monsterName.empty()) monsterName = "野生怪物";
            if (monsterType.empty()) monsterType = "fire";
            if (monsterLevel < 1) monsterLevel = 1;

            auto pet = m_game.get_pet(petIndex);
            auto monster = std::make_shared<Monster>(monsterName, monsterType, monsterLevel);
            BattleResult result = m_game.get_battle_system().start_monster_battle_api(pet, monster);

            std::ostringstream oss;
            oss << "{";
            oss << "\"success\":true,";
            oss << "\"battle\":{";
            if (result.winner) {
                oss << "\"winner\":\"" << json::escape_string(result.winner->get_name()) << "\",";
            }
            oss << "\"log\":[";
            for (size_t i = 0; i < result.log.size(); i++) {
                if (i > 0) oss << ",";
                oss << "\"" << json::escape_string(result.log[i]) << "\"";
            }
            oss << "]";
            oss << "},";
            if (pet->is_alive()) {
                int expG = 50 + monsterLevel * 10;
                int coinsG = 100 + monsterLevel * 20;
                int diamondsG = monsterLevel * 5;
                oss << "\"reward\":{\"exp\":" << expG << ",\"coins\":" << coinsG << ",\"diamonds\":" << diamondsG << "},";
            }
            oss << "\"pet\":" << pet->to_json();
            oss << ",\"monster\":" << monster->to_json();
            oss << "}";
            return HttpResponse::ok(oss.str());

        } catch (const std::exception& e) {
            return error(400, e.what());
        }
    }

    // ============================================================
    // POST /api/save - Save game
    // ============================================================
    HttpResponse save_game(const HttpRequest& req) {
        if (m_game.save()) {
            return success("\"message\":\"Game saved\"");
        }
        return error(500, "Failed to save game");
    }

    // ============================================================
    // Register all routes
    // ============================================================
    void register_routes(HttpServer& server) {
        // State
        server.get("/api/state", [this](const HttpRequest& req) { return get_state(req); });

        // Pets (both plural and singular forms for compatibility)
        server.get("/api/pets", [this](const HttpRequest& req) { return get_pets(req); });
        server.get("/api/pets/:id", [this](const HttpRequest& req) { return get_pet(req); });
        server.get("/api/pet/:id", [this](const HttpRequest& req) { return get_pet(req); });
        server.post("/api/pets", [this](const HttpRequest& req) { return create_pet(req); });
        server.post("/api/pet", [this](const HttpRequest& req) { return create_pet(req); });
        server.post("/api/pets/:id/train", [this](const HttpRequest& req) { return train_pet(req); });
        server.post("/api/pet/:id/train", [this](const HttpRequest& req) { return train_pet(req); });
        server.post("/api/pets/:id/heal", [this](const HttpRequest& req) { return heal_pet(req); });
        server.post("/api/pet/:id/heal", [this](const HttpRequest& req) { return heal_pet(req); });
        server.post("/api/pets/:id/enhance", [this](const HttpRequest& req) { return enhance_pet(req); });
        server.post("/api/pet/:id/enhance", [this](const HttpRequest& req) { return enhance_pet(req); });
        server.del("/api/pets/:id", [this](const HttpRequest& req) { return delete_pet(req); });
        server.del("/api/pet/:id", [this](const HttpRequest& req) { return delete_pet(req); });

        // Backpack
        server.get("/api/backpack", [this](const HttpRequest& req) { return get_backpack(req); });
        server.post("/api/backpack", [this](const HttpRequest& req) { return add_item(req); });
        server.post("/api/backpack/sort", [this](const HttpRequest& req) { return sort_backpack(req); });
        server.del("/api/backpack/:id", [this](const HttpRequest& req) { return remove_item(req); });

        // Battle
        server.post("/api/battle", [this](const HttpRequest& req) { return start_battle(req); });

        // Compare
        server.post("/api/compare", [this](const HttpRequest& req) { return compare_pets(req); });

        // Monsters
        server.get("/api/monsters/random", [this](const HttpRequest& req) { return get_random_monster(req); });
        server.post("/api/monsters/battle", [this](const HttpRequest& req) { return start_monster_battle(req); });

        // Save
        server.post("/api/save", [this](const HttpRequest& req) { return save_game(req); });
    }
};
