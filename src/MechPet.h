#pragma once
#include "Pet.h"
#include <iostream>

// ============================================================
// MechPet: Mech Type Pet
// Balanced stats, special skill can boost ATK
// ============================================================
class MechPet : public Pet {
private:
    int m_boostAmount;

public:
    MechPet(const std::string& name, int level = 1)
        : Pet(name, 100 + level * 12, 12 + level * 3, level),
          m_boostAmount(8) {
        std::cout << "[Created] Mech Pet " << name << " born!\n";
    }

    ~MechPet() override {
        std::cout << "[Destroyed] Mech Pet " << m_name << " powers down.\n";
    }

    std::string get_type() const override {
        return "Mech";
    }

    int attack() override {
        if (m_hp <= 0) {
            throw NotEnoughHpException(m_name + " has fallen, cannot attack!");
        }
        int damage = m_attackPower;
        std::cout << m_name << " Laser Scan! Deals " << damage << " damage!\n";
        return damage;
    }

    void special_skill() override {
        if (m_hp <= 0) {
            throw NotEnoughHpException(m_name + " HP insufficient, cannot use skill!");
        }
        m_attackPower += m_boostAmount;
        std::cout << m_name << " uses [OVERCHARGE MODE]! ATK +"
                  << m_boostAmount << "! Current ATK: " << m_attackPower << "\n";
    }
};
