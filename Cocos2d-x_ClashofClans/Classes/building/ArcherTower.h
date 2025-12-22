#ifndef __ARCHER_TOWER_H__
#define __ARCHER_TOWER_H__

#include "Building.h"

namespace building {

class ArcherTower : public Building {
public:
    static ArcherTower* create();
    virtual bool init() override;

    virtual void upgrade() override;
    virtual int getUpgradeCost() const override;
    virtual std::string getUpgradeCurrencyIcon() const override { return "coin.png"; }
    virtual bool canUpgrade() const override;
    virtual void showInfo() override;
    
    static int getBuildCost() { return 300; }
    static std::string getBuildCurrencyIcon() { return "coin.png"; }

private:
    void updateTexture();
};

} // namespace building

#endif // __ARCHER_TOWER_H__
