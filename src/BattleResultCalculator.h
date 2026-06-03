#pragma once
#include "Pet.h"
#include <memory>


enum class BattleOutcome { PET_WIN, MONSTER_WIN, DRAW };

class BattleResultCalculator {
public:
    explicit BattleResultCalculator() = default;

    BattleOutcome evaluate_winner(const std::shared_ptr<Pet>& pet,
                                 const std::shared_ptr<Monster>& monster) const;

    int calculate_damage(const std::shared_ptr<Pet>& attacker,
                         const std::shared_ptr<Pet>& defender) const;

private:
    // Helper: 判断宠物的type是否对抗 monster的type（岩纸剪子式）
    bool is_advantageous(const std::string& pet_type,
                         const std::string& monster_type) const;
};