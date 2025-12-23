#include "Soldier.h"

namespace soldier {

static void showDamageEffect(cocos2d::Node* node, int damage) {
    if (!node || !node->getParent()) return;
    auto label = cocos2d::Label::createWithSystemFont("-" + std::to_string(damage), "Arial", 24);
    label->setColor(cocos2d::Color3B::RED);
    cocos2d::Vec2 pos = node->getPosition();
    cocos2d::Size size = node->getContentSize();
    float scale = node->getScale();
    pos.x += (size.width * scale) / 2 + 10;
    pos.y += (size.height * scale) / 2;
    label->setPosition(pos);
    node->getParent()->addChild(label, 1000);
    auto move = cocos2d::MoveBy::create(1.0f, cocos2d::Vec2(0, 32));
    auto fade = cocos2d::FadeOut::create(1.0f);
    auto spawn = cocos2d::Spawn::create(move, fade, nullptr);
    label->runAction(cocos2d::Sequence::create(spawn, cocos2d::RemoveSelf::create(), nullptr));
}

Soldier* Soldier::create(SoldierType type) {
    Soldier* ret = new (std::nothrow) Soldier();
    if (ret && ret->init(type)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool Soldier::init(SoldierType type) {
    if (!Sprite::init()) {
        return false;
    }

    _type = type;
    _target = nullptr;
    _attackTimer = 0;

    // Attributes based on type
    std::string textureName;
    
    switch (type) {
        case SoldierType::Barbarian:
            textureName = "barbarian2.png";
            _hp = 320;
            _attackDamage = 85;
            _moveSpeed = 60.0f; // Medium
            _attackRange = 1.0f * 32.0f; // 1 tile
            _attackInterval = 1.0f;
            _space = 1;
            break;
        case SoldierType::Archer:
            textureName = "archer2.png";
            _hp = 130;
            _attackDamage = 75;
            _moveSpeed = 60.0f; // Medium
            _attackRange = 5.0f * 32.0f; // 5 tiles
            _attackInterval = 0.5f; // 2 times/sec
            _space = 1;
            break;
        case SoldierType::Bomber:
            textureName = "bomber2.png";
            _hp = 150;
            _attackDamage = 180; // Explosion damage
            _moveSpeed = 40.0f; // Slow
            _attackRange = 3.0f * 32.0f; // 3 tiles
            _attackInterval = 2.0f; // Not specified, assume slow? "远程丢炸弹" - let's say 2s or 1s. Prompt says nothing on interval. Default 1s.
            // Prompt: "炸弹人... 攻击间隔" missing. Archer has 2/s. Giant 1/s. Barb 1/s.
            // Let's assume 1s.
            _attackInterval = 1.0f; 
            _space = 2;
            break;
        case SoldierType::Giant:
            textureName = "giant2.png";
            _hp = 650;
            _attackDamage = 90;
            _moveSpeed = 30.0f; // Slow
            _attackRange = 1.0f * 32.0f; // 1 tile
            _attackInterval = 1.0f;
            _space = 5;
            break;
    }
    
    _maxHP = _hp;
    this->setTexture(textureName);
    
    // Add HP bar
    // Simple bar above head
    auto bg = cocos2d::LayerColor::create(cocos2d::Color4B::RED, 30, 4);
    bg->setPosition(cocos2d::Vec2(this->getContentSize().width/2 - 15, this->getContentSize().height + 5));
    this->addChild(bg, 1, 99); // Tag 99 for bar
    
    return true;
}

void Soldier::takeDamage(int damage) {
    showDamageEffect(this, damage);
    _hp -= damage;
    if (_hp < 0) _hp = 0;
    
    // Update HP bar
    auto bar = dynamic_cast<cocos2d::LayerColor*>(this->getChildByTag(99));
    if (bar) {
        float percent = (float)_hp / _maxHP;
        bar->setContentSize(cocos2d::Size(30 * percent, 4));
    }
    
    if (_hp == 0) {
        // Die logic handled by Scene
        this->setOpacity(100); // Visual cue
        this->runAction(cocos2d::FadeOut::create(0.5f));
    }
}

void Soldier::setTarget(building::Building* target) {
    _target = target;
}

void Soldier::setPath(const std::vector<cocos2d::Vec2>& path) {
    _path = path;
    _currentPathIndex = 0;
}

void Soldier::update(float dt) {
    // Logic currently handled in BattleScene, but we can put movement here if we want.
    // For now, we'll use a specific method called by BattleScene.
}

void Soldier::moveAlongPath(float dt) {
    if (_path.empty()) return;
    
    if (_currentPathIndex < _path.size()) {
        cocos2d::Vec2 targetPos = _path[_currentPathIndex];
        cocos2d::Vec2 currentPos = this->getPosition();
        float dist = currentPos.distance(targetPos);
        
        float step = _moveSpeed * dt;
        
        if (dist <= step) {
            this->setPosition(targetPos);
            _currentPathIndex++;
        } else {
            cocos2d::Vec2 dir = (targetPos - currentPos).getNormalized();
            this->setPosition(currentPos + dir * step);
        }
    }
}

} // namespace soldier
