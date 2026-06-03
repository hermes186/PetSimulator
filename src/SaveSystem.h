#pragma once
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <memory>
#include "Pet.h"
#include "FirePet.h"
#include "WaterPet.h"
#include "MechPet.h"
#include "Item.h"
#include "Backpack.h"

// ============================================================
// SaveSystem: Save/Load System
// Uses fstream to save pet data and backpack to file
// ============================================================
class SaveSystem {
private:
    std::string m_filename;

public:
    SaveSystem(const std::string& file = "save_data.txt") : m_filename(file) {}

    bool save_exists() const {
        std::ifstream file(m_filename);
        return file.is_open();
    }

    void save(const std::vector<std::shared_ptr<Pet>>& pets,
              const Backpack<Item>& backpack) {
        std::ofstream file(m_filename);
        if (!file.is_open()) {
            // std::cerr << "Cannot open save file!\n";
            return;
        }

        file << "PETS\n";
        for (const auto& pet : pets) {
            file << pet->get_type() << "|"
                 << pet->get_name() << "|"
                 << pet->get_hp() << "|"
                 << pet->get_max_hp() << "|"
                 << pet->get_attack() << "|"
                 << pet->get_level() << "|"
                 << pet->get_exp() << "\n";
        }

        file << "ITEMS\n";
        for (const auto& item : backpack.get_all()) {
            file << item.get_name() << "|"
                 << item.get_type() << "|"
                 << item.get_value() << "\n";
        }

        file << "END\n";
        file.close();
        // std::cout << "Save successful! Data saved to " << m_filename << "\n";
    }

    void load_pets(std::vector<std::shared_ptr<Pet>>& pets) {
        std::ifstream file(m_filename);
        if (!file.is_open()) {
            return;  // Silently return, file existence already checked by caller
        }

        std::string line;
        bool reading_pets = false;

        while (std::getline(file, line)) {
            if (line == "PETS") { reading_pets = true; continue; }
            if (line == "ITEMS" || line == "END") { reading_pets = false; continue; }

            if (reading_pets && !line.empty()) {
                std::stringstream ss(line);
                std::string type, name, hp_s, maxHp_s, atk_s, lv_s, exp_s;
                std::getline(ss, type, '|');
                std::getline(ss, name, '|');
                std::getline(ss, hp_s, '|');
                std::getline(ss, maxHp_s, '|');
                std::getline(ss, atk_s, '|');
                std::getline(ss, lv_s, '|');
                std::getline(ss, exp_s, '|');

                int lv = std::stoi(lv_s);
                std::shared_ptr<Pet> pet;

                if (type == "Fire") {
                    pet = std::make_shared<FirePet>(name, lv);
                } else if (type == "Water") {
                    pet = std::make_shared<WaterPet>(name, lv);
                } else {
                    pet = std::make_shared<MechPet>(name, lv);
                }

                pets.push_back(pet);
            }
        }
        file.close();
        // std::cout << "Load successful! Loaded " << pets.size() << " pets.\n";
    }

    void load_items(Backpack<Item>& backpack) {
        std::ifstream file(m_filename);
        if (!file.is_open()) return;

        std::string line;
        bool reading_items = false;

        while (std::getline(file, line)) {
            if (line == "ITEMS") { reading_items = true; continue; }
            if (line == "END") break;
            if (line == "PETS") { reading_items = false; continue; }

            if (reading_items && !line.empty()) {
                std::stringstream ss(line);
                std::string name, type, val_s;
                std::getline(ss, name, '|');
                std::getline(ss, type, '|');
                std::getline(ss, val_s, '|');
                backpack.add(Item(name, type, std::stoi(val_s)));
            }
        }
        file.close();
    }
};
