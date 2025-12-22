#ifndef __CANNON_H__
#define __CANNON_H__

#include "Building.h"

namespace building {

class Cannon : public Building {
public:
    static Cannon* create();
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

#endif // __CANNON_H__
