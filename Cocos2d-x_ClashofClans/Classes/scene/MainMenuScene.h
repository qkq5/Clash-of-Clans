#ifndef __MAIN_MENU_SCENE_H__
#define __MAIN_MENU_SCENE_H__

#include "cocos2d.h"

namespace scene {

class MainMenuScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();

    virtual bool init() override;
    
    // Implement the "static create()" method manually
    CREATE_FUNC(MainMenuScene);

private:
    void setupUI();
    void onStartGameCallback(cocos2d::Ref* pSender);
    void onExitGameCallback(cocos2d::Ref* pSender);

    cocos2d::Sprite* _background;
    cocos2d::Sprite* _title;
};

} // namespace scene

#endif // __MAIN_MENU_SCENE_H__
