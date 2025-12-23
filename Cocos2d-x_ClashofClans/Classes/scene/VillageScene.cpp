#include "VillageScene.h"
#include "MainMenuScene.h"
#include "BattleScene.h"
#include "building/Cannon.h"
#include "building/Barracks.h"
#include "building/TrainingCamp.h"
#include "building/Wall.h"
#include <vector>

USING_NS_CC;

namespace scene {

Scene* VillageScene::createScene() {
    return VillageScene::create();
}

bool VillageScene::init() {
    if (!Scene::init()) {
        return false;
    }

    _gold = 1000;
    _elixir = 1000;
    _population = 2; // Default builders
    _isBuildMode = false;
    _currentWindow = nullptr;
    _mapGrid = nullptr;

    setupMap();
    setupUI(); // Setup UI layer first so it's ready
    setupBuildings(); // Place initial buildings
    setupTouchHandling();

    this->scheduleUpdate(); // Enable update loop for resources

    return true;
}

void VillageScene::update(float dt) {
    // 1. Production
    double goldRate = 0;
    double elixirRate = 0;

    for (auto b : _buildings) {
        if (auto mine = dynamic_cast<building::GoldMine*>(b)) {
            // Rate depends on level: 1->6, 2->12, 3->20
            int level = mine->getLevel();
            if (level == 1) goldRate += 6;
            else if (level == 2) goldRate += 12;
            else if (level == 3) goldRate += 20;
        } else if (auto collector = dynamic_cast<building::ElixirCollector*>(b)) {
            // Rate: 1->6, 2->12, 3->20
            int level = collector->getLevel();
            if (level == 1) elixirRate += 6;
            else if (level == 2) elixirRate += 12;
            else if (level == 3) elixirRate += 20;
        }
    }

    // Add per second (dt is seconds elapsed)
    double goldToAdd = goldRate * dt;
    double elixirToAdd = elixirRate * dt;

    // 2. Storage Limits
    int maxGold = getResourceCapacity(true);
    int maxElixir = getResourceCapacity(false);

    // Logic: "When gold reaches capacity, gold stops increasing"
    if (_gold < maxGold) {
        _gold += goldToAdd;
        if (_gold > maxGold) _gold = maxGold;
    }
    
    if (_elixir < maxElixir) {
        _elixir += elixirToAdd;
        if (_elixir > maxElixir) _elixir = maxElixir;
    }

    // Update UI (maybe throttle this if performance issue, but text update is cheap)
    updateResourceLabels();
}

int VillageScene::getResourceCapacity(bool isGold) const {
    int capacity = 0;
    for (auto b : _buildings) {
        if (isGold) {
            if (auto storage = dynamic_cast<building::GoldStorage*>(b)) {
                int level = storage->getLevel();
                if (level == 1) capacity += 600;
                else if (level == 2) capacity += 1200;
                else if (level == 3) capacity += 1800;
            }
        } else {
            if (auto storage = dynamic_cast<building::ElixirStorage*>(b)) {
                int level = storage->getLevel();
                if (level == 1) capacity += 600;
                else if (level == 2) capacity += 1200;
                else if (level == 3) capacity += 1800;
            }
        }
    }
    // Base capacity if 0 storages? Usually there is a small base capacity in TownHall (e.g. 1000)
    // Requirement says "Gold Storage determines max limit".
    // "Game starts with 1 Gold Storage Lv1 (Cap 600)".
    // Plus we have initial 1000 gold. This exceeds cap.
    // Let's assume Town Hall also has capacity or initial resources can exceed cap.
    // Requirement: "Gold stops increasing when it reaches cap". Doesn't say it clamps immediately if over.
    // But usually Town Hall has capacity (e.g. 1000).
    // Let's add Town Hall capacity (usually 1000 for Lv1).
    // For this assignment, let's stick to Storage buildings capacity as requested.
    // "Gold Storage... determines max limit".
    // If I have 1000 gold and cap is 600, I just can't collect more.
    return capacity > 0 ? capacity : 1000; // Fallback to 1000 to prevent stuck at 0 if no storage
}

void VillageScene::setupMap() {
    _mapNode = Node::create();
    this->addChild(_mapNode, 0);

    int mapWidth = 50;
    int mapHeight = 50;
    int tileSize = 32;

    try {
        _mapGrid = new core::Grid<Sprite*>(mapWidth, mapHeight);
    } catch (const std::exception& e) {
        CCLOG("Error creating grid: %s", e.what());
        return;
    }

    for (int x = 0; x < mapWidth; ++x) {
        for (int y = 0; y < mapHeight; ++y) {
            std::string textureFile = ((x + y) % 2 == 0) ? "grass1.png" : "grass2.png";
            auto tile = Sprite::create(textureFile);
            if (tile) {
                tile->setAnchorPoint(Vec2::ZERO);
                tile->setPosition(Vec2(x * tileSize, y * tileSize));
                _mapNode->addChild(tile, 0); // z=0 for ground
                _mapGrid->at(x, y) = tile;
            }
        }
    }
    
    // Center map initially
    auto visibleSize = Director::getInstance()->getVisibleSize();
    _mapNode->setPosition(Vec2(visibleSize.width/2 - (mapWidth * tileSize)/2, 
                               visibleSize.height/2 - (mapHeight * tileSize)/2));
}

void VillageScene::setupBuildings() {
    // Initial buildings: TownHall, GoldMine, ElixirCollector, GoldStorage, ElixirStorage
    
    // 1. Town Hall
    placeBuilding(building::BuildingType::TownHall, Vec2(25, 25));

    // 2. Gold Mine
    placeBuilding(building::BuildingType::GoldMine, Vec2(22, 22));

    // 3. Elixir Collector
    placeBuilding(building::BuildingType::ElixirCollector, Vec2(28, 22));

    // 4. Gold Storage
    placeBuilding(building::BuildingType::GoldStorage, Vec2(22, 28));

    // 5. Elixir Storage
    placeBuilding(building::BuildingType::ElixirStorage, Vec2(28, 28));
}

void VillageScene::setupUI() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    
    _uiLayer = Layer::create();
    this->addChild(_uiLayer, 100); // Top layer

