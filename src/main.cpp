#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/ui/GeodeUI.hpp>

using namespace geode::prelude;

bool integratyCheck(PlayerObject* object) {
    GJGameLevel* Level;
     if (!PlayLayer::get()) {
        if (!LevelEditorLayer::get()) {
            return false;
        }
        Level = LevelEditorLayer::get()->m_level;
     } else {
        Level = PlayLayer::get()->m_level;
     };
     
     if (object->m_isSecondPlayer && !Level->m_twoPlayerMode || !object->m_isSecondPlayer) {
        return true;
     } else {
        return false;
     }
}

class $modify(PlayerObject) {
public:
    struct Fields {
        // add only one channel per player object to prevent lag on click by creating 100's of sound channels
         FMOD::Channel* channel;
         bool directionUp = false;
         bool directionRight = false;
         bool directionLeft = false;
    };
    void SetupNewDirections(PlayerButton p0, bool Set) { 
        switch (p0) { 
            case PlayerButton::Jump:   m_fields->directionUp = Set; break; 
            case PlayerButton::Right:  m_fields->directionRight = Set; break; 
            case PlayerButton::Left:  m_fields->directionLeft = Set; break; 
            default:break; 
        } 
    }
    bool GetNewDirections(PlayerButton p0) { 
        switch (p0) { 
            case PlayerButton::Jump:   return m_fields->directionUp; break; 
            case PlayerButton::Right:  return m_fields->directionRight; break; 
            case PlayerButton::Left:  return m_fields->directionLeft; break; 
            default:break; 
        } 
        return false;
    }
    // click sounds
    bool pushButton(PlayerButton p0) {
        bool ret = PlayerObject::pushButton(p0);

        if (!integratyCheck(this)) {
            return ret;
        };
        // play sounds when "only play on jump" settings is enabled and the player input is a jump, left movement, or right movement.
        if (Mod::get()->getSettingValue<bool>("only-on-jump")) {
            if (p0 != PlayerButton::Jump) {
                return ret;
            }
        }

        // only continue if the player isn't in the editor or in gameplay
        if (!GameManager::sharedState()->getPlayLayer() && !GameManager::sharedState()->getEditorLayer()) {
            return ret;
        }

        auto clickSoundFile = Mod::get()->getSettingValue<std::filesystem::path>("custom-presssound").string();
        auto isClickEnabled = Mod::get()->getSettingValue<bool>("enable-clicksounds");
        auto click_vol = Mod::get()->getSettingValue<int64_t>("click-volume");


        if (click_vol <= 0) return ret;
        
        FMODAudioEngine* FMOD = FMODAudioEngine::sharedEngine();
        auto system = FMOD->m_system;
        FMOD::Sound* sound;
        if (system->createSound(clickSoundFile.c_str(), FMOD_DEFAULT, nullptr, &sound) == FMOD_OK && isClickEnabled) {
            system->playSound(sound, nullptr, false, &m_fields->channel);
            m_fields->channel->setVolume(click_vol / 50.f);
        }
        SetupNewDirections(p0,true);
        return ret;
    }

    // release sounds
    bool releaseButton(PlayerButton p0) {
        bool ret = PlayerObject::releaseButton(p0);
        
        if (!GetNewDirections(p0)) {
            return ret;
        };
         if (!integratyCheck(this)) {
            return ret;
        };

        // only continue if the player isnt in the editor or in gameplay
        if (!GameManager::sharedState()->getPlayLayer() && !GameManager::sharedState()->getEditorLayer()) {
            return ret;
        }

        auto releaseSoundFile = Mod::get()->getSettingValue<std::filesystem::path>("custom-releasesound").string();
        auto isReleaseEnabled = Mod::get()->getSettingValue<bool>("enable-releasesounds");
        auto release_vol = Mod::get()->getSettingValue<int64_t>("release-volume");

        auto fae = FMODAudioEngine::sharedEngine();
        auto system = fae->m_system;
        FMOD::Sound* sound;

        if (release_vol <= 0) return ret;

        if (system->createSound(releaseSoundFile.c_str(), FMOD_DEFAULT, nullptr, &sound) == FMOD_OK && isReleaseEnabled) {
            system->playSound(sound, nullptr, false, &m_fields->channel);
            m_fields->channel->setVolume(release_vol / 50.f);
        }
        SetupNewDirections(p0,false);
        return ret;
    }
};

// Create settings button in the pause menu if enabled in settings
class $modify(CSLitePauseLayer, PauseLayer) {
  void openCSLiteSettings(CCObject*) {
      geode::openSettingsPopup(Mod::get());
  }



  void customSetup() {
    PauseLayer::customSetup();
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    // failed to get, either someone doesn't have nodeids or messed something up so don't crash ;)
   CCNode* menu = this->getChildByID("left-button-menu");
    if (!menu) {
        menu = CCMenu::create();menu->setLayout(
            ColumnLayout::create()
                ->setGap(4.f)
                ->setAxisAlignment(AxisAlignment::End)
                ->setAxisReverse(true)
                ->setCrossAxisOverflow(false)
        );
        menu->setPosition({36.f, (winSize.height / 2)});
        menu->setContentSize({40, winSize.height - 40.f});
        menu->setZOrder(10);
        this->addChild(menu);
    }



    auto spr = CCSprite::create("csLiteSettingsSprite.png"_spr);

    auto btn = CCMenuItemSpriteExtra::create(
        spr,
        this,
        menu_selector(CSLitePauseLayer::openCSLiteSettings)
      );
    spr->setScale(0.7f);
    if(Mod::get()->getSettingValue<bool>("settings-button")) {
      btn->setPosition({menu->getContentSize().width / 2, btn->getContentSize().height / 2});
      btn->setID("csLiteSettingsButton");
      menu->addChild(btn);
    }
  }
};