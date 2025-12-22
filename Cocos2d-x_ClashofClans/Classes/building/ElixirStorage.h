#ifndef __ELIXIR_STORAGE_H__
#define __ELIXIR_STORAGE_H__

#include "Building.h"

namespace building {

class ElixirStorage : public Building {
public:
    static ElixirStorage* create();
    virtual bool init() override;

    virtual void upgrade() override;
    virtual int getUpgradeCost() const override;
    virtual std::string getUpgradeCurrencyIcon() const override { return "coin.png"; }
    virtual bool canUpgrade() const override;
    virtual void showInfo() override;
    
    int getCapacity() const;
    
    static int getBuildCost() { return 300; }
    static std::string getBuildCurrencyIcon() { return "coin.png"; }

private:
    void updateTexture();
};

} // namespace building

#endif // __ELIXIR_STORAGE_H__
