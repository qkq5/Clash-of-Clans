#ifndef __ELIXIR_COLLECTOR_H__
#define __ELIXIR_COLLECTOR_H__

#include "Building.h"

namespace building {

class ElixirCollector : public Building {
public:
    static ElixirCollector* create();
    virtual bool init() override;

    virtual void upgrade() override;
    virtual int getUpgradeCost() const override;
    virtual std::string getUpgradeCurrencyIcon() const override { return "coin.png"; } // Usually costs gold to upgrade elixir collector
    virtual bool canUpgrade() const override;
    virtual void showInfo() override;
    
    static int getBuildCost() { return 150; }
    static std::string getBuildCurrencyIcon() { return "coin.png"; }

private:
    void updateTexture();
};

} // namespace building

#endif // __ELIXIR_COLLECTOR_H__
