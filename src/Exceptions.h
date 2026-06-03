#pragma once
#include <stdexcept>
#include <string>

// ============================================================
// Custom Exception Classes (inherited from std::exception)
// ============================================================

/**
 * NotEnoughHpException: thrown when pet HP is insufficient
 * e.g., attempting to attack when HP <= 0
 */
class NotEnoughHpException : public std::exception {
private:
    std::string m_message;
public:
    NotEnoughHpException(const std::string& msg = "Insufficient HP, cannot perform this action!")
        : m_message(msg) {}

    const char* what() const noexcept override {
        return m_message.c_str();
    }
};

/**
 * InvalidMoveException: thrown when user enters invalid command or battle logic error
 * e.g., entering a non-existent operation option
 */
class InvalidMoveException : public std::exception {
private:
    std::string m_message;
public:
    InvalidMoveException(const std::string& msg = "Invalid operation, please re-enter!")
        : m_message(msg) {}

    const char* what() const noexcept override {
        return m_message.c_str();
    }
};
