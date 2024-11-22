#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/binding/FMODAudioEngine.hpp>

using namespace geode::prelude;

class $modify(PlayerObject) {

public:

// Click sounds
	bool pushButton(PlayerButton p0) {
		return PlayerObject::pushButton(p0);

	if (Mod::get()->getSettingValue<bool>("OnlyOnJump")) {
    	if (p0 != PlayerButton::Jump) {
    		return;
    	}
    }

    if (!GameManager::sharedState()->getPlayLayer())
      return;

    geode::log << "Pushed Player Jump (CSLite)";

	auto clickSoundFile = Mod::get()->getSettingValue<std::filesystem::path>("custom-presssound").string();

    // Play click sound
    FMODAudioEngine::sharedEngine()->playEffect(default-click.ogg_spr);
  }

// Release sounds

  bool releaseButton(PlayerButton p0) {
    return PlayerObject::releaseButton(p0);

  if (Mod::get()->getSettingValue<bool>("OnlyOnJump")) {
      if (p0 != PlayerButton::Jump) {
        return;
      }
    }

    if (!GameManager::sharedState()->getPlayLayer())
      return;

	auto releaseSoundFile = Mod::get()->getSettingValue<std::filesystem::path>("custom-releasesound").string();

    // Play release sound
    FMODAudioEngine::sharedEngine()->playEffect(releaseSoundFile);
  }
};
