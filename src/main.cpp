#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <Geode/loader/SettingV3.hpp>

using namespace geode::prelude;

// Custom class for Caching sounds (Make it less laggy for mobile platforms and such)
class SoundCache {
    public:
        std::string m_soundFile;
        FMOD::Sound* m_sound;
        SoundCache() {};

        void Setsound(std::string soundFile) {
                if (soundFile.c_str()) {
                    if (FMODAudioEngine::sharedEngine()->m_system->createSound(soundFile.c_str(), FMOD_DEFAULT, nullptr, &m_sound) == FMOD_OK) {
                        m_sound->setMode(FMOD_LOOP_OFF);
                       m_soundFile = soundFile;
                    }
                } 
        }

        ~SoundCache() {
            if (m_sound) {
                m_sound->release();
            }
        };

};

// Create the classes for Caching
static FMOD::Channel* Soundchannel;
static SoundCache* ClickSound = new SoundCache();
static SoundCache* ReleaseSound = new SoundCache();

// the check to see if you should play the sound or not
bool integrityCheck(PlayerObject* object, PlayerButton Pressed) {
    // play sounds when "only play on jump" settings is enabled and the player input is a jump, left movement, or right movement.
    if (Mod::get()->getSettingValue<bool>("only-on-jump")) {
        if (Pressed != PlayerButton::Jump) {
            return false;
        }
    }
    GJGameLevel* Level;
     if (!PlayLayer::get()) {
        if (!LevelEditorLayer::get()) {
            return false;
        }
        Level = LevelEditorLayer::get()->m_level;
     } else {
        PlayLayer* Pl = PlayLayer::get();
        // Mac fix
        if (Pl->m_isPaused) {
            return false;
        }
        Level = Pl->m_level;
     };
     if (object->m_isSecondPlayer && Level->m_twoPlayerMode || !object->m_isSecondPlayer) {
        return true;
     } else {
        return false;
     }
}

class $modify(PlayerObject) {
public:
        // add it to fields to access later but stored in the object (m_fields->Var)
    struct Fields {
         bool directionUp = false;
         bool directionRight = false;
         bool directionLeft = false;
    };
    
    // For setting bools for setting dir
    void SetupNewDirections(PlayerButton p0, bool Set) { 
        switch (p0) { 
            case PlayerButton::Jump:   m_fields->directionUp = Set; break; 
            case PlayerButton::Right:  m_fields->directionRight = Set; break; 
            case PlayerButton::Left:  m_fields->directionLeft = Set; break; 
            default:break; 
        } 
    }
    // getting the bools in a orderly way
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
        // check if you can and or check if it is correct
        if (!integrityCheck(this,p0)) {
            return ret;
        };

        auto clickSoundFile = Mod::get()->getSettingValue<std::filesystem::path>("custom-presssound").string();
        auto isClickEnabled = Mod::get()->getSettingValue<bool>("enable-clicksounds");
        auto click_vol = Mod::get()->getSettingValue<int64_t>("click-volume");
        // set the direction bool to true
        SetupNewDirections(p0,true);
        
        // is it enabled or is volume < 0
        if (click_vol <= 0 || !isClickEnabled) return ret;
        // should call failsafe if sound isn't the same?
        if (ClickSound->m_soundFile != clickSoundFile) {
            ClickSound->Setsound(clickSoundFile);
        }
        // sound player
        if (ClickSound->m_sound) {
            FMODAudioEngine::sharedEngine()->m_system->playSound(ClickSound->m_sound, nullptr, false, &Soundchannel);
           Soundchannel->setVolume(click_vol / 50.f);
        }
        return ret;
    }

    // release sounds
    bool releaseButton(PlayerButton p0) {
        bool ret = PlayerObject::releaseButton(p0);
        // Did you click? check
        if (!GetNewDirections(p0)) {
            return ret;
        };
        // check if you can and or check if it is correct
         if (!integrityCheck(this,p0)) {
            return ret;
        };

        auto releaseSoundFile = Mod::get()->getSettingValue<std::filesystem::path>("custom-releasesound").string();
        auto isReleaseEnabled = Mod::get()->getSettingValue<bool>("enable-releasesounds");
        auto release_vol = Mod::get()->getSettingValue<int64_t>("release-volume");
        // set the direction bool to false
        SetupNewDirections(p0,false);
        // is it enabled or is volume < 0
        if (release_vol <= 0 || !isReleaseEnabled) return ret;
        // should call failsafe if sound isn't the same?
        if (ReleaseSound->m_soundFile != releaseSoundFile) {
            ReleaseSound->Setsound(releaseSoundFile);
        }
        // sound player
        if (ReleaseSound->m_sound) {
            FMODAudioEngine::sharedEngine()->m_system->playSound(ReleaseSound->m_sound, nullptr, false, &Soundchannel);
            Soundchannel->setVolume(release_vol / 50.f);
        }

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
    CCNode* menu = this->getChildByID("left-button-menu");
    // have you failed to get? so i don't wanna crash
    if (!menu) {
         // failed to get, either someone doesn't have nodeids or messed something up so don't crash ;)
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
      btn->setID("csLiteSettingsButton"_spr);
      menu->addChild(btn);
      menu->updateLayout();
    }
  }
};

// on the mod loading
$execute {
    // Does the release-sound path setting change?
    listenForSettingChanges("custom-releasesound", [](std::filesystem::path releaseSoundFile) {
        ReleaseSound->Setsound(releaseSoundFile.string());
    });
    // Does the click-sound path setting change?
     listenForSettingChanges("custom-presssound", [](std::filesystem::path PressSoundSoundFile) {
        ClickSound->Setsound(PressSoundSoundFile.string());
    });
    // on boot set Sound Caches
    std::string releaseSoundFile = Mod::get()->getSettingValue<std::filesystem::path>("custom-releasesound").string();
    ReleaseSound->Setsound(releaseSoundFile);
    std::string clickSoundFile = Mod::get()->getSettingValue<std::filesystem::path>("custom-presssound").string();
    ClickSound->Setsound(clickSoundFile);
}

