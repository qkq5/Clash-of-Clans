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

} // namespace building
