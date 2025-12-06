#include "ResourceManager.h"

ResourceManager& ResourceManager::getInstance(){ static ResourceManager inst; return inst; }

ResourceManager::ResourceManager(){
    gold=200;
    elixir=200;
    pop=0;
    // 士兵数量上限设置为一个较大的值，避免过早封顶导致看起来"不增长"
    popCap=9999;
    goldRate=0.f;
    elixirRate=0.f;
    // 设置资源上限：金币1000，圣水500
    goldMax=1000;
    elixirMax=500;
}

void ResourceManager::addResource(ResourceType t, int amount){ if(t==ResourceType::Gold){ gold+=amount; if(gold>goldMax) gold=goldMax; } if(t==ResourceType::Elixir){ elixir+=amount; if(elixir>elixirMax) elixir=elixirMax; } if(t==ResourceType::Population) pop+=amount; }

bool ResourceManager::canAfford(const Cost& c) const{ if(gold<c.gold||elixir<c.elixir) return false; if(pop+c.pop>popCap) return false; return true; }

bool ResourceManager::spend(const Cost& c){ if(!canAfford(c)) return false; gold-=c.gold; elixir-=c.elixir; pop+=c.pop; return true; }

int ResourceManager::getGold() const{ return gold; }
int ResourceManager::getElixir() const{ return elixir; }
int ResourceManager::getPop() const{ return pop; }
int ResourceManager::getPopCap() const{ return popCap; }
void ResourceManager::setPopCap(int cap){ popCap=cap; }
bool ResourceManager::canAddPop(int p) const{ return pop + p <= popCap; }
void ResourceManager::addPop(int p){ pop = std::min(pop + p, popCap); }
void ResourceManager::releasePop(int p){ pop = std::max(pop - p, 0); }
int ResourceManager::getGoldMax() const{ return goldMax; }
int ResourceManager::getElixirMax() const{ return elixirMax; }
void ResourceManager::onStorageCapacityChanged(ResourceType t, int oldCap, int newCap){ int delta=newCap-oldCap; if(t==ResourceType::Gold){ goldMax+=delta; if(goldMax<0) goldMax=0; if(gold>goldMax) gold=goldMax; } else if(t==ResourceType::Elixir){ elixirMax+=delta; if(elixirMax<0) elixirMax=0; if(elixir>elixirMax) elixir=elixirMax; } }

void ResourceManager::produce(float dt){ }