    // 1. Attack Mode Button (Top Left)
    MenuItem* attackItem = MenuItemImage::create("attack_mode.png", "attack_mode.png", CC_CALLBACK_1(VillageScene::onAttackModeCallback, this));
    if (attackItem == nullptr || attackItem->getContentSize().width == 0) {
        auto label = Label::createWithSystemFont("Attack", "Arial", 20);
        attackItem = MenuItemLabel::create(label, CC_CALLBACK_1(VillageScene::onAttackModeCallback, this));
    }
    
    // Return Button (Below Attack Mode)
    MenuItem* returnItem = MenuItemImage::create("return_botton.png", "return_botton.png", [](Ref* sender){
        Director::getInstance()->replaceScene(MainMenuScene::createScene());
    });
    if (returnItem == nullptr || returnItem->getContentSize().width == 0) {
        auto label = Label::createWithSystemFont("Return", "Arial", 20);
        returnItem = MenuItemLabel::create(label, [](Ref* sender){
            Director::getInstance()->replaceScene(MainMenuScene::createScene());
        });
    }

    // History Button
    MenuItem* historyItem = MenuItemImage::create("history.png", "history.png", CC_CALLBACK_1(VillageScene::onHistoryCallback, this));
    if (historyItem == nullptr || historyItem->getContentSize().width == 0) {
        auto label = Label::createWithSystemFont("Battle History", "Arial", 20);
        historyItem = MenuItemLabel::create(label, CC_CALLBACK_1(VillageScene::onHistoryCallback, this));
    }
    
    auto attackMenu = Menu::create(attackItem, returnItem, historyItem, nullptr);
    attackMenu->alignItemsVerticallyWithPadding(10);
    attackMenu->setPosition(Vec2(origin.x + 50, origin.y + visibleSize.height - 100)); // Adjusted position to fit 3 buttons
    _uiLayer->addChild(attackMenu);

    // 2. Resources (Top Right)
    int startX = origin.x + visibleSize.width - 100;
    int startY = origin.y + visibleSize.height - 30;

    // Gold
    auto coinIcon = Sprite::create("coin.png");
    if (!coinIcon) coinIcon = Sprite::create();
    coinIcon->setPosition(Vec2(startX - 200, startY));
    _uiLayer->addChild(coinIcon);
    _goldLabel = Label::createWithSystemFont(std::to_string(_gold), "Arial", 16);
    _goldLabel->setPosition(Vec2(startX - 160, startY));
    _uiLayer->addChild(_goldLabel);

    // Elixir
    auto elixirIcon = Sprite::create("elixir.png");
    if (!elixirIcon) elixirIcon = Sprite::create();
    elixirIcon->setPosition(Vec2(startX - 100, startY));
    _uiLayer->addChild(elixirIcon);
    _elixirLabel = Label::createWithSystemFont(std::to_string(_elixir), "Arial", 16);
    _elixirLabel->setPosition(Vec2(startX - 60, startY));
    _uiLayer->addChild(_elixirLabel);

    // Population
    auto peopleIcon = Sprite::create("people.png");
    if (!peopleIcon) peopleIcon = Sprite::create();
    peopleIcon->setPosition(Vec2(startX, startY));
    _uiLayer->addChild(peopleIcon);
    _populationLabel = Label::createWithSystemFont(std::to_string(_population), "Arial", 16);
    _populationLabel->setPosition(Vec2(startX + 40, startY));
    _uiLayer->addChild(_populationLabel);

