#include "Building.h"

namespace building {

Building::Building() 
: _level(1)
, _maxHP(100)
, _currentHP(100)
, _buildingName("Building")
, _type(BuildingType::TownHall)
, _gridPosition(cocos2d::Vec2::ZERO)
, _attackDamage(0)
, _attackRange(0)
, _attackInterval(0)
, _attackTimer(0)
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
    
    updateHPBar();
}

void Building::initHPBar() {
    auto size = this->getContentSize();
    if (size.width == 0) size.width = 64; // Fallback
    
    float width = size.width;
    float height = 6; // Slightly thicker for buildings
    
    auto bar = cocos2d::LayerColor::create(cocos2d::Color4B::RED, width, height);
    bar->setPosition(cocos2d::Vec2(0, size.height + 5));
    this->addChild(bar, 100, 99);
}

void Building::updateHPBar() {
    auto bar = dynamic_cast<cocos2d::LayerColor*>(this->getChildByTag(99));
    if (bar) {
        float percent = (float)_currentHP / _maxHP;
        if (percent < 0) percent = 0;
        float width = this->getContentSize().width;
        if (width == 0) width = 64;
        bar->setContentSize(cocos2d::Size(width * percent, 6));
    }
}

} // namespace building
