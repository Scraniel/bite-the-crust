#pragma once
#include <glm.hpp>
#include <vector>
#include "PhysicsEntity.h"
#include "graphNode.h"

// Representation of the entities in a tile, which will be turned into actual entities by the game engine
// I think we'll want to add some kind of 'type' field to distinguish between static and dynamic entities later
struct TileEntity 
{
	std::vector<std::string> names; // Multiple because it can be a random choice between multiple entities
	glm::vec3 position;
	float lowerRotation, upperRotation;
};

struct Tile
{
	// Should these coordinates be relative to the tile or the map?
	// If it's relative to the tile, need a method to get player space -> tile space
	glm::vec3 entrance, exit;
	std::string groundModel;
	std::vector<TileEntity> entityTemplates;
	std::vector<PhysicsEntity*> staticEntities;
	std::vector<graphNode*> nodes;
	glm::vec3 goal;
	Entity* ground;
	GLuint groundTexture;
	int groundRotationDeg;
	bool deliverable;
	Entity* house; // null if the tile doesn't have a house
	bool pickup;

	bool minimap;

	Tile() {
		house = nullptr;
		ground = nullptr;
	}
};

class Map
{
public:
	Map(void);
	~Map(void);

	int depth, width, tileSize;

	std::vector<graphNode *> allNodes;
	std::vector<std::vector<Tile>> tiles;
	std::vector<Tile*> deliveryTiles;
	glm::vec3 pickup;

	Tile* getTile(glm::vec3 position);
	glm::vec3 getTileLocation(Tile *tile);
	//void offsetPosition(Tile* tile, glm::vec3 &position);
};

