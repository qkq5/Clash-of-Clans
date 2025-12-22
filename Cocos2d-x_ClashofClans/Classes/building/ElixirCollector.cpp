#include "ElixirCollector.h"
#include <string>

namespace building {

ElixirCollector* ElixirCollector::create() {
    ElixirCollector *pRet = new(std::nothrow) ElixirCollector();
    if (pRet && pRet->init()) {
        pRet->autorelease();
        return pRet;
    }
    else {
        delete pRet;
        pRet = nullptr;
        return nullptr;
    }
}

bool ElixirCollector::init() {
    if (!Building::init()) {
        return false;
    }
    
    _type = BuildingType::ElixirCollector;
    _buildingName = "Elixir Collector";
    _level = 1;
    _maxHP = 400;
    _currentHP = _maxHP;
    
    updateTexture();
    
    return true;
}

void ElixirCollector::updateTexture() {
    std::string textureName = "elixir_collector" + std::to_string(_level) + ".png";
    this->setTexture(textureName);
}

void ElixirCollector::upgrade() {
    if (!canUpgrade()) return;
    
    _level++;
    if (_level == 2) {
        _maxHP = 600;
    } else if (_level == 3) {
        _maxHP = 800;
    }
    _currentHP = _maxHP;
    updateTexture();
}

int ElixirCollector::getUpgradeCost() const {
    if (_level == 1) return 300;
    if (_level == 2) return 600;
    return 0; // Max level
}

bool ElixirCollector::canUpgrade() const {
    return _level < 3;
}

void ElixirCollector::showInfo() {
    CCLOG("Show Info for Elixir Collector Level %d", _level);
}

} // namespace building
