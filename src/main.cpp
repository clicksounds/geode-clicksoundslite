#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/ui/GeodeUI.hpp>

using namespace geode::prelude;

class $modify(PlayerObject) {
public:
    // click sounds
    bool pushButton(PlayerButton p0) {
        bool ret = PlayerObject::pushButton(p0);

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

        auto fae = FMODAudioEngine::sharedEngine();
        auto system = fae->m_system;
        FMOD::Channel* channel;
        FMOD::Sound* sound;

        if (click_vol <= 0) return ret;

        if (system->createSound(clickSoundFile.c_str(), FMOD_DEFAULT, nullptr, &sound) == FMOD_OK && isClickEnabled) {
            system->playSound(sound, nullptr, false, &channel);
            channel->setVolume(click_vol / 50.f);
        }
        return ret;
    }

    // release sounds
    bool releaseButton(PlayerButton p0) {
        bool ret = PlayerObject::releaseButton(p0);

        if (Mod::get()->getSettingValue<bool>("only-on-jump")) {
            if (p0 != PlayerButton::Jump) {
                return ret;
            }
        }

        // only continue if the player isnt in the editor or in gameplay
        if (!GameManager::sharedState()->getPlayLayer() && !GameManager::sharedState()->getEditorLayer()) {
            return ret;
        }

        auto clickSoundFile = Mod::get()->getSettingValue<std::filesystem::path>("custom-releasesound").string();
        auto isClickEnabled = Mod::get()->getSettingValue<bool>("enable-releasesounds");
        auto click_vol = Mod::get()->getSettingValue<int64_t>("release-volume");

        auto fae = FMODAudioEngine::sharedEngine();
        auto system = fae->m_system;
        FMOD::Channel* channel;
        FMOD::Sound* sound;

        if (click_vol <= 0) return ret;

        if (system->createSound(clickSoundFile.c_str(), FMOD_DEFAULT, nullptr, &sound) == FMOD_OK && isClickEnabled) {
            system->playSound(sound, nullptr, false, &channel);
            channel->setVolume(click_vol / 50.f);
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
    auto menu = this->getChildByID("left-button-menu");
    auto winSize = CCDirector::sharedDirector()->getWinSize();
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