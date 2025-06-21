#include "anor.h"
#include "enums.h"

const json kTexts = LoadTexts("texts.json");

json LoadTexts(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open texts.json" << std::endl;
        exit(1);
    }
    json data;
    file >> data;
    return data;
}

// EnemyStats implementation
EnemyStats::EnemyStats(const json& data) {
    hp = data["hp"].get<int>();
    damage = data["damage"].get<int>();

    if (data.contains("playerDamage")) {
        player_damage[AttackType::MELEE] = data["playerDamage"]["MELEE"].get<int>();
        player_damage[AttackType::RANGED] = data["playerDamage"]["RANGED"].get<int>();
        player_damage[AttackType::MIXED] = data["playerDamage"]["MIXED"].get<int>();
    }
}

// BossFormStats implementation
BossFormStats::BossFormStats(const json& data) {
    damage = data["damage"].get<int>();
    player_damage[AttackType::MELEE] = data["playerDamage"]["MELEE"].get<int>();
    player_damage[AttackType::RANGED] = data["playerDamage"]["RANGED"].get<int>();
    player_damage[AttackType::MIXED] = data["playerDamage"]["MIXED"].get<int>();
}

// Inventory implementation
Inventory::Inventory() : max_size_(kTexts["game"]["inventory_size"].get<int>()) {}

bool Inventory::AddItem(const std::string& item) {
    if (items_.size() >= max_size_) return false;
    items_.push_back(item);
    return true;
}

bool Inventory::UseMedKit() {
    for (auto it = items_.begin(); it != items_.end(); ++it) {
        if (*it == "MedKit") {
            items_.erase(it);
            return true;
        }
    }
    return false;
}

int Inventory::Count(const std::string& item) const {
    int count = 0;
    for (auto& it : items_) {
        if (it == item) count++;
    }
    return count;
}

void Inventory::Print() const {
    char buffer[100];
    snprintf(buffer, sizeof(buffer), kTexts["ui"]["inventory_title"].get<std::string>().c_str());
    std::cout << buffer << "\n";
    snprintf(buffer, sizeof(buffer), kTexts["ui"]["medkit_count"].get<std::string>().c_str(), Count("MedKit"));
    std::cout << buffer << std::endl;
}

// Player implementation
Player::Player() :
    hp(kTexts["player"]["start_hp"].get<int>()),
    medkit_heal(kTexts["player"]["medkit_heal"].get<int>()) {}

void Player::UseMedKit() {
    if (inventory.UseMedKit()) {
        hp += medkit_heal;
        char buffer[100];
        snprintf(buffer, sizeof(buffer), kTexts["ui"]["medkit_used"].get<std::string>().c_str(), hp);
        std::cout << buffer << "\n";
    }
    else {
        std::cout << kTexts["ui"]["no_medkits"].get<std::string>() << "\n";
    }
}

// Game implementation
Game::Game() : rng_(static_cast<unsigned>(time(nullptr))),
current_location_(Location::NONE),
chips_collected_(0),
kill_count_(0) {
    int start_medkits = kTexts["player"]["start_medkits"].get<int>();
    for (int i = 0; i < start_medkits; i++) {
        player_.inventory.AddItem("MedKit");
    }
}

void Game::Run() {
    std::cout << kTexts["ui"]["intro_message"].get<std::string>() << "\n\n";
    ChooseStartLocation();
    while (current_location_ != Location::NONE) {
        if (current_location_ == Location::CORE_SYSTEM) {
            if (chips_collected_ < kTexts["game"]["chips_required"].get<int>()) {
                std::cout << kTexts["ui"]["access_denied"].get<std::string>() << "\n";
                ChooseNextLocation();
                continue;
            }
            else {
                DescribeLocation(current_location_);
                bool win = HandleBossCombat();
                char buffer[200];
                if (win) {
                    snprintf(buffer, sizeof(buffer), kTexts["ui"]["victory"].get<std::string>().c_str());
                }
                else {
                    snprintf(buffer, sizeof(buffer), kTexts["ui"]["defeat"].get<std::string>().c_str());
                }
                std::cout << "\n" << buffer << "\n";
                std::cout << kTexts["ui"]["game_over"].get<std::string>() << "\n";
                break;
            }
        }
        DescribeLocation(current_location_);
        ExploreLocation();
        if (player_.hp <= 0) break;
        ChooseNextLocation();
    }
    if (player_.hp <= 0 && current_location_ != Location::CORE_SYSTEM) {
        std::cout << "\n" << kTexts["ui"]["defeated"].get<std::string>() << "\n";
        std::cout << kTexts["ui"]["game_over"].get<std::string>() << "\n";
    }
}

