#include "VillageScene.h"
#include "MainMenuScene.h"

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

    return true;
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
    auto attackMenu = Menu::create(attackItem, nullptr);
    attackMenu->setPosition(Vec2(origin.x + 50, origin.y + visibleSize.height - 50));
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
    MenuItem* simpleItem = MenuItemImage::create("simple_mode.png", "simple_mode.png", [this](Ref*){ onLevelSelectCallback(nullptr, 1); });
    MenuItem* mediumItem = MenuItemImage::create("medium_mode.png", "medium_mode.png", [this](Ref*){ onLevelSelectCallback(nullptr, 2); });
    MenuItem* hardItem = MenuItemImage::create("hard_mode.png", "hard_mode.png", [this](Ref*){ onLevelSelectCallback(nullptr, 3); });

    // Fallbacks
    if (!simpleItem || simpleItem->getContentSize().width == 0) simpleItem = MenuItemLabel::create(Label::createWithSystemFont("Simple", "Arial", 24), [this](Ref*){ onLevelSelectCallback(nullptr, 1); });
    if (!mediumItem || mediumItem->getContentSize().width == 0) mediumItem = MenuItemLabel::create(Label::createWithSystemFont("Medium", "Arial", 24), [this](Ref*){ onLevelSelectCallback(nullptr, 2); });
    if (!hardItem || hardItem->getContentSize().width == 0) hardItem = MenuItemLabel::create(Label::createWithSystemFont("Hard", "Arial", 24), [this](Ref*){ onLevelSelectCallback(nullptr, 3); });

    auto menu = Menu::create(simpleItem, mediumItem, hardItem, nullptr);
    menu->alignItemsVerticallyWithPadding(20);
    windowLayer->addChild(menu);
}

void VillageScene::onLevelSelectCallback(Ref* pSender, int level) {
    CCLOG("Level Selected: %d", level);
    // TODO: Switch to BattleScene
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

    // Building Buttons
    // Gold Mine
    MenuItem* mineItem = MenuItemImage::create("gold_mine1.png", "gold_mine1.png", [this](Ref*){ 
        if (trySpendResources(0, building::GoldMine::getBuildCost())) { // Costs Elixir
             onBuildingTypeSelected(nullptr, building::BuildingType::GoldMine); 
        }
    });
    // Elixir Collector
    MenuItem* collectorItem = MenuItemImage::create("elixir_collector1.png", "elixir_collector1.png", [this](Ref*){ 
        if (trySpendResources(building::ElixirCollector::getBuildCost(), 0)) { // Costs Gold
             onBuildingTypeSelected(nullptr, building::BuildingType::ElixirCollector); 
        }
    });
    // Gold Storage
    MenuItem* gStoreItem = MenuItemImage::create("gold_storage1.png", "gold_storage1.png", [this](Ref*){ 
        if (trySpendResources(0, building::GoldStorage::getBuildCost())) { 
             onBuildingTypeSelected(nullptr, building::BuildingType::GoldStorage); 
        }
    });
    // Elixir Storage
    MenuItem* eStoreItem = MenuItemImage::create("elixir_storage1.png", "elixir_storage1.png", [this](Ref*){ 
        if (trySpendResources(building::ElixirStorage::getBuildCost(), 0)) { 
             onBuildingTypeSelected(nullptr, building::BuildingType::ElixirStorage); 
        }
    });
    
    // Layout
    auto menu = Menu::create(mineItem, collectorItem, gStoreItem, eStoreItem, nullptr);
    menu->alignItemsHorizontallyWithPadding(20);
    windowLayer->addChild(menu);
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
    if (_goldLabel) _goldLabel->setString(std::to_string(_gold));
    if (_elixirLabel) _elixirLabel->setString(std::to_string(_elixir));
    if (_populationLabel) _populationLabel->setString(std::to_string(_population));
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
    
    // Content
    std::string name = building->getBuildingName();
    int level = building->getLevel();
    int hp = building->getCurrentHP();
    int maxHP = building->getMaxHP();
    
    std::string infoStr = name + " (Lv." + std::to_string(level) + ")\n" +
                          "HP: " + std::to_string(hp) + " / " + std::to_string(maxHP);
                          
    auto infoLabel = Label::createWithSystemFont(infoStr, "Arial", 24);
    infoLabel->setPosition(Vec2(visibleSize.width/2, visibleSize.height/2 + 50));
    windowLayer->addChild(infoLabel);
    
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
        
        auto upgradeMenu = Menu::create(upgradeItem, nullptr);
        upgradeMenu->setPosition(Vec2(visibleSize.width/2, visibleSize.height/2 - 50));
        windowLayer->addChild(upgradeMenu);
        
        // Show cost
        auto costLabel = Label::createWithSystemFont("Cost: " + std::to_string(cost), "Arial", 20);
        costLabel->setPosition(Vec2(visibleSize.width/2, visibleSize.height/2 - 90));
        windowLayer->addChild(costLabel);
    } else {
        auto maxLabel = Label::createWithSystemFont("Max Level", "Arial", 24);
        maxLabel->setPosition(Vec2(visibleSize.width/2, visibleSize.height/2 - 50));
        windowLayer->addChild(maxLabel);
    }
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
