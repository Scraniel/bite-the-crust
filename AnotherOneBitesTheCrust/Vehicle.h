#pragma once
#include <PxPhysicsAPI.h>
#include "DynamicEntity.h"
#include <sigslot.h>

struct VehicleInput
{
	float leftSteer;
	float rightSteer;
	float forward;
	float backward;
	bool handBrake;
	bool shootPizza;
};

struct VehicleTuning
{
	physx::PxF32 chassisMass;
	physx::PxVec3 chassisDims;
	physx::PxVec3 chassisMOI;
	physx::PxVec3 chassisCMOffset;
	physx::PxMaterial* chassisMaterial;
	physx::PxF32 wheelMass;
	physx::PxF32 wheelWidth;
	physx::PxF32 wheelRadius;
	physx::PxF32 wheelMOI;
	physx::PxMaterial* wheelMaterial;
	physx::PxU32 numWheels;
	physx::PxReal chassisStaticFriction;
	physx::PxReal chassisDynamicFriction;
	physx::PxReal chassisRestitution;
	physx::PxReal wheelStaticFriction;
	physx::PxReal wheelDynamicFriction;
	physx::PxReal wheelRestitution;
};

class Vehicle :
	public DynamicEntity
{
public:
	Vehicle(void);
	~Vehicle(void);

	physx::PxVehicleDrive4W* physicsVehicle;

	// AI stuff; might be moved into 'Player' class
	std::vector<glm::vec3> currentPath;

	void updateTuning();
	void handleInput();
	VehicleInput* getInputStruct();
	VehicleTuning* getTuningStruct();
	physx::PxVehicleDrive4W* getPhysicsVehicle();

	sigslot::signal1<Vehicle*> ShootPizzaSignal;

private:
	VehicleInput input;
	VehicleTuning tuning;

	void defaultTuning();
};

