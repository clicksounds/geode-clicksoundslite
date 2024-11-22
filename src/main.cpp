#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/binding/FMODAudioEngine.hpp>

using namespace geode::prelude;

class $modify(PlayerObject) {

public:

// Click sounds
	bool pushButton(PlayerButton p0) {
		bool ret = PlayerObject::pushButton(p0);

	  if (Mod::get()->getSettingValue<bool>("OnlyOnJump")) {
    	if (p0 != PlayerButton::Jump) {
    		return ret;
    	}
    }

    if (!GameManager::sharedState()->getPlayLayer())
      return ret;

    log::debug("CSLite: Player jumped.");

    auto clickSoundFile = Mod::get()->getSettingValue<std::filesystem::path>("custom-presssound").string();

	  if (Mod::get()->getSettingValue<std::filesystem::path>("custom-presssound").string() == "Select a file") {
      clickSoundFile = "default-click.ogg"_spr;
    }

    // Play click sound
    FMODAudioEngine::sharedEngine()->playEffect(clickSoundFile);
    return ret;
  }

// Release sounds
	bool releaseButton(PlayerButton p0) {
		bool ret = PlayerObject::releaseButton(p0);

	  if (Mod::get()->getSettingValue<bool>("OnlyOnJump")) {
      if (p0 != PlayerButton::Jump) {
      	return ret;
      }
    }

    if (!GameManager::sharedState()->getPlayLayer())
      return ret;

    log::debug("CSLite: Player jumped.");

    auto releaseSoundFile = Mod::get()->getSettingValue<std::filesystem::path>("custom-releasesound").string();

    if (Mod::get()->getSettingValue<std::filesystem::path>("custom-releasesound").string() == "Select a file") {
      releaseSoundFile = "default-release.ogg"_spr;
    }

    // Play click sound
    FMODAudioEngine::sharedEngine()->playEffect(releaseSoundFile);
    return ret;
  }
};
