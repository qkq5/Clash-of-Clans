#include "Barracks.h"

USING_NS_CC;
using namespace ui;

Barracks* Barracks::create(int level, const Vec2& pos){ auto p=new Barracks(); if(p->initWithLevel(level,pos)){ p->autorelease(); return p; } delete p; return nullptr; }

bool Barracks::initWithLevel(int level, const Vec2& pos){ type=BuildingType::Barracks; this->level=level; hp=500+level*200; training=false; timer=0.f; speedFactor=(level==1?1.f:(level==2?0.8f:0.6f)); setPosition(pos); auto sp=Sprite::create("barrack.png"); if(sp){ auto vs=Director::getInstance()->getVisibleSize(); float targetW=vs.width/10.f; float s=targetW/sp->getContentSize().width; sp->setScale(s); addChild(sp); } return true; }

void Barracks::upgrade(){ level=std::min(3, level+1); speedFactor=(level==1?1.f:(level==2?0.8f:0.6f)); }

int Barracks::elixirCost(UnitType t) const{ if(t==UnitType::Barbarian) return 50; if(t==UnitType::Archer) return 50; if(t==UnitType::Bomber) return 80; if(t==UnitType::Giant) return 150; return 0; }
int Barracks::popCost(UnitType t) const{ if(t==UnitType::Giant) return 2; return 1; }
float Barracks::trainingTime(UnitType t) const{ if(t==UnitType::Barbarian) return 3.f; if(t==UnitType::Archer) return 3.5f; if(t==UnitType::Bomber) return 5.f; if(t==UnitType::Giant) return 8.f; return 1.f; }
bool Barracks::isUnlocked(UnitType t) const{
    if(level==1) return t==UnitType::Barbarian || t==UnitType::Archer;
    if(level==2) return t==UnitType::Barbarian || t==UnitType::Archer || t==UnitType::Bomber;
    if(level>=3) return true;
    return false;
}

bool Barracks::enqueue(UnitType t){ if(!isUnlocked(t)) return false; Cost c{0, elixirCost(t), popCost(t)}; auto& rm=ResourceManager::getInstance(); if(!rm.canAfford(c)) return false; if(!rm.spend(c)) return false; q.push(t); if(!training) startNext(); return true; }

void Barracks::startNext(){ if(q.empty()){ training=false; return; } current=q.front(); q.pop(); training=true; timer=trainingTime(current)*speedFactor; }

void Barracks::updateBarracks(float dt){ if(!training) return; timer-=dt; if(timer<=0.f){ training=false; if(onTrained) onTrained(current); startNext(); } }

void Barracks::setOnUnitTrained(std::function<void(UnitType)> cb){ onTrained=cb; }
