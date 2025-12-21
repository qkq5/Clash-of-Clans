#include "TownHall.h"
#include "core/GameException.h"

namespace building {

TownHall* TownHall::create() {
    TownHall* pRet = new(std::nothrow) TownHall();
    if (pRet && pRet->init()) {
        pRet->autorelease();
        return pRet;
    } else {
        delete pRet;
        pRet = nullptr;
        return nullptr;
    }
}

bool TownHall::init() {
    if (!Building::init()) {
        return false;
    }
    
    _type = BuildingType::TownHall;
    _buildingName = "Town Hall";
    _level = 1;
    _maxHP = 1000;
    _currentHP = _maxHP;
    
    updateTexture();
    
    return true;
}

void TownHall::updateTexture() {
    std::string textureName = "town_hall" + std::to_string(_level) + ".png";
    this->setTexture(textureName);
}

void TownHall::upgrade() {
    if (!canUpgrade()) {
        return;
    }
    
    _level++;
    
    // Update properties based on level
    if (_level == 2) {
        _maxHP = 1500;
    } else if (_level == 3) {
        _maxHP = 2000;
    }
    _currentHP = _maxHP; // Heal on upgrade? usually yes in simple games
    
    updateTexture();
}

int TownHall::getUpgradeCost() const {
    if (_level == 1) return 500;
    if (_level == 2) return 1000;
    return 0; // Max level
}

bool TownHall::canUpgrade() const {
    return _level < 3;
}

void TownHall::showInfo() {
    // This will be used by the UI system to populate the info panel
    CCLOG("Town Hall Level %d selected", _level);
}

} // namespace building