    // 3. Build Button (Bottom Right)
    MenuItem* buildItem = MenuItemImage::create("build.png", "build.png", CC_CALLBACK_1(VillageScene::onBuildModeCallback, this));
    if (buildItem == nullptr || buildItem->getContentSize().width == 0) {
         auto label = Label::createWithSystemFont("Build", "Arial", 20);
         buildItem = MenuItemLabel::create(label, CC_CALLBACK_1(VillageScene::onBuildModeCallback, this));
    }
    auto buildMenu = Menu::create(buildItem, nullptr);
    buildMenu->setPosition(Vec2(origin.x + visibleSize.width - 50, origin.y + 50));
    _uiLayer->addChild(buildMenu);
}

void VillageScene::setupTouchHandling() {
    auto listener = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = [this](Touch* touch, Event* event) {
        if (_currentWindow) return true; // Consume touch if window open
        
        _touchStartPos = touch->getLocation();
        _mapStartPos = _mapNode->getPosition();
        _isDragging = false;
        return true;
    };

    listener->onTouchMoved = [this](Touch* touch, Event* event) {
        if (_currentWindow) return;

        auto currentPos = touch->getLocation();
        if (currentPos.distance(_touchStartPos) > 10) {
            _isDragging = true;
        }
        
        if (_isDragging) {
            Vec2 diff = currentPos - _touchStartPos;
            _mapNode->setPosition(_mapStartPos + diff);
        }
    };

    listener->onTouchEnded = [this](Touch* touch, Event* event) {
        if (_currentWindow) return;
        
        if (!_isDragging) {
            Vec2 touchLocationInMap = _mapNode->convertToNodeSpace(touch->getLocation());
            int gridX = touchLocationInMap.x / 32;
            int gridY = touchLocationInMap.y / 32;
            
            // Check bounds
            if (gridX < 0 || gridX >= 50 || gridY < 0 || gridY >= 50) return;

            if (_isBuildMode) {
                // Try to place building
                placeBuilding(_pendingBuildingType, Vec2(gridX, gridY));
                _isBuildMode = false; // Exit build mode after one placement
            } else {
                // Check if clicked on a building
                for (auto building : _buildings) {
                    Vec2 bPos = building->getGridPosition();
                    // Simple hit test (assuming 1x1 or based on building size, for now all 1x1 center anchored?)
                    // Actually sprites are center anchored in placeBuilding, but logical pos is grid coords.
                    // Let's match grid coords.
                    // Note: In placeBuilding I setPosition to center of tile.
                    // Distance check is better for usability.
                    if (bPos.distance(Vec2(gridX, gridY)) < 1.0f) {
                        showBuildingInfo(building);
                        return;
                    }
                }
            }
        }
    };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

void VillageScene::placeBuilding(building::BuildingType type, const cocos2d::Vec2& gridPos) {
    building::Building* building = nullptr;
    
    switch (type) {
        case building::BuildingType::TownHall: building = building::TownHall::create(); break;
        case building::BuildingType::GoldMine: building = building::GoldMine::create(); break;
        case building::BuildingType::ElixirCollector: building = building::ElixirCollector::create(); break;
        case building::BuildingType::GoldStorage: building = building::GoldStorage::create(); break;
        case building::BuildingType::ElixirStorage: building = building::ElixirStorage::create(); break;
        case building::BuildingType::ArcherTower: building = building::ArcherTower::create(); break;
        case building::BuildingType::Cannon: building = building::Cannon::create(); break;
        case building::BuildingType::Barracks: building = building::Barracks::create(); break;
        case building::BuildingType::TrainingCamp: building = building::TrainingCamp::create(); break;
        case building::BuildingType::Wall: building = building::Wall::create(); break;
        default: break;
    }

    if (building) {
        building->setGridPosition(gridPos);
        building->setPosition(Vec2(gridPos.x * 32 + 16, gridPos.y * 32 + 16));
        _mapNode->addChild(building, 10);
        _buildings.push_back(building);
    }
}

// UI Callbacks
void VillageScene::onAttackModeCallback(Ref* pSender) {
    if (_currentWindow) return;
    showLevelSelectWindow();
}

void VillageScene::onHistoryCallback(Ref* pSender) {
    if (_currentWindow) return;
    showHistoryWindow();
}



void VillageScene::onBuildModeCallback(Ref* pSender) {
    if (_currentWindow) return;
    showBuildWindow();
}

void VillageScene::onCloseWindowCallback(Ref* pSender) {
    closeCurrentWindow();
}

void VillageScene::closeCurrentWindow() {
    if (_currentWindow) {
        _currentWindow->removeFromParent();
        _currentWindow = nullptr;
    }
}

void VillageScene::showLevelSelectWindow() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    
    // Create a container layer (semitransparent bg)
    auto windowLayer = LayerColor::create(Color4B(0, 0, 0, 150));
    this->addChild(windowLayer, 200);
    _currentWindow = windowLayer;
    
    // Close Button
    MenuItem* closeItem = MenuItemImage::create("false.png", "false.png", CC_CALLBACK_1(VillageScene::onCloseWindowCallback, this));
    if (!closeItem || closeItem->getContentSize().width == 0) {
         auto label = Label::createWithSystemFont("X", "Arial", 20);
         closeItem = MenuItemLabel::create(label, CC_CALLBACK_1(VillageScene::onCloseWindowCallback, this));
    }
    auto closeMenu = Menu::create(closeItem, nullptr);
    closeMenu->setPosition(Vec2(visibleSize.width - 50, visibleSize.height - 50));
    windowLayer->addChild(closeMenu);
    
    // Level Buttons
    MenuItem* simpleItem = MenuItemImage::create("1_botton.png", "1_botton.png", [this](Ref*){ onLevelSelectCallback(nullptr, 1); });
    MenuItem* mediumItem = MenuItemImage::create("2_botton.png", "2_botton.png", [this](Ref*){ onLevelSelectCallback(nullptr, 2); });
    MenuItem* hardItem = MenuItemImage::create("3_botton.png", "3_botton.png", [this](Ref*){ onLevelSelectCallback(nullptr, 3); });

    // Fallbacks
    if (!simpleItem || simpleItem->getContentSize().width == 0) simpleItem = MenuItemLabel::create(Label::createWithSystemFont("Simple Mode", "Arial", 30), [this](Ref*){ onLevelSelectCallback(nullptr, 1); });
    if (!mediumItem || mediumItem->getContentSize().width == 0) mediumItem = MenuItemLabel::create(Label::createWithSystemFont("Medium Mode", "Arial", 30), [this](Ref*){ onLevelSelectCallback(nullptr, 2); });
    if (!hardItem || hardItem->getContentSize().width == 0) hardItem = MenuItemLabel::create(Label::createWithSystemFont("Hard Mode", "Arial", 30), [this](Ref*){ onLevelSelectCallback(nullptr, 3); });

    auto menu = Menu::create(simpleItem, mediumItem, hardItem, nullptr);
    menu->alignItemsVerticallyWithPadding(20);
    menu->setPosition(Vec2(visibleSize.width/2, visibleSize.height/2));
    windowLayer->addChild(menu);
}

