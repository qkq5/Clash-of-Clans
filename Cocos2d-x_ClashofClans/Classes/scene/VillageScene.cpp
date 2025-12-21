#include "VillageScene.h"
#include "building/TownHall.h"
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

    setupMap();
    setupBuildings(); // Must be after map setup
    setupUI(); // Must be after map to be on top
    setupTouchHandling();

    return true;
}

void VillageScene::setupMap() {
    _mapNode = Node::create();
    this->addChild(_mapNode, 0);

    int mapWidth = 50;
    int mapHeight = 50;
    int tileSize = 32;

    // Initialize Grid using the template class
    // We allocate it on heap, remember to delete in destructor (omitted for brevity in this snippet but important)
    // Or use std::unique_ptr in header. For now, raw pointer.
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
                _mapNode->addChild(tile);
                
                // Store in grid
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
    // 1. Town Hall
    auto townHall = building::TownHall::create();
    if (townHall) {
        // Place in center-ish
        int gridX = 25;
        int gridY = 25;
        townHall->setGridPosition(Vec2(gridX, gridY));
        townHall->setPosition(Vec2(gridX * 32 + 16, gridY * 32 + 16)); // Center of tile
        _mapNode->addChild(townHall, 10); // Higher z-order than grass
        _buildings.push_back(townHall);
    }

    // Add other initial buildings here (Gold Mine, etc.) as requested
    // "初始阶段，地图上已放置一个大本营、一个金库、一个金矿、一个圣水收集器、一个圣水瓶。"
    // For now just Town Hall to verify integration
}

void VillageScene::setupUI() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    
    // UI Layer
    auto uiLayer = Layer::create();
    this->addChild(uiLayer, 100); // Top layer

    // 1. Attack Mode Button (Top Left)
    MenuItem* attackItem = MenuItemImage::create("attack_mode.png", "attack_mode.png", CC_CALLBACK_1(VillageScene::onAttackModeCallback, this));
    if (attackItem == nullptr || attackItem->getContentSize().width == 0) {
        auto label = Label::createWithSystemFont("Attack", "Arial", 20);
        attackItem = MenuItemLabel::create(label, CC_CALLBACK_1(VillageScene::onAttackModeCallback, this));
    }
    auto attackMenu = Menu::create(attackItem, nullptr);
    attackMenu->setPosition(Vec2(origin.x + 50, origin.y + visibleSize.height - 50));
    uiLayer->addChild(attackMenu);

    // 2. Resources (Top Right)
    // Backgrounds or Icons
    int margin = 20;
    int startX = origin.x + visibleSize.width - 100;
    int startY = origin.y + visibleSize.height - 30;

    // Gold
    auto coinIcon = Sprite::create("coin.png");
    if (!coinIcon) coinIcon = Sprite::create(); // Fallback empty sprite
    coinIcon->setPosition(Vec2(startX - 200, startY));
    uiLayer->addChild(coinIcon);
    _goldLabel = Label::createWithSystemFont(std::to_string(_gold), "Arial", 16);
    _goldLabel->setPosition(Vec2(startX - 160, startY));
    uiLayer->addChild(_goldLabel);

    // Elixir
    auto elixirIcon = Sprite::create("elixir.png");
    if (!elixirIcon) elixirIcon = Sprite::create();
    elixirIcon->setPosition(Vec2(startX - 100, startY));
    uiLayer->addChild(elixirIcon);
    _elixirLabel = Label::createWithSystemFont(std::to_string(_elixir), "Arial", 16);
    _elixirLabel->setPosition(Vec2(startX - 60, startY));
    uiLayer->addChild(_elixirLabel);

    // Population
    auto peopleIcon = Sprite::create("people.png");
    if (!peopleIcon) peopleIcon = Sprite::create();
    peopleIcon->setPosition(Vec2(startX, startY));
    uiLayer->addChild(peopleIcon);
    _populationLabel = Label::createWithSystemFont(std::to_string(_population), "Arial", 16);
    _populationLabel->setPosition(Vec2(startX + 40, startY));
    uiLayer->addChild(_populationLabel);

    // 3. Build Button (Bottom Right)
    MenuItem* buildItem = MenuItemImage::create("build.png", "build.png", CC_CALLBACK_1(VillageScene::onBuildModeCallback, this));
    if (buildItem == nullptr || buildItem->getContentSize().width == 0) {
        auto label = Label::createWithSystemFont("Build", "Arial", 20);
        buildItem = MenuItemLabel::create(label, CC_CALLBACK_1(VillageScene::onBuildModeCallback, this));
    }
    auto buildMenu = Menu::create(buildItem, nullptr);
    buildMenu->setPosition(Vec2(origin.x + visibleSize.width - 50, origin.y + 50));
    uiLayer->addChild(buildMenu);
}

void VillageScene::setupTouchHandling() {
    auto listener = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = [this](Touch* touch, Event* event) {
        _touchStartPos = touch->getLocation();
        _mapStartPos = _mapNode->getPosition();
        _isDragging = false;
        return true;
    };

    listener->onTouchMoved = [this](Touch* touch, Event* event) {
        auto currentPos = touch->getLocation();
        if (currentPos.distance(_touchStartPos) > 10) {
            _isDragging = true;
        }
        
        if (_isDragging) {
            Vec2 diff = currentPos - _touchStartPos;
            _mapNode->setPosition(_mapStartPos + diff);
            // Optional: Clamp map position so it doesn't fly off screen completely
        }
    };

    listener->onTouchEnded = [this](Touch* touch, Event* event) {
        if (!_isDragging) {
            // Click handling (select building, etc.)
            // Logic to find building at touch location relative to mapNode
            Vec2 touchLocationInMap = _mapNode->convertToNodeSpace(touch->getLocation());
            CCLOG("Clicked at map pos: %f, %f", touchLocationInMap.x, touchLocationInMap.y);
        }
    };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

void VillageScene::onAttackModeCallback(Ref* pSender) {
    CCLOG("Attack Mode Clicked");
}

void VillageScene::onBuildModeCallback(Ref* pSender) {
    CCLOG("Build Mode Clicked");
}

void VillageScene::onCloseWindowCallback(Ref* pSender) {
    // Generic close callback
}

} // namespace scene
