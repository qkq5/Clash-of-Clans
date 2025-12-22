#include "ElixirStorage.h"
#include <string>

namespace building {

ElixirStorage* ElixirStorage::create() {
    ElixirStorage *pRet = new(std::nothrow) ElixirStorage();
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

bool ElixirStorage::init() {
    if (!Building::init()) {
        return false;
    }
    
    _type = BuildingType::ElixirStorage;
    _buildingName = "Elixir Storage";
    _level = 1;
    _maxHP = 800;
    _currentHP = _maxHP;
    
    updateTexture();
    
    return true;
}

void ElixirStorage::updateTexture() {
    std::string textureName = "elixir_storage" + std::to_string(_level) + ".png";
    this->setTexture(textureName);
}

void ElixirStorage::upgrade() {
    if (!canUpgrade()) return;
    
    _level++;
    if (_level == 2) {
        _maxHP = 1200;
    } else if (_level == 3) {
        _maxHP = 1600;
    }
    _currentHP = _maxHP;
    updateTexture();
}

int ElixirStorage::getUpgradeCost() const {
    if (_level == 1) return 600;
    if (_level == 2) return 1200;
    return 0; // Max level
}

bool ElixirStorage::canUpgrade() const {
    return _level < 3;
}

void ElixirStorage::showInfo() {
    CCLOG("Show Info for Elixir Storage Level %d", _level);
}

int ElixirStorage::getCapacity() const {
    if (_level == 1) return 600;
    if (_level == 2) return 1200;
    return 1800;
}

} // namespace building
