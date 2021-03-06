#include "AIEngine.h"
#include <iostream>
#include <stdlib.h>

const int MIN_DIST = 5;
const int MAX_ATTEMPTS = 120;
const float MIN_ANGLE = 0.05f;
const float PIZZA_SHOOT_DISTANCE = 25;

AIEngine::AIEngine(void)
{
	srand((int)time(0));
}


inline bool equals(glm::vec3 x, glm::vec3 y)
{
	return x.x == y.x && x.y == y.y && x.z == y.z;
}

std::vector<glm::vec3> AIEngine::aStar(graphNode * start, graphNode * destination, vector<graphNode *> allNodes, glm::vec3 posDestination = glm::vec3(0,0,0))
 {
	std::map<graphNode *, double> distances;
	std::map<graphNode *, double> heuristic;
	std::map<graphNode *, graphNode *> previous;
	std::set<graphNode *> toVisit, visited;
	bool travelingToPoint = false;

	if(equals(posDestination, glm::vec3(0,0,0)))
		posDestination = destination->getPosition();
	else
		travelingToPoint = true;


	toVisit.insert(start);
	for(graphNode * n : allNodes)
	{
		distances[n] = DBL_MAX;
		heuristic[n] = DBL_MAX;
	}

	
	distances[start] = 0;
	heuristic[start] = glm::length(start->getPosition() - posDestination);

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
				minDist = heuristic[*i];
			}
		}
		
		graphNode * current = *position;
		toVisit.erase(position);
		visited.insert(current);

		if(glm::length(current->getPosition() - posDestination) < MIN_DIST * 5 && travelingToPoint)
			destination = current;

		if(current == destination)
			break;

		for(graphNode * neighbour : current->getNeighbours())
		{
			//// If the node has been visited, don't check
			if(visited.find(neighbour) != visited.end())
				continue;

			double distance = 0;

			distance += distances[current] + glm::length(neighbour->getPosition() - current->getPosition());

			if(toVisit.find(neighbour) == toVisit.end())
				toVisit.insert(neighbour);
			else if(distance >= distances[neighbour])
				continue;

			previous[neighbour] = current;
			distances[neighbour] = distance;
			heuristic[neighbour] = distances[neighbour] + glm::length(neighbour->getPosition() - posDestination);
			
		}
	}
	// Constructing path
	std::vector<glm::vec3> path;
	graphNode * current = destination;
	while(current != nullptr)
	{	
		path.push_back(current->getPosition() + glm::vec3(0,1,0));
		current = previous[current];
	}
	std::reverse(path.begin(), path.end());

	if(travelingToPoint)
		path.push_back(posDestination);

	return path; 
}

// TODO: add logic for firing pizzas
void AIEngine::goToPoint(Vehicle* driver, const glm::vec3 & desiredPos, const float & distanceToGoal)
{
	VehicleInput* input = &driver->input;
	
	glm::vec3 desiredDirection = desiredPos - driver->getPosition();
	desiredDirection = glm::normalize(desiredDirection);
	glm::vec3 forward(glm::normalize(driver->getModelMatrix() * glm::vec4(0,0,1,0)));
	glm::vec3 left(glm::normalize(driver->getModelMatrix() * glm::vec4(1,0,0,0)));
	float cosAngle = glm::dot(desiredDirection, forward);
	float leftCosAngle = glm::dot(desiredDirection, left);

	float ratio = glm::acos(cosAngle) / glm::pi<float>();

	// TODO: Should divide by half the map or something
	float gas = glm::clamp(distanceToGoal / 100, 0.3f, 0.65f);

	// TODO: Maybe put these in an init file for tuning purposes?
	if (gas > 0.15)
	{
		input->forward = gas;
		input->backward = 0;
	}
	else
		brake(driver, 1 -gas);
	
	input->handBrake = false;

	if(ratio > MIN_ANGLE)
	{
		if(leftCosAngle > 0)
			input->steer = glm::clamp(ratio*2.5f, 0.f, 1.f);
		else
			input->steer = -1 * glm::clamp(ratio*2.5f, 0.f, 1.f);;
	}
	else if(distanceToGoal < MIN_DIST)
	{
		input->steer = 0;
		/*if(!driver->pizzaDelivered)
		{
			input->shootPizza = true;
			driver->pizzaDelivered = true;
		}*/
	}
}

inline bool sphereIntersect(const glm::vec3& raydir, const glm::vec3& rayorig, const glm::vec3& pos, const float& radius)
{
	glm::vec3 posToRay = rayorig - pos;

	float radius2 = radius * radius;
	float a = glm::dot(raydir,raydir);
	float b = glm::dot(raydir,  (2.0f * posToRay));
	float c = glm::dot(posToRay, posToRay) - radius2;
	float D = b * b - (4.0f * a * c);

	// If ray can not intersect then stop
	if (D < 0)
			return false;

	return true;
}

