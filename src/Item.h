#pragma once
#include <string>
#include <iostream>

// ============================================================
// Item: Item Class
// Represents items in backpack (potions, enhancement stones, etc.)
// ============================================================
class Item {
private:
    std::string m_name;
    std::string m_type;
    int m_value;

public:
    Item(const std::string& name, const std::string& type, int value)
        : m_name(name), m_type(type), m_value(value) {}

    Item() : m_name("Unknown Item"), m_type("Potion"), m_value(0) {}

    std::string get_name() const { return m_name; }
    std::string get_type() const { return m_type; }
    int get_value() const { return m_value; }

    void show() const {
        std::cout << "[" << m_type << "] " << m_name << " - Value: " << m_value << "\n";
    }

    friend std::ostream& operator<<(std::ostream& os, const Item& item) {
        os << "[" << item.m_type << "] " << item.m_name << " (Value:" << item.m_value << ")";
        return os;
    }

    bool operator>(const Item& other) const { return m_value > other.m_value; }
    bool operator<(const Item& other) const { return m_value < other.m_value; }

    // JSON serialization
    std::string to_json() const {
        return "{\"name\":\"" + m_name + "\",\"type\":\"" + m_type + "\",\"value\":" + std::to_string(m_value) + "}";
    }
};
