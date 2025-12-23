#ifndef __BATTLE_SCENE_H__
#define __BATTLE_SCENE_H__

#include "cocos2d.h"
#include "../soldier/Soldier.h"
#include "../building/Building.h"
#include <vector>
#include <string>

namespace scene {

struct DeploymentEvent {
    float time;
    int soldierType; // Cast from soldier::SoldierType
    float x, y;
};

struct BattleRecord {
    int id;
    int level;
    int initBarb;
    int initArch;
    int initBomb;
    int initGiant;
    std::vector<DeploymentEvent> events;
    bool isWin;
    std::string timestamp;
};

class BattleScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene(int level, int barbarianCount, int archerCount, int bomberCount, int giantCount);
    static BattleScene* create(int level, int barbarianCount, int archerCount, int bomberCount, int giantCount);
    
    // Replay
    static cocos2d::Scene* createReplayScene(const BattleRecord& record);
    static BattleScene* createReplay(const BattleRecord& record);
    static std::vector<BattleRecord> s_battleHistory;

    virtual bool init(int level, int barbarianCount, int archerCount, int bomberCount, int giantCount);
    virtual bool initReplay(const BattleRecord& record);
    
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
    cocos2d::Label* _totalTroopLabel;
    cocos2d::Label* _debugLabel;
    
    cocos2d::Node* _troopSelectionNode;

    soldier::SoldierType _selectedTroop;
    bool _isDeployMode;
    
    cocos2d::Vector<building::Building*> _enemyBuildings;
    cocos2d::Vector<soldier::Soldier*> _friendlyTroops;
    cocos2d::Vector<building::Building*> _projectiles; // Using Building? No, need Sprite or Node.
    cocos2d::Vector<cocos2d::Sprite*> _bulletSprites;

    cocos2d::Node* _mapNode; 
    
    void setupLevel(int level);
    void setupUI();
    void updateTotalTroopsUI();
    
    // Input
    bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);
    void onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event);
    void onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event);
    
    // Combat
    void spawnTroop(cocos2d::Vec2 position);
    void checkWinCondition();
    void showResult(bool win);
    void returnToVillage();
    
    // AI
    building::Building* findTargetForSoldier(soldier::Soldier* s);
    
    // Projectiles
    void fireProjectile(cocos2d::Vec2 start, cocos2d::Vec2 end, std::function<void()> onHit);

private:
    // Replay / Recording
    bool _isReplay;
    BattleRecord _replayRecord; // The record being played back
    int _replayEventIndex;
    float _battleTimer;
    
    // Current recording
    BattleRecord _currentRecord; 
};

} // namespace scene

#endif // __BATTLE_SCENE_H__