std::string Game::GetLocationName(Location loc) {
    switch (loc) {
    case Location::TRADING_CENTER: return kTexts["locationNames"]["TRADING_CENTER"].get<std::string>();
    case Location::MINE: return kTexts["locationNames"]["MINE"].get<std::string>();
    case Location::CITY_CENTER: return kTexts["locationNames"]["CITY_CENTER"].get<std::string>();
    case Location::SEWER: return kTexts["locationNames"]["SEWER"].get<std::string>();
    case Location::LAB: return kTexts["locationNames"]["LAB"].get<std::string>();
    case Location::CORE_SYSTEM: return kTexts["locationNames"]["CORE_SYSTEM"].get<std::string>();
    default: return "Unknown";
    }
}

std::string Game::GetLocationDescription(Location loc) {
    switch (loc) {
    case Location::TRADING_CENTER: return kTexts["locationDescriptions"]["TRADING_CENTER"].get<std::string>();
    case Location::MINE: return kTexts["locationDescriptions"]["MINE"].get<std::string>();
    case Location::CITY_CENTER: return kTexts["locationDescriptions"]["CITY_CENTER"].get<std::string>();
    case Location::SEWER: return kTexts["locationDescriptions"]["SEWER"].get<std::string>();
    case Location::LAB: return kTexts["locationDescriptions"]["LAB"].get<std::string>();
    case Location::CORE_SYSTEM: return kTexts["locationDescriptions"]["CORE_SYSTEM"].get<std::string>();
    default: return "Unknown location";
    }
}

std::string Game::GetAttackName(AttackType at) {
    switch (at) {
    case AttackType::MELEE: return kTexts["attackNames"]["MELEE"].get<std::string>();
    case AttackType::RANGED: return kTexts["attackNames"]["RANGED"].get<std::string>();
    case AttackType::MIXED: return kTexts["attackNames"]["MIXED"].get<std::string>();
    default: return "Unknown";
    }
}

std::string Game::GetEnemyName(EnemyType et) {
    switch (et) {
    case EnemyType::SILICON: return kTexts["enemyNames"]["SILICON"].get<std::string>();
    case EnemyType::PLANTER: return kTexts["enemyNames"]["PLANTER"].get<std::string>();
    case EnemyType::AIROBOT: return kTexts["enemyNames"]["AIROBOT"].get<std::string>();
    case EnemyType::BLAME: return kTexts["enemyNames"]["BLAME"].get<std::string>();
    default: return "Unknown";
    }
}

EnemyStats Game::GetEnemyStats(EnemyType type) {
    std::string enemy_key;
    switch (type) {
    case EnemyType::SILICON: enemy_key = "SILICON"; break;
    case EnemyType::PLANTER: enemy_key = "PLANTER"; break;
    case EnemyType::AIROBOT: enemy_key = "AIROBOT"; break;
    case EnemyType::BLAME: enemy_key = "BLAME"; break;
    default: enemy_key = "SILICON";
    }
    return EnemyStats(kTexts["enemyData"][enemy_key]);
}

void Game::ChooseStartLocation() {
    std::cout << kTexts["ui"]["choose_start_location"].get<std::string>() << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::cout << "[" << (i + 1) << "] " << GetLocationName(static_cast<Location>(i)) << std::endl;
    }
    int sel;
    std::cin >> sel;
    if (sel < 1 || sel > 5) sel = 1;
    current_location_ = static_cast<Location>(sel - 1);
}

