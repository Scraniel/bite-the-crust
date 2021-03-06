#pragma once
#include "RenderingEngine.h"
#include "PhysicsEngine.h"
#include "InputEngine.h"
#include "AIEngine.h"
#include "AudioEngine.h"
#include "Renderable.h"
#include "PhysicsEntityInfo.h"
#include "DeliveryManager.h"
#include "MenuLogic.h"
#include "Map.h"
#include "GameState.h"

#include <SDL.h>
#include <glew.h>
#include <map>
#include <string>
#include <random>

class Game :
	public sigslot::has_slots<>
{
public:
	Game(void);
	~Game(void);
	
	AIEngine *aiEngine;
	AudioEngine *audioEngine;
	InputEngine *inputEngine;
	PhysicsEngine *physicsEngine;
	RenderingEngine *renderingEngine;
	DeliveryManager* deliveryManager;
	MenuLogic* menuLogic;
	static const int MAX_PLAYERS = 4;
	static const int DEF_WINDOW_WIDTH = 1280;
	static const int DEF_WINDOW_HEIGHT = 720;

	void run();

private:
	static const unsigned int PHYSICS_STEP_MS = 16;

	void startGame();
	void setGameState(GameState state);
	void initSystems();
	void loadJSONfiles();
	void reset();

	Tile* setupTile(int i, int j);
	void setupRegularEntity(std::string name, Tile* tile, glm::vec3 pos);
	void setupPhysicsEntity(std::string name, Tile* tile, TileEntity tileEntity, glm::vec3 pos);
	void setupEntities();
	void setupVehicle(Vehicle* vehicle, physx::PxTransform transform, int num);

	void connectSystems();
	void processSDLEvents();
	void quitGame();
	void endGame(std::map<Vehicle*, int> scores);
	void shootPizza(Vehicle* vehicle);
	void enableSpaceShips();

	void gameDisplay(int player);
	void splitscreenViewports();
	void playHUD(int player);
	void endHUD();
	void playLoop();
	void menuLoop();
	void pauseLoop();
	void endLoop();
	void mainLoop();

	GameState gameState;
	int numHumans;

	unsigned int oldTimeMs;
	unsigned int newTimeMs;
	unsigned int deltaTimeMs;
	unsigned int deltaTimeAccMs;
	unsigned int outOfGameTimeAccMs;

	bool isFullscreen;
	bool isVSync;
	SDL_GLContext glContext;
	SDL_Window* window;
	int windowWidth;
	int windowHeight;
	int displayWidth;
	int displayHeight;

	void toggleVSync();
	void toggleFullscreen();

	Camera* camera[MAX_PLAYERS];
	Vehicle* players[MAX_PLAYERS];
	std::map<Vehicle*, int> scores;
	bool spaceMode;

	PhysicsEntityInfo* pizzaInfo;
	std::vector<Entity*> entities;
	std::map<std::string, Renderable*> renderablesMap;
	std::map<std::string, PhysicsEntityInfo*> physicsEntityInfoMap;
	std::map<std::string, GLuint> textureMap;
	Map map;

	std::mt19937 generator;
};

