#ifndef __LANDMINE_H__
#define __LANDMINE_H__

#include "Building.h"

namespace building {

class Landmine : public Building {
public:
    static Landmine* create();
    virtual bool init() override;

    virtual void upgrade() override;
    virtual int getUpgradeCost() const override;
    virtual std::string getUpgradeCurrencyIcon() const override { return "elixir.png"; }
    virtual bool canUpgrade() const override;
    virtual void showInfo() override;
    
    void startTriggerSequence(std::function<void()> onExplode);
    bool isTriggered() const { return _isTriggered; }

    static int getBuildCost() { return 75; }
    static std::string getBuildCurrencyIcon() { return "elixir.png"; }

private:
    bool _isTriggered;
};

} // namespace building

#endif // __LANDMINE_H__
