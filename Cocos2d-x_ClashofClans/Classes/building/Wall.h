#ifndef __WALL_H__
#define __WALL_H__

#include "Building.h"

namespace building {

class Wall : public Building {
public:
    static Wall* create();
    virtual bool init() override;

    virtual void upgrade() override;
    virtual int getUpgradeCost() const override;
    virtual std::string getUpgradeCurrencyIcon() const override { return "coin.png"; }
    virtual bool canUpgrade() const override;
    virtual void showInfo() override;
    
    static int getBuildCost() { return 50; }
    static std::string getBuildCurrencyIcon() { return "coin.png"; }

private:
    void updateTexture();
};

} // namespace building

#endif // __WALL_H__
