#include "Wall.h"

namespace building {

Wall* Wall::create() {
    Wall* ret = new (std::nothrow) Wall();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool Wall::init() {
    if (!Building::init()) {
        return false;
    }

    _type = BuildingType::Wall;
    _buildingName = "Wall";
    _level = 1;
    _maxHP = 300;
    _currentHP = _maxHP;

    updateTexture();
    return true;
}

void Wall::updateTexture() {
    std::string filename = "wall" + std::to_string(_level) + ".png";
    this->setTexture(filename);
}

void Wall::upgrade() {
    if (!canUpgrade()) return;

    _level++;
    
    if (_level == 2) {
        _maxHP = 500;
    } else if (_level == 3) {
        _maxHP = 700;
    }
    
    _currentHP = _maxHP;
    updateTexture();
}

int Wall::getUpgradeCost() const {
    if (_level == 1) return 1000;
    if (_level == 2) return 2000;
    return 0;
}

bool Wall::canUpgrade() const {
    return _level < 3;
}

void Wall::showInfo() {
    // Handled by VillageScene
}

} // namespace building
