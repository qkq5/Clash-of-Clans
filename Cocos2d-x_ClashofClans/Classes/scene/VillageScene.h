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
#include "building/ArcherTower.h"
#include "building/Cannon.h"
#include "building/Barracks.h"
#include "building/TrainingCamp.h"
#include <vector>

namespace scene {

class VillageScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    virtual void update(float dt) override; // Added update loop
    
    // Implement the "static create()" method manually
    CREATE_FUNC(VillageScene);

private:
    void setupMap();
    void setupUI();
    void setupBuildings();
    void setupTouchHandling();

    // UI Callbacks
    void onAttackModeCallback(cocos2d::Ref* pSender);
    void onHistoryCallback(cocos2d::Ref* pSender); // Added
    void onBuildModeCallback(cocos2d::Ref* pSender);
    void onCloseWindowCallback(cocos2d::Ref* pSender);
    void onLevelSelectCallback(cocos2d::Ref* pSender, int level); // 1=Simple, 2=Medium, 3=Hard
    void onHistoryItemCallback(cocos2d::Ref* pSender, int index); // Added
    void onBuildingTypeSelected(cocos2d::Ref* pSender, building::BuildingType type);
    void onUpgradeCallback(cocos2d::Ref* pSender, building::Building* building);
    void onTrainSoldierCallback(cocos2d::Ref* pSender, building::TrainingCamp* camp, std::string soldierName);

    // Helpers
    void showLevelSelectWindow();
    void showHistoryWindow(); // Added
    void showBuildWindow();
    void showBuildingInfo(building::Building* building);
    void showTrainingWindow(building::TrainingCamp* camp);
    void closeCurrentWindow();
    bool trySpendResources(int gold, int elixir);
    void updateResourceLabels();
    void placeBuilding(building::BuildingType type, const cocos2d::Vec2& gridPos);
    
    // Logic Helpers
    int getTownHallLevel() const;
    int getBuildingCount(building::BuildingType type) const;
    int getMaxBuildingCount(building::BuildingType type, int townHallLevel) const;
    int getResourceCapacity(bool isGold) const;
    int getTroopCapacity() const;
    int getCurrentTroopCount() const;

    // Member variables
    cocos2d::Node* _mapNode;
    core::Grid<cocos2d::Sprite*>* _mapGrid;
    std::vector<building::Building*> _buildings;
    
    cocos2d::Layer* _uiLayer;
    cocos2d::Node* _currentWindow; // Currently open window (Level Select, Build, Info)
    
    cocos2d::Label* _goldLabel;
    cocos2d::Label* _elixirLabel;
    cocos2d::Label* _populationLabel;

    double _gold; // Changed to double for smoother accumulation, display as int
    double _elixir;
    int _population; // Builders? Or separate concept? Requirement says "population", image "people.png". Assuming builders/citizens for now.
    // Wait, D1 says Barracks provide "Troop Capacity". 
    // "Population" in top right usually refers to Builders in similar games, or maybe available troops? 
    // Requirement: "右上角：展示金币 / 圣水 / 人口数量... 人口的图片为“people.png”"
    // Requirement D1: "Barracks... provide troop capacity limit... consume troop count"
    // Usually Population = Troops. Let's track troops here.
    
    // Troops storage (Simple count for now, or map of types)
    std::map<std::string, int> _troops;

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
