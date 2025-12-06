#include "HomeScene.h"
#include "Mine.h"
#include "StorageBuilding.h"
#include "Barracks.h"
#include "Unit.h"

USING_NS_CC;
using namespace ui;

Scene* HomeScene::createScene(){ return HomeScene::create(); }

bool HomeScene::init(){ if(!Scene::init()) return false; setupHUD(); goldStorage = GoldStorage::create(1, Vec2(80,80)); addChild(goldStorage); elixirStorage = ElixirStorage::create(1, Vec2(140,80)); addChild(elixirStorage); goldMine = GoldMine::create(1, Vec2(80,160)); addChild(goldMine); elixirCollector = ElixirCollector::create(1, Vec2(140,160)); addChild(elixirCollector); barracks = Barracks::create(1, Vec2(220,160)); addChild(barracks); barracks->setOnUnitTrained([&](UnitType t){ Vec2 p = barracks->getPosition()+Vec2(40,0); Node* u=nullptr; if(t==UnitType::Barbarian) u=Barbarian::create(p); else if(t==UnitType::Archer) u=Archer::create(p); else if(t==UnitType::Bomber) u=WallBreaker::create(p); else if(t==UnitType::Giant) u=Giant::create(p); if(u){ addChild(u); } }); auto vs=Director::getInstance()->getVisibleSize(); auto org=Director::getInstance()->getVisibleOrigin(); auto btnBarb=ui::Button::create("CloseNormal.png","CloseSelected.png"); btnBarb->setScale(0.5f); btnBarb->setPosition(Vec2(org.x+260, org.y+vs.height-40)); btnBarb->addClickEventListener([&](Ref*){ barracks->enqueue(UnitType::Barbarian); }); addChild(btnBarb,1); auto btnArch=ui::Button::create("CloseNormal.png","CloseSelected.png"); btnArch->setScale(0.5f); btnArch->setPosition(Vec2(org.x+300, org.y+vs.height-40)); btnArch->addClickEventListener([&](Ref*){ barracks->enqueue(UnitType::Archer); }); addChild(btnArch,1); auto btnWB=ui::Button::create("CloseNormal.png","CloseSelected.png"); btnWB->setScale(0.5f); btnWB->setPosition(Vec2(org.x+340, org.y+vs.height-40)); btnWB->addClickEventListener([&](Ref*){ barracks->enqueue(UnitType::Bomber); }); addChild(btnWB,1); auto btnG=ui::Button::create("CloseNormal.png","CloseSelected.png"); btnG->setScale(0.5f); btnG->setPosition(Vec2(org.x+380, org.y+vs.height-40)); btnG->addClickEventListener([&](Ref*){ barracks->enqueue(UnitType::Giant); }); addChild(btnG,1); scheduleUpdate(); return true; }

void HomeScene::setupHUD(){ auto vs = Director::getInstance()->getVisibleSize(); auto org = Director::getInstance()->getVisibleOrigin(); hudLabel = Label::createWithTTF("", "fonts/Marker Felt.ttf", 20); hudLabel->setAnchorPoint(Vec2(0,1)); hudLabel->setPosition(Vec2(org.x+10, org.y+vs.height-10)); addChild(hudLabel, 10); refreshHUD(); }

void HomeScene::refreshHUD(){ auto& rm = ResourceManager::getInstance(); char buf[128]; sprintf(buf, "Gold:%d  Elixir:%d  Pop:%d/%d", rm.getGold(), rm.getElixir(), rm.getPop(), rm.getPopCap()); hudLabel->setString(buf); }

void HomeScene::update(float dt){ if(goldMine) goldMine->updateMine(dt); if(elixirCollector) elixirCollector->updateCollector(dt); if(barracks) barracks->updateBarracks(dt); refreshHUD(); }
