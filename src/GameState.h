/**
 * GameState.h - Game State Manager
 *
 * Central state management for the Pet Simulator
 * Provides JSON serialization for REST API responses
 */

#pragma once

#include "HttpServer.h"

#include "Pet.h"
#include "FirePet.h"
#include "WaterPet.h"
#include "MechPet.h"
#include "Item.h"
#include "Backpack.h"
#include "BattleSystem.h"
#include "SaveSystem.h"
#include "Exceptions.h"

// ============================================================
// GameState - Singleton Game Manager
// ============================================================
class GameState {
private:
    std::vector<std::shared_ptr<Pet>> m_pets;
    Backpack<Item> m_backpack;
    SaveSystem m_saveSystem;
    BattleSystem m_battleSys;
    std::string m_saveFile;

    // Singleton instance
    static GameState* s_instance;

public:
    static GameState& get_instance() {
        if (!s_instance) {
            s_instance = new GameState("save_data.json");
        }
        return *s_instance;
    }

    static void destroy() {
        delete s_instance;
        s_instance = nullptr;
    }

    explicit GameState(const std::string& saveFile)
        : m_backpack(20), m_saveSystem(saveFile), m_saveFile(saveFile) {
        load_or_init();
    }

    // ============================================================
    // Pet Management
    // ============================================================
    const std::vector<std::shared_ptr<Pet>>& get_pets() const { return m_pets; }
    Backpack<Item>& get_backpack() { return m_backpack; }

    int get_pet_count() const { return (int)m_pets.size(); }
    int get_max_pets() const { return 20; }

    std::shared_ptr<Pet> get_pet(int index) {
        if (index < 0 || index >= (int)m_pets.size()) {
            return nullptr;
        }
        return m_pets[index];
    }

    std::string create_pet(const std::string& type, const std::string& name) {
        if ((int)m_pets.size() >= get_max_pets()) {
            throw std::runtime_error("Pet list full (max " + std::to_string(get_max_pets()) + ")");
        }

        std::shared_ptr<Pet> pet;
        if (type == "Fire") {
            pet = std::make_shared<FirePet>(name);
        } else if (type == "Water") {
            pet = std::make_shared<WaterPet>(name);
        } else if (type == "Mech") {
            pet = std::make_shared<MechPet>(name);
        } else {
            throw std::invalid_argument("Invalid pet type: " + type);
        }

        m_pets.push_back(pet);
        auto_save();
        return pet->to_json();
    }

    bool delete_pet(int index) {
        if (index < 0 || index >= (int)m_pets.size()) {
            return false;
        }
        m_pets.erase(m_pets.begin() + index);
        auto_save();
        return true;
    }

    std::string train_pet(int index) {
        if (index < 0 || index >= (int)m_pets.size()) {
            throw std::out_of_range("Invalid pet index");
        }
        m_pets[index]->train();
        auto_save();
        return m_pets[index]->to_json();
    }

    std::string heal_pet(int petIndex, int itemIndex) {
        if (petIndex < 0 || petIndex >= (int)m_pets.size()) {
            throw std::out_of_range("Invalid pet index");
        }
        if (itemIndex < 0 || itemIndex >= m_backpack.size()) {
            throw std::out_of_range("Invalid item index");
        }

        Item& item = m_backpack.get(itemIndex);
        if (item.get_type() == "Potion") {
            m_pets[petIndex]->heal(item.get_value());
            m_backpack.remove(itemIndex);
        } else {
            throw std::invalid_argument("Item is not a potion");
        }

        auto_save();
        return m_pets[petIndex]->to_json();
    }

    std::string use_enhancement(int petIndex, int itemIndex) {
        if (petIndex < 0 || petIndex >= (int)m_pets.size()) {
            throw std::out_of_range("Invalid pet index");
        }
        if (itemIndex < 0 || itemIndex >= m_backpack.size()) {
            throw std::out_of_range("Invalid item index");
        }

        Item& item = m_backpack.get(itemIndex);
        if (item.get_type() == "Enhancement") {
            m_pets[petIndex]->train();
            m_backpack.remove(itemIndex);
        } else {
            throw std::invalid_argument("Item is not an enhancement");
        }

        auto_save();
        return m_pets[petIndex]->to_json();
    }

    // ============================================================
    // Backpack Management
    // ============================================================
    std::string add_item(const std::string& name, const std::string& type, int value) {
        if (m_backpack.size() >= 20) {
            throw std::runtime_error("Backpack is full");
        }
        m_backpack.add(Item(name, type, value));
        auto_save();
        return "{\"success\":true,\"message\":\"Item added\"}";
    }

