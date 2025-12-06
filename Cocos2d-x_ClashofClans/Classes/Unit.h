#pragma once
#include "cocos2d.h"

enum class UnitType { Archer, Barbarian, Bomber, Giant };

class Unit : public cocos2d::Node {
public:
    UnitType type;
    int hp;
    int attack;
    float speed;
    float range;
    static int populationCost(UnitType t){ return t==UnitType::Giant?2:1; }
    virtual void update(float dt);
    float attackInterval = 1.0f;
    class Building* target = nullptr;
    float attackTimer = 0.0f;
};

class Archer : public Unit {
public:
    static Archer* create(const cocos2d::Vec2& pos);
};

class Barbarian : public Unit {
public:
    static Barbarian* create(const cocos2d::Vec2& pos);
};

class WallBreaker : public Unit {
public:
    static WallBreaker* create(const cocos2d::Vec2& pos);
};

class Giant : public Unit {
public:
    static Giant* create(const cocos2d::Vec2& pos);
};
