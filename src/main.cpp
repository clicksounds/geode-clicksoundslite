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

    auto clickSoundFile = Mod::get()->getSettingValue<std::filesystem::path>("custom-presssound").string();
    auto isClickEnabled = Mod::get()->getSettingValue<bool>("enable-clicksounds");

    // Play click sound
    if (isClickEnabled)
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

    auto releaseSoundFile = Mod::get()->getSettingValue<std::filesystem::path>("custom-releasesound").string();
    auto isReleaseEnabled = Mod::get()->getSettingValue<bool>("enable-releasesounds");

    // Play click sound
    if (isReleaseEnabled)
        FMODAudioEngine::sharedEngine()->playEffect(releaseSoundFile);
    
    return ret;
  }
};
