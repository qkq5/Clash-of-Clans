#pragma once
#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Building.h"
#include "Unit.h"
#include <vector>

// 掠夺模式场景：简单 AI 战斗
class BattleScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    CREATE_FUNC(BattleScene);

private:
    void setupBackground();
    void setupHUD();
    void setupEnemyBase();
    void setupBottomUI();

    void onTouchSpawn(const cocos2d::Vec2& worldPos);
    void updateBattle(float dt);
    void checkBattleResult();

    cocos2d::Label* hudLabel = nullptr;

    // 资源在本局战斗中的快照
    int battleGold = 0;
    int battleElixir = 0;
    int battleSoldiers = 0;

    // 兵种选择
    UnitType currentPick = UnitType::Barbarian;
    cocos2d::ui::Button* btnArcher = nullptr;
    cocos2d::ui::Button* btnBarbarian = nullptr;
    cocos2d::ui::Button* btnGiant = nullptr;

    // 敌方建筑与我方单位
    std::vector<Building*> enemyBuildings;
    std::vector<Unit*> playerUnits;

    // 敌方攻击计时
    float enemyAttackTimer = 0.0f;
};



