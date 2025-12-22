#ifndef __VILLAGE_SCENE_H__
#define __VILLAGE_SCENE_H__

#include "cocos2d.h"
#include "core/Grid.h"
#include "building/Building.h"
#include "building/TownHall.h"
#include "building/GoldMine.h"
#include "building/ElixirCollector.h"
#include "building/GoldStorage.h"
#include "building/ElixirStorage.h"
#include <vector>

namespace scene {

class VillageScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    
    // Implement the "static create()" method manually
    CREATE_FUNC(VillageScene);

private:
    void setupMap();
    void setupUI();
    void setupBuildings();
    void setupTouchHandling();

    // UI Callbacks
    void onAttackModeCallback(cocos2d::Ref* pSender);
    void onBuildModeCallback(cocos2d::Ref* pSender);
    void onCloseWindowCallback(cocos2d::Ref* pSender);
    void onLevelSelectCallback(cocos2d::Ref* pSender, int level); // 1=Simple, 2=Medium, 3=Hard
    void onBuildingTypeSelected(cocos2d::Ref* pSender, building::BuildingType type);
    void onUpgradeCallback(cocos2d::Ref* pSender, building::Building* building);

    // Helpers
    void showLevelSelectWindow();
    void showBuildWindow();
    void showBuildingInfo(building::Building* building);
    void closeCurrentWindow();
    bool trySpendResources(int gold, int elixir);
    void updateResourceLabels();
    void placeBuilding(building::BuildingType type, const cocos2d::Vec2& gridPos);

    // Member variables
    cocos2d::Node* _mapNode;
    core::Grid<cocos2d::Sprite*>* _mapGrid;
    std::vector<building::Building*> _buildings;
    
    cocos2d::Layer* _uiLayer;
    cocos2d::Node* _currentWindow; // Currently open window (Level Select, Build, Info)
    
    cocos2d::Label* _goldLabel;
    cocos2d::Label* _elixirLabel;
    cocos2d::Label* _populationLabel;

    int _gold;
    int _elixir;
    int _population;

    // Touch handling
    cocos2d::Vec2 _touchStartPos;
    cocos2d::Vec2 _mapStartPos;
    bool _isDragging;
    
    // Build Mode State
    bool _isBuildMode;
    building::BuildingType _pendingBuildingType;
};

} // namespace scene

#endif // __VILLAGE_SCENE_H__
