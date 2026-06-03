#pragma once
#include <memory>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <cctype>
#include "Pet.h"
#include "BattleResultCalculator.h"
#include "Exceptions.h"

// ============================================================
// Battle Result Structure
// ============================================================
struct BattleResult {
    std::shared_ptr<Pet> winner;
    std::shared_ptr<Pet> loser;
    std::vector<std::string> log;
    std::shared_ptr<Pet> attacker;
    std::shared_ptr<Pet> defender;
};

// ============================================================
// BattleSystem: Battle System
// Supports both Console Mode and API Mode
// ============================================================
class BattleSystem {
public:
    // Console mode: interactive battle
    void start_battle(std::shared_ptr<Pet> pet1, std::shared_ptr<Pet> pet2) {
        if (!pet1->is_alive() || !pet2->is_alive()) {
            throw NotEnoughHpException("Insufficient HP, cannot start battle!");
        }

        std::cout << "\n========================================\n";
        std::cout << "       BATTLE START!\n";
        std::cout << *pet1 << "\n";
        std::cout << "       VS\n";
        std::cout << *pet2 << "\n";
        std::cout << "========================================\n\n";

        int round = 1;

        while (pet1->is_alive() && pet2->is_alive()) {
            std::cout << "---------- Round " << round << " ----------\n";

            int choice = 0;
            std::cout << "Choose action:\n";
            std::cout << "1. Normal Attack\n";
            std::cout << "2. Special Skill\n";
            std::cout << "Enter (1/2): ";
            std::cin >> choice;

            try {
                if (choice == 1) {
                    int damage = pet1->attack();
                    pet2->take_damage(damage);
                } else if (choice == 2) {
                    pet1->special_skill();
                    if (pet1->get_type() == "Fire") {
                        pet2->take_damage(8);
                    }
                } else {
                    throw InvalidMoveException("Please enter 1 or 2!");
                }
            } catch (const InvalidMoveException& e) {
                std::cout << "WARNING: " << e.what() << "\n";
                continue;
            } catch (const NotEnoughHpException& e) {
                std::cout << "WARNING: " << e.what() << "\n";
                break;
            }

            std::cout << pet2->get_name() << " HP: "
                      << pet2->get_hp() << "/" << pet2->get_max_hp() << "\n";

            if (!pet2->is_alive()) break;

            std::cout << "\n[Enemy Turn]\n";
            try {
                int enemy_dmg = pet2->attack();
                pet1->take_damage(enemy_dmg);
            } catch (const NotEnoughHpException& e) {
                std::cout << "Enemy: " << e.what() << "\n";
            }

            std::cout << pet1->get_name() << " HP: "
                      << pet1->get_hp() << "/" << pet1->get_max_hp() << "\n\n";

            round++;

            if (round > 50) {
                std::cout << "Too many rounds, draw!\n";
                return;
            }
        }

        print_result(pet1, pet2);
    }

    // API mode: automated battle for REST API
    BattleResult start_battle_api(std::shared_ptr<Pet> pet1, std::shared_ptr<Pet> pet2) {
        BattleResult result;
        result.attacker = pet1;
        result.defender = pet2;
        result.winner = pet1;  // Default
        result.loser = pet2;   // Default

        if (!pet1->is_alive() || !pet2->is_alive()) {
            throw NotEnoughHpException("Insufficient HP, cannot start battle!");
        }

        result.log.push_back("Battle starts: " + pet1->get_name() + " vs " + pet2->get_name());

        int round = 1;
        bool attacker_turn = true;  // Player attacks first

        while (pet1->is_alive() && pet2->is_alive()) {
            std::string round_msg = "Round " + std::to_string(round);

            if (attacker_turn) {
                int damage = pet1->attack();
                pet2->take_damage(damage);
                round_msg += ": " + pet1->get_name() + " attacks " + pet2->get_name()
                           + " for " + std::to_string(damage) + " damage!";
                round_msg += " (" + pet2->get_name() + " HP: " + std::to_string(pet2->get_hp())
                           + "/" + std::to_string(pet2->get_max_hp()) + ")";
            } else {
                int damage = pet2->attack();
                pet1->take_damage(damage);
                round_msg += ": " + pet2->get_name() + " attacks " + pet1->get_name()
                           + " for " + std::to_string(damage) + " damage!";
                round_msg += " (" + pet1->get_name() + " HP: " + std::to_string(pet1->get_hp())
                           + "/" + std::to_string(pet1->get_max_hp()) + ")";
            }
            result.log.push_back(round_msg);

            attacker_turn = !attacker_turn;
            round++;

            if (round > 50) {
                result.log.push_back("Battle ended in draw after 50 rounds!");
                return result;
            }
        }

        if (pet1->is_alive()) {
            result.winner = pet1;
            result.loser = pet2;
            result.log.push_back(pet1->get_name() + " wins the battle!");
        } else {
            result.winner = pet2;
            result.loser = pet1;
            result.log.push_back(pet2->get_name() + " wins the battle!");
        }

        return result;
    }

