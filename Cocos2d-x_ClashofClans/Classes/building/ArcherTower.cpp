#include "ArcherTower.h"

namespace building {

ArcherTower* ArcherTower::create() {
    ArcherTower* ret = new (std::nothrow) ArcherTower();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool ArcherTower::init() {
    if (!Building::init()) {
        return false;
    }

    _type = BuildingType::ArcherTower;
    _buildingName = "Archer Tower";
    _level = 1;
    _maxHP = 200;
    _currentHP = _maxHP;
    
    // Combat
    _attackRange = 6.0f * 32.0f; // 6 tiles
    _attackInterval = 1.0f;
    _attackDamage = 45; // Level 1

    updateTexture();
    return true;
}

void ArcherTower::updateTexture() {
    std::string filename = "archer_tower" + std::to_string(_level) + ".png";
    this->setTexture(filename);
}

void ArcherTower::upgrade() {
    if (!canUpgrade()) return;

    _level++;
    
    if (_level == 2) {
        _maxHP = 250;
        _attackDamage = 90;
    } else if (_level == 3) {
        _maxHP = 300;
        _attackDamage = 130;
    }
    
    _currentHP = _maxHP; // Heal on upgrade? Usually yes in simple implementations
    updateTexture();
}

int ArcherTower::getUpgradeCost() const {
    if (_level == 1) return 300;
    if (_level == 2) return 350;
    return 0;
}

bool ArcherTower::canUpgrade() const {
    return _level < 3;
}

void ArcherTower::showInfo() {
    // This will be handled by VillageScene, calling getters
}

} // namespace building
