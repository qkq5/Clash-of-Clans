#ifndef __GOLD_MINE_H__
#define __GOLD_MINE_H__

#include "Building.h"

namespace building {

class GoldMine : public Building {
public:
    static GoldMine* create();
    virtual bool init() override;

    virtual void upgrade() override;
    virtual int getUpgradeCost() const override;
    virtual std::string getUpgradeCurrencyIcon() const override { return "elixir.png"; } // Usually costs elixir to upgrade gold mine
    virtual bool canUpgrade() const override;
    virtual void showInfo() override;
    
    int getProductionRate() const;

    static int getBuildCost() { return 150; }
    static std::string getBuildCurrencyIcon() { return "elixir.png"; }

private:
    void updateTexture();
};

} // namespace building

#endif // __GOLD_MINE_H__