    // API mode: automated battle between pet and monster
    BattleResult start_monster_battle_api(std::shared_ptr<Pet> pet, std::shared_ptr<Monster> monster) {
        BattleResult result;
        result.attacker = pet;
        result.defender = monster;
        result.winner = pet;  // Default
        result.loser = monster;   // Default

        if (!pet->is_alive() || !monster->is_alive()) {
            throw NotEnoughHpException("Insufficient HP, cannot start battle!");
        }

        result.log.push_back("Battle starts: " + pet->get_name() + " vs " + monster->get_name());

        int round = 1;
        bool attacker_turn = true;  // Player attacks first

        while (pet->is_alive() && monster->is_alive()) {
            std::string round_msg = "Round " + std::to_string(round);

            if (attacker_turn) {
                int damage = pet->attack();
                double multiplier = get_type_multiplier(pet->get_type(), monster->get_type());
                damage = (int)(damage * multiplier);
                monster->take_damage(damage);
                round_msg += ": " + pet->get_name() + " attacks " + monster->get_name()
                           + " for " + std::to_string(damage) + " damage!";
                if (multiplier > 1.0) round_msg += " (属性克制!)";
                else if (multiplier < 1.0) round_msg += " (属性被克制!)";
                round_msg += " (" + monster->get_name() + " HP: " + std::to_string(monster->get_hp())
                           + "/" + std::to_string(monster->get_max_hp()) + ")";
            } else {
                int damage = monster->attack();
                double multiplier = get_type_multiplier(monster->get_type(), pet->get_type());
                damage = (int)(damage * multiplier);
                pet->take_damage(damage);
                round_msg += ": " + monster->get_name() + " attacks " + pet->get_name()
                           + " for " + std::to_string(damage) + " damage!";
                if (multiplier > 1.0) round_msg += " (属性克制!)";
                else if (multiplier < 1.0) round_msg += " (属性被克制!)";
                round_msg += " (" + pet->get_name() + " HP: " + std::to_string(pet->get_hp())
                           + "/" + std::to_string(pet->get_max_hp()) + ")";
            }
            result.log.push_back(round_msg);

            attacker_turn = !attacker_turn;
            round++;

            if (round > 50) {
                result.log.push_back("Battle ended in draw after 50 rounds!");
                return result;
            }
        }

        if (pet->is_alive()) {
            result.winner = pet;
            result.loser = monster;
            result.log.push_back(pet->get_name() + " wins the battle!");
        } else {
            result.winner = monster;
            result.loser = pet;
            result.log.push_back(monster->get_name() + " wins the battle!");
        }

        return result;
    }

private:
    static std::string to_lower_str(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }

    // Returns attack multiplier based on type advantage
    // 2.0 = advantage, 0.5 = disadvantage, 1.0 = neutral
    static double get_type_multiplier(const std::string& attackerType, const std::string& defenderType) {
        // Two 4-type cycles:
        // Cycle 1: fire -> grass -> electric -> water -> fire
        // Cycle 2: rock -> wind -> ice -> mech -> rock
        static const std::vector<std::pair<std::string, std::string>> advantages = {
            {"fire", "grass"},
            {"grass", "electric"},
            {"electric", "water"},
            {"water", "fire"},
            {"rock", "wind"},
            {"wind", "ice"},
            {"ice", "mech"},
            {"mech", "rock"}
        };

        std::string atk = to_lower_str(attackerType);
        std::string def = to_lower_str(defenderType);

        for (const auto& adv : advantages) {
            if (adv.first == atk && adv.second == def) return 2.0;
            if (adv.first == def && adv.second == atk) return 0.5;
        }
        return 1.0;
    }

    BattleResultCalculator calculator_;
    void print_result(std::shared_ptr<Pet> pet1, std::shared_ptr<Pet> pet2) {
        std::cout << "\n========================================\n";
        std::cout << "           BATTLE END!\n";
        std::cout << "========================================\n";
        if (pet1->is_alive()) {
            std::cout << "VICTORY! " << pet1->get_name() << " wins!\n";
        } else if (pet2->is_alive()) {
            std::cout << "DEFEAT! " << pet2->get_name() << " wins...\n";
        } else {
            std::cout << "Draw! Both sides defeated!\n";
        }
        std::cout << "========================================\n\n";
    }
};
