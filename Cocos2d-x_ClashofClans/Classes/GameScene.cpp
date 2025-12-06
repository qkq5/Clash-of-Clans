#include "GameScene.h"
#include "Mine.h"
#include "Barracks.h"
#include "ResourceManager.h"
#include "BattleScene.h"

USING_NS_CC;
using namespace ui;

Scene* GameScene::createScene(){ return GameScene::create(); }

bool GameScene::init(){ if(!Scene::init()) return false; pick=BuildPick::None; goldTick=0.f; elixirTick=0.f; soldierTick=0.f; setupBackground(); setupHUD(); setupBottomUI(); auto vs=Director::getInstance()->getVisibleSize(); auto org=Director::getInstance()->getVisibleOrigin(); auto home=Sprite::create("home.png"); if(home){ float targetW=vs.width/10.f; float s=targetW/home->getContentSize().width; home->setScale(s); home->setPosition(Vec2(org.x+vs.width*0.5f, org.y+vs.height*0.55f)); addChild(home, 50); } auto gm=GoldMine::create(1, Vec2(org.x+vs.width*0.35f, org.y+vs.height*0.6f)); if(gm){ addBuilding(gm); addChild(gm, 50); } auto ec=ElixirCollector::create(1, Vec2(org.x+vs.width*0.65f, org.y+vs.height*0.6f)); if(ec){ addBuilding(ec); addChild(ec, 50); } auto bk=Barracks::create(1, Vec2(org.x+vs.width*0.5f, org.y+vs.height*0.7f)); if(bk){ addBuilding(bk); addChild(bk, 50); } auto listener=EventListenerTouchOneByOne::create(); listener->onTouchBegan=CC_CALLBACK_2(GameScene::onTouchBegan, this); _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this); scheduleUpdate(); return true; }

void GameScene::setupBackground(){ auto vs=Director::getInstance()->getVisibleSize(); auto org=Director::getInstance()->getVisibleOrigin(); auto bg=Sprite::create("background.png"); if(bg){ auto sz = bg->getContentSize(); float sx = vs.width / sz.width; float sy = vs.height / sz.height; float s = std::min(sx, sy); bg->setScale(s); bg->setPosition(Vec2(org.x+vs.width/2, org.y+vs.height/2)); addChild(bg, -100); } }

void GameScene::setupHUD(){ 
    auto vs=Director::getInstance()->getVisibleSize(); 
    auto org=Director::getInstance()->getVisibleOrigin(); 
    // 增大字体，提高可读性
    resourceLabel=Label::createWithTTF("", "fonts/Marker Felt.ttf", 24); 
    resourceLabel->setAnchorPoint(Vec2(0,1)); 
    // 确保文字在按钮上方，不被遮挡
    resourceLabel->setPosition(Vec2(org.x+10, org.y+vs.height - 10)); 
    addChild(resourceLabel, 300); // 提高图层优先级，确保在按钮上方
}

