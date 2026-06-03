#pragma once
#include "Pet.h"
#include <iostream>

// ============================================================
// WaterPet: Water Type Pet
// High HP, special skill can heal itself
// ============================================================
class WaterPet : public Pet {
private:
    int m_healAmount;

public:
    WaterPet(const std::string& name, int level = 1)
        : Pet(name, 120 + level * 15, 10 + level * 2, level),
          m_healAmount(15) {
        std::cout << "[Created] Water Pet " << name << " born!\n";
    }

    ~WaterPet() override {
        std::cout << "[Destroyed] Water Pet " << m_name << " returns to the stream.\n";
    }

    std::string get_type() const override {
        return "Water";
    }

    int attack() override {
        if (m_hp <= 0) {
            throw NotEnoughHpException(m_name + " has fallen, cannot attack!");
        }
        int damage = m_attackPower;
        std::cout << m_name << " shoots Water Cannon! Deals " << damage << " damage!\n";
        return damage;
    }

    void special_skill() override {
        if (m_hp <= 0) {
            throw NotEnoughHpException(m_name + " HP insufficient, cannot use skill!");
        }
        heal(m_healAmount);
        std::cout << m_name << " uses [AQUA SHIELD]! Recovers " << m_healAmount << " HP!\n";
    }

    int get_heal_amount() const { return m_healAmount; }
};
