#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

class $modify(PlayerObject) {

public:

// Click sounds
	void pushButton(PlayerButton p0) {
		PlayerObject::pushButton(p0);

	if (Mod::get()->getSettingValue<bool>("OnlyOnJump")) {
    	if (p0 != PlayerButton::Jump) {
    		return;
    	}
    }

    if (!GameManager::sharedState()->getPlayLayer())
      return;

	auto clickSoundFile = Mod::get()->getSettingValue<std::filesystem::path>("custom-presssound");

    // Play click sound
    FMODAudioEngine::sharedEngine()->playEffect(clickSoundFile_spr);
  }

// Release sounds

  void releaseButton(PlayerButton p0) {
    PlayerObject::releaseButton(p0);

  if (Mod::get()->getSettingValue<bool>("OnlyOnJump")) {
      if (p0 != PlayerButton::Jump) {
        return;
      }
    }

    if (!GameManager::sharedState()->getPlayLayer())
      return;

	auto releaseSoundFile = Mod::get()->getSettingValue<std::filesystem::path>("custom-releasesound");

    // Play release sound
    FMODAudioEngine::sharedEngine()->playEffect(releaseSoundFile_spr);
  }
};