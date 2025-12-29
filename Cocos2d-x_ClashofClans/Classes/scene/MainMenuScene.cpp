#include "MainMenuScene.h"
#include "SimpleAudioEngine.h"
#include "VillageScene.h"

USING_NS_CC;

namespace scene {

Scene* MainMenuScene::createScene() {
    return MainMenuScene::create();
}



// Helper to create a MenuItemSprite with cropped texture to remove transparent borders
static MenuItemSprite* createTrimmedButton(const std::string& normalImage, const std::string& selectedImage, const ccMenuCallback& callback) {
    auto image = new Image();
    if (!image->initWithImageFile(normalImage)) {
        delete image;
        return MenuItemImage::create(normalImage, selectedImage, callback);
    }
    
    int w = image->getWidth();
    int h = image->getHeight();
    unsigned char* data = image->getData();
    
    int minX = w, maxX = 0, minY = h, maxY = 0;
    bool found = false;
    
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            if (data[(y * w + x) * 4 + 3] > 0) {
                if (x < minX) minX = x;
                if (x > maxX) maxX = x;
                if (y < minY) minY = y;
                if (y > maxY) maxY = y;
                found = true;
            }
        }
    }
    image->release();
    
    if (found) {
        float cropW = maxX - minX + 1;
        float cropH = maxY - minY + 1;
        Rect rect(minX, minY, cropW, cropH);
        
        auto normalSprite = Sprite::create(normalImage, rect);
        auto selectedSprite = Sprite::create(selectedImage, rect);
        
        return MenuItemSprite::create(normalSprite, selectedSprite, callback);
    }
    
    return MenuItemImage::create(normalImage, selectedImage, callback);
}


static void problemLoading(const char* filename) {
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in MainMenuScene.cpp\n");
}

bool MainMenuScene::init() {
    //////////////////////////////
    // 1. super init first
    if (!Scene::init()) {
        return false;
    }

    // Play Background Music
    CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic("menu.mp3", true);

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    //////////////////////////////
    // 2. Add Background
    _background = Sprite::create("background.png");
    if (_background == nullptr) {
        problemLoading("background.png");
    } else {
        // Position the background in the center
        _background->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));
        

        float scaleX = visibleSize.width / _background->getContentSize().width;
        float scaleY = visibleSize.height / _background->getContentSize().height;
        _background->setScale(MAX(scaleX, scaleY));

        this->addChild(_background, -1);
    }

    //////////////////////////////
    // 3. Add Title
    _title = Sprite::create("title.png");
    if (_title == nullptr) {
        problemLoading("title.png");
    } else {
        // Center Top
        _title->setPosition(Vec2(visibleSize.width/2 + origin.x, 
                                visibleSize.height - _title->getContentSize().height/2 - 50)); // 50px padding from top
        this->addChild(_title, 0);
    }

    //////////////////////////////
    // 4. Add Buttons
    
    // Start Game Button
    MenuItem* startItem = createTrimmedButton(
                                           "begin_botton.png",
                                           "begin_botton.png",
                                           CC_CALLBACK_1(MainMenuScene::onStartGameCallback, this));

    if (startItem == nullptr) {
        problemLoading("begin_botton.png");
        // Fallback to text button
        auto label = Label::createWithSystemFont("Start Game", "Arial", 24);
        startItem = MenuItemLabel::create(label, CC_CALLBACK_1(MainMenuScene::onStartGameCallback, this));
    }

    // Exit Game Button
    MenuItem* exitItem = createTrimmedButton(
                                          "exit_botton.png",
                                          "exit_botton.png",
                                          CC_CALLBACK_1(MainMenuScene::onExitGameCallback, this));
    
    if (exitItem == nullptr) {
        problemLoading("exit_botton.png");
        // Fallback to text button
        auto label = Label::createWithSystemFont("Exit Game", "Arial", 24);
        exitItem = MenuItemLabel::create(label, CC_CALLBACK_1(MainMenuScene::onExitGameCallback, this));
    }

    // Create Menu
    auto menu = Menu::create(startItem, exitItem, nullptr);
    menu->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y - 100)); // Lower the menu position
    menu->alignItemsVerticallyWithPadding(20);
    this->addChild(menu, 1);
    
    // Debug info
    CCLOG("Menu created at: %f, %f", menu->getPositionX(), menu->getPositionY());
    CCLOG("Visible Size: %f, %f", visibleSize.width, visibleSize.height);
    CCLOG("Origin: %f, %f", origin.x, origin.y);

    return true;
}

void MainMenuScene::onStartGameCallback(Ref* pSender) {
    CCLOG("Start Game Button Clicked");
    auto scene = VillageScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5, scene));
}

void MainMenuScene::onExitGameCallback(Ref* pSender) {
    // Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();

}

} // namespace scene
