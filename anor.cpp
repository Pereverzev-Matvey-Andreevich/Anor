#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <random>

// Локации
enum class Location {
    TRADING_CENTER,
    MINE,
    CITY_CENTER,
    SEWER,
    LAB,
    CORE_SYSTEM,
    NONE
};
static std::map<Location, std::string> locationNames = {
    {Location::TRADING_CENTER, "Trading Center"},
    {Location::MINE, "Mines"},
    {Location::CITY_CENTER, "City Center"},
    {Location::SEWER, "Sewers"},
    {Location::LAB, "Laboratory"},
    {Location::CORE_SYSTEM, "Core System"}
};

// Тип атаки
enum class AttackType { MELEE, RANGED, MIXED };
static std::map<AttackType, std::string> attackNames = {
    {AttackType::MELEE, "Melee"},
    {AttackType::RANGED, "Ranged"},
    {AttackType::MIXED, "Mixed"}
};

// Тип врага
enum class EnemyType { SILICON, PLANTER, AIROBOT };
static std::map<EnemyType, std::string> enemyNames = {
    {EnemyType::SILICON, "Silicon Life"},
    {EnemyType::PLANTER, "Planter"},
    {EnemyType::AIROBOT, "AI Robot"}
};

// Статистика врагов из ГДД
struct EnemyStats {
    int hp;
    int damage;
    std::map<AttackType, int> playerDamage;
};
static std::map<EnemyType, EnemyStats> enemyData = {
    {EnemyType::SILICON, {200, 60, {{AttackType::MELEE,30}, {AttackType::RANGED,120}, {AttackType::MIXED,70}}}},
    {EnemyType::PLANTER, {300, 30, {{AttackType::MELEE,150}, {AttackType::RANGED,80}, {AttackType::MIXED,65}}}},
    {EnemyType::AIROBOT, {150, 100, {{AttackType::MELEE,25}, {AttackType::RANGED,40}, {AttackType::MIXED,75}}}}
};

// Инвентарь
class Inventory {
public:
    bool addItem(const std::string& item) {
        if (items.size() >= maxSize) return false;
        items.push_back(item);
        return true;
    }
    bool useMedKit() {
        for (auto it = items.begin(); it != items.end(); ++it) {
            if (*it == "MedKit") { items.erase(it); return true; }
        }
        return false;
    }
    int count(const std::string& item) const {
        int cnt = 0;
        for (auto& it : items) if (it == item) cnt++;
        return cnt;
    }
private:
    std::vector<std::string> items;
    static constexpr int maxSize = 15;
};

// Игрок
struct Player { int hp = 500; Inventory inventory; };

class Game {
public:
    Game() { }
    void run() {
        chooseStartLocation();
        while (currentLocation != Location::NONE) {
            if (currentLocation == Location::CORE_SYSTEM) {
                if (chipsCollected < 5) {
                    std::cout << "Access Denied: You need 5 chips to enter Core System.\n";
                    chooseNextLocation();
                    continue;
                }
                else break;
            }
            describeLocation(currentLocation);
            exploreLocation();
            if (player.hp <= 0) break;
            chooseNextLocation();
        }
        if (currentLocation == Location::CORE_SYSTEM) {
            std::cout << "\nYou enter the Core System with 5 chips. The truth awaits...\nGame Over.\n";
        }
        else {
            std::cout << "\nYou have fallen in battle. Game Over.\n";
        }
    }

private:
    Player player;
    Location currentLocation = Location::NONE;
    std::set<Location> chipsLocations;
    int chipsCollected = 0;
    int killCount = 0;

    void chooseStartLocation() {
        std::cout << "Choose your starting location:" << std::endl;
        listLocations(false);
        int sel = userSelect(6);
        currentLocation = static_cast<Location>(sel - 1);
    }

    void listLocations(bool includeCore) {
        for (int i = 0; i < 5; ++i)
            std::cout << "[" << (i + 1) << "] " << locationNames[static_cast<Location>(i)] << std::endl;
        if (includeCore)
            std::cout << "[6] " << locationNames[Location::CORE_SYSTEM] << std::endl;
    }

    int userSelect(int max) {
        int sel; std::cin >> sel;
        if (sel < 1 || sel > max) sel = 1;
        return sel;
    }

    void describeLocation(Location loc) {
        std::cout << "\n--- " << locationNames[loc] << " ---" << std::endl;
        // TODO: добавить атмосферные описания
    }

    void exploreLocation() {
        EnemyType enemyType = randomEnemy();
        std::cout << "An enemy " << enemyNames[enemyType] << " appears!\n";
        handleCombat(enemyType);
        if (player.hp <= 0) return;
        if (chipsLocations.insert(currentLocation).second) {
            chipsCollected++;
            player.inventory.addItem("Chip");
            std::cout << "You found a Chip! Total: " << chipsCollected << "/5\n";
        }
    }

    EnemyType randomEnemy() {
        static std::mt19937 rng((unsigned)time(nullptr));
        std::uniform_int_distribution<int> dist(0, 2);
        return static_cast<EnemyType>(dist(rng));
    }

    void handleCombat(EnemyType type) {
        auto stats = enemyData[type]; int enemyHp = stats.hp;
        while (player.hp > 0 && enemyHp > 0) {
            std::cout << "\nYour HP: " << player.hp << " | Enemy HP: " << enemyHp << "\n";
            std::cout << "Choose attack program:" << std::endl;
            for (int i = 0; i < 3; ++i)
                std::cout << "[" << i + 1 << "] " << attackNames[static_cast<AttackType>(i)] << std::endl;
            int choice = userSelect(3);
            AttackType at = static_cast<AttackType>(choice - 1);
            int dmg = stats.playerDamage[at];
            std::cout << "You deal " << dmg << " damage with " << attackNames[at] << "." << std::endl;
            enemyHp -= dmg;
            if (enemyHp <= 0) break;
            std::cout << enemyNames[type] << " deals " << stats.damage << " damage." << std::endl;
            player.hp -= stats.damage;
        }
        if (player.hp <= 0) {
            std::cout << "You have been defeated...\n";
            currentLocation = Location::NONE;
        }
        else {
            std::cout << "Enemy defeated!\n";
            killCount++;
            if (killCount % 3 == 0 && player.inventory.addItem("MedKit"))
                std::cout << "MedKit dropped!\n";
            if (player.inventory.count("MedKit") > 0) {
                std::cout << "Use MedKit? [1]Yes [2]No\n";
                if (userSelect(2) == 1 && player.inventory.useMedKit()) {
                    player.hp += 150;
                    std::cout << "Recovered 150 HP. Current HP: " << player.hp << "\n";
                }
            }
        }
    }

    void chooseNextLocation() {
        std::cout << "\nChoose next location:" << std::endl;
        listLocations(true);
        int sel = userSelect(6);
        currentLocation = static_cast<Location>(sel - 1);
    }
};

int main() {
    Game game;
    game.run();
    return 0;
}

