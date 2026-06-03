#include "BattleResultCalculator.h"
#include "Pet.h"
#include <memory>
#include <unordered_map>
#include <cmath>

BattleOutcome BattleResultCalculator::evaluate_winner(
    const std::shared_ptr<Pet>& pet,
    const std::shared_ptr<Monster>& monster) const {
    if (!pet || !monster) return BattleOutcome::MONSTER_WIN;
    
    // (a) Higher-level pet wins directly
    if ((*pet).get_level() > (*monster).get_level()) {
        return BattleOutcome::PET_WIN;
    }

    // (b) 等级差距 ≤ 5 且宠物拥有タイプ优势时，宠物仍可获胜
    int level_diff = std::abs(static_cast<int>((*pet).get_level()) -
                              static_cast<int>((*monster).get_level()));
    if (level_diff <= 5 && is_advantageous((*pet).get_type(), (*monster).get_type())) {
        return BattleOutcome::PET_WIN;
    }

    return BattleOutcome::MONSTER_WIN;
}

int BattleResultCalculator::calculate_damage(
    const std::shared_ptr<Pet>& attacker,
    const std::shared_ptr<Pet>& defender) const {
    int base = (*defender).attack();  // 目标的基础攻击力

    // 若攻击者处于劣势（等级更低且没有タイプ优势），则额外 +25% 伤害
    bool disadvantage = ((*attacker).get_level() < (*defender).get_level()) &&
                        !is_advantageous((*attacker).get_type(), (*defender).get_type());

    if (disadvantage) {
        return static_cast<int>(base * 1.25);  // 四舍五入向下取整（整数乘法已自动截断）
    }
    return base;
}

bool BattleResultCalculator::is_advantageous(
    const std::string& pet_type,
    const std::string& monster_type) const {
    // 简单的岩纸剪子式相生关系（示例），可自行扩展
    static const std::unordered_map<std::string, std::string> advantages = {
        {"fire",   "grass"}, {"grass", "water"},
        {"water",  "fire"},  {"rock",  "fire"},
        {"electric","water"},{"wind",   "rock"},
        {"ice",    "wind"},  {"mech",  "fire"}
    };
    auto it = advantages.find(pet_type);
    if (it == advantages.end()) return true;            // 默认无明确优势
    return it->second == monster_type;                  // 只有对应关系才算优势
}