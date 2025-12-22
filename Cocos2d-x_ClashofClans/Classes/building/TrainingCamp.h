#ifndef __TRAINING_CAMP_H__
#define __TRAINING_CAMP_H__

#include "Building.h"
#include <vector>

namespace building {

class TrainingCamp : public Building {
public:
    static TrainingCamp* create();
    virtual bool init() override;

    virtual void upgrade() override;
    virtual int getUpgradeCost() const override;
    virtual std::string getUpgradeCurrencyIcon() const override { return "elixir.png"; }
    virtual bool canUpgrade() const override;
    virtual void showInfo() override;
    
    std::vector<std::string> getUnlockableSoldiers() const;
    
    static int getBuildCost() { return 300; }
    static std::string getBuildCurrencyIcon() { return "elixir.png"; }

private:
    void updateTexture();
};

} // namespace building

#endif // __TRAINING_CAMP_H__
