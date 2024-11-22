#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/binding/FMODAudioEngine.hpp>

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

	auto clickSoundFile = Mod::get()->getSettingValue<std::filesystem::path>("custom-presssound").toString();

    // Play click sound
    FMODAudioEngine::sharedEngine()->playEffect(clickSoundFile, 1, 1, 1);
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

	auto releaseSoundFile = Mod::get()->getSettingValue<std::filesystem::path>("custom-releasesound").toString();

    // Play release sound
    FMODAudioEngine::sharedEngine()->playEffect(releaseSoundFile, 1, 1, 1);
  }
};
