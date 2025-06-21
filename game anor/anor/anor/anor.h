#ifndef ANOR_H
#define ANOR_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <random>
#include <ctime>
#include <fstream>
#include "D:\game anor\anor\json.hpp"

using json = nlohmann::json;

// Forward declarations
enum class Location;
enum class AttackType;
enum class EnemyType;

// Load game texts
json LoadTexts(const std::string& filename);
extern const json kTexts;

// Enemy statistics
struct EnemyStats {
    int hp;
    int damage;
    std::map<AttackType, int> player_damage;

    EnemyStats() = default;
    explicit EnemyStats(const json& data);
};

// Boss form statistics
struct BossFormStats {
    int damage;
    std::map<AttackType, int> player_damage;

    BossFormStats() = default;
    explicit BossFormStats(const json& data);
};

// Inventory class
class Inventory {
public:
    Inventory();
    bool AddItem(const std::string& item);
    bool UseMedKit();
    int Count(const std::string& item) const;
    void Print() const;

private:
    std::vector<std::string> items_;
    int max_size_;
};

// Player struct
struct Player {
    int hp;
    Inventory inventory;
    int medkit_heal;

    Player();
    void UseMedKit();
};

// Main Game class
class Game {
public:
    Game();
    void Run();

private:
    Player player_;
    Location current_location_;
    std::set<Location> chips_locations_;
    int chips_collected_;
    int kill_count_;
    std::mt19937 rng_;

    std::string GetLocationName(Location loc);
    std::string GetLocationDescription(Location loc);
    std::string GetAttackName(AttackType at);
    std::string GetEnemyName(EnemyType et);
    EnemyStats GetEnemyStats(EnemyType type);
    void ChooseStartLocation();
    int UserSelect(int max);
    void DescribeLocation(Location loc);
    void ExploreLocation();
    EnemyType RandomEnemy();
    void HandleCombat(EnemyType type);
    bool HandleBossCombat();
    void ChooseNextLocation();
    void HandleInventory();
};

#endif 