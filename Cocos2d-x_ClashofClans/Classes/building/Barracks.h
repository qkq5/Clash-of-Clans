#ifndef __BARRACKS_H__
#define __BARRACKS_H__

#include "Building.h"

namespace building {

class Barracks : public Building {
public:
    static Barracks* create();
    virtual bool init() override;

    virtual void upgrade() override;
    virtual int getUpgradeCost() const override;
    virtual std::string getUpgradeCurrencyIcon() const override { return "elixir.png"; }
    virtual bool canUpgrade() const override;
    virtual void showInfo() override;
    
    int getCapacity() const;
    
    static int getBuildCost() { return 300; }
    static std::string getBuildCurrencyIcon() { return "elixir.png"; }

private:
    void updateTexture();
};

} // namespace building

#endif // __BARRACKS_H__
