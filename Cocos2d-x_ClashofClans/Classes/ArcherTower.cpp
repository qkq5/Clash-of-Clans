#include "ArcherTower.h"

USING_NS_CC;

ArcherTower* ArcherTower::create(int level, const Vec2& pos){ auto p=new ArcherTower(); if(p->initWithLevel(level,pos)){ p->autorelease(); return p; } delete p; return nullptr; }

bool ArcherTower::initWithLevel(int level, const Vec2& pos){ type=BuildingType::DefenseTower; this->level=level; hp=600+level*200; setPosition(pos); auto n=DrawNode::create(); n->drawSolidRect(Vec2(-20,-20),Vec2(20,20), Color4F::RED); addChild(n); return true; }

void ArcherTower::upgrade(){ level=std::min(3, level+1); hp=600+level*200; }