int Game::UserSelect(int max) {
    int sel;
    std::cin >> sel;
    if (sel < 1 || sel > max) sel = 1;
    return sel;
}

void Game::DescribeLocation(Location loc) {
    char buffer[100];
    snprintf(buffer, sizeof(buffer), kTexts["ui"]["location_header"].get<std::string>().c_str(),
        GetLocationName(loc).c_str());
    std::cout << buffer << std::endl;
    std::cout << GetLocationDescription(loc) << std::endl;
}

void Game::ExploreLocation() {
    EnemyType enemy_type = RandomEnemy();
    char buffer[100];
    snprintf(buffer, sizeof(buffer), kTexts["ui"]["enemy_appears"].get<std::string>().c_str(),
        GetEnemyName(enemy_type).c_str());
    std::cout << buffer << "\n";
    HandleCombat(enemy_type);
    if (player_.hp <= 0) return;
    if (chips_locations_.insert(current_location_).second) {
        chips_collected_++;
        player_.inventory.AddItem("Chip");
        snprintf(buffer, sizeof(buffer), kTexts["ui"]["chip_found"].get<std::string>().c_str(),
            chips_collected_);
        std::cout << buffer << "\n";
    }
}

EnemyType Game::RandomEnemy() {
    std::uniform_int_distribution<int> dist(0, 2);
    return static_cast<EnemyType>(dist(rng_));
}

void Game::HandleCombat(EnemyType type) {
    EnemyStats stats = GetEnemyStats(type);
    int enemy_hp = stats.hp;
    while (player_.hp > 0 && enemy_hp > 0) {
        char buffer[200];
        snprintf(buffer, sizeof(buffer), kTexts["ui"]["combat_status"].get<std::string>().c_str(),
            player_.hp, enemy_hp);
        std::cout << "\n" << buffer << "\n";
        std::cout << kTexts["ui"]["choose_action"].get<std::string>() << std::endl;
        for (int i = 0; i < 3; ++i) {
            std::cout << "[" << i + 1 << "] " << GetAttackName(static_cast<AttackType>(i)) << std::endl;
        }
        std::cout << "[4] " << kTexts["ui"]["use_medkit_question"].get<std::string>().substr(0, 9) << std::endl;

        int choice = UserSelect(4);
        if (choice == 4) {
            if (player_.inventory.Count("MedKit") > 0) {
                player_.UseMedKit();
            }
            else {
                std::cout << kTexts["ui"]["no_medkits"].get<std::string>() << std::endl;
            }
        }
        else {
            AttackType at = static_cast<AttackType>(choice - 1);
            int dmg = stats.player_damage[at];
            snprintf(buffer, sizeof(buffer), kTexts["ui"]["attack_damage"].get<std::string>().c_str(),
                dmg, GetAttackName(at).c_str());
            std::cout << buffer << std::endl;
            enemy_hp -= dmg;
        }

        if (enemy_hp <= 0) break;

        snprintf(buffer, sizeof(buffer), kTexts["ui"]["enemy_damage"].get<std::string>().c_str(),
            GetEnemyName(type).c_str(), stats.damage);
        std::cout << buffer << std::endl;
        player_.hp -= stats.damage;
    }

    if (player_.hp <= 0) {
        std::cout << kTexts["ui"]["defeated"].get<std::string>() << "\n";
        current_location_ = Location::NONE;
    }
    else {
        std::cout << kTexts["ui"]["enemy_defeated"].get<std::string>() << "\n";
        kill_count_++;

        double drop_chance = kTexts["game"]["medkit_drop_chance"].get<double>();
        std::uniform_real_distribution<double> drop_dist(0.0, 1.0);
        if (drop_dist(rng_) < drop_chance) {
            if (player_.inventory.AddItem("MedKit")) {
                std::cout << kTexts["ui"]["medkit_dropped"].get<std::string>() << "\n";
            }
        }

        if (player_.inventory.Count("MedKit") > 0) {
            std::cout << kTexts["ui"]["use_medkit_question"].get<std::string>() << "\n";
            if (UserSelect(2) == 1) {
                player_.UseMedKit();
            }
        }
    }
}

