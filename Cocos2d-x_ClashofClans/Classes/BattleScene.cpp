#include "BattleScene.h"
#include "ResourceManager.h"
#include "GameScene.h"

USING_NS_CC;
using namespace ui;

// 简单的敌方建筑实现，用于战斗场景（实现空的 upgrade，避免抽象类无法实例化）
class SimpleEnemyBuilding : public Building {
public:
    static SimpleEnemyBuilding* create(BuildingType t, int hpValue, const Vec2& pos, const std::string& spritePath, float targetWidthFactor){
        auto p = new SimpleEnemyBuilding();
        if(p && p->initInternal(t, hpValue, pos, spritePath, targetWidthFactor)){
            p->autorelease();
            return p;
        }
        delete p;
        return nullptr;
    }

    virtual void upgrade() override { /* 敌方建筑暂不支持升级 */ }

private:
    bool initInternal(BuildingType t, int hpValue, const Vec2& pos, const std::string& spritePath, float targetWidthFactor){
        type = t;
        level = 1;
        hp = hpValue;
        setPosition(pos);
        auto sp = Sprite::create(spritePath);
        if(sp){
            auto vs = Director::getInstance()->getVisibleSize();
            float targetW = targetWidthFactor;
            float s = targetW / sp->getContentSize().width;
            sp->setScale(s);
            addChild(sp);
        }
        return true;
    }
};

Scene* BattleScene::createScene(){ return BattleScene::create(); }

