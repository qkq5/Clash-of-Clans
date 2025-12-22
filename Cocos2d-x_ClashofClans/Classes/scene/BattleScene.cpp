#include "BattleScene.h"
#include "VillageScene.h"
#include "../building/TownHall.h"
#include "../building/Cannon.h"
#include "../building/ArcherTower.h"
#include "../building/Barracks.h"
#include "../building/TrainingCamp.h"
#include "../building/GoldStorage.h"
#include "../building/ElixirStorage.h"
#include "MainMenuScene.h" // For end button maybe? Or just direct

USING_NS_CC;

namespace scene {

Scene* BattleScene::createScene(int level, int barbarianCount, int archerCount, int bomberCount, int giantCount) {
    return BattleScene::create(level, barbarianCount, archerCount, bomberCount, giantCount);
}

BattleScene* BattleScene::create(int level, int barbarianCount, int archerCount, int bomberCount, int giantCount) {
    BattleScene *pRet = new(std::nothrow) BattleScene();
    if (pRet && pRet->init(level, barbarianCount, archerCount, bomberCount, giantCount)) {
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

bool BattleScene::init(int level, int barbarianCount, int archerCount, int bomberCount, int giantCount) {
    if (!Scene::init()) {
        return false;
    }
    
    _level = level;
    _barbarianCount = barbarianCount;
    _archerCount = archerCount;
    _bomberCount = bomberCount;
    _giantCount = giantCount;
    _isDeployMode = false;
    _selectedTroop = soldier::SoldierType::Barbarian; // Default
    
    // Background & Map
    _mapNode = Node::create();
    this->addChild(_mapNode);

    int mapWidth = 50;
    int mapHeight = 50;
    int tileSize = 32;

    for (int x = 0; x < mapWidth; ++x) {
        for (int y = 0; y < mapHeight; ++y) {
            std::string textureFile = ((x + y) % 2 == 0) ? "grass1.png" : "grass2.png";
            auto tile = Sprite::create(textureFile);
            if (tile) {
                tile->setAnchorPoint(Vec2::ZERO);
                tile->setPosition(Vec2(x * tileSize, y * tileSize));
                _mapNode->addChild(tile, 0); 
            }
        }
    }
    
    // Center map
    Size visibleSize = Director::getInstance()->getVisibleSize();
    _mapNode->setPosition(Vec2(visibleSize.width/2 - (mapWidth * tileSize)/2, 
                               visibleSize.height/2 - (mapHeight * tileSize)/2));
    
    setupLevel(level);
    setupUI();
    
    // Touch
    auto listener = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = CC_CALLBACK_2(BattleScene::onTouchBegan, this);
    listener->onTouchMoved = CC_CALLBACK_2(BattleScene::onTouchMoved, this);
    listener->onTouchEnded = CC_CALLBACK_2(BattleScene::onTouchEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
    
    this->scheduleUpdate();
    
    return true;
}

void BattleScene::setupLevel(int level) {
    // Center of the MAP (50x50 * 32)
    Vec2 center = Vec2(50 * 32 / 2, 50 * 32 / 2);
    
    // Helper to add building
    auto addB = [&](building::Building* b, Vec2 pos) {
        b->setPosition(pos);
        _mapNode->addChild(b);
        _enemyBuildings.pushBack(b);
        // Force texture update or init if needed, usually init called in create
    };
    
    // Level 1
    if (level >= 1) {
        addB(building::TownHall::create(), center);
        addB(building::Cannon::create(), center + Vec2(100, 100));
        addB(building::ArcherTower::create(), center + Vec2(-100, -100));
        addB(building::Barracks::create(), center + Vec2(100, -100));
        addB(building::TrainingCamp::create(), center + Vec2(-100, 100));
    }
    
    // Level 2
    if (level >= 2) {
        addB(building::Cannon::create(), center + Vec2(200, 0));
        addB(building::ArcherTower::create(), center + Vec2(-200, 0));
        addB(building::GoldStorage::create(), center + Vec2(0, 200));
        addB(building::ElixirStorage::create(), center + Vec2(0, -200));
    }
    
    // Level 3
    if (level >= 3) {
        addB(building::Cannon::create(), center + Vec2(200, 200));
        addB(building::Cannon::create(), center + Vec2(-200, -200));
        addB(building::ArcherTower::create(), center + Vec2(200, -200));
    }
}

void BattleScene::setupUI() {
    Size visibleSize = Director::getInstance()->getVisibleSize();
    
    // 1. Put Soldier Button (Bottom Left)
    MenuItem* putBtn = MenuItemImage::create("put_soldier.png", "put_soldier.png", [&](Ref*){
        if (_troopSelectionNode) {
            _troopSelectionNode->setVisible(!_troopSelectionNode->isVisible());
        }
    });
    // Fallback if image missing
    if (!putBtn || putBtn->getContentSize().width == 0) {
        auto label = Label::createWithSystemFont("Put Soldier", "Arial", 20);
        putBtn = MenuItemLabel::create(label, [&](Ref*){
            if (_troopSelectionNode) {
                _troopSelectionNode->setVisible(!_troopSelectionNode->isVisible());
            }
        });
    }
    putBtn->setAnchorPoint(Vec2(0, 0));
    putBtn->setPosition(Vec2(20, 20)); // Bottom Left with padding
    
    auto mainMenu = Menu::create(putBtn, nullptr);
    mainMenu->setPosition(Vec2::ZERO);
    this->addChild(mainMenu, 20);

    // 2. Bottom Bar (Initially Hidden)
    _troopSelectionNode = Node::create();
    this->addChild(_troopSelectionNode, 10);
    _troopSelectionNode->setVisible(false);
    
    auto bar = LayerColor::create(Color4B(0, 0, 0, 150), visibleSize.width, 100);
    bar->setPosition(Vec2::ZERO);
    _troopSelectionNode->addChild(bar);
    
    float startX = visibleSize.width / 2 - 150;
    float y = 50;
    
    // Barbarian: barbarian1.png
    auto barbItem = MenuItemImage::create("barbarian1.png", "barbarian1.png", [&](Ref*){
        _selectedTroop = soldier::SoldierType::Barbarian;
        _isDeployMode = true;
        if (_debugLabel) _debugLabel->setString("Selected: Barbarian");
    });
    // Fallback
    if (!barbItem || barbItem->getContentSize().width == 0) {
         barbItem = MenuItemImage::create("barbarian.png", "barbarian.png", [&](Ref*){
            _selectedTroop = soldier::SoldierType::Barbarian;
            _isDeployMode = true;
            if (_debugLabel) _debugLabel->setString("Selected: Barbarian");
         });
    }

    barbItem->setPosition(Vec2(startX, y));
    _barbarianLabel = Label::createWithSystemFont(std::to_string(_barbarianCount), "Arial", 20);
    _barbarianLabel->setPosition(Vec2(20, -10));
    barbItem->addChild(_barbarianLabel);
    
    // Archer: archer1.png
    auto archerItem = MenuItemImage::create("archer1.png", "archer1.png", [&](Ref*){
        _selectedTroop = soldier::SoldierType::Archer;
        _isDeployMode = true;
        if (_debugLabel) _debugLabel->setString("Selected: Archer");
    });
    if (!archerItem || archerItem->getContentSize().width == 0) {
         archerItem = MenuItemImage::create("archer.png", "archer.png", [&](Ref*){
            _selectedTroop = soldier::SoldierType::Archer;
            _isDeployMode = true;
            if (_debugLabel) _debugLabel->setString("Selected: Archer");
         });
    }

    archerItem->setPosition(Vec2(startX + 100, y));
    _archerLabel = Label::createWithSystemFont(std::to_string(_archerCount), "Arial", 20);
    _archerLabel->setPosition(Vec2(20, -10));
    archerItem->addChild(_archerLabel);
    
    // Bomber: bomber1.png
    auto bomberItem = MenuItemImage::create("bomber1.png", "bomber1.png", [&](Ref*){
        _selectedTroop = soldier::SoldierType::Bomber;
        _isDeployMode = true;
        if (_debugLabel) _debugLabel->setString("Selected: Bomber");
    });
    if (!bomberItem || bomberItem->getContentSize().width == 0) {
         bomberItem = MenuItemImage::create("wall_breaker.png", "wall_breaker.png", [&](Ref*){
            _selectedTroop = soldier::SoldierType::Bomber;
            _isDeployMode = true;
            if (_debugLabel) _debugLabel->setString("Selected: Bomber");
         });
    }

    bomberItem->setPosition(Vec2(startX + 200, y));
    _bomberLabel = Label::createWithSystemFont(std::to_string(_bomberCount), "Arial", 20);
    _bomberLabel->setPosition(Vec2(20, -10));
    bomberItem->addChild(_bomberLabel);
    
    // Giant: giant.png
    auto giantItem = MenuItemImage::create("giant.png", "giant.png", [&](Ref*){
        _selectedTroop = soldier::SoldierType::Giant;
        _isDeployMode = true;
        if (_debugLabel) _debugLabel->setString("Selected: Giant");
    });
    // Fallback
    if (!giantItem || giantItem->getContentSize().width == 0) {
         giantItem = MenuItemImage::create("giant1.png", "giant1.png", [&](Ref*){
            _selectedTroop = soldier::SoldierType::Giant;
            _isDeployMode = true;
            if (_debugLabel) _debugLabel->setString("Selected: Giant");
         });
    }

    giantItem->setPosition(Vec2(startX + 300, y));
    _giantLabel = Label::createWithSystemFont(std::to_string(_giantCount), "Arial", 20);
    _giantLabel->setPosition(Vec2(20, -10));
    giantItem->addChild(_giantLabel);
    
    auto menu = Menu::create(barbItem, archerItem, bomberItem, giantItem, nullptr);
    menu->setPosition(Vec2::ZERO);
    _troopSelectionNode->addChild(menu);

    // Debug Label
    _debugLabel = Label::createWithSystemFont("Debug: Ready", "Arial", 24);
    _debugLabel->setPosition(Vec2(visibleSize.width/2, visibleSize.height - 50));
    _debugLabel->setColor(Color3B::YELLOW);
    this->addChild(_debugLabel, 100);

    // Top Right Info
    auto trLabel = Label::createWithSystemFont("Remaining Troops to Deploy", "Arial", 16);
    trLabel->setPosition(Vec2(visibleSize.width - 120, visibleSize.height - 20));
    this->addChild(trLabel, 20);
    
    _totalTroopLabel = Label::createWithSystemFont("0", "Arial", 24);
    _totalTroopLabel->setPosition(Vec2(visibleSize.width - 120, visibleSize.height - 50));
    this->addChild(_totalTroopLabel, 20);
    updateTotalTroopsUI();
}

void BattleScene::updateTotalTroopsUI() {
    int total = _barbarianCount + _archerCount + _bomberCount + _giantCount;
    if (_totalTroopLabel) _totalTroopLabel->setString(std::to_string(total));
}

bool BattleScene::onTouchBegan(Touch* touch, Event* event) {
    Vec2 loc = touch->getLocation();
    if (loc.y < 100) return false; // Clicked on UI
    return true; // Claim touch for drag/tap
}

void BattleScene::onTouchMoved(Touch* touch, Event* event) {
    Vec2 delta = touch->getDelta();
    _mapNode->setPosition(_mapNode->getPosition() + delta);
}

void BattleScene::onTouchEnded(Touch* touch, Event* event) {
    if (touch->getStartLocation().distance(touch->getLocation()) < 10.0f) {
        // Tap
        if (_isDeployMode) {
            Vec2 loc = touch->getLocation();
            if (loc.y < 100) return;
            Vec2 nodePos = _mapNode->convertToNodeSpace(loc);
            spawnTroop(nodePos);
        }
    }
}

void BattleScene::spawnTroop(Vec2 pos) {
    int* count = nullptr;
    Label* label = nullptr;
    
    switch (_selectedTroop) {
        case soldier::SoldierType::Barbarian: count = &_barbarianCount; label = _barbarianLabel; break;
        case soldier::SoldierType::Archer: count = &_archerCount; label = _archerLabel; break;
        case soldier::SoldierType::Bomber: count = &_bomberCount; label = _bomberLabel; break;
        case soldier::SoldierType::Giant: count = &_giantCount; label = _giantLabel; break;
    }
    
    if (count && *count > 0) {
        (*count)--;
        if (label) label->setString(std::to_string(*count));
        updateTotalTroopsUI();
        
        auto s = soldier::Soldier::create(_selectedTroop);
        s->setPosition(pos);
        _mapNode->addChild(s, 10);
        _friendlyTroops.pushBack(s);
        
        if (_debugLabel) _debugLabel->setString("Spawned!");
    } else {
        // Show error
        auto err = Label::createWithSystemFont("Not enough troops!", "Arial", 30);
        err->setColor(Color3B::RED);
        err->setPosition(Director::getInstance()->getVisibleSize() / 2);
        this->addChild(err, 100);
        err->runAction(Sequence::create(DelayTime::create(0.5f), FadeOut::create(0.5f), RemoveSelf::create(), nullptr));
    }
}

void BattleScene::update(float dt) {
    // 1. Troops Logic
    for (auto s : _friendlyTroops) {
        if (s->isDead()) continue;
        
        // Find Target if none
        if (!s->getTarget() || s->getTarget()->getCurrentHP() <= 0) {
            s->setTarget(findTargetForSoldier(s));
        }
        
        auto target = s->getTarget();
        if (target) {
            float dist = s->getPosition().distance(target->getPosition());
            
            // Attack or Move
            if (dist <= s->getAttackRange()) {
                // Attack
                s->setAttackTimer(s->getAttackTimer() + dt);
                if (s->getAttackTimer() >= s->getAttackInterval()) {
                    s->setAttackTimer(0);
                    
                    if (s->getType() == soldier::SoldierType::Barbarian || s->getType() == soldier::SoldierType::Giant) {
                         // Melee: Bump animation
                         auto move = MoveBy::create(0.1f, (target->getPosition() - s->getPosition()).getNormalized() * 10);
                         s->runAction(Sequence::create(move, move->reverse(), nullptr));
                         target->takeDamage(s->getAttackDamage());
                    } else if (s->getType() == soldier::SoldierType::Archer) {
                         // Ranged: Projectile
                         fireProjectile(s->getPosition(), target->getPosition(), [target, s](){
                             if (target) target->takeDamage(s->getAttackDamage());
                         });
                    } else if (s->getType() == soldier::SoldierType::Bomber) {
                         // Bomber: Projectile + AOE
                         fireProjectile(s->getPosition(), target->getPosition(), [target, s, this](){
                             // AOE Logic
                             Vec2 hitPos = target->getPosition();
                             // Deal damage to target and nearby
                             for (auto b : _enemyBuildings) {
                                 if (b->getPosition().distance(hitPos) <= 64) { // 2x2 grid approx radius
                                     b->takeDamage(s->getAttackDamage());
                                 }
                             }
                         });
                    }
                    
                    // Mark as attacked
                    if (target->getChildren().empty()) { // Simple check to avoid multi-adding
                         // Add goal.png? Prompt says "marked with goal.png"
                         // Maybe just check if child exists by tag
                         if (!target->getChildByTag(123)) {
                             auto mark = Sprite::create("goal.png");
                             mark->setPosition(Vec2(0, target->getContentSize().height + 20));
                             mark->setTag(123);
                             target->addChild(mark);
                         }
                    }
                }
            } else {
                // Move
                Vec2 dir = (target->getPosition() - s->getPosition()).getNormalized();
                s->setPosition(s->getPosition() + dir * s->getMoveSpeed() * dt);
            }
        }
    }
    
    // 2. Defense Logic
    for (auto b : _enemyBuildings) {
        if (b->getCurrentHP() <= 0) {
            b->setVisible(false); // Die
            continue; 
        }
        
        // Is it a defense tower?
        // Check if it has attack damage > 0 (set in init)
        if (b->getAttackDamage() > 0) {
            // Cooldown
            b->setAttackTimer(b->getAttackTimer() + dt);
            if (b->getAttackTimer() >= b->getAttackInterval()) {
                
                // Find nearest troop
                soldier::Soldier* targetTroop = nullptr;
                float minD = 9999;
                float range = b->getAttackRange();
                
                for (auto s : _friendlyTroops) {
                    if (s->isDead()) continue;
                    float d = b->getPosition().distance(s->getPosition());
                    if (d < minD && d <= range) {
                        minD = d;
                        targetTroop = s;
                    }
                }
                
                if (targetTroop) {
                    // Fire
                    b->setAttackTimer(0);
                    
                    // Visuals
                    if (dynamic_cast<building::Cannon*>(b)) {
                        // Cannon: Fire bullet or just recoil
                        // Let's fire a projectile for visibility
                        fireProjectile(b->getPosition(), targetTroop->getPosition(), [targetTroop, b](){
                            if (!targetTroop->isDead()) targetTroop->takeDamage(b->getAttackDamage());
                        });
                        
                        // Recoil animation
                        auto move = MoveBy::create(0.1f, (targetTroop->getPosition() - b->getPosition()).getNormalized() * -5);
                        b->runAction(Sequence::create(move, move->reverse(), nullptr));
                        
                    } else {
                        // Archer Tower: Projectile
                        fireProjectile(b->getPosition(), targetTroop->getPosition(), [targetTroop, b](){
                            if (!targetTroop->isDead()) targetTroop->takeDamage(b->getAttackDamage());
                        });
                    }
                }
            }
        }
    }
    
    // Clean up dead
    // ...
    
    checkWinCondition();
}

building::Building* BattleScene::findTargetForSoldier(soldier::Soldier* s) {
    building::Building* target = nullptr;
    float minD = 99999;
    
    bool prioritizeDefense = (s->getType() == soldier::SoldierType::Bomber || s->getType() == soldier::SoldierType::Giant);
    
    if (prioritizeDefense) {
        for (auto b : _enemyBuildings) {
            if (b->getCurrentHP() <= 0) continue;
            bool isDef = (dynamic_cast<building::Cannon*>(b) || dynamic_cast<building::ArcherTower*>(b));
            if (isDef) {
                float d = s->getPosition().distance(b->getPosition());
                if (d < minD) {
                    minD = d;
                    target = b;
                }
            }
        }
    }
    
    if (!target) {
        // Find nearest any
        minD = 99999;
        for (auto b : _enemyBuildings) {
            if (b->getCurrentHP() <= 0) continue;
            float d = s->getPosition().distance(b->getPosition());
            if (d < minD) {
                minD = d;
                target = b;
            }
        }
    }
    
    return target;
}

void BattleScene::fireProjectile(Vec2 start, Vec2 end, std::function<void()> onHit) {
    auto bullet = Sprite::create("bullet.png");
    bullet->setPosition(start);
    _mapNode->addChild(bullet, 100);
    
    float dist = start.distance(end);
    float duration = dist / 300.0f; // Speed 300
    
    auto move = MoveTo::create(duration, end);
    auto cb = CallFunc::create([bullet, onHit](){
        bullet->removeFromParent();
        if (onHit) onHit();
    });
    
    bullet->runAction(Sequence::create(move, cb, nullptr));
}

void BattleScene::checkWinCondition() {
    bool enemyTHAlive = false;
    bool anyEnemyAlive = false;
    for (auto b : _enemyBuildings) {
        if (b->getCurrentHP() > 0) {
            anyEnemyAlive = true;
            if (dynamic_cast<building::TownHall*>(b)) {
                enemyTHAlive = true;
            }
        }
    }
    
    if (!enemyTHAlive) {
        showResult(true);
        return;
    }
    
    bool anyTroopAlive = false;
    for (auto s : _friendlyTroops) {
        if (!s->isDead()) {
            anyTroopAlive = true;
            break;
        }
    }
    
    if (!anyTroopAlive && _barbarianCount == 0 && _archerCount == 0 && _bomberCount == 0 && _giantCount == 0) {
        // No troops left on field or in reserve
        showResult(false);
    }
}

void BattleScene::showResult(bool win) {
    this->unscheduleUpdate();
    
    auto layer = LayerColor::create(Color4B(0, 0, 0, 150));
    this->addChild(layer, 999);
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    
    auto img = Sprite::create(win ? "victory.png" : "failure.png");
    if (img) {
        img->setPosition(Vec2(visibleSize.width/2, visibleSize.height/2 + 50));
        layer->addChild(img);
    } else {
        auto label = Label::createWithSystemFont(win ? "VICTORY!" : "DEFEAT", "Arial", 60);
        label->setColor(win ? Color3B::YELLOW : Color3B::RED);
        label->setPosition(Vec2(visibleSize.width/2, visibleSize.height/2 + 50));
        layer->addChild(label);
    }
    
    auto btn = MenuItemImage::create("end_botton.png", "end_botton.png", [&](Ref*){
        returnToVillage();
    });
    auto menu = Menu::create(btn, nullptr);
    menu->setPosition(Vec2(visibleSize.width/2, visibleSize.height/2 - 100));
    layer->addChild(menu);
}

void BattleScene::returnToVillage() {
    Director::getInstance()->popScene();
}

} // namespace scene