    bool remove_item(int index) {
        if (index < 0 || index >= m_backpack.size()) {
            return false;
        }
        m_backpack.remove(index);
        auto_save();
        return true;
    }

    void sort_backpack() {
        m_backpack.sort_by_value();
        auto_save();
    }

    // ============================================================
    // Battle System
    // ============================================================
    BattleSystem& get_battle_system() { return m_battleSys; }

    std::string start_battle(int attackerIndex, int defenderIndex) {
        if (attackerIndex < 0 || attackerIndex >= (int)m_pets.size() ||
            defenderIndex < 0 || defenderIndex >= (int)m_pets.size()) {
            throw std::out_of_range("Invalid pet index");
        }
        if (attackerIndex == defenderIndex) {
            throw std::invalid_argument("Cannot battle yourself");
        }

        auto result = m_battleSys.start_battle_api(m_pets[attackerIndex], m_pets[defenderIndex]);
        auto_save();

        // Return battle result as JSON
        std::ostringstream oss;
        oss << "{";
        if (result.winner) oss << "\"winner\":\"" << result.winner->get_name() << "\",";
        if (result.loser) oss << "\"loser\":\"" << result.loser->get_name() << "\",";
        oss << "\"log\":[";
        for (size_t i = 0; i < result.log.size(); i++) {
            if (i > 0) oss << ",";
            oss << "\"" << json::escape_string(result.log[i]) << "\"";
        }
        oss << "],";
        if (result.attacker) oss << "\"attacker\":" << result.attacker->to_json() << ",";
        if (result.defender) oss << "\"defender\":" << result.defender->to_json();
        oss << "}";
        return oss.str();
    }

    std::string compare_pets(int indexA, int indexB) {
        if (indexA < 0 || indexA >= (int)m_pets.size() ||
            indexB < 0 || indexB >= (int)m_pets.size()) {
            throw std::out_of_range("Invalid pet index");
        }

        auto& petA = m_pets[indexA];
        auto& petB = m_pets[indexB];
        int powerA = petA->get_attack() * petA->get_level();
        int powerB = petB->get_attack() * petB->get_level();

        std::string winner;
        if (powerA > powerB) winner = petA->get_name();
        else if (powerB > powerA) winner = petB->get_name();
        else winner = "TIE";

        std::ostringstream oss;
        oss << "{";
        oss << "\"petA\":" << petA->to_json() << ",";
        oss << "\"petB\":" << petB->to_json() << ",";
        oss << "\"powerA\":" << powerA << ",";
        oss << "\"powerB\":" << powerB << ",";
        oss << "\"winner\":\"" << winner << "\"";
        oss << "}";
        return oss.str();
    }

    // ============================================================
    // JSON Serialization
    // ============================================================
    std::string to_json() const {
        std::ostringstream oss;
        oss << "{";
        oss << "\"pets\":[";
        for (int i = 0; i < (int)m_pets.size(); i++) {
            if (i > 0) oss << ",";
            oss << m_pets[i]->to_json();
        }
        oss << "],";
        oss << "\"backpack\":" << m_backpack.to_json() << ",";
        oss << "\"maxPets\":" << 5 << ",";
        oss << "\"petCount\":" << m_pets.size();
        oss << "}";
        return oss.str();
    }

    std::string pets_to_json() const {
        std::ostringstream oss;
        oss << "[";
        for (int i = 0; i < (int)m_pets.size(); i++) {
            if (i > 0) oss << ",";
            oss << m_pets[i]->to_json();
        }
        oss << "]";
        return oss.str();
    }

    // ============================================================
    // Persistence
    // ============================================================
    void auto_save() {
        m_saveSystem.save(m_pets, m_backpack);
    }

    bool save() {
        m_saveSystem.save(m_pets, m_backpack);
        return true;
    }

    bool load() {
        return load_or_init();
    }

private:
    bool load_or_init() {
        if (m_saveSystem.save_exists()) {
            m_saveSystem.load_pets(m_pets);
            m_saveSystem.load_items(m_backpack);
            return true;
        } else {
            // Add default items for new game
            m_backpack.add(Item("Small Potion", "Potion", 30));
            m_backpack.add(Item("Medium Potion", "Potion", 60));
            m_backpack.add(Item("Small Enhance Stone", "Enhancement", 5));
            return false;
        }
    }
};

// Initialize static member
GameState* GameState::s_instance = nullptr;
