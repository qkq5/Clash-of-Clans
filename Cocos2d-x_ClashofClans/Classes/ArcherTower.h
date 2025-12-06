#pragma once
#include "cocos2d.h"
#include "Building.h"

class ArcherTower : public Building {
public:
    static ArcherTower* create(int level, const cocos2d::Vec2& pos);
    virtual void upgrade() override;
private:
    bool initWithLevel(int level, const cocos2d::Vec2& pos);
};

