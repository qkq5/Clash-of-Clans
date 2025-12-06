#include "Unit.h"
#include "GameScene.h"
#include "Building.h"
#include "cocos2d.h"
#include "2d/CCSprite.h"

USING_NS_CC;

Archer* Archer::create(const Vec2& pos){ 
    auto u=new Archer(); 
    u->type=UnitType::Archer; 
    u->hp=80; 
    u->attack=15; 
    u->speed=100.f; 
    u->range=120.f; 
    u->attackInterval=0.7f; 
    u->setPosition(pos); 
    u->init(); 
    // 使用弓箭手贴图
    Sprite* sprite = Sprite::create("arrow_man.png");
    if(sprite){
        auto vs = Director::getInstance()->getVisibleSize();
        float targetW = vs.width / 20.f;
        float scale = targetW / sprite->getContentSize().width;
        sprite->setScale(scale);
        u->addChild(sprite);
    }
    u->autorelease(); 
    return u; 
}

Barbarian* Barbarian::create(const Vec2& pos){ 
    auto u=new Barbarian(); 
    u->type=UnitType::Barbarian; 
    u->hp=120; 
    u->attack=20; 
    u->speed=90.f; 
    u->range=24.f; 
    u->attackInterval=0.6f; 
    u->setPosition(pos); 
    u->init(); 
    // 使用野蛮人贴图
    cocos2d::Sprite* sprite = cocos2d::Sprite::create("barbarian.png");
    if(sprite){
        auto vs = Director::getInstance()->getVisibleSize();
        float targetW = vs.width / 20.f;
        float scale = targetW / sprite->getContentSize().width;
        sprite->setScale(scale);
        u->addChild(sprite);
    }
    u->autorelease(); 
    return u; 
}

WallBreaker* WallBreaker::create(const Vec2& pos) {
    auto u = new WallBreaker();
    u->type = UnitType::Bomber;
    u->hp = 70;
    u->attack = 60;
    u->speed = 80.f;
    u->range = 40.f;
    u->attackInterval = 1.2f;
    u->setPosition(pos);
    u->init();
    

    u->autorelease();
    return u;
}

Giant* Giant::create(const Vec2& pos){ 
    auto u=new Giant(); 
    u->type=UnitType::Giant; 
    u->hp=400; 
    u->attack=25; 
    u->speed=60.f; 
    u->range=28.f; 
    u->attackInterval=1.0f; 
    u->setPosition(pos); 
    u->init(); 
    // 使用巨人贴图
    cocos2d::Sprite* sprite = cocos2d::Sprite::create("giant.png");
    if(sprite){
        auto vs = Director::getInstance()->getVisibleSize();
        float targetW = vs.width / 15.f; // 巨人稍大一些
        float scale = targetW / sprite->getContentSize().width;
        sprite->setScale(scale);
        u->addChild(sprite);
    }
    u->autorelease(); 
    return u; 
}

void Unit::update(float dt){
    auto* scene = dynamic_cast<GameScene*>(getScene()); if(!scene) return;
    if(!target || target->isDestroyed()){
        const auto& bs = scene->getBuildings();
        float best=1e9f; Building* pick=nullptr;
        for(auto* b : bs){ if(!b || b->isDestroyed()) continue; float d = getPosition().distance(b->getPosition()); if(d<best){ best=d; pick=b; } }
        target = pick;
    }
    if(!target) return;
    float dist = getPosition().distance(target->getPosition());
    float attackRange = std::max(range, 50.0f);
    if(dist > attackRange){
        Vec2 dir = target->getPosition() - getPosition();
        float len = dir.length();
        if(len > 1e-3f){ dir.normalize(); setPosition(getPosition() + dir * speed * dt); }
        if(len < 20.0f) { /* stop near target */ }
    } else {
        attackTimer += dt;
        if(attackTimer >= attackInterval){ attackTimer = 0.0f; target->takeDamage(attack); }
    }
}
