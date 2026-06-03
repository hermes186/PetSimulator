#pragma once
#include <string>
#include <iostream>
#include <sstream>
#include "Exceptions.h"

// ============================================================
// Pet Abstract Base Class
// All pet types must inherit from this class
// ============================================================
class Pet {
protected:
    std::string m_name;
    int m_hp;
    int m_maxHp;
    int m_attackPower;
    int m_level;
    int m_exp;
    int m_expToNextLevel;

public:
    Pet(const std::string& name, int hp, int attack, int level = 1)
        : m_name(name), m_hp(hp), m_maxHp(hp), m_attackPower(attack),
          m_level(level), m_exp(0), m_expToNextLevel(100) {}

    virtual ~Pet() {
        std::cout << "[Destroy] Pet " << m_name << " released from memory.\n";
    }

    // Pure virtual functions (must be implemented by derived classes)
    virtual int attack() = 0;
    virtual std::string get_type() const = 0;
    virtual void special_skill() = 0;

    // Common methods
    void take_damage(int damage) {
        m_hp -= damage;
        if (m_hp < 0) m_hp = 0;
    }

    void heal(int amount) {
        m_hp += amount;
        if (m_hp > m_maxHp) m_hp = m_maxHp;
        std::cout << m_name << " recovered " << amount << " HP, current HP: " << m_hp << "/" << m_maxHp << "\n";
    }

    void train() {
        if (m_hp <= 0) {
            throw NotEnoughHpException(m_name + " HP is 0, cannot train!");
        }
        m_exp += 30;
        m_attackPower += 2;
        std::cout << m_name << " trained! ATK +2, EXP +30\n";
        check_level_up();
    }

    void check_level_up() {
        if (m_exp >= m_expToNextLevel) {
            m_exp -= m_expToNextLevel;
            m_level++;
            m_maxHp += 20;
            m_hp = m_maxHp;
            m_attackPower += 5;
            m_expToNextLevel = m_level * 100;
            std::cout << "LEVEL UP! " << m_name << " is now level " << m_level
                      << ", Max HP: " << m_maxHp
                      << ", ATK: " << m_attackPower << "\n";
            if (m_level == 3) {
                std::cout << "EVOLVED! " << m_name << " has become stronger!\n";
            }
        }
    }

    bool is_alive() const {
        return m_hp > 0;
    }

    // Getters
    std::string get_name() const { return m_name; }
    int get_hp() const { return m_hp; }
    int get_max_hp() const { return m_maxHp; }
    int get_attack() const { return m_attackPower; }
    int get_level() const { return m_level; }
    int get_exp() const { return m_exp; }

    // Operator overloading
    bool operator>(const Pet& other) const {
        return (m_attackPower * m_level) > (other.m_attackPower * other.m_level);
    }

    bool operator<(const Pet& other) const {
        return (m_attackPower * m_level) < (other.m_attackPower * other.m_level);
    }

    bool operator==(const Pet& other) const {
        return (m_attackPower * m_level) == (other.m_attackPower * other.m_level);
    }

    friend std::ostream& operator<<(std::ostream& os, const Pet& pet) {
        os << "[" << pet.get_type() << " Pet] " << pet.m_name
           << " | Lv: " << pet.m_level
           << " | HP: " << pet.m_hp << "/" << pet.m_maxHp
           << " | ATK: " << pet.m_attackPower
           << " | EXP: " << pet.m_exp << "/" << pet.m_expToNextLevel;
        return os;
    }

    // JSON serialization for REST API
    virtual std::string to_json() const {
        std::ostringstream oss;
        oss << "{";
        oss << "\"name\":" << "\"" << m_name << "\",";
        oss << "\"type\":" << "\"" << get_type() << "\",";
        oss << "\"hp\":" << m_hp << ",";
        oss << "\"maxHp\":" << m_maxHp << ",";
        oss << "\"attack\":" << m_attackPower << ",";
        oss << "\"level\":" << m_level << ",";
        oss << "\"exp\":" << m_exp << ",";
        oss << "\"expToNextLevel\":" << m_expToNextLevel << ",";
        oss << "\"alive\":" << (is_alive() ? "true" : "false");
        oss << "}";
        return oss.str();
    }
};

// ============================================================
// Monster Class
// Monster class for battles
// ============================================================
class Monster : public Pet {
private:
    std::string m_type;  // 怪物类型: "fire", "water", "grass", "electric", "rock", "wind", "ice", "mech"
    int m_difficulty;    // 难度等级 1-10

public:
    Monster(const std::string& name, const std::string& type, int difficulty = 1)
        : Pet(name, 50 + difficulty * 10, 10 + difficulty * 2, 1),  // 基础属性
          m_type(type), m_difficulty(difficulty) {
        // 根据难度调整属性
        m_maxHp = m_hp = 50 + difficulty * 10;
        m_attackPower = 10 + difficulty * 2;
        m_level = difficulty;  // 怪物等级等于难度
        m_exp = 0;
        m_expToNextLevel = 100;
    }

    virtual ~Monster() {
        std::cout << "[Destroy] Monster " << m_name << " released from memory.\n";
    }

    // 重写获取类型方法
    virtual std::string get_type() const override { return m_type; }

    // 怪物没有特殊技能
    virtual void special_skill() override {}

    // 攻击方法
    virtual int attack() override { return m_attackPower; }

    int get_difficulty() const { return m_difficulty; }

    // JSON serialization for REST API
    std::string to_json() const override {
        std::ostringstream oss;
        oss << "{";
        oss << "\"name\":\"" << m_name << "\",";
        oss << "\"type\":\"" << m_type << "\",";
        oss << "\"hp\":" << m_hp << ",";
        oss << "\"maxHp\":" << m_maxHp << ",";
        oss << "\"attack\":" << m_attackPower << ",";
        oss << "\"level\":" << m_level << ",";
        oss << "\"difficulty\":" << m_difficulty;
        oss << "}";
        return oss.str();
    }
};
