#pragma once
#include <PxPhysicsAPI.h>
#include "PhysicsEntityInfo.h"

class PhysicsHelper
{
public:
	PhysicsHelper(physx::PxPhysics* physics, physx::PxCooking* cooking);
	~PhysicsHelper(void);

	physx::PxRigidStatic* createDrivablePlane(physx::PxMaterial* material);

	physx::PxConvexMesh* createConvexMesh(const physx::PxVec3* verts, const physx::PxU32 numVerts);

	physx::PxTriangleMesh* createTriangleMesh(const physx::PxVec3* verts, const physx::PxU32 numVerts, const physx::PxU32* faces, const physx::PxU32 numFaces);

private:
	physx::PxPhysics* physics;
	physx::PxCooking* cooking;
};