bool BattleScene::init(){
    if(!Scene::init()) return false;

    setupBackground();
    setupHUD();
    setupEnemyBase();
    setupBottomUI();

    // 从全局资源中获取一次快照，锁定为本局战斗可用资源
    auto& rm = ResourceManager::getInstance();
    battleGold = rm.getGold();
    battleElixir = rm.getElixir();
    battleSoldiers = rm.getPop();

    // 触摸投放兵种
    auto listener = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = [&](Touch* t, Event*){
        onTouchSpawn(t->getLocation());
        return false;
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    schedule(CC_SCHEDULE_SELECTOR(BattleScene::updateBattle));
    return true;
}

void BattleScene::setupBackground(){
    auto vs = Director::getInstance()->getVisibleSize();
    auto org = Director::getInstance()->getVisibleOrigin();
    auto bg = Sprite::create("background.png");
    if(bg){
        auto sz = bg->getContentSize();
        float sx = vs.width / sz.width;
        float sy = vs.height / sz.height;
        float s = std::min(sx, sy);
        bg->setScale(s);
        bg->setPosition(Vec2(org.x+vs.width/2, org.y+vs.height/2));
        addChild(bg, -100);
    }
}

void BattleScene::setupHUD(){
    auto vs = Director::getInstance()->getVisibleSize();
    auto org = Director::getInstance()->getVisibleOrigin();
    hudLabel = Label::createWithTTF("", "fonts/Marker Felt.ttf", 18);
    hudLabel->setAnchorPoint(Vec2(0,1));
    hudLabel->setPosition(Vec2(org.x+10, org.y+vs.height-5));
    addChild(hudLabel, 200);
}

void BattleScene::setupEnemyBase(){
    auto vs = Director::getInstance()->getVisibleSize();
    auto org = Director::getInstance()->getVisibleOrigin();

    // 敌方大本营
    auto town = SimpleEnemyBuilding::create(
        BuildingType::TownHall,
        3000,
        Vec2(org.x+vs.width*0.7f, org.y+vs.height*0.5f),
        "hometower.png",
        vs.width/6.f
    );
    if(town){
        addChild(town, 50);
        enemyBuildings.push_back(town);
    }

    // 弓箭塔
    auto tower = SimpleEnemyBuilding::create(
        BuildingType::DefenseTower,
        1000,
        Vec2(org.x+vs.width*0.55f, org.y+vs.height*0.7f),
        "arrow_tower.png",
        vs.width/10.f
    );
    if(tower){
        addChild(tower, 50);
        enemyBuildings.push_back(tower);
    }

    // 加农炮
    auto cannon = SimpleEnemyBuilding::create(
        BuildingType::DefenseTower,
        400,
        Vec2(org.x+vs.width*0.5f, org.y+vs.height*0.3f),
        "cannon.png",
        vs.width/10.f
    );
    if(cannon){
        addChild(cannon, 50);
        enemyBuildings.push_back(cannon);
    }
}

void BattleScene::setupBottomUI(){
    auto vs = Director::getInstance()->getVisibleSize();
    auto org = Director::getInstance()->getVisibleOrigin();

    auto bar = Layout::create();
    float h = vs.height/8.f;
    bar->setContentSize(Size(vs.width, h));
    bar->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
    bar->setBackGroundColor(Color3B(40,40,40));
    bar->setAnchorPoint(Vec2(0,0));
    bar->setPosition(Vec2(org.x, org.y));
    addChild(bar, 100);

    // battle 结束后返回大本营 - 缩小到1/5
    auto btnBack = Button::create("button.png","button.png");
    btnBack->setTitleText("Home");
    btnBack->setTitleFontSize(4); // 缩小字体
    float btnBackW = vs.width * 0.04f; // 缩小到原来的1/5左右
    float btnBackScale = btnBackW / btnBack->getContentSize().width;
    btnBack->setScale(btnBackScale);
    btnBack->setPosition(Vec2(org.x + btnBackW/2 + 10, h/2));
    btnBack->addClickEventListener([&](Ref*){
        auto scene = GameScene::createScene();
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
    });
    bar->addChild(btnBack, 1);
    // Home按钮的提示文字
    auto homeLabel = Label::createWithTTF("Home", "fonts/Marker Felt.ttf", 20);
    homeLabel->setColor(Color3B::WHITE);
    homeLabel->setPosition(Vec2(org.x + btnBackW/2 + 10, h/2 + btnBackW/2 + 15));
    addChild(homeLabel, 300);

    float startX = org.x + btnBackW + 30;
    float btnW = vs.width * 0.04f; // 缩小按钮
    float step = btnW + 20;

    btnArcher = Button::create("button.png","button.png");
    btnArcher->setTitleText("Archer");
    btnArcher->setTitleFontSize(4);
    float btnScale = btnW / btnArcher->getContentSize().width;
    btnArcher->setScale(btnScale);
    btnArcher->setPosition(Vec2(startX + btnW/2, h/2));
    btnArcher->addClickEventListener([&](Ref*){
        currentPick = UnitType::Archer;
    });
    bar->addChild(btnArcher, 1);
    // Archer按钮的提示文字
    auto archerLabel = Label::createWithTTF("Archer", "fonts/Marker Felt.ttf", 20);
    archerLabel->setColor(Color3B::WHITE);
    archerLabel->setPosition(Vec2(startX + btnW/2, h/2 + btnW/2 + 15));
    addChild(archerLabel, 300);

    btnBarbarian = Button::create("button.png","button.png");
    btnBarbarian->setTitleText("Barbarian");
    btnBarbarian->setTitleFontSize(4);
    btnBarbarian->setScale(btnScale);
    btnBarbarian->setPosition(Vec2(startX + btnW + step, h/2));
    btnBarbarian->addClickEventListener([&](Ref*){
        currentPick = UnitType::Barbarian;
    });
    bar->addChild(btnBarbarian, 1);
    // Barbarian按钮的提示文字
    auto barbLabel = Label::createWithTTF("Barbarian", "fonts/Marker Felt.ttf", 20);
    barbLabel->setColor(Color3B::WHITE);
    barbLabel->setPosition(Vec2(startX + btnW + step, h/2 + btnW/2 + 15));
    addChild(barbLabel, 300);

    btnGiant = Button::create("button.png","button.png");
    btnGiant->setTitleText("Giant");
    btnGiant->setTitleFontSize(4);
    btnGiant->setScale(btnScale);
    btnGiant->setPosition(Vec2(startX + btnW*2 + step*2, h/2));
    btnGiant->addClickEventListener([&](Ref*){
        currentPick = UnitType::Giant;
    });
    bar->addChild(btnGiant, 1);
    // Giant按钮的提示文字
    auto giantLabel = Label::createWithTTF("Giant", "fonts/Marker Felt.ttf", 20);
    giantLabel->setColor(Color3B::WHITE);
    giantLabel->setPosition(Vec2(startX + btnW*2 + step*2, h/2 + btnW/2 + 15));
    addChild(giantLabel, 300);
}

void BattleScene::onTouchSpawn(const Vec2& worldPos){
    auto vs = Director::getInstance()->getVisibleSize();
    auto org = Director::getInstance()->getVisibleOrigin();

    // 不允许点击底部 UI 区域
    float bottomH = vs.height/8.f;
    if(worldPos.y <= org.y + bottomH) return;

    // 兵力用完则不能再投放
    if(battleSoldiers <= 0) return;

    // 每次投放消耗 1 士兵名额
    battleSoldiers -= 1;

    Unit* u = nullptr;
    if(currentPick == UnitType::Archer){
        u = Archer::create(worldPos);
        u->hp = 300;
        u->attack = 300;
        u->range = 30 * (Director::getInstance()->getVisibleSize().width/100.f);
    } else if(currentPick == UnitType::Barbarian){
        u = Barbarian::create(worldPos);
        u->hp = 600;
        u->attack = 200;
        u->range = 8 * (Director::getInstance()->getVisibleSize().width/100.f);
    } else if(currentPick == UnitType::Giant){
        u = Giant::create(worldPos);
        u->hp = 1000;
        u->attack = 200;
        u->range = 10 * (Director::getInstance()->getVisibleSize().width/100.f);
    }
    if(!u) return;

    addChild(u, 60);
    playerUnits.push_back(u);
}

static float unitPriority(UnitType t){
    // 敌方攻击优先级：巨人，野蛮人，弓箭手
    if(t == UnitType::Giant) return 0.f;
    if(t == UnitType::Barbarian) return 1.f;
    if(t == UnitType::Archer) return 2.f;
    return 3.f;
}

void BattleScene::updateBattle(float dt){
    // 简单 HUD 文本
    if(hudLabel){
        char buf[128];
        sprintf(buf, "Gold:%d  Elixir:%d  Soldiers:%d", battleGold, battleElixir, battleSoldiers);
        hudLabel->setString(buf);
    }

    // 我方单位 AI：攻击范围内最近的目标
    for(auto* u : playerUnits){
        if(!u || u->hp<=0) continue;

        Building* target = nullptr;
        float bestDist = 1e9f;
        for(auto* b : enemyBuildings){
            if(!b || b->isDestroyed()) continue;
            float d = u->getPosition().distance(b->getPosition());
            if(d < bestDist){
                bestDist = d;
                target = b;
            }
        }
        if(!target) continue;

        float attackRange = u->range;
        float dist = u->getPosition().distance(target->getPosition());
        if(dist > attackRange){
            Vec2 dir = target->getPosition() - u->getPosition();
            float len = dir.length();
            if(len > 1e-3f){
                dir.normalize();
                u->setPosition(u->getPosition() + dir * u->speed * dt);
            }
        }else{
            u->attackTimer += dt;
            if(u->attackTimer >= u->attackInterval){
                u->attackTimer = 0.f;
                target->takeDamage(u->attack);

                // 攻击动画：弓箭手使用红点飞行，野蛮人和巨人使用旋转
                if(u->type == UnitType::Archer){
                    // 弓箭手：红色圆点从位置移动到目标
                    auto dot = DrawNode::create();
                    dot->drawDot(Vec2::ZERO, 4.f, Color4F::RED);
                    dot->setPosition(u->getPosition());
                    addChild(dot, 200);
                    auto move = MoveTo::create(0.2f, target->getPosition());
                    auto seq = Sequence::create(move, RemoveSelf::create(), nullptr);
                    dot->runAction(seq);
                }else{
                    // 野蛮人和巨人：绕中心快速顺时针旋转30度再逆时针旋转回原处
                    auto act1 = RotateBy::create(0.05f, 30);
                    auto act2 = RotateBy::create(0.05f, -30);
                    u->runAction(Sequence::create(act1, act2, nullptr));
                }
            }
        }
    }

    // 敌方防御塔简单 AI：攻击最近的单位，距离相同按优先级
    enemyAttackTimer += dt;
    if(enemyAttackTimer >= 1.0f){
        enemyAttackTimer = 0.f;
        for(auto* b : enemyBuildings){
            if(!b || b->isDestroyed()) continue;
            if(b->type != BuildingType::DefenseTower) continue;

            Unit* targetU = nullptr;
            float bestDist = 1e9f;
            float bestPrio = 1e9f;
            for(auto* u : playerUnits){
                if(!u || u->hp<=0) continue;
                float d = u->getPosition().distance(b->getPosition());
                float pr = unitPriority(u->type);
                if(d < bestDist - 1e-3f || (fabs(d-bestDist)<1e-3f && pr < bestPrio)){
                    bestDist = d;
                    bestPrio = pr;
                    targetU = u;
                }
            }
            if(!targetU) continue;

            // 简单射击：红点飞向单位
            auto dot = DrawNode::create();
            dot->drawDot(Vec2::ZERO, 4.f, Color4F::RED);
            dot->setPosition(b->getPosition());
            addChild(dot, 200);
            auto move = MoveTo::create(0.2f, targetU->getPosition());
            auto cb = CallFunc::create([targetU](){
                if(targetU && targetU->hp>0){
                    targetU->hp -= 100;
                }
            });
            auto seq = Sequence::create(move, cb, RemoveSelf::create(), nullptr);
            dot->runAction(seq);
        }
    }

    // 清理死亡单位和建筑
    {
        std::vector<Unit*> aliveU;
        for(auto* u : playerUnits){
            if(!u || u->hp<=0){
                if(u) removeChild(u);
            }else{
                aliveU.push_back(u);
            }
        }
        playerUnits.swap(aliveU);
    }
    {
        std::vector<Building*> aliveB;
        for(auto* b : enemyBuildings){
            if(!b || b->isDestroyed()){
                if(b) removeChild(b);
            }else{
                aliveB.push_back(b);
            }
        }
        enemyBuildings.swap(aliveB);
    }

    checkBattleResult();
}

void BattleScene::checkBattleResult(){
    bool enemyAllDead = true;
    for(auto* b : enemyBuildings){
        if(b && !b->isDestroyed()){
            enemyAllDead = false;
            break;
        }
    }
    if(enemyAllDead){
        auto vs = Director::getInstance()->getVisibleSize();
        auto org = Director::getInstance()->getVisibleOrigin();
        auto lab = Label::createWithTTF("Victory", "fonts/Marker Felt.ttf", 28);
        lab->setPosition(Vec2(org.x+vs.width*0.5f, org.y+vs.height*0.6f));
        addChild(lab, 300);
        lab->runAction(Sequence::create(DelayTime::create(2.f),
                                        CallFunc::create([](){
                                            auto scene = GameScene::createScene();
                                            Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
                                        }),
                                        nullptr));
        unschedule(CC_SCHEDULE_SELECTOR(BattleScene::updateBattle));
        return;
    }

    // 士兵与单位都耗尽则失败
    if(battleSoldiers <= 0 && playerUnits.empty()){
        auto vs = Director::getInstance()->getVisibleSize();
        auto org = Director::getInstance()->getVisibleOrigin();
        auto lab = Label::createWithTTF("Defeat", "fonts/Marker Felt.ttf", 28);
        lab->setPosition(Vec2(org.x+vs.width*0.5f, org.y+vs.height*0.6f));
        addChild(lab, 300);
        lab->runAction(Sequence::create(DelayTime::create(2.f),
                                        CallFunc::create([](){
                                            auto scene = GameScene::createScene();
                                            Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
                                        }),
                                        nullptr));
        unschedule(CC_SCHEDULE_SELECTOR(BattleScene::updateBattle));
    }
}


