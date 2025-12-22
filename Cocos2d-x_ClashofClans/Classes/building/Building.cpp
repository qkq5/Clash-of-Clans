#include "Building.h"

namespace building {

Building::Building() 
: _level(1)
, _maxHP(100)
, _currentHP(100)
, _buildingName("Building")
, _type(BuildingType::TownHall)
, _gridPosition(cocos2d::Vec2::ZERO)
{
}

Building::~Building() {
}

void Building::takeDamage(int damage) {
    _currentHP -= damage;
    if (_currentHP < 0) _currentHP = 0;
    
    // Add red flash effect
    this->runAction(cocos2d::Sequence::create(
        cocos2d::TintTo::create(0.1f, 255, 0, 0),
        cocos2d::TintTo::create(0.1f, 255, 255, 255),
        nullptr
    ));
}

} // namespace building
