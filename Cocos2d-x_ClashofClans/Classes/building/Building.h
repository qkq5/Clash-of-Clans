#ifndef __BUILDING_H__
#define __BUILDING_H__

#include "cocos2d.h"
#include <string>

namespace building {

enum class BuildingType {
    TownHall,
    GoldMine,
    ElixirCollector,
    GoldStorage,
    ElixirStorage,
    ArcherTower,
    Cannon,
    Barracks,
    TrainingCamp,
    Landmine
};

class Building : public cocos2d::Sprite {
public:
    Building();
    virtual ~Building();

    virtual bool init() override;

    // Core attributes
    CC_SYNTHESIZE(int, _level, Level);
    CC_SYNTHESIZE(int, _maxHP, MaxHP);
    CC_SYNTHESIZE(int, _currentHP, CurrentHP);
    CC_SYNTHESIZE(std::string, _buildingName, BuildingName);
    CC_SYNTHESIZE(BuildingType, _type, Type);
    CC_SYNTHESIZE(cocos2d::Vec2, _gridPosition, GridPosition); // Position in grid coordinates (0-49)
    CC_SYNTHESIZE(cocos2d::Size, _gridSize, GridSize); // Size in grid tiles (e.g., 3x3)

    // Combat attributes (for Defenses)
    CC_SYNTHESIZE(int, _attackDamage, AttackDamage);
    CC_SYNTHESIZE(float, _attackRange, AttackRange);
    CC_SYNTHESIZE(float, _attackInterval, AttackInterval);
    CC_SYNTHESIZE(float, _attackTimer, AttackTimer);

    // Abstract methods to be implemented by subclasses
    virtual void upgrade() = 0;
    virtual int getUpgradeCost() const = 0;
    virtual std::string getUpgradeCurrencyIcon() const = 0; // "coin.png" or "elixir.png"
    virtual bool canUpgrade() const = 0;

    // Common methods
    virtual void showInfo() = 0; // To be implemented for UI display logic
    
    virtual void takeDamage(int damage);
    
    void initHPBar();
    void updateHPBar();

    // Factory method pattern could be useful here, but for now we'll use specific create methods in subclasses
};

} // namespace building

#endif // __BUILDING_H__