void VillageScene::showHistoryWindow() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto windowLayer = LayerColor::create(Color4B(0, 0, 0, 150));
    this->addChild(windowLayer, 200);
    _currentWindow = windowLayer;

    // Close Button
    MenuItem* closeItem = MenuItemImage::create("false.png", "false.png", CC_CALLBACK_1(VillageScene::onCloseWindowCallback, this));
    if (!closeItem || closeItem->getContentSize().width == 0) {
         auto label = Label::createWithSystemFont("X", "Arial", 20);
         closeItem = MenuItemLabel::create(label, CC_CALLBACK_1(VillageScene::onCloseWindowCallback, this));
    }
    auto closeMenu = Menu::create(closeItem, nullptr);
    closeMenu->setPosition(Vec2(visibleSize.width - 50, visibleSize.height - 50));
    windowLayer->addChild(closeMenu);
    
    // History List
    Vector<MenuItem*> items;
    int index = 0;
    for (const auto& record : BattleScene::s_battleHistory) {
        std::string text = "Battle " + std::to_string(record.id) + " (Lvl " + std::to_string(record.level) + ") - " + (record.isWin ? "WIN" : "LOSE");
        auto label = Label::createWithSystemFont(text, "Arial", 24);
        auto item = MenuItemLabel::create(label, [this, index](Ref*){
             onHistoryItemCallback(nullptr, index);
        });
        items.pushBack(item);
        index++;
    }
    
    if (items.empty()) {
        auto label = Label::createWithSystemFont("No History Yet", "Arial", 30);
        label->setPosition(visibleSize.width/2, visibleSize.height/2);
        windowLayer->addChild(label);
    } else {
        auto menu = Menu::createWithArray(items);
        menu->alignItemsVerticallyWithPadding(10);
        menu->setPosition(Vec2(visibleSize.width/2, visibleSize.height/2));
        windowLayer->addChild(menu);
    }
}

void VillageScene::onHistoryItemCallback(Ref* pSender, int index) {
    if (index >= 0 && index < BattleScene::s_battleHistory.size()) {
        auto scene = BattleScene::createReplayScene(BattleScene::s_battleHistory[index]);
        Director::getInstance()->pushScene(TransitionFade::create(0.5f, scene));
        closeCurrentWindow();
    }
}

void VillageScene::onLevelSelectCallback(Ref* pSender, int level) {
    int barb = 0, arch = 0, bomb = 0, giant = 0;
    if (_troops.find("Barbarian") != _troops.end()) barb = _troops["Barbarian"];
    if (_troops.find("Archer") != _troops.end()) arch = _troops["Archer"];
    if (_troops.find("WallBreaker") != _troops.end()) bomb = _troops["WallBreaker"];
    if (_troops.find("Giant") != _troops.end()) giant = _troops["Giant"];
    
    auto scene = BattleScene::createScene(level, barb, arch, bomb, giant);
    Director::getInstance()->pushScene(TransitionFade::create(0.5f, scene));
    closeCurrentWindow();
}

