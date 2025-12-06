#pragma once
#include "cocos2d.h"
#include "Building.h"
#include "ResourceManager.h"

class GoldStorage : public Building {
public:
    static GoldStorage* create(int level, const cocos2d::Vec2& pos);
    virtual void upgrade() override;
private:
    bool initWithLevel(int level, const cocos2d::Vec2& pos);
    int capacity;
};

class ElixirStorage : public Building {
public:
    static ElixirStorage* create(int level, const cocos2d::Vec2& pos);
    virtual void upgrade() override;
private:
    bool initWithLevel(int level, const cocos2d::Vec2& pos);
    int capacity;
};

