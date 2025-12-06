#pragma once
#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Building.h"
#include "Unit.h"
#include "ResourceManager.h"
#include <vector>

class Building;

enum class BuildPick { None, GoldMine, ElixirCollector, Barracks };

class GameScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    virtual bool init();
    CREATE_FUNC(GameScene);
    const std::vector<Building*>& getBuildings() const;
    void addUnit(Unit* u);
    void addBuilding(Building* b);
    void update(float dt) override;
private:
    void setupBackground();
    void setupHUD();
    void setupBottomUI();
    void updateHighlight();
    bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);
    cocos2d::ui::Layout* bottomBar;
    cocos2d::ui::Button* battleBtn;
    cocos2d::ui::Button* btnGM;
    cocos2d::ui::Button* btnEC;
    cocos2d::ui::Button* btnBK;
    BuildPick pick;
    std::vector<Building*> buildings;
    std::vector<Unit*> units;
    cocos2d::Label* resourceLabel;
    float goldTick;
    float elixirTick;
    float soldierTick;
};