bool Game::HandleBossCombat() {
    int boss_hp = kTexts["boss"]["hp"].get<int>();
    std::map<std::string, BossFormStats> boss_forms;
    for (auto& form : kTexts["boss"]["forms"].items()) {
        boss_forms[form.key()] = BossFormStats(form.value());
    }

    std::vector<std::string> form_keys;
    for (auto& form : boss_forms) {
        form_keys.push_back(form.first);
    }
    std::uniform_int_distribution<int> dist(0, static_cast<int>(form_keys.size()) - 1);

    while (player_.hp > 0 && boss_hp > 0) {
        std::string form_key = form_keys[dist(rng_)];
        BossFormStats& form_stats = boss_forms[form_key];
        char buffer[100];
        snprintf(buffer, sizeof(buffer), kTexts["ui"]["boss_change"].get<std::string>().c_str(),
            form_key.c_str());
        std::cout << buffer << "\n";

        snprintf(buffer, sizeof(buffer), kTexts["ui"]["combat_status"].get<std::string>().c_str(),
            player_.hp, boss_hp);
        std::cout << buffer << "\n";
        std::cout << kTexts["ui"]["choose_action"].get<std::string>() << "\n";
        for (int i = 0; i < 3; ++i) {
            std::cout << "[" << i + 1 << "] " << GetAttackName(static_cast<AttackType>(i)) << std::endl;
        }
        std::cout << "[4] " << kTexts["ui"]["use_medkit_question"].get<std::string>().substr(0, 9) << "\n";

        int choice = UserSelect(4);
        if (choice == 4) {
            if (player_.inventory.Count("MedKit") > 0) {
                player_.UseMedKit();
            }
            else {
                std::cout << kTexts["ui"]["no_medkits"].get<std::string>() << "\n";
            }
        }
        else {
            AttackType at = static_cast<AttackType>(choice - 1);
            int dmg = form_stats.player_damage[at];
            snprintf(buffer, sizeof(buffer), kTexts["ui"]["attack_damage"].get<std::string>().c_str(),
                dmg, GetAttackName(at).c_str());
            std::cout << buffer << "\n";
            boss_hp -= dmg;
        }
        if (boss_hp <= 0) break;

        int boss_dmg = form_stats.damage;
        snprintf(buffer, sizeof(buffer), kTexts["ui"]["boss_damage"].get<std::string>().c_str(), boss_dmg);
        std::cout << buffer << "\n";
        player_.hp -= boss_dmg;
    }

    return (boss_hp <= 0);
}

void Game::ChooseNextLocation() {
    while (true) {
        std::cout << "\n" << kTexts["ui"]["choose_next_location"].get<std::string>() << std::endl;

        for (int i = 0; i < 5; ++i) {
            char buffer[100];
            snprintf(buffer, sizeof(buffer), kTexts["ui"]["go_to_location"].get<std::string>().c_str(),
                GetLocationName(static_cast<Location>(i)).c_str());
            std::cout << "[" << (i + 1) << "] " << buffer << std::endl;
        }
        char core_buffer[100];
        snprintf(core_buffer, sizeof(core_buffer), kTexts["ui"]["go_to_location"].get<std::string>().c_str(),
            GetLocationName(Location::CORE_SYSTEM).c_str());
        std::cout << "[6] " << core_buffer << std::endl;
        std::cout << "[7] " << kTexts["ui"]["open_inventory"].get<std::string>() << std::endl;

        int sel = UserSelect(7);

        if (sel == 7) {
            HandleInventory();
        }
        else {
            current_location_ = static_cast<Location>(sel - 1);
            break;
        }
    }
}

void Game::HandleInventory() {
    player_.inventory.Print();
    if (player_.inventory.Count("MedKit") > 0) {
        std::cout << kTexts["ui"]["use_medkit_question"].get<std::string>() << "\n";
        if (UserSelect(2) == 1) {
            player_.UseMedKit();
        }
    }
    else {
        std::cout << kTexts["ui"]["no_items"].get<std::string>() << "\n";
    }
}

int main() {
    Game game;
    game.Run();
    return 0;
}