float AIEngine::angleToGoal(Vehicle * driver, const glm::vec3 & goal)
{
	glm::vec3 desiredDirection = glm::normalize(goal - driver->getPosition());
	glm::vec3 left(glm::normalize(driver->getModelMatrix() * glm::vec4(1,0,0,0)));
	glm::vec3 forward(glm::normalize(driver->getModelMatrix() * glm::vec4(0,0,1,0)));
	float leftCosAngle = glm::dot(desiredDirection, left);
	float cosAngle = glm::dot(desiredDirection, forward);

	float ratio = glm::acos(cosAngle) / glm::pi<float>();

	return ratio;
}

void AIEngine::shootPizza(Vehicle * driver)
{
	driver->pizzaDelivered = true;
	driver->input.shootPizza = true;
	brake(driver, 1);
}

void AIEngine::fireAt(Vehicle * driver, const glm::vec3 & goal)
{
	float ratio = angleToGoal(driver, goal);

	if(driver->pizzaDelivered == false && ratio < MIN_ANGLE)
		shootPizza(driver);


	if(driver->getPhysicsVehicle()->computeForwardSpeed() > 0.2)
		brake(driver, 1);
	else
		facePoint(driver, goal);
	
}

void AIEngine::facePoint(Vehicle * driver, const glm::vec3 & pointTo, bool backwards)
{
	glm::vec3 desiredDirection = glm::normalize(pointTo - driver->getPosition());
	glm::vec3 left(glm::normalize(driver->getModelMatrix() * glm::vec4(1,0,0,0)));
	float leftCosAngle = glm::dot(desiredDirection, left);

	if(backwards)
	{
		driver->input.backward = 1.0;
		driver->input.forward = 0.0;

		if(leftCosAngle > 0)
			driver->input.steer = -1.0;
		else	
			driver->input.steer = 1.0;
	}
	else
	{
		driver->input.backward = 1.0;
		driver->input.forward = 0.0;

		if(leftCosAngle > 0)
			driver->input.steer = -1.0;
		else	
			driver->input.steer = 1.0;
	}
}

graphNode * findClosestNode(glm::vec3 position, Map & map)
{
	Tile * currentTile = map.getTile(position);
	graphNode * closest;
	std::vector<graphNode*> toSearch;

	if(currentTile->nodes.empty())
		toSearch = map.allNodes;
	else
		toSearch = currentTile->nodes;

	closest = toSearch.at(0);
	if(!closest)
		return closest;
	double minDist = DBL_MAX;
	for(graphNode * n : toSearch)
	{
		double currentDist = glm::length(position - n->getPosition());
		if(currentDist < minDist)
		{
			closest = n;
			minDist = currentDist;
		}
	}

	return closest;
}

void AIEngine::updatePath(Vehicle* toUpdate, Delivery destination, Map & map)
{
	// Should be 'goal node' of this tile
	glm::vec3 destinationNode = destination.location->goal;

	
	if(glm::length(destinationNode - toUpdate->getPosition()) <= MIN_DIST)
		toUpdate->currentPath.push_back(destinationNode);
	else
	{
		graphNode * closest = findClosestNode(toUpdate->getPosition(), map);

		toUpdate->currentPath = aStar(closest, nullptr, map.allNodes, destinationNode);
	}
}

void AIEngine::brake(Vehicle* toUpdate, const float & amount)
{
	float speed = toUpdate->getPhysicsVehicle()->computeForwardSpeed();

	if(speed > 0)
			toUpdate->input.backward = amount;
		else
			toUpdate->input.backward = 0;
		
		toUpdate->input.forward = 0;
}

void AIEngine::trimPath(Vehicle* toUpdate)
{
	if(toUpdate->currentPath.size() < 2)
		return;


	int closestIndex;
	double minDist = DBL_MAX;
	for(unsigned int i = 0; i < toUpdate->currentPath.size(); i++)
	{
		double currentDistance = glm::length(toUpdate->currentPath[i] - toUpdate->getPosition());

		if( currentDistance < minDist)
		{
			closestIndex = i;
			minDist = currentDistance;
		}
	}

	for(int i = 0; i < closestIndex; i++)
		toUpdate->currentPath.erase(toUpdate->currentPath.begin());
}

