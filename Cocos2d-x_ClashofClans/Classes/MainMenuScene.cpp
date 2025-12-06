#include "MainMenuScene.h"
#include "GameScene.h"

USING_NS_CC;
using namespace ui;

Scene* MainMenuScene::createScene(){ return MainMenuScene::create(); }

bool MainMenuScene::init(){ if(!Scene::init()) return false; auto vs=Director::getInstance()->getVisibleSize(); auto org=Director::getInstance()->getVisibleOrigin(); auto btn=Button::create("CloseNormal.png","CloseSelected.png"); btn->setScale(1.0f); btn->setTitleText("Single Player"); btn->setTitleFontSize(24); btn->setPosition(Vec2(org.x+vs.width/2, org.y+vs.height/2)); btn->addClickEventListener([&](Ref*){ auto next=GameScene::createScene(); auto trans=TransitionFade::create(0.5f, next); Director::getInstance()->replaceScene(trans); }); addChild(btn); return true; }