void VillageScene::showBuildWindow() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto windowLayer = LayerColor::create(Color4B(0, 0, 0, 150));
    this->addChild(windowLayer, 200);
    _currentWindow = windowLayer;

    // Close Button
    MenuItem* closeItem = MenuItemImage::create("false.png", "false.png", CC_CALLBACK_1(VillageScene::onCloseWindowCallback, this));
    if (!closeItem || closeItem->getContentSize().width == 0) {
         auto label = Label::createWithSystemFont("X", "Arial", 20);
         closeItem = MenuItemLabel::create(label, CC_CALLBACK_1(VillageScene::onCloseWindowCallback, this));
    }
    auto closeMenu = Menu::create(closeItem, nullptr);
    closeMenu->setPosition(Vec2(visibleSize.width - 50, visibleSize.height - 50));
    windowLayer->addChild(closeMenu);

    // Helper lambda to create building button
    auto createBtn = [this](const std::string& img, building::BuildingType type, int cost, bool isGold) -> MenuItem* {
        // Check limits
        int currentCount = getBuildingCount(type);
        int maxCount = getMaxBuildingCount(type, getTownHallLevel());
        
        MenuItem* item = nullptr;

        if (currentCount >= maxCount) {
             auto sprite = Sprite::create(img);
             if (sprite) {
                 sprite->setColor(Color3B::GRAY);
                 item = MenuItemSprite::create(sprite, sprite, nullptr);
             }
        } else {
            item = MenuItemImage::create(img, img, [this, type, cost, isGold](Ref*){
                if (isGold) {
                    if (trySpendResources(cost, 0)) onBuildingTypeSelected(nullptr, type);
                } else {
                    if (trySpendResources(0, cost)) onBuildingTypeSelected(nullptr, type);
                }
            });
        }
        
        if (item) {
             // Cost Label
             auto label = Label::createWithSystemFont(std::to_string(cost), "Arial", 16);
             label->setPosition(Vec2(item->getContentSize().width/2 - 15, -15));
             item->addChild(label);

             // Icon
             auto icon = Sprite::create(isGold ? "coin.png" : "elixir.png");
             if (icon) {
                 icon->setScale(0.5f); 
                 icon->setPosition(Vec2(item->getContentSize().width/2 + 15, -15));
                 item->addChild(icon);
             }

             // Count Label
             std::string countStr = std::to_string(currentCount) + "/" + std::to_string(maxCount);
             auto countLabel = Label::createWithSystemFont(countStr, "Arial", 14);
             countLabel->setPosition(Vec2(item->getContentSize().width/2, -35));
             item->addChild(countLabel);
        }
        return item;
    };

    // Building Buttons
    Vector<MenuItem*> items;
    
    // Resource Buildings
    if (auto btn = createBtn("gold_mine1.png", building::BuildingType::GoldMine, building::GoldMine::getBuildCost(), false)) items.pushBack(btn);
    if (auto btn = createBtn("elixir_collector1.png", building::BuildingType::ElixirCollector, building::ElixirCollector::getBuildCost(), true)) items.pushBack(btn);
    if (auto btn = createBtn("gold_storage1.png", building::BuildingType::GoldStorage, building::GoldStorage::getBuildCost(), false)) items.pushBack(btn);
    if (auto btn = createBtn("elixir_storage1.png", building::BuildingType::ElixirStorage, building::ElixirStorage::getBuildCost(), true)) items.pushBack(btn);
    
    // Defense
    if (auto btn = createBtn("archer_tower1.png", building::BuildingType::ArcherTower, building::ArcherTower::getBuildCost(), true)) items.pushBack(btn);
    if (auto btn = createBtn("cannon1.png", building::BuildingType::Cannon, building::Cannon::getBuildCost(), true)) items.pushBack(btn);
    
    // Army
    if (auto btn = createBtn("barracks1.png", building::BuildingType::Barracks, building::Barracks::getBuildCost(), false)) items.pushBack(btn);
    if (auto btn = createBtn("training_camp1.png", building::BuildingType::TrainingCamp, building::TrainingCamp::getBuildCost(), false)) items.pushBack(btn);

    // Layout
    auto menu = Menu::createWithArray(items);
    menu->alignItemsHorizontallyWithPadding(30);
    menu->setPosition(Vec2(visibleSize.width/2, visibleSize.height/2));
    
    windowLayer->addChild(menu);
}

int VillageScene::getTownHallLevel() const {
    for (auto b : _buildings) {
        if (b->getType() == building::BuildingType::TownHall) {
            return b->getLevel();
        }
    }
    return 1;
}

int VillageScene::getBuildingCount(building::BuildingType type) const {
    int count = 0;
    for (auto b : _buildings) {
        if (b->getType() == type) {
            count++;
        }
    }
    return count;
}