void GameScene::setupBottomUI(){
    auto vs=Director::getInstance()->getVisibleSize();
    auto org=Director::getInstance()->getVisibleOrigin();
    bottomBar = Layout::create();
    float barHeight = vs.height * 0.12f; // 底部栏高度为屏幕的12%
    bottomBar->setContentSize(Size(vs.width, barHeight));
    bottomBar->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
    bottomBar->setBackGroundColor(Color3B(50,50,50));
    bottomBar->setAnchorPoint(Vec2(0,0));
    bottomBar->setPosition(Vec2(org.x, org.y));
    addChild(bottomBar, 200);

    // battle 按钮（左下角）- 缩小到原来的1/2
    float battleBtnW = vs.width * 0.06f; // 原来是0.12f，现在缩小一半
    battleBtn = Button::create("button.png","button.png");
    battleBtn->setTitleText("Battle");
    battleBtn->setTitleFontSize(7); // 缩小字体
    float battleBtnScale = battleBtnW / battleBtn->getContentSize().width;
    battleBtn->setScale(battleBtnScale);
    battleBtn->setPosition(Vec2(org.x + battleBtnW/2 + 10, bottomBar->getContentSize().height/2));
    battleBtn->addClickEventListener([&](Ref*){
        auto scene = BattleScene::createScene();
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
    });
    bottomBar->addChild(battleBtn, 1);
    
    // Battle按钮的红色提示文字
    auto battleLabel = Label::createWithTTF("Battle", "fonts/Marker Felt.ttf", 72); // 放大3倍：24*3=72
    battleLabel->setColor(Color3B::RED);
    battleLabel->setPosition(Vec2(org.x + battleBtnW/2 + 10, bottomBar->getContentSize().height/2 + battleBtnW/2 + 20));
    addChild(battleLabel, 400); // 最高图层，确保不被遮挡

    // 建筑选择按钮 - 缩小到原来的1/2
    float btnSpacing = vs.width * 0.11f; // 按钮间距也缩小
    float btnStartX = org.x + battleBtnW + 30;
    float btnW = vs.width * 0.075f; // 原来是0.15f，现在缩小一半
    
    btnGM = Button::create("button.png","button.png");
    btnGM->setTitleText("GoldMine");
    btnGM->setTitleFontSize(7); // 缩小字体
    float btnScale = btnW / btnGM->getContentSize().width;
    btnGM->setScale(btnScale);
    btnGM->setPosition(Vec2(btnStartX + btnW/2, bottomBar->getContentSize().height/2));
    bottomBar->addChild(btnGM, 1);
    // GoldMine按钮的红色提示文字
    auto gmLabel = Label::createWithTTF("GoldMine", "fonts/Marker Felt.ttf", 72);
    gmLabel->setColor(Color3B::RED);
    gmLabel->setPosition(Vec2(btnStartX + btnW/2, bottomBar->getContentSize().height/2 + btnW/2 + 20));
    addChild(gmLabel, 400);

    btnEC = Button::create("button.png","button.png");
    btnEC->setTitleText("Elixir");
    btnEC->setTitleFontSize(7);
    btnEC->setScale(btnScale);
    btnEC->setPosition(Vec2(btnStartX + btnW + btnSpacing, bottomBar->getContentSize().height/2));
    bottomBar->addChild(btnEC, 1);
    // Elixir按钮的红色提示文字
    auto ecLabel = Label::createWithTTF("Elixir", "fonts/Marker Felt.ttf", 72);
    ecLabel->setColor(Color3B::RED);
    ecLabel->setPosition(Vec2(btnStartX + btnW + btnSpacing, bottomBar->getContentSize().height/2 + btnW/2 + 20));
    addChild(ecLabel, 400);

    btnBK = Button::create("button.png","button.png");
    btnBK->setTitleText("Barracks");
    btnBK->setTitleFontSize(7);
    btnBK->setScale(btnScale);
    btnBK->setPosition(Vec2(btnStartX + btnW*2 + btnSpacing*2, bottomBar->getContentSize().height/2));
    bottomBar->addChild(btnBK, 1);
    // Barracks按钮的红色提示文字
    auto bkLabel = Label::createWithTTF("Barracks", "fonts/Marker Felt.ttf", 72);
    bkLabel->setColor(Color3B::RED);
    bkLabel->setPosition(Vec2(btnStartX + btnW*2 + btnSpacing*2, bottomBar->getContentSize().height/2 + btnW/2 + 20));
    addChild(bkLabel, 400);

    btnGM->addClickEventListener([&](Ref*){ pick = (pick==BuildPick::GoldMine?BuildPick::None:BuildPick::GoldMine); updateHighlight(); });
    btnEC->addClickEventListener([&](Ref*){ pick = (pick==BuildPick::ElixirCollector?BuildPick::None:BuildPick::ElixirCollector); updateHighlight(); });
    btnBK->addClickEventListener([&](Ref*){ pick = (pick==BuildPick::Barracks?BuildPick::None:BuildPick::Barracks); updateHighlight(); });

    updateHighlight();
}

void GameScene::updateHighlight(){ if(!btnGM||!btnEC||!btnBK) return; btnGM->setTitleColor(pick==BuildPick::GoldMine?Color3B::YELLOW:Color3B::WHITE); btnEC->setTitleColor(pick==BuildPick::ElixirCollector?Color3B::YELLOW:Color3B::WHITE); btnBK->setTitleColor(pick==BuildPick::Barracks?Color3B::YELLOW:Color3B::WHITE); }

