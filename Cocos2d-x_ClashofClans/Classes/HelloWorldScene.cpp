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

#include "HelloWorldScene.h"
#include "audio/include/AudioEngine.h"
#include <queue>
#include <algorithm>
#include <cstdio>
#include <utility>

USING_NS_CC;

using namespace cocos2d::experimental;

Scene* HelloWorld::createScene()
{
    return HelloWorld::create();
}

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}

// on "init" you need to initialize your instance
static int sgn(int v){ return v<0?-1:(v>0?1:0); }

BuildingNode* BuildingNode::create(BuildingType type, int level, const BuildingStats& s, const GridPos& g){
    auto n = new BuildingNode();
    n->type = type;
    n->level = level;
    n->stats = s;
    n->hp = s.hp;
    n->grid = g;
    n->init();
    n->autorelease();
    return n;
}

void BuildingNode::takeDamage(int d){ hp = std::max(0, hp - d); }
bool BuildingNode::isDestroyed() const{ return hp <= 0; }

UnitNode* UnitNode::create(UnitType type, const UnitStats& s, const Vec2& pos){
    auto n = new UnitNode();
    n->type = type;
    n->stats = s;
    n->hp = s.hp;
    n->attackTimer = 0.f;
    n->target = nullptr;
    n->setPosition(pos);
    n->init();
    n->autorelease();
    return n;
}

bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Scene::init() )
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    selectedUnit = UnitType::Barbarian;
    mapId = 1;
    gridW = 20;
    gridH = 15;
    cellSize = 32.f;
    gold = 200;
    elixir = 200;
    pop = 0;
    popCap = 10;
    setupUI();
    setupMap(mapId);
    AudioEngine::play2d("bgm.mp3", true, 0.5f);
    auto listener = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = [&](Touch* t, Event*){
        onSpawnUnit(selectedUnit, t->getLocation());
        return true;
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
    auto kb = EventListenerKeyboard::create();
    kb->onKeyPressed = [&](EventKeyboard::KeyCode k, Event*){
        if(k==EventKeyboard::KeyCode::KEY_1) selectedUnit=UnitType::Barbarian;
        if(k==EventKeyboard::KeyCode::KEY_2) selectedUnit=UnitType::Archer;
        if(k==EventKeyboard::KeyCode::KEY_3) selectedUnit=UnitType::Bomber;
        if(k==EventKeyboard::KeyCode::KEY_4) selectedUnit=UnitType::Giant;
        if(k==EventKeyboard::KeyCode::KEY_M) switchMap();
        if(k==EventKeyboard::KeyCode::KEY_U) upgradeSelected();
        refreshHUD();
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(kb, this);
    schedule(CC_SCHEDULE_SELECTOR(HelloWorld::tick));
    return true;
}


void HelloWorld::menuCloseCallback(Ref* pSender)
{
    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();

    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() as given above,instead trigger a custom event created in RootViewController.mm as below*/

    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);


}

void HelloWorld::setupUI(){
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    hudLabel = Label::createWithTTF("", "fonts/Marker Felt.ttf", 18);
    hudLabel->setAnchorPoint(Vec2(0,1));
    hudLabel->setPosition(Vec2(origin.x+10, origin.y+visibleSize.height-10));
    addChild(hudLabel, 10);
    auto closeItem = MenuItemImage::create("CloseNormal.png","CloseSelected.png",CC_CALLBACK_1(HelloWorld::menuCloseCallback,this));
    float x = origin.x + visibleSize.width - closeItem->getContentSize().width/2;
    float y = origin.y + closeItem->getContentSize().height/2;
    closeItem->setPosition(Vec2(x,y));
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    addChild(menu, 1);
    refreshHUD();
}

BuildingStats HelloWorld::getBuildingStats(BuildingType t, int level) const{
    int lv = std::max(1, std::min(3, level));
    BuildingStats s{};
    if(t==BuildingType::TownHall){ s.hp=1000+lv*500; }
    if(t==BuildingType::Mine){ s.hp=300+lv*100; s.prodGold=1.f*lv; }
    if(t==BuildingType::ElixirCollector){ s.hp=300+lv*100; s.prodElixir=1.f*lv; }
    if(t==BuildingType::Barracks){ s.hp=500+lv*200; }
    if(t==BuildingType::DefenseTower){ s.hp=600+lv*200; s.attack=30+lv*10; s.range=128.f+lv*16; }
    if(t==BuildingType::Storage){ s.hp=400+lv*150; s.storageCapacity=500*lv; }
    return s;
}

