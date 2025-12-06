#pragma once
#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Building.h"
#include "ResourceManager.h"
#include "Unit.h"
#include <queue>
#include <functional>

class Barracks : public Building {
public:
    static Barracks* create(int level, const cocos2d::Vec2& pos);
    virtual void upgrade() override;
    void updateBarracks(float dt);
    bool enqueue(UnitType t);
    void setOnUnitTrained(std::function<void(UnitType)> cb);
private:
    bool initWithLevel(int level, const cocos2d::Vec2& pos);
    bool isUnlocked(UnitType t) const;
    float trainingTime(UnitType t) const;
    int elixirCost(UnitType t) const;
    int popCost(UnitType t) const;
    void startNext();
    std::queue<UnitType> q;
    bool training;
    UnitType current;
    float timer;
    float speedFactor;
    std::function<void(UnitType)> onTrained;
};