int VillageScene::getMaxBuildingCount(building::BuildingType type, int thLevel) const {
    // Rules from prompt
    switch (type) {
        case building::BuildingType::TownHall: return 1;
        case building::BuildingType::Barracks: return (thLevel >= 2) ? 2 : 1;
        case building::BuildingType::TrainingCamp: return 1;
        case building::BuildingType::GoldMine: return (thLevel >= 3) ? 3 : (thLevel >= 2 ? 2 : 1);
        case building::BuildingType::GoldStorage: return (thLevel >= 3) ? 2 : 1;
        case building::BuildingType::ElixirCollector: return (thLevel >= 3) ? 3 : (thLevel >= 2 ? 2 : 1);
        case building::BuildingType::ElixirStorage: return (thLevel >= 3) ? 2 : 1;
        case building::BuildingType::Cannon: return (thLevel >= 3) ? 3 : (thLevel >= 2 ? 2 : 1);
        case building::BuildingType::ArcherTower: return (thLevel >= 3) ? 2 : 1;
        case building::BuildingType::Wall: return (thLevel >= 3) ? 40 : (thLevel >= 2 ? 10 : 0);
        default: return 0;
    }
}

bool VillageScene::trySpendResources(int gold, int elixir) {
    if (_gold >= gold && _elixir >= elixir) {
        _gold -= gold;
        _elixir -= elixir;
        updateResourceLabels();
        return true;
    }
    
    // Show Error
    auto label = Label::createWithSystemFont("Not enough resources!", "Arial", 30);
    label->setPosition(Director::getInstance()->getVisibleSize() / 2);
    label->setColor(Color3B::RED);
    _currentWindow->addChild(label);
    
    // Fade out error
    auto seq = Sequence::create(DelayTime::create(1.0f), FadeOut::create(0.5f), RemoveSelf::create(), nullptr);
    label->runAction(seq);
    
    return false;
}

void VillageScene::updateResourceLabels() {
    if (_goldLabel) _goldLabel->setString(std::to_string((int)_gold));
    if (_elixirLabel) _elixirLabel->setString(std::to_string((int)_elixir));
    if (_populationLabel) {
        std::string popStr = std::to_string(getCurrentTroopCount()) + "/" + std::to_string(getTroopCapacity());
        _populationLabel->setString(popStr);
    }
}

void VillageScene::onBuildingTypeSelected(Ref* pSender, building::BuildingType type) {
    closeCurrentWindow();
    _isBuildMode = true;
    _pendingBuildingType = type;
    
    auto label = Label::createWithSystemFont("Click map to place building", "Arial", 24);
    label->setPosition(Vec2(Director::getInstance()->getVisibleSize().width / 2, 100));
    this->addChild(label, 200, 999); // Tag 999 to remove later
    
    auto seq = Sequence::create(DelayTime::create(3.0f), RemoveSelf::create(), nullptr);
    label->runAction(seq);
}