UnitStats HelloWorld::getUnitStats(UnitType t) const{
    UnitStats s{};
    if(t==UnitType::Barbarian){ s.hp=120; s.attack=20; s.range=24.f; s.speed=90.f; s.attackInterval=0.6f; }
    if(t==UnitType::Archer){ s.hp=80; s.attack=15; s.range=120.f; s.speed=100.f; s.attackInterval=0.7f; }
    if(t==UnitType::Bomber){ s.hp=70; s.attack=60; s.range=40.f; s.speed=80.f; s.attackInterval=1.2f; }
    if(t==UnitType::Giant){ s.hp=400; s.attack=25; s.range=28.f; s.speed=60.f; s.attackInterval=1.0f; }
    return s;
}

void HelloWorld::setupMap(int id){
    for(auto b:buildings) removeChild(b);
    buildings.clear();
    grid.assign(gridH, std::vector<int>(gridW, 0));
    std::vector<std::pair<BuildingType, GridPos>> layout;
    if(id==1){
        layout={{BuildingType::TownHall,{10,7}},{BuildingType::Mine,{6,6}},{BuildingType::ElixirCollector,{14,6}},{BuildingType::Barracks,{6,9}},{BuildingType::DefenseTower,{12,8}},{BuildingType::Storage,{8,6}}};
    } else {
        layout={{BuildingType::TownHall,{4,7}},{BuildingType::Mine,{2,5}},{BuildingType::ElixirCollector,{6,5}},{BuildingType::Barracks,{3,10}},{BuildingType::DefenseTower,{7,8}},{BuildingType::DefenseTower,{10,6}},{BuildingType::Storage,{5,6}}};
    }
    for(auto& p:layout){
        auto s = getBuildingStats(p.first, 1);
        auto b = BuildingNode::create(p.first, 1, s, p.second);
        auto pos = gridToWorld(p.second);
        auto n = DrawNode::create();
        if(p.first==BuildingType::DefenseTower) n->drawSolidRect(Vec2(-cellSize/2,-cellSize/2), Vec2(cellSize/2,cellSize/2), Color4F::RED);
        else n->drawSolidRect(Vec2(-cellSize/2,-cellSize/2), Vec2(cellSize/2,cellSize/2), Color4F::GRAY);
        b->addChild(n);
        addChild(b);
        b->setPosition(pos);
        buildings.push_back(b);
        grid[p.second.y][p.second.x]=1;
    }
}

void HelloWorld::tick(float dt){
    updateResources(dt);
    updateAI(dt);
    refreshHUD();
    bool allDestroyed=true;
    for(auto b:buildings){ if(!b->isDestroyed() && b->type!=BuildingType::Mine && b->type!=BuildingType::ElixirCollector && b->type!=BuildingType::Storage) allDestroyed=false; }
    if(allDestroyed){ unschedule(CC_SCHEDULE_SELECTOR(HelloWorld::tick)); }
}

void HelloWorld::updateResources(float dt){
    float g=0,e=0; int sc=0;
    for(auto b:buildings){ if(b->isDestroyed()) continue; g+=b->stats.prodGold*dt; e+=b->stats.prodElixir*dt; sc+=b->stats.storageCapacity; }
    gold = std::min(gold + (int)g, std::max(500, sc));
    elixir = std::min(elixir + (int)e, std::max(500, sc));
}

bool HelloWorld::isWalkable(const GridPos& g) const{
    if(g.x<0||g.y<0||g.x>=gridW||g.y>=gridH) return false;
    return grid[g.y][g.x]==0;
}

GridPos HelloWorld::worldToGrid(const Vec2& p) const{
    int x = (int)(p.x/cellSize);
    int y = (int)(p.y/cellSize);
    return {x,y};
}

Vec2 HelloWorld::gridToWorld(const GridPos& g) const{
    return Vec2((g.x+0.5f)*cellSize,(g.y+0.5f)*cellSize);
}

std::vector<GridPos> HelloWorld::findPath(const GridPos& start, const GridPos& goal){
    std::queue<GridPos> q;
    std::vector<std::vector<int>> vis(gridH, std::vector<int>(gridW,-1));
    std::vector<GridPos> dirs={{1,0},{-1,0},{0,1},{0,-1}};
    q.push(start); vis[start.y][start.x]=start.y*gridW+start.x;
    while(!q.empty()){
        auto c=q.front();q.pop();
        if(c.x==goal.x && c.y==goal.y) break;
        for(auto d:dirs){ GridPos n{c.x+d.x,c.y+d.y}; if(isWalkable(n) && vis[n.y][n.x]==-1){ vis[n.y][n.x]=c.y*gridW+c.x; q.push(n);} }
    }
    std::vector<GridPos> path;
    if(vis[goal.y][goal.x]==-1){ return path; }
    GridPos cur=goal;
    while(!(cur.x==start.x && cur.y==start.y)){
        path.push_back(cur);
        int idx=vis[cur.y][cur.x];
        cur={idx%gridW, idx/gridW};
    }
    std::reverse(path.begin(), path.end());
    return path;
}

