#include "Cannon.h"

namespace building {

Cannon* Cannon::create() {
    Cannon* ret = new (std::nothrow) Cannon();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool Cannon::init() {
    if (!Building::init()) {
        return false;
    }

    _type = BuildingType::Cannon;
    _buildingName = "Cannon";
    _level = 1;
    _maxHP = 300;
    _currentHP = _maxHP;
    
    // Combat
    _attackRange = 6.0f * 32.0f; // 6 tiles
    _attackInterval = 1.0f;
    _attackDamage = 70; // Level 1

    updateTexture();
    return true;
}

void Cannon::updateTexture() {
    std::string filename = "cannon" + std::to_string(_level) + ".png";
    this->setTexture(filename);
}

void Cannon::upgrade() {
    if (!canUpgrade()) return;

    _level++;
    
    if (_level == 2) {
        _maxHP = 350;
        _attackDamage = 140;
    } else if (_level == 3) {
        _maxHP = 400;
        _attackDamage = 210;
    }
    
    _currentHP = _maxHP;
    updateTexture();
}

int Cannon::getUpgradeCost() const {
    if (_level == 1) return 300;
    if (_level == 2) return 350;
    return 0;
}

bool Cannon::canUpgrade() const {
    return _level < 3;
}

void Cannon::showInfo() {
    // Handled by VillageScene
}

} // namespace building
