#include "Mine.h"

USING_NS_CC;
using namespace ui;

static float mineRateForLevel(int lv){ return 2.f*lv; }
static int mineCapForLevel(int lv){ return 100*lv; }

GoldMine* GoldMine::create(int level, const Vec2& pos){ auto p = new GoldMine(); if(p->initWithLevel(level,pos)){ p->autorelease(); return p; } delete p; return nullptr; }

bool GoldMine::initWithLevel(int level, const Vec2& pos){ type=BuildingType::Mine; this->level=level; hp=300+level*100; productionRate=mineRateForLevel(level); capacity=mineCapForLevel(level); stored=0.f; setPosition(pos); auto sp=Sprite::create("gold_mine.png"); if(sp){ auto vs=Director::getInstance()->getVisibleSize(); float targetW=vs.width/10.f; float s=targetW/sp->getContentSize().width; sp->setScale(s); addChild(sp); } collectBtn=Button::create("CloseNormal.png","CloseSelected.png"); collectBtn->setScale(0.5f); collectBtn->setPosition(Vec2(0,30)); collectBtn->addClickEventListener([&](Ref*){ collect(); }); addChild(collectBtn,1); return true; }

void GoldMine::upgrade(){ int oldCap=capacity; level=std::min(3, level+1); productionRate=mineRateForLevel(level); capacity=mineCapForLevel(level); if(capacity<stored) stored=(float)capacity; }

void GoldMine::updateMine(float dt){ stored += productionRate*dt; if(stored>capacity) stored=(float)capacity; }

void GoldMine::collect(){ auto& rm=ResourceManager::getInstance(); int can=std::max(0, rm.getGoldMax()-rm.getGold()); int take=std::min((int)stored, can); if(take>0){ rm.addResource(ResourceType::Gold, take); stored -= take; }
}

static float collectorRateForLevel(int lv){ return 2.f*lv; }
static int collectorCapForLevel(int lv){ return 100*lv; }

ElixirCollector* ElixirCollector::create(int level, const Vec2& pos){ auto p = new ElixirCollector(); if(p->initWithLevel(level,pos)){ p->autorelease(); return p; } delete p; return nullptr; }

bool ElixirCollector::initWithLevel(int level, const Vec2& pos){ type=BuildingType::ElixirCollector; this->level=level; hp=300+level*100; productionRate=collectorRateForLevel(level); capacity=collectorCapForLevel(level); stored=0.f; setPosition(pos); auto sp=Sprite::create("elixir_collector.png"); if(sp){ auto vs=Director::getInstance()->getVisibleSize(); float targetW=vs.width/10.f; float s=targetW/sp->getContentSize().width; sp->setScale(s); addChild(sp); } collectBtn=Button::create("CloseNormal.png","CloseSelected.png"); collectBtn->setScale(0.5f); collectBtn->setPosition(Vec2(0,30)); collectBtn->addClickEventListener([&](Ref*){ collect(); }); addChild(collectBtn,1); return true; }

void ElixirCollector::upgrade(){ int oldCap=capacity; level=std::min(3, level+1); productionRate=collectorRateForLevel(level); capacity=collectorCapForLevel(level); if(capacity<stored) stored=(float)capacity; }

void ElixirCollector::updateCollector(float dt){ stored += productionRate*dt; if(stored>capacity) stored=(float)capacity; }

void ElixirCollector::collect(){ auto& rm=ResourceManager::getInstance(); int can=std::max(0, rm.getElixirMax()-rm.getElixir()); int take=std::min((int)stored, can); if(take>0){ rm.addResource(ResourceType::Elixir, take); stored -= take; }
}
