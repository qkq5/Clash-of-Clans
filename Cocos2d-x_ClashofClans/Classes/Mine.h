#pragma once
#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Building.h"
#include "ResourceManager.h"

class GoldMine : public Building {
public:
    static GoldMine* create(int level, const cocos2d::Vec2& pos);
    virtual void upgrade() override;
    void updateMine(float dt);
    void collect(); // 改为public，允许外部调用自动收集
private:
    bool initWithLevel(int level, const cocos2d::Vec2& pos);
    float productionRate;
    int capacity;
    float stored;
    cocos2d::ui::Button* collectBtn;
};

class ElixirCollector : public Building {
public:
    static ElixirCollector* create(int level, const cocos2d::Vec2& pos);
    virtual void upgrade() override;
    void updateCollector(float dt);
    void collect(); // 改为public，允许外部调用自动收集
private:
    bool initWithLevel(int level, const cocos2d::Vec2& pos);
    float productionRate;
    int capacity;
    float stored;
    cocos2d::ui::Button* collectBtn;
};