void VillageScene::showBuildingInfo(building::Building* building) {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto windowLayer = LayerColor::create(Color4B(0, 0, 0, 150));
    this->addChild(windowLayer, 200);
    _currentWindow = windowLayer;
    
    // Close
    MenuItem* closeItem = MenuItemImage::create("false.png", "false.png", CC_CALLBACK_1(VillageScene::onCloseWindowCallback, this));
    if (!closeItem || closeItem->getContentSize().width == 0) {
        auto label = Label::createWithSystemFont("X", "Arial", 20);
        closeItem = MenuItemLabel::create(label, CC_CALLBACK_1(VillageScene::onCloseWindowCallback, this));
    }
    auto closeMenu = Menu::create(closeItem, nullptr);
    closeMenu->setPosition(Vec2(visibleSize.width - 50, visibleSize.height - 50));
    windowLayer->addChild(closeMenu);
    
    // Common Content
    std::string name = building->getBuildingName();
    int level = building->getLevel();
    int hp = building->getCurrentHP();
    int maxHP = building->getMaxHP();
    
    std::string infoStr = name + " (Lv." + std::to_string(level) + ")\n" +
                          "HP: " + std::to_string(hp) + " / " + std::to_string(maxHP);

    // Specific Attributes
    if (auto mine = dynamic_cast<building::GoldMine*>(building)) {
        infoStr += "\nProduction: " + std::to_string(mine->getProductionRate()) + " Gold/s";
    } else if (auto collector = dynamic_cast<building::ElixirCollector*>(building)) {
        infoStr += "\nProduction: " + std::to_string(collector->getProductionRate()) + " Elixir/s";
    } else if (auto storage = dynamic_cast<building::GoldStorage*>(building)) {
        infoStr += "\nCapacity: " + std::to_string(storage->getCapacity());
    } else if (auto storage = dynamic_cast<building::ElixirStorage*>(building)) {
        infoStr += "\nCapacity: " + std::to_string(storage->getCapacity());
    } else if (auto tower = dynamic_cast<building::ArcherTower*>(building)) {
        infoStr += "\nAttack: " + std::to_string(tower->getAttackDamage());
        infoStr += "\nRange: 6";
    } else if (auto cannon = dynamic_cast<building::Cannon*>(building)) {
        infoStr += "\nAttack: " + std::to_string(cannon->getAttackDamage());
        infoStr += "\nRange: 6";
    } else if (auto barracks = dynamic_cast<building::Barracks*>(building)) {
        int cap = barracks->getCapacity();
        infoStr += "\nCapacity: " + std::to_string(cap);
        infoStr += "\nTroops:";
        for (auto const& [name, num] : _troops) {
            if (num > 0) {
                infoStr += "\n " + name + ": " + std::to_string(num);
            }
        }
    } else if (auto camp = dynamic_cast<building::TrainingCamp*>(building)) {
        infoStr += "\nUnlocks: ";
        auto soldiers = camp->getUnlockableSoldiers();
        for (size_t i = 0; i < soldiers.size(); ++i) {
            infoStr += soldiers[i] + (i < soldiers.size() - 1 ? ", " : "");
        }
    }

    auto infoLabel = Label::createWithSystemFont(infoStr, "Arial", 24);
    infoLabel->setPosition(Vec2(visibleSize.width/2, visibleSize.height/2 + 50));
    windowLayer->addChild(infoLabel);
    
    // Buttons Menu
    Vector<MenuItem*> menuItems;

    // Upgrade Button
    if (building->canUpgrade()) {
        int cost = building->getUpgradeCost();
        std::string costIcon = building->getUpgradeCurrencyIcon(); 
        
        MenuItem* upgradeItem = MenuItemImage::create("upgrade.png", "upgrade.png", [this, building](Ref*){
            onUpgradeCallback(nullptr, building);
        });
        if (!upgradeItem || upgradeItem->getContentSize().width == 0) {
            upgradeItem = MenuItemLabel::create(Label::createWithSystemFont("Upgrade", "Arial", 24), [this, building](Ref*){
                onUpgradeCallback(nullptr, building);
            });
        }
        
        // Label for cost
        auto costLabel = Label::createWithSystemFont(std::to_string(cost), "Arial", 18);
        costLabel->setPosition(Vec2(upgradeItem->getContentSize().width/2 - 15, -15));
        upgradeItem->addChild(costLabel);

        // Icon for cost
        auto icon = Sprite::create(costIcon);
        if (icon) {
            icon->setScale(0.5f);
            icon->setPosition(Vec2(upgradeItem->getContentSize().width/2 + 15, -15));
            upgradeItem->addChild(icon);
        }

        menuItems.pushBack(upgradeItem);
    } else {
        // Max Level Label instead of button? Or just show nothing/disabled button.
        // For layout simplicity, maybe just text on screen.
    }

    // Train Button (Training Camp only)
    if (auto camp = dynamic_cast<building::TrainingCamp*>(building)) {
        MenuItem* trainItem = MenuItemImage::create("practice.png", "practice.png", [this, camp](Ref*){
            showTrainingWindow(camp);
        });
        if (!trainItem || trainItem->getContentSize().width == 0) {
            trainItem = MenuItemLabel::create(Label::createWithSystemFont("Train", "Arial", 24), [this, camp](Ref*){
                showTrainingWindow(camp);
            });
        }
        menuItems.pushBack(trainItem);
    }

    auto menu = Menu::createWithArray(menuItems);
    menu->alignItemsHorizontallyWithPadding(40);
    menu->setPosition(Vec2(visibleSize.width/2, visibleSize.height/2 - 60));
    windowLayer->addChild(menu);
    
    // Max Level Indicator if needed
    if (!building->canUpgrade()) {
        auto maxLabel = Label::createWithSystemFont("Max Level", "Arial", 20);
        maxLabel->setPosition(Vec2(visibleSize.width/2, visibleSize.height/2 - 120));
        windowLayer->addChild(maxLabel);
    }
}

