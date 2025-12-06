/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"
#include <vector>

enum class ResourceType { Gold, Elixir, Population };
enum class BuildingType { TownHall, Mine, ElixirCollector, Barracks, DefenseTower, Storage };
enum class UnitType { Archer, Barbarian, Bomber, Giant };

struct BuildingStats { int hp; int attack; float range; float prodGold; float prodElixir; int storageCapacity; };
struct UnitStats { int hp; int attack; float range; float speed; float attackInterval; };

struct GridPos { int x; int y; };

class BuildingNode : public cocos2d::Node {
public:
    BuildingType type;
    int level;
    BuildingStats stats;
    int hp;
    GridPos grid;
    static BuildingNode* create(BuildingType type, int level, const BuildingStats& s, const GridPos& g);
    void takeDamage(int d);
    bool isDestroyed() const;
};

class UnitNode : public cocos2d::Node {
public:
    UnitType type;
    UnitStats stats;
    int hp;
    cocos2d::Vec2 targetPos;
    BuildingNode* target;
    float attackTimer;
    std::vector<GridPos> path;
    static UnitNode* create(UnitType type, const UnitStats& s, const cocos2d::Vec2& pos);
};

class HelloWorld : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init();
    
    // a selector callback
    void menuCloseCallback(cocos2d::Ref* pSender);
    
    void onSpawnUnit(UnitType t, const cocos2d::Vec2& pos);
    void switchMap();
    void upgradeSelected();
    
private:
    void setupUI();
    void setupMap(int id);
    void tick(float dt);
    void updateResources(float dt);
    void updateAI(float dt);
    BuildingNode* pickTargetForUnit(UnitNode* u);
    std::vector<GridPos> findPath(const GridPos& start, const GridPos& goal);
    GridPos worldToGrid(const cocos2d::Vec2& p) const;
    cocos2d::Vec2 gridToWorld(const GridPos& g) const;
    bool isWalkable(const GridPos& g) const;
    BuildingStats getBuildingStats(BuildingType t, int level) const;
    UnitStats getUnitStats(UnitType t) const;
    void refreshHUD();
    
    int mapId;
    int gridW;
    int gridH;
    float cellSize;
    std::vector<std::vector<int>> grid;
    std::vector<BuildingNode*> buildings;
    std::vector<UnitNode*> units;
    UnitType selectedUnit;
    int gold;
    int elixir;
    int pop;
    int popCap;
    cocos2d::Label* hudLabel;

    // implement the "static create()" method manually
    CREATE_FUNC(HelloWorld);
};

#endif // __HELLOWORLD_SCENE_H__
