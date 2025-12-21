#ifndef __VILLAGE_SCENE_H__
#define __VILLAGE_SCENE_H__

#include "cocos2d.h"
#include "core/Grid.h"
#include "building/Building.h"
#include <vector>

namespace scene {

class VillageScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
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

    // Map properties
    cocos2d::Node* _mapNode;
    core::Grid<cocos2d::Sprite*>* _mapGrid; // Use pointer to manage lifetime or use shared_ptr if preferred
    
    // Buildings
    std::vector<building::Building*> _buildings;

    // UI Elements
    cocos2d::Label* _goldLabel;
    cocos2d::Label* _elixirLabel;
    cocos2d::Label* _populationLabel;
    
    // Resources
    int _gold;
    int _elixir;
    int _population;

    // Touch handling
    cocos2d::Vec2 _touchStartPos;
    cocos2d::Vec2 _mapStartPos;
    bool _isDragging;
};

} // namespace scene

#endif // __VILLAGE_SCENE_H__
