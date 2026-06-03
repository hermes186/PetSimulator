#pragma once
#include "Pet.h"
#include <iostream>

// ============================================================
// FirePet: Fire Type Pet
// High attack power with 25% critical hit chance (2x damage)
// ============================================================
class FirePet : public Pet {
private:
    int m_burnDamage;

public:
    FirePet(const std::string& name, int level = 1)
        : Pet(name, 80 + level * 10, 15 + level * 3, level),
          m_burnDamage(8) {
        std::cout << "[Created] Fire Pet " << name << " born!\n";
    }

    ~FirePet() override {
        std::cout << "[Destroyed] Fire Pet " << m_name << " fades away.\n";
    }

    std::string get_type() const override {
        return "Fire";
    }

    int attack() override {
        if (m_hp <= 0) {
            throw NotEnoughHpException(m_name + " has fallen, cannot attack!");
        }
        std::cout << m_name << " breathes fire! ";
        int damage = m_attackPower;
        if (rand() % 4 == 0) {
            damage *= 2;
            std::cout << "CRITICAL HIT!";
        }
        std::cout << " Deals " << damage << " damage!\n";
        return damage;
    }

    void special_skill() override {
        if (m_hp <= 0) {
            throw NotEnoughHpException(m_name + " HP insufficient, cannot use skill!");
        }
        std::cout << m_name << " uses [BURNING STRIKE]! Deals "
                  << m_burnDamage << " continuous burn damage!\n";
    }

    int get_burn_damage() const { return m_burnDamage; }
};
