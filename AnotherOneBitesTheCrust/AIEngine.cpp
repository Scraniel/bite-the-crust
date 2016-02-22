#include "AIEngine.h"
#include <iostream>
#include <stdlib.h>

AIEngine::AIEngine(void)
{
	srand((int)time(0));
}

void AIEngine::goToPoint(Vehicle* driver, glm::vec3 desiredPos)
{
	VehicleInput* input = driver->getInputStruct();
	input->forward = 1.0;
	input->backward = 0.0;
	input->handBrake = false;

	// Pizza shooting proof of concept
	int pizzaRand = rand() % 100;
	if (pizzaRand == 0)
	{
		input->shootPizza = true;
	}
	
	glm::vec3 desiredDirection = glm::normalize(desiredPos - driver->getPosition());
	glm::vec3 forward(glm::normalize(driver->getModelMatrix() * glm::vec4(0,0,1,0)));

	float ratio = glm::acos(glm::dot(desiredDirection, forward)) / glm::pi<float>();

	if(ratio > 0.1)
	{
		if(desiredPos.x < driver->getPosition().x)
		{
			input->rightSteer = ratio;
			input->leftSteer = 0;
		}
		else
		{
			input->leftSteer = ratio;
			input->rightSteer = 0;
		}
	}
	else
	{
		input->rightSteer = 0;
		input->leftSteer = 0;
	}
}

void AIEngine::updatePath(Vehicle* toUpdate)
{
	toUpdate->currentPath.push_back(glm::vec3(-Map::MAP_SIZE/2, 0, Map::MAP_SIZE/2));
	toUpdate->currentPath.push_back(glm::vec3(Map::MAP_SIZE/2, 0, Map::MAP_SIZE/2));
	toUpdate->currentPath.push_back(glm::vec3(-Map::MAP_SIZE/2, 0, -Map::MAP_SIZE/2));
	toUpdate->currentPath.push_back(glm::vec3(Map::MAP_SIZE/2, 0, -Map::MAP_SIZE/2));
}

void AIEngine::updateAI(Vehicle* toUpdate) 
{ 
	if(toUpdate->currentPath.empty())
	{
		updatePath(toUpdate);
	}

	// May want to consider something more efficient, uses square root in here
	float distanceToNext = glm::length(toUpdate->currentPath.at(0) - toUpdate->getPosition());

	if(distanceToNext < 10)
	{
		toUpdate->currentPath.erase(toUpdate->currentPath.begin());
		std::cout << "Waypoint get!" << std::endl;
		if(toUpdate->currentPath.empty())
		{
			// return DrivingInput();
			return;
		} 
	}
	goToPoint(toUpdate, toUpdate->currentPath.at(0));
}

AIEngine::~AIEngine(void)
{
}