bool GameScene::onTouchBegan(Touch* touch, Event* event){ auto p = touch->getLocation(); if(pick==BuildPick::None) return false; auto barPos=bottomBar->getPosition(); auto barSize=bottomBar->getContentSize(); if(p.y <= barPos.y + barSize.height) return false; Cost c{0,0,0}; if(pick==BuildPick::GoldMine){ c.gold=10; } else if(pick==BuildPick::ElixirCollector){ c.gold=30; } else if(pick==BuildPick::Barracks){ c.elixir=10; } auto& rm=ResourceManager::getInstance(); if(!rm.canAfford(c)){ auto vs=Director::getInstance()->getVisibleSize(); auto org=Director::getInstance()->getVisibleOrigin(); auto lab=Label::createWithTTF("Resource Not Enough", "fonts/Marker Felt.ttf", 24); lab->setPosition(Vec2(org.x+vs.width*0.5f, org.y+vs.height*0.5f)); addChild(lab, 20); lab->runAction(Sequence::create(DelayTime::create(3.f), RemoveSelf::create(), nullptr)); return true; } if(!rm.spend(c)) return true; Building* obj=nullptr; if(pick==BuildPick::GoldMine){ obj = GoldMine::create(1, p); } else if(pick==BuildPick::ElixirCollector){ obj = ElixirCollector::create(1, p); } else if(pick==BuildPick::Barracks){ obj = Barracks::create(1, p); } if(obj){ addBuilding(obj); addChild(obj, 50); pick=BuildPick::None; updateHighlight(); } return true; }

const std::vector<Building*>& GameScene::getBuildings() const{ return buildings; }
void GameScene::addUnit(Unit* u){ if(!u) return; units.push_back(u); addChild(u); }
void GameScene::addBuilding(Building* b){ if(!b) return; buildings.push_back(b); }
void GameScene::update(float dt){
    for(auto* u: units){ if(u) u->update(dt); }

    // 清理已摧毁的建筑
    std::vector<Building*> alive; alive.reserve(buildings.size());
    for(auto* b: buildings){
        if(!b) continue;
        if(b->isDestroyed()){
            removeChild(b);
        } else {
            alive.push_back(b);
        }
    }
    buildings.swap(alive);

    // 统计各类建筑数量
    int g=0,e=0,s=0;
    for(auto* b: buildings){
        if(!b) continue;
        if(b->type==BuildingType::Mine) g++;
        else if(b->type==BuildingType::ElixirCollector) e++;
        else if(b->type==BuildingType::Barracks) s++;
    }

    goldTick+=dt;
    elixirTick+=dt;
    soldierTick+=dt;

    auto& rm=ResourceManager::getInstance();

    // 金币自动产出：每秒每个金矿 1 金币，并自动收集
    while(goldTick>=1.f){
        goldTick-=1.f;
        if(g>0){
            // 自动收集所有金矿的资源
            for(auto* b: buildings){
                if(!b || b->isDestroyed()) continue;
                if(b->type==BuildingType::Mine){
                    auto* mine = dynamic_cast<GoldMine*>(b);
                    if(mine){
                        mine->updateMine(1.f); // 更新存储
                        mine->collect(); // 自动收集
                    }
                }
            }
        }
    }

    // 圣水自动产出：每秒每个采集器 1 圣水，并自动收集
    while(elixirTick>=1.f){
        elixirTick-=1.f;
        if(e>0){
            // 自动收集所有圣水采集器的资源
            for(auto* b: buildings){
                if(!b || b->isDestroyed()) continue;
                if(b->type==BuildingType::ElixirCollector){
                    auto* collector = dynamic_cast<ElixirCollector*>(b);
                    if(collector){
                        collector->updateCollector(1.f); // 更新存储
                        collector->collect(); // 自动收集
                    }
                }
            }
        }
    }

    // 士兵自动训练：每秒每个军营 1 士兵，消耗 10 圣水
    while(soldierTick>=1.f){
        soldierTick-=1.f;
        if(s>0){
            // 尝试为每个军营生产1名士兵
            int produced = 0;
            for(int i=0;i<s;++i){
                Cost c{0,10,0};
                if(rm.canAfford(c) && rm.spend(c)){
                    rm.addPop(1);
                    produced++;
                } else {
                    // 资源不足，停止生产
                    break;
                }
            }
        }
    }

    if(resourceLabel){
        char buf[128];
        sprintf(buf, "Gold:%d  Elixir:%d  Soldiers:%d", rm.getGold(), rm.getElixir(), rm.getPop());
        resourceLabel->setString(buf);
    }
}

