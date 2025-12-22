#ifndef __BATTLE_SCENE_H__
#define __BATTLE_SCENE_H__

#include "cocos2d.h"
#include "../soldier/Soldier.h"
#include "../building/Building.h"

namespace scene {

class BattleScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene(int level, int barbarianCount, int archerCount, int bomberCount, int giantCount);
    static BattleScene* create(int level, int barbarianCount, int archerCount, int bomberCount, int giantCount);
    
    virtual bool init(int level, int barbarianCount, int archerCount, int bomberCount, int giantCount);
    
    void update(float dt) override;

private:
    int _level;
    int _barbarianCount;
    int _archerCount;
    int _bomberCount;
    int _giantCount;
    
    cocos2d::Label* _barbarianLabel;
    cocos2d::Label* _archerLabel;
    cocos2d::Label* _bomberLabel;
    cocos2d::Label* _giantLabel;
    
    soldier::SoldierType _selectedTroop;
    bool _isDeployMode;
    
    cocos2d::Vector<building::Building*> _enemyBuildings;
    cocos2d::Vector<soldier::Soldier*> _friendlyTroops;
    cocos2d::Vector<building::Building*> _projectiles; // Using Building? No, need Sprite or Node.
    cocos2d::Vector<cocos2d::Sprite*> _bulletSprites;

    cocos2d::Node* _mapNode; 
    
    void setupLevel(int level);
    void setupUI();
    
    // Input
    bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);
    
    // Combat
    void spawnTroop(cocos2d::Vec2 position);
    void checkWinCondition();
    void showResult(bool win);
    void returnToVillage();
    
    // AI
    building::Building* findTargetForSoldier(soldier::Soldier* s);
    
    // Projectiles
    void fireProjectile(cocos2d::Vec2 start, cocos2d::Vec2 end, std::function<void()> onHit);
};

} // namespace scene

#endif // __BATTLE_SCENE_H__
