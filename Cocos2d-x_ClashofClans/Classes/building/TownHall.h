#ifndef __TOWN_HALL_H__
#define __TOWN_HALL_H__

#include "Building.h"

namespace building {

class TownHall : public Building {
public:
    static TownHall* create();
    virtual bool init() override;

    virtual void upgrade() override;
    virtual int getUpgradeCost() const override;
    virtual std::string getUpgradeCurrencyIcon() const override { return "coin.png"; }
    virtual bool canUpgrade() const override;
    virtual void showInfo() override;

private:
    void updateTexture();
};

} // namespace building

#endif // __TOWN_HALL_H__
