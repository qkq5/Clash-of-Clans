#include "TrainingCamp.h"

namespace building {

TrainingCamp* TrainingCamp::create() {
    TrainingCamp* ret = new (std::nothrow) TrainingCamp();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool TrainingCamp::init() {
    if (!Building::init()) {
        return false;
    }

    _type = BuildingType::TrainingCamp;
    _buildingName = "Training Camp";
    _level = 1;
    _maxHP = 200;
    _currentHP = _maxHP;

    updateTexture();
    return true;
}

void TrainingCamp::updateTexture() {
    std::string filename = "training_camp" + std::to_string(_level) + ".png";
    this->setTexture(filename);
}

void TrainingCamp::upgrade() {
    if (!canUpgrade()) return;

    _level++;
    
    if (_level == 2) {
        _maxHP = 250;
    } else if (_level == 3) {
        _maxHP = 300;
    }
    
    _currentHP = _maxHP;
    updateTexture();
}

int TrainingCamp::getUpgradeCost() const {
    if (_level == 1) return 300;
    if (_level == 2) return 350;
    return 0;
}

bool TrainingCamp::canUpgrade() const {
    return _level < 3;
}

void TrainingCamp::showInfo() {
    // Handled by VillageScene
}

std::vector<std::string> TrainingCamp::getUnlockableSoldiers() const {
    std::vector<std::string> soldiers;
    soldiers.push_back("Barbarian");
    if (_level >= 2) soldiers.push_back("Archer");
    if (_level >= 3) {
        soldiers.push_back("WallBreaker");
        soldiers.push_back("Giant");
    }
    return soldiers;
}

} // namespace building
