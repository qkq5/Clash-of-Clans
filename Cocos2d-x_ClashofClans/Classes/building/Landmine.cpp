#include "Landmine.h"

namespace building {

Landmine* Landmine::create() {
    Landmine* ret = new (std::nothrow) Landmine();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool Landmine::init() {
    if (!Building::init()) {
        return false;
    }

    _type = BuildingType::Landmine;
    _buildingName = "Landmine";
    _maxHP = 1; 
    _currentHP = _maxHP;
    _level = 1;
    _gridSize = cocos2d::Size(2, 2);
    _isTriggered = false;
    
    _attackDamage = 500;
    _attackRange = 3.0f * 32.0f; // 3 tiles
    
    if (!this->initWithFile("landmine.png")) {
        this->setTexture("landmine.png");
    }

    return true;
}

void Landmine::upgrade() {
}

int Landmine::getUpgradeCost() const {
    return 0;
}

bool Landmine::canUpgrade() const {
    return false;
}

void Landmine::showInfo() {
    cocos2d::log("Landmine: Damage 500, Range 3");
}

void Landmine::startTriggerSequence(std::function<void()> onExplode) {
    if (_isTriggered) return;
    _isTriggered = true;
    
    // Ensure visible
    this->setVisible(true);
    
    // Blink Red for 0.5s
    auto blink = cocos2d::Blink::create(0.5f, 5);
    auto callback = cocos2d::CallFunc::create([this, onExplode]() {
        if (onExplode) onExplode();
        this->removeFromParent();
    });
    
    this->runAction(cocos2d::Sequence::create(blink, callback, nullptr));
}

} // namespace building