void VillageScene::showTrainingWindow(building::TrainingCamp* camp) {
    closeCurrentWindow(); // Close info window
    
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto windowLayer = LayerColor::create(Color4B(0, 0, 0, 150));
    this->addChild(windowLayer, 200);
    _currentWindow = windowLayer;
    
    // Close
    MenuItem* closeItem = MenuItemImage::create("false.png", "false.png", CC_CALLBACK_1(VillageScene::onCloseWindowCallback, this));
    auto closeMenu = Menu::create(closeItem, nullptr);
    closeMenu->setPosition(Vec2(visibleSize.width - 50, visibleSize.height - 50));
    windowLayer->addChild(closeMenu);
    
    // Soldiers
    auto soldiers = camp->getUnlockableSoldiers();
    Vector<MenuItem*> items;
    
    for (const auto& soldier : soldiers) {
        std::string img = "barbarian1.png"; 
        int cost = 25;
        int space = 1;
        
        if (soldier == "Barbarian") { img = "barbarian1.png"; cost = 25; space = 1; }
        if (soldier == "Archer") { img = "archer1.png"; cost = 50; space = 1; }
        if (soldier == "Giant") { img = "giant1.png"; cost = 250; space = 5; }
        if (soldier == "WallBreaker") { img = "bomber1.png"; cost = 100; space = 2; }
        
        // Label pointer to be captured
        auto countLabel = Label::createWithSystemFont("", "Arial", 14);
        
        MenuItem* item = MenuItemImage::create(img, img, [this, camp, soldier, cost, space, countLabel](Ref*){
            // Check cost and capacity
            if (_elixir < cost) {
                 auto label = Label::createWithSystemFont("Not enough Elixir!", "Arial", 30);
                 label->setPosition(Director::getInstance()->getVisibleSize() / 2);
                 label->setColor(Color3B::RED);
                 _currentWindow->addChild(label);
                 label->runAction(Sequence::create(DelayTime::create(0.5f), FadeOut::create(0.5f), RemoveSelf::create(), nullptr));
                 return;
            }
            if (getCurrentTroopCount() + space > getTroopCapacity()) {
                auto label = Label::createWithSystemFont("Camp Full!", "Arial", 30);
                label->setPosition(Director::getInstance()->getVisibleSize() / 2);
                label->setColor(Color3B::RED);
                _currentWindow->addChild(label);
                label->runAction(Sequence::create(DelayTime::create(0.5f), FadeOut::create(0.5f), RemoveSelf::create(), nullptr));
                return;
            }
            
            _elixir -= cost;
            _troops[soldier]++;
            updateResourceLabels();
            
            // Update the specific label immediately
            std::string countStr = std::to_string(_troops[soldier]) + "/" + std::to_string(getTroopCapacity());
            countLabel->setString(countStr);
        });
        
        // Fallback text if image missing
        if (!item || item->getContentSize().width == 0) {
            item = MenuItemLabel::create(Label::createWithSystemFont(soldier, "Arial", 20), [this, camp, soldier, cost, space, countLabel](Ref*){
                 if (_elixir < cost) return;
                 if (getCurrentTroopCount() + space > getTroopCapacity()) return;
                 _elixir -= cost;
                 _troops[soldier]++;
                 updateResourceLabels();
                 std::string countStr = std::to_string(_troops[soldier]) + "/" + std::to_string(getTroopCapacity());
                 countLabel->setString(countStr);
            });
        }

        // Cost label
        auto label = Label::createWithSystemFont(std::to_string(cost), "Arial", 16);
        label->setPosition(Vec2(item->getContentSize().width/2 - 15, -15));
        item->addChild(label);
        
        // Icon for cost (Troops cost Elixir)
        auto icon = Sprite::create("elixir.png");
        if (icon) {
            icon->setScale(0.5f);
            icon->setPosition(Vec2(item->getContentSize().width/2 + 15, -15));
            item->addChild(icon);
        }

        // Count/Capacity Label
        std::string initCountStr = std::to_string(_troops[soldier]) + "/" + std::to_string(getTroopCapacity());
        countLabel->setString(initCountStr);
        countLabel->setPosition(Vec2(item->getContentSize().width/2, -35));
        item->addChild(countLabel);
        
        items.pushBack(item);
    }
    
    auto menu = Menu::createWithArray(items);
    menu->alignItemsHorizontallyWithPadding(20);
    menu->setPosition(Vec2(visibleSize.width/2, visibleSize.height/2));
    windowLayer->addChild(menu);
}

void VillageScene::onTrainSoldierCallback(Ref* pSender, building::TrainingCamp* camp, std::string soldierName) {
    // Moved logic into lambda above for simplicity in accessing local vars like cost
}

int VillageScene::getTroopCapacity() const {
    int capacity = 0;
    for (auto b : _buildings) {
        if (auto barracks = dynamic_cast<building::Barracks*>(b)) {
            capacity += barracks->getCapacity();
        }
    }
    return capacity;
}

int VillageScene::getCurrentTroopCount() const {
    int count = 0;
    for (auto const& [name, num] : _troops) {
        int space = 1;
        if (name == "Giant") space = 5;
        if (name == "WallBreaker") space = 2;
        count += num * space;
    }
    return count;
}



void VillageScene::onUpgradeCallback(Ref* pSender, building::Building* building) {
    if (!building) return;
    
    int cost = building->getUpgradeCost();
    // Check currency type
    // Simple logic: TownHall uses Gold? Yes.
    // GoldMine uses Elixir.
    // ElixirCollector uses Gold.
    
    bool enough = false;
    std::string currency = building->getUpgradeCurrencyIcon();
    
    if (currency == "coin.png") {
        if (_gold >= cost) {
            _gold -= cost;
            enough = true;
        }
    } else {
        if (_elixir >= cost) {
            _elixir -= cost;
            enough = true;
        }
    }
    
    if (enough) {
        building->upgrade();
        updateResourceLabels();
        closeCurrentWindow();
        showBuildingInfo(building); // Re-open to show updated stats
    } else {
        auto label = Label::createWithSystemFont("Not enough resources!", "Arial", 30);
        label->setPosition(Director::getInstance()->getVisibleSize() / 2);
        label->setColor(Color3B::RED);
        _currentWindow->addChild(label);
        label->runAction(Sequence::create(DelayTime::create(1.0f), FadeOut::create(0.5f), RemoveSelf::create(), nullptr));
    }
}

} // namespace scene
