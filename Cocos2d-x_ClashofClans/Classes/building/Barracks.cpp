#include "Barracks.h"

namespace building {

Barracks* Barracks::create() {
    Barracks* ret = new (std::nothrow) Barracks();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool Barracks::init() {
    if (!Building::init()) {
        return false;
    }

    _type = BuildingType::Barracks;
    _buildingName = "Barracks";
    _level = 1;
    _maxHP = 100;
    _currentHP = _maxHP;

    updateTexture();
    return true;
}

void Barracks::updateTexture() {
    std::string filename = "barracks" + std::to_string(_level) + ".png";
    this->setTexture(filename);
}

void Barracks::upgrade() {
    if (!canUpgrade()) return;

    _level++;
    
    if (_level == 2) {
        _maxHP = 150;
    } else if (_level == 3) {
        _maxHP = 200;
    }
    
    _currentHP = _maxHP;
    updateTexture();
}

int Barracks::getUpgradeCost() const {
    if (_level == 1) return 300;
    if (_level == 2) return 350;
    return 0;
}

bool Barracks::canUpgrade() const {
    return _level < 3;
}

void Barracks::showInfo() {
    // Handled by VillageScene
}

int Barracks::getCapacity() const {
    if (_level == 1) return 10;
    if (_level == 2) return 15;
    return 20;
}

} // namespace building