BuildingNode* HelloWorld::pickTargetForUnit(UnitNode* u){
    BuildingNode* best=nullptr;
    float bestDist=1e9f;
    for(auto b:buildings){ if(b->isDestroyed()) continue; if(b->type==BuildingType::DefenseTower){ float d=u->getPosition().distance(b->getPosition()); if(d<bestDist){ bestDist=d; best=b; } } }
    if(!best){ for(auto b:buildings){ if(b->isDestroyed()) continue; float d=u->getPosition().distance(b->getPosition()); if(d<bestDist){ bestDist=d; best=b; } } }
    return best;
}

void HelloWorld::updateAI(float dt){
    for(auto u:units){
        if(u->hp<=0) continue;
        if(!u->target || u->target->isDestroyed()){
            u->target = pickTargetForUnit(u);
            if(u->target){ auto s=worldToGrid(u->getPosition()); auto g=worldToGrid(u->target->getPosition()); u->path = findPath(s,g); }
        }
        if(u->target){
            float dist = u->getPosition().distance(u->target->getPosition());
            if(dist>u->stats.range){
                if(!u->path.empty()){
                    auto gp=u->path.front();
                    auto wp=gridToWorld(gp);
                    auto dir=(wp-u->getPosition());
                    float len=dir.length();
                    if(len>1e-3f){ dir.normalize(); u->setPosition(u->getPosition()+dir*u->stats.speed*dt); }
                    if(len<8.f) u->path.erase(u->path.begin());
                } else {
                    auto dir=(u->target->getPosition()-u->getPosition()); dir.normalize(); u->setPosition(u->getPosition()+dir*u->stats.speed*dt);
                }
            } else {
                u->attackTimer += dt;
                if(u->attackTimer>=u->stats.attackInterval){
                    u->attackTimer=0;
                    u->target->takeDamage(u->stats.attack);
                    AudioEngine::play2d("hit.wav", false, 0.7f);
                }
            }
        }
    }
    for(auto b:buildings){ if(b->isDestroyed()) continue; if(b->type==BuildingType::DefenseTower){ for(auto u:units){ if(u->hp<=0) continue; float d=u->getPosition().distance(b->getPosition()); if(d<=b->stats.range){ u->hp=std::max(0, u->hp-b->stats.attack); } } } }
    std::vector<UnitNode*> alive; for(auto u:units){ if(u->hp>0) alive.push_back(u); else removeChild(u); }
    units.swap(alive);
}

void HelloWorld::onSpawnUnit(UnitType t, const Vec2& pos){
    bool hasBarracks=false; for(auto b:buildings){ if(!b->isDestroyed() && b->type==BuildingType::Barracks) { hasBarracks=true; break; } }
    if(!hasBarracks) return;
    if(pop>=popCap) return;
    auto s = getUnitStats(t);
    int costG=0,costE=0, popUse=1;
    if(t==UnitType::Barbarian){ costG=50; }
    if(t==UnitType::Archer){ costE=50; }
    if(t==UnitType::Bomber){ costE=80; }
    if(t==UnitType::Giant){ costG=150; popUse=2; }
    if(gold<costG || elixir<costE) return;
    gold-=costG; elixir-=costE; pop+=popUse;
    auto u = UnitNode::create(t, s, pos);
    auto n = DrawNode::create(); n->drawSolidCircle(Vec2::ZERO, cellSize*0.3f, 0, 24, Color4F::GREEN); u->addChild(n);
    addChild(u);
    units.push_back(u);
}

void HelloWorld::switchMap(){ mapId = (mapId==1?2:1); setupMap(mapId); }

void HelloWorld::upgradeSelected(){ for(auto b:buildings){ if(!b->isDestroyed()){ b->level=std::min(3,b->level+1); b->stats=getBuildingStats(b->type,b->level); } } }

void HelloWorld::refreshHUD(){ char buf[256]; sprintf(buf,"Gold:%d  Elixir:%d  Pop:%d/%d  Unit[1-4] M:Map U:Upgrade",gold,elixir,pop,popCap); hudLabel->setString(buf); }
