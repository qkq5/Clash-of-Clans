#include "StorageBuilding.h"

USING_NS_CC;

static int storageCapForLevel(int lv){ return 500*lv; }

GoldStorage* GoldStorage::create(int level, const Vec2& pos){ auto p=new GoldStorage(); if(p->initWithLevel(level,pos)){ p->autorelease(); return p; } delete p; return nullptr; }

bool GoldStorage::initWithLevel(int level, const Vec2& pos){ type=BuildingType::Storage; this->level=level; hp=400+level*150; capacity=storageCapForLevel(level); setPosition(pos); auto n=DrawNode::create(); n->drawSolidRect(Vec2(-22,-22),Vec2(22,22),Color4F(0.8f,0.6f,0.2f,1.f)); addChild(n); ResourceManager::getInstance().onStorageCapacityChanged(ResourceType::Gold,0,capacity); return true; }

void GoldStorage::upgrade(){ int old=capacity; level=std::min(3, level+1); capacity=storageCapForLevel(level); ResourceManager::getInstance().onStorageCapacityChanged(ResourceType::Gold,old,capacity); }

ElixirStorage* ElixirStorage::create(int level, const Vec2& pos){ auto p=new ElixirStorage(); if(p->initWithLevel(level,pos)){ p->autorelease(); return p; } delete p; return nullptr; }

bool ElixirStorage::initWithLevel(int level, const Vec2& pos){ type=BuildingType::Storage; this->level=level; hp=400+level*150; capacity=storageCapForLevel(level); setPosition(pos); auto n=DrawNode::create(); n->drawSolidRect(Vec2(-22,-22),Vec2(22,22),Color4F(0.6f,0.2f,0.8f,1.f)); addChild(n); ResourceManager::getInstance().onStorageCapacityChanged(ResourceType::Elixir,0,capacity); return true; }

void ElixirStorage::upgrade(){ int old=capacity; level=std::min(3, level+1); capacity=storageCapForLevel(level); ResourceManager::getInstance().onStorageCapacityChanged(ResourceType::Elixir,old,capacity); }

