#pragma once
#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "ResourceManager.h"

class HomeScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    virtual bool init();
    void update(float dt) override;
    CREATE_FUNC(HomeScene);
private:
    void setupHUD();
    void refreshHUD();
    cocos2d::Label* hudLabel;
    class GoldMine* goldMine;
    class ElixirCollector* elixirCollector;
    class GoldStorage* goldStorage;
    class ElixirStorage* elixirStorage;
    class Barracks* barracks;
};
