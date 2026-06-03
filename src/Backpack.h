#pragma once
#include <vector>
#include <iostream>
#include <algorithm>
#include <stdexcept>

// ============================================================
// Backpack<T>: Template Backpack Class
// Can hold items of any type T (demonstrating template generality)
// ============================================================
template <typename T>
class Backpack {
private:
    std::vector<T> m_items;
    int m_maxSize;

public:
    Backpack(int maxSize = 10) : m_maxSize(maxSize) {}

    void add(T item) {
        if ((int)m_items.size() >= m_maxSize) {
            throw std::runtime_error("Backpack is full, cannot add more items!");
        }
        m_items.push_back(item);
        std::cout << "Item added to backpack!\n";
    }

    void remove(int index) {
        if (index < 0 || index >= (int)m_items.size()) {
            throw std::out_of_range("Backpack index out of bounds!");
        }
        std::cout << "Removed item: " << m_items[index] << "\n";
        m_items.erase(m_items.begin() + index);
    }

    T& get(int index) {
        if (index < 0 || index >= (int)m_items.size()) {
            throw std::out_of_range("Backpack index out of bounds!");
        }
        return m_items[index];
    }

    void show() const {
        if (m_items.empty()) {
            std::cout << "Backpack is empty.\n";
            return;
        }
        std::cout << "===== Backpack Contents (" << m_items.size() << "/" << m_maxSize << ") =====\n";
        for (int i = 0; i < (int)m_items.size(); i++) {
            std::cout << i + 1 << ". " << m_items[i] << "\n";
        }
        std::cout << "========================\n";
    }

    void sort_by_value() {
        std::sort(m_items.begin(), m_items.end(), [](const T& a, const T& b) {
            return a > b;
        });
        std::cout << "Backpack sorted by value!\n";
    }

    int size() const {
        return (int)m_items.size();
    }

    bool is_empty() const {
        return m_items.empty();
    }

    const std::vector<T>& get_all() const {
        return m_items;
    }

    int get_max_size() const { return m_maxSize; }

    // JSON serialization
    std::string to_json() const {
        std::string json = "[";
        for (int i = 0; i < (int)m_items.size(); i++) {
            if (i > 0) json += ",";
            json += m_items[i].to_json();
        }
        json += "]";
        return json;
    }
};
