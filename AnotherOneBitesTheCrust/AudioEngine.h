#pragma once

#include <fmod.hpp>
#include <fmod_errors.h>
#include <cstdio>
#include <glm.hpp>
#include <queue>
#include <list>
#include <map>
#include <sigslot.h>
#include <random>

#include "Vehicle.h"
struct VehicleSounds
{
	FMOD::Channel * engineIdleChannel;
	FMOD::Channel * engineRevChannel;
	bool brake;
};

class AudioEngine : public sigslot::has_slots<>
{
public:
	AudioEngine(void);
	~AudioEngine(void);

	void startBackgroundMusic();
	void playDeliveryGetSound(Vehicle * source);
	void playDeliveryFailSound(Vehicle * source);
	void playReloadSound(Vehicle * source);
	void playDryFireSound(Vehicle * source);
	void playCollisionSound(Vehicle * source);
	void playCannonSound(Vehicle * source);
	void playBrakeSound(Vehicle * source);
	void playEngineIdleSound(Vehicle * source);
	void playEngineRevSound(Vehicle * source);
	void setNumListeners(int numPlayers);
	FMOD::Channel * playSound(FMOD::Sound *, glm::vec3, PhysicsEntity *,float volume);
	void update(std::vector<glm::mat4>);

private:
	FMOD::System *fmodSystem;
	FMOD::Sound *backgroundMusic, *cannonSound, *brakeSound, *engineIdleSound, *engineRevSound, *reloadSound, *dryFireSound, *crashSound, *deliveryFailSound, *deliveryGetSound;
	std::vector<FMOD::Sound*> backgroundSongs;
	FMOD::Channel *backgroundChannel, *engineChannel;
	static FMOD_RESULT result;
	std::map<FMOD::Channel*, PhysicsEntity*> currentlyPlaying;

	std::map<PhysicsEntity *, VehicleSounds> vehicleLoops;

	unsigned int numChannels;
	int backgroundSongChoice;

	std::mt19937 generator;

	void initStreams();
	FMOD_VECTOR glmVec3ToFmodVec(glm::vec3);
	bool stillPlaying(FMOD::Channel *);
	void update3DPositions();

	static inline void	errorCheck();

	static FMOD_RESULT F_CALLBACK channelCallback(FMOD_CHANNELCONTROL *chanControl, FMOD_CHANNELCONTROL_TYPE controlType, FMOD_CHANNELCONTROL_CALLBACK_TYPE callbackType, void *commandData1, void *commandData2);
};