#ifndef ENUMS_H
#define ENUMS_H

// Locations
enum class Location {
    TRADING_CENTER,
    MINE,
    CITY_CENTER,
    SEWER,
    LAB,
    CORE_SYSTEM,
    NONE
};

// Attack type
enum class AttackType { MELEE, RANGED, MIXED };

// Enemy type
enum class EnemyType { SILICON, PLANTER, AIROBOT, BLAME };

#endif 