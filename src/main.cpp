#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

class SoundCache {
    public:
        std::string m_soundFile;
        FMOD::Sound* m_sound = nullptr;
        SoundCache() {};

        void Setsound(std::string soundFile) {
            if (!soundFile.empty()) {
                if (FMODAudioEngine::sharedEngine()->m_system->createSound(soundFile.c_str(), FMOD_DEFAULT, nullptr, &m_sound) == FMOD_OK) {
                    m_sound->setMode(FMOD_LOOP_OFF);
                    m_soundFile = soundFile;
                }
            } 
        }

        ~SoundCache() {
            if (m_sound) m_sound->release();
        };
};

static FMOD::Channel* Soundchannel;
static SoundCache* ClickSound = new SoundCache();
static SoundCache* ReleaseSound = new SoundCache();
static SoundCache* SoftClickSound = new SoundCache();
static SoundCache* SoftReleaseSound = new SoundCache();

bool integrityCheck(PlayerObject* object, PlayerButton Pressed) {
    if (Mod::get()->getSettingValue<bool>("only-on-jump")) {
        if (Pressed != PlayerButton::Jump) return false;
    }
    GJBaseGameLayer* LayerCheck = GJBaseGameLayer::get();
    if (!LayerCheck) return false;

    if (PlayLayer::get() && PlayLayer::get()->m_isPaused) return false;

    if (LayerCheck->m_player2 == object || LayerCheck->m_player1 == object) return true;
    return false;
}

class $modify(CSPlayerObject, PlayerObject) {
public:
    struct Fields {
         bool directionUp = false;
         bool directionRight = false;
         bool directionLeft = false;
         
         // true if previous press was within 0.3s
         bool m_withinSoftWindow = false; 
         
         bool m_isSoftJump = false;
         bool m_isSoftRight = false;
         bool m_isSoftLeft = false;
         
         CCNode* m_timerNode = nullptr;
    };

    bool init(int p0, int p1, GJBaseGameLayer* p2, CCLayer* p3, bool p4) {
        if (!PlayerObject::init(p0, p1, p2, p3, p4)) return false;
        m_fields->m_timerNode = CCNode::create();
        this->addChild(m_fields->m_timerNode);
        return true;
    }
    
    void setButtonSoftState(PlayerButton btn, bool isSoft) {
        if (btn == PlayerButton::Jump) m_fields->m_isSoftJump = isSoft;
        else if (btn == PlayerButton::Right) m_fields->m_isSoftRight = isSoft;
        else if (btn == PlayerButton::Left) m_fields->m_isSoftLeft = isSoft;
    }

    bool getButtonSoftState(PlayerButton btn) {
        if (btn == PlayerButton::Jump) return m_fields->m_isSoftJump;
        if (btn == PlayerButton::Right) return m_fields->m_isSoftRight;
        if (btn == PlayerButton::Left) return m_fields->m_isSoftLeft;
        return false;
    }

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
            case PlayerButton::Jump:   return m_fields->directionUp;
            case PlayerButton::Right:  return m_fields->directionRight;
            case PlayerButton::Left:   return m_fields->directionLeft;
            default:break; 
        } 
        return false;
    }

    bool pushButton(PlayerButton p0) {
        bool ret = PlayerObject::pushButton(p0);
        if (!integrityCheck(this, p0)) return ret;

        auto isClickEnabled = Mod::get()->getSettingValue<bool>("enable-clicksounds");
        auto click_vol = Mod::get()->getSettingValue<int64_t>("click-volume");
        if (click_vol <= 0 || !isClickEnabled) return ret;

        SetupNewDirections(p0, true);

        bool useSoft = false;
        if (Mod::get()->getSettingValue<bool>("enable-softsounds") && m_fields->m_withinSoftWindow) {
            useSoft = true;
        }

        setButtonSoftState(p0, useSoft);

        if (Mod::get()->getSettingValue<bool>("enable-softsounds")) {
            m_fields->m_withinSoftWindow = true;
            if (m_fields->m_timerNode) {
                m_fields->m_timerNode->stopAllActions();
                auto delay = CCDelayTime::create(0.3f);
                auto callback = CallFuncExt::create([this]() {
                    this->m_fields->m_withinSoftWindow = false;
                });
                m_fields->m_timerNode->runAction(CCSequence::create(delay, callback, nullptr));
            }
        }

        SoundCache* currentCache = useSoft ? SoftClickSound : ClickSound;
        std::string settingName = useSoft ? "custom-softpresssound" : "custom-presssound";
        auto clickSoundFile = Mod::get()->getSettingValue<std::filesystem::path>(settingName).string();

        if (currentCache->m_soundFile != clickSoundFile) {
            currentCache->Setsound(clickSoundFile);
        }

        if (currentCache->m_sound) {
            FMODAudioEngine::sharedEngine()->m_system->playSound(currentCache->m_sound, nullptr, false, &Soundchannel);
            Soundchannel->setVolume(click_vol / 50.f);
        }
        return ret;
    }

    bool releaseButton(PlayerButton p0) {
        bool ret = PlayerObject::releaseButton(p0);
        if (!GetNewDirections(p0) || !integrityCheck(this, p0)) return ret;

        auto isReleaseEnabled = Mod::get()->getSettingValue<bool>("enable-releasesounds");
        auto release_vol = Mod::get()->getSettingValue<int64_t>("release-volume");
        if (release_vol <= 0 || !isReleaseEnabled) return ret;

        bool useSoft = getButtonSoftState(p0);
        SetupNewDirections(p0, false);

        SoundCache* currentCache = useSoft ? SoftReleaseSound : ReleaseSound;
        std::string settingName = useSoft ? "custom-softreleasesound" : "custom-releasesound";
        auto releaseSoundFile = Mod::get()->getSettingValue<std::filesystem::path>(settingName).string();

        if (currentCache->m_soundFile != releaseSoundFile) {
            currentCache->Setsound(releaseSoundFile);
        }

        if (currentCache->m_sound) {
            FMODAudioEngine::sharedEngine()->m_system->playSound(currentCache->m_sound, nullptr, false, &Soundchannel);
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
    auto loadSound = [](std::string setting, SoundCache* cache) {
        std::string path = Mod::get()->getSettingValue<std::filesystem::path>(setting).string();
        cache->Setsound(path);
    };

    loadSound("custom-presssound", ClickSound);
    loadSound("custom-releasesound", ReleaseSound);
    loadSound("custom-softpresssound", SoftClickSound);
    loadSound("custom-softreleasesound", SoftReleaseSound);

    listenForSettingChanges<std::filesystem::path>("custom-presssound", [](std::filesystem::path p) { ClickSound->Setsound(p.string()); });
    listenForSettingChanges<std::filesystem::path>("custom-releasesound", [](std::filesystem::path p) { ReleaseSound->Setsound(p.string()); });
    listenForSettingChanges<std::filesystem::path>("custom-softpresssound", [](std::filesystem::path p) { SoftClickSound->Setsound(p.string()); });
    listenForSettingChanges<std::filesystem::path>("custom-softreleasesound", [](std::filesystem::path p) { SoftReleaseSound->Setsound(p.string()); });
}