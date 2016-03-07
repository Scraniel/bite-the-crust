#include "AIEngine.h"
#include <iostream>
#include <stdlib.h>

const int MIN_DIST = 15;

AIEngine::AIEngine(void)
{
	srand((int)time(0));
}

std::vector<glm::vec3> AIEngine::dijkstras(graphNode * start, graphNode * destination, vector<graphNode *> allNodes)
{
	std::map<graphNode *, double> distances;
	std::map<graphNode *, double> heuristic;
	std::map<graphNode *, graphNode *> previous;
	std::set<graphNode *> toVisit;

	toVisit.insert(start);
	for(graphNode * n : allNodes)
	{
		distances[n] = DBL_MAX;
		heuristic[n] = DBL_MAX;
	}

	
	distances[start] = 0;
	heuristic[start] = glm::length(start->getPosition() - destination->getPosition());

	previous[start] = nullptr;

	while(!toVisit.empty())
	{

		std::set<graphNode*>::iterator position = toVisit.begin();
		double minDist = DBL_MAX;
		for(std::set<graphNode*>::iterator i = toVisit.begin(); i != toVisit.end(); i++)
		{
			if(heuristic[*i] < minDist)
			{
				position = i;
				minDist = distances[*i];
			}
		}
		
		graphNode * current = *position;
		toVisit.erase(position);
		if(current == destination)
			break;

		for(graphNode * neighbour : current->getNeighbours())
		{
			// If the node has been visited, don't check
			if(distances[neighbour] != DBL_MAX)
				continue;

			double distance = 0;

			distance += distances[current] + glm::length(neighbour->getPosition() - current->getPosition());

			if(toVisit.find(neighbour) == toVisit.end())
				toVisit.insert(neighbour);
			else if(distance >= distances[neighbour])
				continue;

			previous[neighbour] = current;
			distances[neighbour] = distance;
			heuristic[neighbour] = distances[neighbour] + glm::length(neighbour->getPosition() - destination->getPosition());
			
		}
	}

	std::vector<glm::vec3> path;
	graphNode * current = destination;
	while(current != nullptr)
	{	
		path.push_back(current->getPosition());
		current = previous[current];
	}
	std::reverse(path.begin(), path.end());

	return path; 
}

// TODO: add logic for firing pizzas
void AIEngine::goToPoint(Vehicle* driver, const glm::vec3 & desiredPos)
{
	VehicleInput* input = &driver->input;
	
	glm::vec3 desiredDirection = desiredPos - driver->getPosition();
	float distance = glm::length(desiredDirection);
	desiredDirection = glm::normalize(desiredDirection);
	glm::vec3 forward(glm::normalize(driver->getModelMatrix() * glm::vec4(0,0,1,0)));
	glm::vec3 left(glm::normalize(driver->getModelMatrix() * glm::vec4(1,0,0,0)));
	float cosAngle = glm::dot(desiredDirection, forward);
	float leftCosAngle = glm::dot(desiredDirection, left);

	float speed = driver->getPhysicsVehicle()->computeForwardSpeed();
	float ratio = glm::acos(cosAngle) / glm::pi<float>();

	if (distance > speed)
	{
		input->forward = 1.0;
		input->backward = 0.0;
		input->handBrake = false;
	}
	else
	{
		input->forward = 0.0;
		input->backward = 1.0;
		input->handBrake = false;
	}
	std::cout << "d = " << distance << " s = " << speed << " backward = " << input->backward << std::endl;
	//std::cout << input->backward << std::endl;

	/*ratio *= 2;
	if (ratio > 1)
	{
		ratio = 1;
	}
	if(leftCosAngle > 0)
	{
		input->steer = ratio;
	}
	else
	{
		input->steer = -ratio;
	}*/

	if(ratio > 0.1)
	{
		if(leftCosAngle > 0)
		{
			input->steer = ratio;
		}
		else
		{
			input->steer = -ratio;
		}
	}
	else
	{
		input->steer = 0;
		if (rand()%100 < 5)
		{
			input->shootPizza = true;
		}
	}
}

void AIEngine::updatePath(Vehicle* toUpdate, Delivery destination, Map & map)
{

	Tile * currentTile = map.getTile(toUpdate->getPosition());

	// Find closest node
	graphNode * closest = currentTile->nodes.at(0);
	if(!closest)
		return;
	double minDist = -1;
	for(graphNode * n : currentTile->nodes)
	{
		double currentDist = glm::length(toUpdate->getPosition() - closest->getPosition());
		if(currentDist < minDist)
		{
			closest = n;
			minDist = currentDist;
		}
	}


	// Should be 'goal node' of this tile
	graphNode * destinationNode = destination.location->nodes.at(0);
	if(glm::length(destinationNode->getPosition() - toUpdate->getPosition()) <= MIN_DIST)
		toUpdate->currentPath.push_back(destinationNode->getPosition());
	else
	{
		// Uncomment this to use dijkstras. Right now it's a little buggy and needs some work,
		// heading straight to the destination works better for now. I believe re-running
		// dijkstras each frame would fix the issue (right now I'm uncertain whether this would
		// cause too much overhead)
		//
		toUpdate->currentPath = dijkstras(closest, destinationNode, map.allNodes);
		//toUpdate->currentPath.push_back(destinationNode->getPosition());
	}
}

inline bool equals(glm::vec3 x, glm::vec3 y)
{
	return x.x == y.x && x.y == y.y && x.z == y.z;
}

void AIEngine::updateAI(Vehicle* toUpdate, Delivery destination, Map & map) 
{ 
	
	if(toUpdate->currentPath.empty())
	{
		updatePath(toUpdate, destination, map);
		if(toUpdate->currentPath.empty())
			return;
	}

	// Should be goal node
	if(!equals(toUpdate->getDestination(), destination.location->nodes.at(0)->getPosition()))
	{
		toUpdate->currentPath.clear();
		return;
	}

	// May want to consider something more efficient, uses square root in here
	float distanceToNext = glm::length(toUpdate->currentPath.at(0) - toUpdate->getPosition());

	if(distanceToNext < MIN_DIST)
	{
		//std::cout << "Waypoint get! Position: "<< toUpdate->currentPath.at(0).x << "," << toUpdate->currentPath.at(0).y << ", " << toUpdate->currentPath.at(0).z << std::endl;
		toUpdate->currentPath.erase(toUpdate->currentPath.begin());

		if(toUpdate->currentPath.empty())
		{
			toUpdate->input.forward = 0;
			toUpdate->input.backward = 0;
			toUpdate->input.handBrake = true;
			return;
		} 
	}
	goToPoint(toUpdate, toUpdate->currentPath.at(0));
}

AIEngine::~AIEngine(void)
{
}
