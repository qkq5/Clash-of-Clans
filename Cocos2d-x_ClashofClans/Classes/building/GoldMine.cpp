#include "GoldMine.h"
#include <string>

namespace building {

GoldMine* GoldMine::create() {
    GoldMine *pRet = new(std::nothrow) GoldMine();
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

bool GoldMine::init() {
    if (!Building::init()) {
        return false;
    }
    
    _type = BuildingType::GoldMine;
    _buildingName = "Gold Mine";
    _level = 1;
    _maxHP = 400;
    _currentHP = _maxHP;
    
    updateTexture();
    
    return true;
}

void GoldMine::updateTexture() {
    std::string textureName = "gold_mine" + std::to_string(_level) + ".png";
    this->setTexture(textureName);
}

void GoldMine::upgrade() {
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

int GoldMine::getUpgradeCost() const {
    if (_level == 1) return 300;
    if (_level == 2) return 600;
    return 0; // Max level
}

bool GoldMine::canUpgrade() const {
    return _level < 3;
}

void GoldMine::showInfo() {
    // This will be handled by the Scene calling this object's getters
    // Or we can emit an event. For now, we rely on Scene to query.
    CCLOG("Show Info for Gold Mine Level %d", _level);
}

} // namespace building
