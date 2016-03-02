#pragma once
#include "Vehicle.h"
#include <iostream>
#include <filereadstream.h>
#include <document.h>
#include <map>
#include <vector>
#include "glm.hpp"
#include <string>
#include "Map.h"
#include "PhysicsEngine.h"

namespace ContentLoading
{
	Renderable* createRenderable(std::string modelFile);
	PhysicsEntityInfo* createPhysicsInfo(const char* filename, Renderable* model);

	bool loadVehicleData(char* filename, Vehicle* vehicle);
	bool verifyEntityList(const rapidjson::Document &d);
	bool loadEntityList(char* filename, std::map<std::string, Renderable*> &modelMap, std::map<std::string, PhysicsEntityInfo*> &physicsMap);
	bool loadRenderables(char* filename, std::map<std::string, Renderable*> &map);
	bool validateMap(rapidjson::Document &d);
	bool loadMap(char* filename, Map &map);

	bool loadOBJNonIndexed(
		const char * path, 
		std::vector<glm::vec3> & out_vertices, 
		//std::vector<glm::vec2> & out_uvs,
		std::vector<glm::vec3> & out_normals
	);
	bool loadOBJ(
		const char * path, 
		std::vector<glm::vec3> & out_vertices, 
		std::vector<glm::vec2> & out_uvs,
		std::vector<glm::vec3> & out_normals
	);
	GLuint loadDDS(const char * imagepath);
};

