#include "BattleScene.h"
#include "VillageScene.h"
#include "SimpleAudioEngine.h"
#include "../building/TownHall.h"
#include "../building/Cannon.h"
#include "../building/ArcherTower.h"
#include "../building/Barracks.h"
#include "../building/TrainingCamp.h"
#include "../building/GoldStorage.h"
#include "../building/ElixirStorage.h"
#include "../building/Landmine.h"
#include "MainMenuScene.h"
#include <queue>
#include <set>
#include <cmath>
#include <algorithm>

USING_NS_CC;

namespace scene {

std::vector<BattleRecord> BattleScene::s_battleHistory;

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

Scene* BattleScene::createReplayScene(const BattleRecord& record) {
    return BattleScene::createReplay(record);
}

BattleScene* BattleScene::createReplay(const BattleRecord& record) {
    BattleScene *pRet = new(std::nothrow) BattleScene();
    if (pRet && pRet->initReplay(record)) {
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
    
    // Initialize current record for recording
    _currentRecord.level = level;
    _currentRecord.initBarb = barbarianCount;
    _currentRecord.initArch = archerCount;
    _currentRecord.initBomb = bomberCount;
    _currentRecord.initGiant = giantCount;
    _currentRecord.events.clear();
    _currentRecord.isWin = false;

    _battleTimer = 0;
    _isReplay = false;
    _isDeployMode = false;
    
    // Background & Map
    _mapNode = cocos2d::Node::create();
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

    // Play Battle Music
    CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic("combat_music.mp3", true);
    
    return true;
}

bool BattleScene::initReplay(const BattleRecord& record) {
    // Reuse normal init with record params
    if (!this->init(record.level, record.initBarb, record.initArch, record.initBomb, record.initGiant)) return false;
    
    _isReplay = true;
    _replayRecord = record;
    _replayEventIndex = 0;
    
    // Hide UI
    if (_troopSelectionNode) _troopSelectionNode->setVisible(false);
    
    // Hide Surrender Button
    auto surrenderMenu = this->getChildByTag(999);
    if (surrenderMenu) {
        surrenderMenu->setVisible(false);
    }
    
    // Hide Put Soldier Button
    auto putSoldierMenu = this->getChildByTag(998);
    if (putSoldierMenu) {
        putSoldierMenu->setVisible(false);
    }
    
    // Add "Replay Mode" Sprite
    auto replaySprite = Sprite::create("replay_mode.png");
    if (replaySprite) {
        Vec2 origin = Director::getInstance()->getVisibleOrigin();
        Size visibleSize = Director::getInstance()->getVisibleSize();
        
        replaySprite->setAnchorPoint(Vec2(0, 1)); // Top-Left
        replaySprite->setPosition(Vec2(origin.x + 20, origin.y + visibleSize.height - 20));
        this->addChild(replaySprite, 100);
    }
    
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
        b->initHPBar(); // Initialize HP Bar
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

    // Landmines
    _landmines.clear();
    auto addMine = [&](Vec2 pos) {
        auto m = building::Landmine::create();
        m->setPosition(pos);
        m->setVisible(false); // Invisible in Battle
        _mapNode->addChild(m);
        _landmines.pushBack(m);
    };

    int tileSize = 32;

    // Simple (Level >= 1)
    if (level >= 1) {
        addMine(center + Vec2(0, 3 * tileSize));
        addMine(center + Vec2(0, -3 * tileSize));
        addMine(center + Vec2(3 * tileSize, 0));
        addMine(center + Vec2(-3 * tileSize, 0));
    }
    
    // Medium (Level >= 2)
    if (level >= 2) {
        // Outskirts 4 directions (Diagonals at 12 tiles)
        int dist = 12 * tileSize;
        addMine(center + Vec2(dist, dist));
        addMine(center + Vec2(-dist, dist));
        addMine(center + Vec2(dist, -dist));
        addMine(center + Vec2(-dist, -dist));
    }
    
    // Hard (Level >= 3)
    if (level >= 3) {
        // Outskirts 8 directions (Add Cardinals at 12 tiles)
        int dist = 12 * tileSize;
        addMine(center + Vec2(0, dist));
        addMine(center + Vec2(0, -dist));
        addMine(center + Vec2(dist, 0));
        addMine(center + Vec2(-dist, 0));
    }

    initCollisionMap();

    // Visualize Invalid Deployment Zone (Red Envelope)
    if (!_enemyBuildings.empty()) {
        float minX = 0, minY = 0, maxX = 0, maxY = 0;
        bool hasBounds = false;

        for (auto b : _enemyBuildings) {
            Rect r = b->getBoundingBox();
            if (!hasBounds) {
                minX = r.getMinX();
                minY = r.getMinY();
                maxX = r.getMaxX();
                maxY = r.getMaxY();
                hasBounds = true;
            } else {
                if (r.getMinX() < minX) minX = r.getMinX();
                if (r.getMinY() < minY) minY = r.getMinY();
                if (r.getMaxX() > maxX) maxX = r.getMaxX();
                if (r.getMaxY() > maxY) maxY = r.getMaxY();
            }
        }

        if (hasBounds) {
            float margin = 64.0f;
            Vec2 bl(minX - margin, minY - margin);
            Vec2 tr(maxX + margin, maxY + margin);
            
            auto drawNode = DrawNode::create();
            // Draw red rectangle frame
            Vec2 points[4] = {
                Vec2(bl.x, bl.y),
                Vec2(tr.x, bl.y),
                Vec2(tr.x, tr.y),
                Vec2(bl.x, tr.y)
            };
            drawNode->drawPolygon(points, 4, Color4F(0, 0, 0, 0), 0.7f, Color4F::RED); // Transparent fill, Red border, thickness 0.7
            _mapNode->addChild(drawNode, 1000); // High Z-order to be visible
        }
    }
}

void BattleScene::setupUI() {
    Size visibleSize = Director::getInstance()->getVisibleSize();
    
    // 1. Put Soldier Button (Bottom Left)
    MenuItem* putBtn = MenuItemImage::create("put_soldier.png", "put_soldier.png", [&](Ref*){
        CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("botton.mp3");
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
    mainMenu->setTag(998); // Tag for Put Soldier menu
    this->addChild(mainMenu, 20);

    // Surrender Button (Bottom Right)
    MenuItem* surrenderItem = MenuItemImage::create("surrend_botton.png", "surrend_botton.png", [this](Ref*){
        CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("botton.mp3");
        showResult(false);
    });
    if (!surrenderItem || surrenderItem->getContentSize().width == 0) {
        auto label = Label::createWithSystemFont("Surrender", "Arial", 20);
        surrenderItem = MenuItemLabel::create(label, [this](Ref*){
            showResult(false);
        });
    }
    auto surrenderMenu = Menu::create(surrenderItem, nullptr);
    surrenderMenu->setPosition(Vec2(visibleSize.width - 100, 50)); // Moved 50px left (from -50 to -100)
    surrenderMenu->setTag(999); // Tag for finding it later
    this->addChild(surrenderMenu, 20);

    // 2. Bottom Bar (Initially Hidden)
    _troopSelectionNode = cocos2d::Node::create();
    this->addChild(_troopSelectionNode, 10);
    _troopSelectionNode->setVisible(false);
    
    auto bar = LayerColor::create(Color4B(0, 0, 0, 150), visibleSize.width, 100);
    bar->setPosition(Vec2::ZERO);
    _troopSelectionNode->addChild(bar);
    
    float startX = visibleSize.width / 2 - 150;
    float y = 50;
    
    // Barbarian: barbarian1.png
    auto barbItem = MenuItemImage::create("barbarian1.png", "barbarian1.png", [&](Ref*){
        CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("botton.mp3");
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
        CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("botton.mp3");
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
    if (_isReplay) return true; // Consume touch but do nothing
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
                
                // Validate deploy position: must be OUTSIDE the envelope of enemy buildings
                float minX = 0, minY = 0, maxX = 0, maxY = 0;
                bool hasBounds = false;
                for (auto b : _enemyBuildings) {
                    Rect r = b->getBoundingBox();
                    if (!hasBounds) {
                        minX = r.getMinX();
                        minY = r.getMinY();
                        maxX = r.getMaxX();
                        maxY = r.getMaxY();
                        hasBounds = true;
                    } else {
                        if (r.getMinX() < minX) minX = r.getMinX();
                        if (r.getMinY() < minY) minY = r.getMinY();
                        if (r.getMaxX() > maxX) maxX = r.getMaxX();
                        if (r.getMaxY() > maxY) maxY = r.getMaxY();
                    }
                }
                
                bool isLegal = true;
                if (hasBounds) {
                    float margin = 64.0f; // Reasonable margin outside the village envelope
                    Rect envelope(minX - margin, minY - margin, (maxX - minX) + margin * 2, (maxY - minY) + margin * 2);
                    if (envelope.containsPoint(nodePos)) {
                        isLegal = false;
                    }
                }
                
                if (!isLegal) {
                    auto err = cocos2d::Label::createWithSystemFont("Invalid placement", "Arial", 28);
                    err->setColor(Color3B::RED);
                    err->setPosition(loc);
                    this->addChild(err, 200);
                    err->runAction(Sequence::create(DelayTime::create(0.5f), FadeOut::create(0.5f), RemoveSelf::create(), nullptr));
                    return;
                }
                
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

        // Record event if not replay
        if (!_isReplay) {
            DeploymentEvent evt;
            evt.time = _battleTimer;
            evt.soldierType = (int)_selectedTroop;
            evt.x = pos.x;
            evt.y = pos.y;
            _currentRecord.events.push_back(evt);
        }

    } else {
        // Show error
        auto err = cocos2d::Label::createWithSystemFont("Not enough troops!", "Arial", 30);
        err->setColor(Color3B::RED);
        err->setPosition(Director::getInstance()->getVisibleSize() / 2);
        this->addChild(err, 100);
        err->runAction(Sequence::create(DelayTime::create(0.5f), FadeOut::create(0.5f), RemoveSelf::create(), nullptr));
    }
}

void BattleScene::update(float dt) {
    _battleTimer += dt;
    
    if (_isReplay) {
        while (_replayEventIndex < _replayRecord.events.size()) {
            const auto& evt = _replayRecord.events[_replayEventIndex];
            if (_battleTimer >= evt.time) {
                _selectedTroop = (soldier::SoldierType)evt.soldierType;
                spawnTroop(Vec2(evt.x, evt.y));
                _replayEventIndex++;
            } else {
                break;
            }
        }
        
        // End replay check
        if (_replayEventIndex >= _replayRecord.events.size()) {
            // All events played, wait a bit then show result
            // Check if all troops dead or won?
            // Or just check if simulation settled.
            // For now, let's trust the simulation to reach the same end state,
            // but we need to trigger "End Replay" eventually.
            // Let's just check win condition manually OR wait for no troops?
            // Actually, we should just let the simulation run.
            // But we need to know when to stop.
            // Let's use the win/loss state from record? Or recalculate?
            // If we recalculate, we can see if it diverges.
            // But the user said "Directly shows victory". 
            // That was because checkWinCondition() was running immediately on init because no troops were spawned yet!
            
            // Now that checkWinCondition() is skipped for replay, we need a way to end it.
            // Let's check win condition ONLY if some time has passed or events finished.
            
            bool anyTroopAlive = false;
            for (auto s : _friendlyTroops) {
                if (!s->isDead()) {
                    anyTroopAlive = true;
                    break;
                }
            }
            
            // If all events done AND (no troops left OR win), then show result.
            // Or simply, if we want to mimic the exact end time, we should record end time.
            // Without end time, we wait until no action.
            
            // Simple approach: Allow standard checkWinCondition to run AFTER all events are spawned.
            if (_replayEventIndex >= _replayRecord.events.size()) {
                 // Re-enable win check logic locally
                 bool enemyTHAlive = false;
                 for (auto b : _enemyBuildings) {
                     if (dynamic_cast<building::TownHall*>(b) && b->getCurrentHP() > 0) {
                         enemyTHAlive = true;
                         break;
                     }
                 }
                 if (!enemyTHAlive) {
                     showResult(true);
                 } else if (!anyTroopAlive) {
                      // Only if all troops died
                      showResult(false);
                 }
            }
        }
    }

    // 1. Troops Logic
    for (auto s : _friendlyTroops) {
        if (s->isDead()) continue;
        
        // Find Target if none
        if (!s->getTarget() || s->getTarget()->getCurrentHP() <= 0) {
            auto newTarget = findTargetForSoldier(s);
            s->setTarget(newTarget);
            if (newTarget) {
                s->setPath(findPath(s->getPosition(), newTarget->getPosition()));
            }
        }
        
        auto target = s->getTarget();
        if (target) {
            float dist = s->getPosition().distance(target->getPosition());
            
            // Adjust range for target size (Edge-to-Edge)
            Rect targetBox = target->getBoundingBox();
            // Use slightly smaller radius to ensure we are well within range, or full radius?
            // Usually Range is "distance to target". If target is big, center is far.
            // Let's add the "radius" of the target to the soldier's range.
            float targetRadius = std::max(targetBox.size.width, targetBox.size.height) / 2.0f;
            // Reduce slightly (e.g. -5) to require getting a bit closer than just touching the bounding box, 
            // or just use full radius. Full radius is safer to avoid "stuck but cant attack".
            float effectiveRange = s->getAttackRange() + targetRadius;
            
            // Attack or Move
            if (dist <= effectiveRange) {
                // Attack
                s->setAttackTimer(s->getAttackTimer() + dt);
                if (s->getAttackTimer() >= s->getAttackInterval()) {
                    s->setAttackTimer(0);
                    
                    if (s->getType() == soldier::SoldierType::Barbarian || s->getType() == soldier::SoldierType::Giant) {
                         // Melee: Bump animation
                         auto move = MoveBy::create(0.1f, (target->getPosition() - s->getPosition()).getNormalized() * 10);
                         s->runAction(Sequence::create(move, move->reverse(), nullptr));
                         target->takeDamage(s->getAttackDamage());

                         if (s->getType() == soldier::SoldierType::Barbarian) {
                             CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("barbarian_attacksound.mp3");
                         } else {
                             CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("giant_attacksound.mp3");
                         }
                    } else if (s->getType() == soldier::SoldierType::Archer) {
                         // Ranged: Projectile
                         CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("archery.mp3");
                         fireProjectile(s->getPosition(), target->getPosition(), [target, s](){
                             if (target) target->takeDamage(s->getAttackDamage());
                         });
                    } else if (s->getType() == soldier::SoldierType::Bomber) {
                         // Bomber: Projectile + AOE
                         fireProjectile(s->getPosition(), target->getPosition(), [target, s, this](){
                             // Explosion Sound
                             CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("boom.mp3");
                             // AOE Logic
                             Vec2 hitPos = target->getPosition();
                             // Deal damage to target and nearby
                             for (auto b : _enemyBuildings) {
                                 if (b->getPosition().distance(hitPos) <= 64) { // 2x2 grid approx radius
                                     b->takeDamage(s->getAttackDamage());
                                 }
                             }
                         }, "bomb.png");
                    }
                    
                    // Mark as attacked
                }
            } else {
                // Move using Pathfinding
                if (!s->hasPath()) {
                     // Try to calculate path again if lost or finished but not in range
                     s->setPath(findPath(s->getPosition(), target->getPosition()));
                }
                s->moveAlongPath(dt);
            }
        }
    }
    
    // 2. Defense Logic
    updateLandmines(dt);
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
                        // Rotate Cannon to face target (Assuming texture faces LEFT)
                        Vec2 diff = targetTroop->getPosition() - b->getPosition();
                        float angleRad = atan2(diff.y, diff.x);
                        float angleDeg = CC_RADIANS_TO_DEGREES(angleRad);
                        b->setRotation(180 - angleDeg);

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

void BattleScene::fireProjectile(Vec2 start, Vec2 end, std::function<void()> onHit, std::string texture) {
    auto bullet = Sprite::create(texture);
    if (!bullet) {
        bullet = Sprite::create("bullet.png");
    }
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
    if (_isReplay) return; // Replay controls its own ending or follows events, don't auto-check logic

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
    if (!_isReplay) {
        _currentRecord.isWin = win;
        _currentRecord.id = s_battleHistory.size() + 1;
        _currentRecord.timestamp = "Battle " + std::to_string(_currentRecord.id);
        s_battleHistory.push_back(_currentRecord);
    }

    this->unscheduleUpdate();
    
    auto layer = LayerColor::create(Color4B(0, 0, 0, 150));
    this->addChild(layer, 999);
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    
    auto img = Sprite::create(win ? "victory.png" : "lose.png");
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
        CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("botton.mp3");
        returnToVillage();
    });
    auto menu = Menu::create(btn, nullptr);
    menu->setPosition(Vec2(visibleSize.width/2, visibleSize.height/2 - 100));
    layer->addChild(menu);
}

void BattleScene::returnToVillage() {
    Director::getInstance()->popScene();
}

// Pathfinding Implementation
void BattleScene::initCollisionMap() {
    _collisionMap.assign(_mapWidth * _mapHeight, true);
    for (auto b : _enemyBuildings) {
        // Walls are also obstacles (unless destroyed, but for now static map)
        
        Rect bbox = b->getBoundingBox();
        
        // Reduce collision size by 50% as requested (centered)
        float cx = bbox.getMidX();
        float cy = bbox.getMidY();
        float newW = bbox.size.width * 0.5f;
        float newH = bbox.size.height * 0.5f;
        
        // Apply shrinking to avoid boundary issues
        float minX = cx - newW / 2.0f + 1;
        float minY = cy - newH / 2.0f + 1;
        float maxX = cx + newW / 2.0f - 1;
        float maxY = cy + newH / 2.0f - 1;
        
        Vec2 min = worldToGrid(Vec2(minX, minY));
        Vec2 max = worldToGrid(Vec2(maxX, maxY));
        
        for (int y = (int)min.y; y <= (int)max.y; ++y) {
            for (int x = (int)min.x; x <= (int)max.x; ++x) {
                if (x >= 0 && x < _mapWidth && y >= 0 && y < _mapHeight) {
                    _collisionMap[y * _mapWidth + x] = false;
                }
            }
        }
    }
}

Vec2 BattleScene::worldToGrid(Vec2 worldPos) {
    return Vec2((int)(worldPos.x / 32), (int)(worldPos.y / 32));
}

Vec2 BattleScene::gridToWorld(Vec2 gridPos) {
    return Vec2(gridPos.x * 32 + 16, gridPos.y * 32 + 16);
}

bool BattleScene::isWalkable(int x, int y) {
    if (x < 0 || x >= _mapWidth || y < 0 || y >= _mapHeight) return false;
    return _collisionMap[y * _mapWidth + x];
}

std::vector<Vec2> BattleScene::findPath(Vec2 start, Vec2 end) {
    Vec2 startGrid = worldToGrid(start);
    Vec2 endGrid = worldToGrid(end);
    
    // If start == end, return empty
    if (startGrid == endGrid) return {};
    
    // If end is blocked, find nearest walkable neighbor? 
    // Or just path to it as close as possible?
    // For now, if end is blocked (it IS blocked because it's a building), we target a neighbor.
    // The target is the building, so endGrid is inside the building.
    // We need to find a path to a tile ADJACENT to the building.
    
    if (!isWalkable(endGrid.x, endGrid.y)) {
        // Find nearest walkable neighbor to start
        // Simple BFS or just check neighbors
        // Actually, A* to the "nearest walkable tile to end" is better.
        // Let's modify A* goal condition: if we reach a tile adjacent to endGrid (if endGrid is the target building).
        // But generic A* goes to exact tile.
        
        // Strategy: Find all walkable tiles adjacent to the target building.
        // Pick the one closest to start.
        // Set that as new endGrid.
        
        float minD = 99999;
        Vec2 bestEnd = endGrid;
        bool found = false;
        
        // Search radius around endGrid (assuming building size up to 5x5)
        for (int dy = -4; dy <= 4; ++dy) {
            for (int dx = -4; dx <= 4; ++dx) {
                int nx = endGrid.x + dx;
                int ny = endGrid.y + dy;
                if (isWalkable(nx, ny)) {
                    // Check if this tile is adjacent to the blocked region of the target?
                    // Or simply: closest walkable tile to the original endGrid.
                    float d = startGrid.distance(Vec2(nx, ny)) + Vec2(nx, ny).distance(endGrid)*0.1f; // Heuristic
                    // Actually we want closest to TARGET, but reachable from START.
                    // Just closest to target center is good.
                    float d2 = Vec2(nx, ny).distance(endGrid);
                    
                    if (d2 < minD) {
                         minD = d2;
                         bestEnd = Vec2(nx, ny);
                         found = true;
                    }
                }
            }
        }
        if (found) endGrid = bestEnd;
        else return {}; // No reachable tile found
    }
    
    // Standard A*
    struct NodeWrapper {
        int x, y;
        int g, h;
        NodeWrapper* parent;
        NodeWrapper(int _x, int _y) : x(_x), y(_y), g(0), h(0), parent(nullptr) {}
        int f() const { return g + h; }
    };
    
    auto comp = [](NodeWrapper* a, NodeWrapper* b) { return a->f() > b->f(); };
    std::priority_queue<NodeWrapper*, std::vector<NodeWrapper*>, decltype(comp)> openList(comp);
    std::vector<NodeWrapper*> allNodes; // For cleanup
    std::set<std::pair<int, int>> closedList;
    std::set<std::pair<int, int>> openSet; // For fast lookup
    
    NodeWrapper* startNode = new NodeWrapper(startGrid.x, startGrid.y);
    startNode->g = 0;
    startNode->h = (std::abs(endGrid.x - startGrid.x) + std::abs(endGrid.y - startGrid.y)) * 10;
    
    openList.push(startNode);
    openSet.insert({startNode->x, startNode->y});
    allNodes.push_back(startNode);
    
    NodeWrapper* finalNode = nullptr;
    
    int directions[8][2] = {
        {0, 1}, {0, -1}, {1, 0}, {-1, 0}, // Cardinal
        {1, 1}, {1, -1}, {-1, 1}, {-1, -1} // Diagonal
    };
    
    while (!openList.empty()) {
        NodeWrapper* current = openList.top();
        openList.pop();
        openSet.erase({current->x, current->y});
        
        if (current->x == endGrid.x && current->y == endGrid.y) {
            finalNode = current;
            break;
        }
        
        closedList.insert({current->x, current->y});
        
        for (int i = 0; i < 8; ++i) {
            int nx = current->x + directions[i][0];
            int ny = current->y + directions[i][1];
            
            if (!isWalkable(nx, ny)) continue;
            if (closedList.count({nx, ny})) continue;
            
            // Diagonal check: prevent cutting corners if adjacent cardinals are blocked
            if (i >= 4) {
                if (!isWalkable(current->x + directions[i][0], current->y) ||
                    !isWalkable(current->x, current->y + directions[i][1])) {
                    continue;
                }
            }
            
            int moveCost = (i >= 4) ? 14 : 10;
            int newG = current->g + moveCost;
            
            bool inOpen = openSet.count({nx, ny});
            
            // Simplification: Not checking if new path to existing open node is better (usually not needed for simple grid)
            // But strict A* requires it. 
            // For now, simple implementation: if in open, skip (suboptimal but faster), or update.
            // Let's just add if not in open.
            
            if (!inOpen) {
                NodeWrapper* neighbor = new NodeWrapper(nx, ny);
                neighbor->g = newG;
                neighbor->h = (std::abs(endGrid.x - nx) + std::abs(endGrid.y - ny)) * 10;
                neighbor->parent = current;
                openList.push(neighbor);
                openSet.insert({nx, ny});
                allNodes.push_back(neighbor);
            }
        }
    }
    
    std::vector<Vec2> path;
    if (finalNode) {
        NodeWrapper* curr = finalNode;
        while (curr) {
            path.push_back(gridToWorld(Vec2(curr->x, curr->y)));
            curr = curr->parent;
        }
        std::reverse(path.begin(), path.end());
    }
    
    // Cleanup
    for (auto n : allNodes) delete n;
    
    return path;
}

void BattleScene::updateLandmines(float dt) {
    for (auto it = _landmines.begin(); it != _landmines.end(); ) {
        auto mine = *it;
        if (!mine->getParent()) {
            it = _landmines.erase(it);
            continue;
        }
        
        if (mine->isTriggered()) {
            ++it;
            continue;
        }
        
        bool triggered = false;
        // Trigger Range 2x2: approx distance check
        // Center to Center distance. If tile is 32, 2x2 is 64x64.
        // Radius approx 40-50 pixels.
        for (auto s : _friendlyTroops) {
            if (s->isDead()) continue;
            if (s->getPosition().distance(mine->getPosition()) < 50.0f) {
                triggered = true;
                break;
            }
        }
        
        if (triggered) {
            mine->startTriggerSequence([this, mine](){
                CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("boom.mp3");
                Vec2 minePos = mine->getPosition();
                float range = 3.0f * 32.0f;
                int damage = 500;
                
                // Explosion Visual (Fallback)
                auto node = DrawNode::create();
                node->drawSolidCircle(Vec2::ZERO, range, 0, 20, Color4F(1, 0, 0, 0.5f));
                node->setPosition(minePos);
                _mapNode->addChild(node, 100);
                node->runAction(Sequence::create(FadeOut::create(0.5f), RemoveSelf::create(), nullptr));
                
                // Damage and Pushback
                for (auto s : _friendlyTroops) {
                    if (s->isDead()) continue;
                    float dist = s->getPosition().distance(minePos);
                    if (dist <= range) {
                        s->takeDamage(damage);
                        
                        // Pushback
                        if (!s->isDead()) {
                             Vec2 dir = (s->getPosition() - minePos).getNormalized();
                             if (dir.isZero()) dir = Vec2(1,0);
                             
                             // Push to range + 20
                             Vec2 newPos = minePos + dir * (range + 20.0f);
                             
                             auto push = MoveTo::create(0.1f, newPos);
                             s->runAction(push);
                        }
                    }
                }
            });
        }
        
        ++it;
    }
}

} // namespace scene
