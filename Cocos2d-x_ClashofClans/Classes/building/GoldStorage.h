#ifndef __GOLD_STORAGE_H__
#define __GOLD_STORAGE_H__

#include "Building.h"

namespace building {

class GoldStorage : public Building {
public:
    static GoldStorage* create();
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

#endif // __GOLD_STORAGE_H__