bool AIEngine::isStuck(Vehicle * driver)
{
	float sigma = 0.1f;

	if((driver->getPhysicsVehicle()->computeForwardSpeed() < sigma && driver->input.forward > 0) || 
		(driver->getPhysicsVehicle()->computeForwardSpeed() > -sigma && driver->input.backward < 0))
		driver->stuckDuration++;
	else
		driver->stuckDuration = 0;

	return driver->stuckDuration > MAX_ATTEMPTS*2;
}

void AIEngine::refillPizzas(Vehicle* toUpdate, Map & map)
{

	if(!toUpdate->pickingUp)
	{
		toUpdate->pickingUp = true;
		std::vector<glm::vec3> pathToPickup = aStar(findClosestNode(toUpdate->getPosition(), map), nullptr, map.allNodes, map.pickup);
		toUpdate->currentPath = pathToPickup;
	}
}

void AIEngine::avoid(Vehicle * toUpdate, const glm::vec3 & goal)
{
	toUpdate->avoidAttempts++;
	if(toUpdate->avoidAttempts > MAX_ATTEMPTS)
	{
		toUpdate->avoiding = false;
		toUpdate->avoidForward = false;
		toUpdate->avoidAttempts = 0;
	}
	if(toUpdate->currentPath.size() > 0)
		facePoint(toUpdate, toUpdate->currentPath.front(), toUpdate->avoidForward);
	else
		facePoint(toUpdate, goal, toUpdate->avoidForward);
}

void AIEngine::backToDestination(Vehicle * toUpdate, Map * map, Delivery destination)
{
	toUpdate->pickingUp = false;
	updatePath(toUpdate, destination, *map);	
}

void AIEngine::updateAI(Vehicle* toUpdate, Delivery destination, Map & map, AICollisionEntity & obstacle) 
{
	toUpdate->input.shootPizza = false;
	glm::vec3 goal = destination.location->goal;

	if(toUpdate->avoiding)
	{
		avoid(toUpdate, goal);
		return;
	}


	// Should pathfind to pickup followed by pathfinding to destination
	if(toUpdate->pizzaCount == 0)
	{	
		goal = map.pickup;
		refillPizzas(toUpdate, map);
	}
	if(toUpdate->currentPath.empty())
	{
		updatePath(toUpdate, destination, map);
		if(toUpdate->currentPath.empty())
			return;
	}
	else
		trimPath(toUpdate);
	
	// Should be goal node
	if(toUpdate->newDestination)
	{
		toUpdate->currentPath.clear();
		toUpdate->pizzaDelivered = false;
		toUpdate->newDestination = false;
		toUpdate->pickingUp = false;
		return;
	}

	glm::vec3 nextPoint;

	if(toUpdate->currentPath.size() >= 5)
		nextPoint = toUpdate->currentPath.at(4);
	else if(toUpdate->currentPath.size() >= 3)
		nextPoint = toUpdate->currentPath.at(2);
	else
		nextPoint = toUpdate->currentPath.at(0);

	// May want to consider something more efficient, uses square root in here
	float distanceToNext = glm::length(nextPoint - toUpdate->getPosition());
	float distanceToGoal = glm::length(toUpdate->getPosition() - goal);

	if(distanceToGoal < MIN_DIST * 5 && toUpdate->pizzaCount != 0)
	{
		fireAt(toUpdate, destination.location->goal);
		return;
	}
	else if(distanceToGoal < toUpdate->getPhysicsVehicle()->computeForwardSpeed() + PIZZA_SHOOT_DISTANCE && angleToGoal(toUpdate, goal) < MIN_ANGLE && toUpdate->pizzaDelivered == false)
	{
		shootPizza(toUpdate);
		return;
	}

	if(distanceToNext < MIN_DIST)
	{
		//std::cout << "Waypoint get! Position: "<< toUpdate->currentPath.at(0).x << "," << toUpdate->currentPath.at(0).y << ", " << toUpdate->currentPath.at(0).z << std::endl;
		toUpdate->currentPath.erase(toUpdate->currentPath.begin());

		if(toUpdate->currentPath.empty() && distanceToNext < MIN_DIST * 5)
		{
			brake(toUpdate, 1);
			return;
		} 
	}

	Tile * currentTile = map.getTile(toUpdate->getPosition());
	// Should be 'goal node' of this tile
	if((obstacle.entity != nullptr && obstacle.distance < 1 && obstacle.entity->type == EntityType::STATIC) || isStuck(toUpdate))
	{
		toUpdate->avoiding = true;
		if(toUpdate->input.backward > 0)
			toUpdate->avoidForward = true;
	}

	goToPoint(toUpdate, nextPoint, glm::length(goal - toUpdate->getPosition()));
}

AIEngine::~AIEngine(void)
{
}
