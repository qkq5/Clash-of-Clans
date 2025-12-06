#pragma once
#include "cocos2d.h"

enum class BuildingType { TownHall, Mine, ElixirCollector, Barracks, DefenseTower, Storage };

class Building : public cocos2d::Node {
public:
    BuildingType type;
    int level;
    int hp;
    virtual void upgrade() = 0;
    void takeDamage(int damage);
    bool isDestroyed() const;
};
