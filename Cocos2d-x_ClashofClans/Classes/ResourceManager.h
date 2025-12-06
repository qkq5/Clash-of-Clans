#pragma once
#include "cocos2d.h"

enum class ResourceType { Gold, Elixir, Population };

struct Cost { int gold; int elixir; int pop; };

class ResourceManager {
public:
    static ResourceManager& getInstance();
    void addResource(ResourceType t, int amount);
    bool canAfford(const Cost& c) const;
    bool spend(const Cost& c);
    int getGold() const; 
    int getElixir() const; 
    int getPop() const; 
    int getPopCap() const; 
    void setPopCap(int cap);
    bool canAddPop(int p) const;
    void addPop(int p);
    void releasePop(int p);
    int getGoldMax() const;
    int getElixirMax() const;
    void onStorageCapacityChanged(ResourceType t, int oldCap, int newCap);
    void produce(float dt);
private:
    ResourceManager();
    int gold;
    int elixir;
    int pop;
    int popCap;
    float goldRate;
    float elixirRate;
    int goldMax;
    int elixirMax;
};
