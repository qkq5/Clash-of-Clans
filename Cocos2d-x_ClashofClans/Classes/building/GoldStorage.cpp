#include "GoldStorage.h"
#include <string>

namespace building {

GoldStorage* GoldStorage::create() {
    GoldStorage *pRet = new(std::nothrow) GoldStorage();
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

bool GoldStorage::init() {
    if (!Building::init()) {
        return false;
    }
    
    _type = BuildingType::GoldStorage;
    _buildingName = "Gold Storage";
    _level = 1;
    _maxHP = 800;
    _currentHP = _maxHP;
    
    updateTexture();
    
    return true;
}

void GoldStorage::updateTexture() {
    std::string textureName = "gold_storage" + std::to_string(_level) + ".png";
    this->setTexture(textureName);
}

void GoldStorage::upgrade() {
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

int GoldStorage::getUpgradeCost() const {
    if (_level == 1) return 600;
    if (_level == 2) return 1200;
    return 0; // Max level
}

bool GoldStorage::canUpgrade() const {
    return _level < 3;
}

void GoldStorage::showInfo() {
    CCLOG("Show Info for Gold Storage Level %d", _level);
}

int GoldStorage::getCapacity() const {
    if (_level == 1) return 600;
    if (_level == 2) return 1200;
    return 1800;
}

} // namespace building
