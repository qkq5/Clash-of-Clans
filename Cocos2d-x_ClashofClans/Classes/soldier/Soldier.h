#ifndef __SOLDIER_H__
#define __SOLDIER_H__

#include "cocos2d.h"
#include "../building/Building.h"

namespace soldier {

enum class SoldierType {
    Barbarian,
    Archer,
    Bomber,
    Giant
};

class Soldier : public cocos2d::Sprite {
public:
    static Soldier* create(SoldierType type);
    virtual bool init(SoldierType type);

    // Attributes
    CC_SYNTHESIZE(int, _hp, HP);
    CC_SYNTHESIZE(int, _maxHP, MaxHP);
    CC_SYNTHESIZE(int, _attackDamage, AttackDamage);
    CC_SYNTHESIZE(float, _moveSpeed, MoveSpeed); // pixels per second
    CC_SYNTHESIZE(float, _attackRange, AttackRange); // in pixels (tiles * 32)
    CC_SYNTHESIZE(float, _attackInterval, AttackInterval);
    CC_SYNTHESIZE(float, _attackTimer, AttackTimer);
    CC_SYNTHESIZE(SoldierType, _type, Type);
    CC_SYNTHESIZE(int, _space, Space);
    
    // AI
    void setTarget(building::Building* target);
    building::Building* getTarget() const { return _target; }
    
    void setPath(const std::vector<cocos2d::Vec2>& path);
    void moveAlongPath(float dt);
    bool hasPath() const { return !_path.empty() && _currentPathIndex < _path.size(); }
    
    virtual void update(float dt) override;

    bool isDead() const { return _hp <= 0; }
    void takeDamage(int damage);

private:
    building::Building* _target;
    std::vector<cocos2d::Vec2> _path;
    int _currentPathIndex;
};

} // namespace soldier

#endif // __SOLDIER_H__
