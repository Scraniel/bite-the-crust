#include "Game.h"
#include "DrivingInput.h"
#include "DynamicEntity.h"

using namespace std;

void fatalError(string errorString)
{
	cout << errorString << endl;
	cout << "Enter any key to quit...";
	int tmp;
	cin >> tmp;
	SDL_Quit();
	exit(1);
}

Game::Game(void)
{
	window = nullptr;
	screenWidth = 1024;		//pro csgo resolution
	screenHeight = 768;
	gameState = GameState::PLAY;
	renderingEngine = nullptr;
	physicsEngine = nullptr;
	inputEngine = nullptr;
	aiEngine = nullptr;
	audioEngine = nullptr;
	screen = nullptr;
	p1Vehicle = nullptr;
}

// The entry point of the game
void Game::run()
{
	// Preload data, initialize subsystems, anything to do before entering the main loop
	initSystems();
	setupEntities();
	connectSignals();

	mainLoop();
}

void Game::initSystems()
{
	SDL_Init(SDL_INIT_EVERYTHING);		//Initialize SDL
	window = SDL_CreateWindow("Another One Bites The Crust", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, SDL_WINDOW_OPENGL);

	if(window == nullptr)
	{
		fatalError("SDL Window could not be created");
	}

	SDL_GLContext glContext = SDL_GL_CreateContext(window);
	if(glContext == nullptr)
	{
		fatalError("SDL_GL context could not be created");
	}

	glewExperimental = GL_TRUE;
	GLenum error = glewInit();
	if(error != GLEW_OK)
	{
		fatalError("Could not init GLEW");
	}
	
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);		//enable double buffering
	if( SDL_GL_SetSwapInterval( 1 ) < 0 )
	{
		printf( "Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
	}

	glClearColor(0.2f, 0.2f, 0.5f, 1.0f);				//blue background

	aiEngine = new AIEngine();
	audioEngine = new AudioEngine();
	inputEngine = new InputEngine();
	physicsEngine = new PhysicsEngine();
	renderingEngine = new RenderingEngine();
}

void Game::setupEntities()
{
	ContentLoading::loadRenderables("res\\JSON\\renderables.json", renderablesMap);
	// Assign the buffers for all the renderables
	std::map<std::string, Renderable*>::iterator it;
	for (it = renderablesMap.begin(); it != renderablesMap.end(); ++it) {
		renderingEngine->assignBuffers(it->second);
	}
	// Set up the colours, since we don't have textures yet
	renderablesMap["floor2"]->setColor(glm::vec3(1,1,0));
	renderablesMap["box"]->setColor(glm::vec3(0,1,1));
	renderablesMap["van"]->setColor(glm::vec3(1,0,0));

	Entity* ground = new Entity();
	ground->setRenderable(renderablesMap["floor"]);
	entities.push_back(ground);

	Entity* ground2 = new Entity();
	ground2->setRenderable(renderablesMap["floor2"]);
	ground2->setDefaultTranslation(glm::vec3(70.0f,0.0f,0.0f));
	entities.push_back(ground2);

	Entity* ground3 = new Entity();
	ground3->setRenderable(renderablesMap["floor"]);
	ground3->setDefaultTranslation(glm::vec3(70.0f,0.0f,70.0f));
	entities.push_back(ground3);

	Entity* ground4 = new Entity();
	ground4->setRenderable(renderablesMap["floor2"]);
	ground4->setDefaultTranslation(glm::vec3(0.0f,0.0f,70.0f));
	entities.push_back(ground4);

	DynamicEntity* cube = new DynamicEntity();
	cube->setRenderable(renderablesMap["cube"]);
	physicsEngine->createDynamicEntity(cube, glm::vec3(10,0,5), glm::vec3(0,0,0));
	entities.push_back(cube);

	/*DynamicEntity* barrier = new DynamicEntity();
	barrier->setRenderable(renderablesMap["barrier"]);
	physicsEngine->createDynamicEntity(barrier, glm::vec3(-10,0,15), glm::vec3(0,0,0));
	entities.push_back(barrier);*/

	/**********************************************************
						Creating Vechicles
	**********************************************************/
	p1Vehicle = new Vehicle();
	ContentLoading::loadVehicleData("res\\JSON\\car.json", p1Vehicle);
	p1Vehicle->setRenderable(renderablesMap["van"]);
	p1Vehicle->setDefaultRotation(-1.5708f, glm::vec3(0,1,0));
	p1Vehicle->setDefaultTranslation(renderablesMap["van"]->getCenter());

	// TODO get dimensions working properly for vehicle
	p1Vehicle->chassisDims = physx::PxVec3(2, 2, 5);
	physicsEngine->createVehicle(p1Vehicle);
	entities.push_back(p1Vehicle);

	//// Player 2 (ie. AI)
	p2Vehicle = new Vehicle();
	ContentLoading::loadVehicleData("res\\JSON\\car.json", p2Vehicle);
	p2Vehicle->setRenderable(renderablesMap["van"]);
	p2Vehicle->setDefaultRotation(-1.5708f, glm::vec3(0,1,0));
	p2Vehicle->setDefaultTranslation(renderablesMap["van"]->getCenter());

	// TODO get dimensions working properly for vehicle
	p2Vehicle->chassisDims = physx::PxVec3(2, 2, 5);
	physicsEngine->createVehicle(p2Vehicle);
	p2Vehicle->setPosition(glm::vec3(10, 3, 0));
	entities.push_back(p2Vehicle);

	for (unsigned int i = 0; i < CAMERA_POS_BUFFER_SIZE; i++)
	{
		cameraPosBuffer[i] = p1Vehicle->getPosition() + glm::vec3(p1Vehicle->getModelMatrix() * glm::vec4(0,8,-15,0));
	}
	cameraPosBufferIndex = 0;
	camera.setUpVector(glm::vec3(0,1,0));
}

// TODO decide how signals will be used and set them up
void Game::connectSignals()
{
	//inputEngine->DrivingSignal.connect(p1Vehicle, &Vehicle::handleInput);
	inputEngine->FireSignal.connect(this, &Game::firePizza);
}

void Game::mainLoop()
{
	unsigned int oldTimeMs = SDL_GetTicks();
	
	// Game loop
	while (gameState!= GameState::EXIT)
	{
		// Update the player and AI cars

		processSDLEvents();
		p1Vehicle->handleInput(inputEngine->getInput());
		p2Vehicle->handleInput(&aiEngine->updateAI(p2Vehicle));

		// Figure out timestep and run physics
		unsigned int newTimeMs = SDL_GetTicks();
		unsigned int deltaTimeMs = newTimeMs - oldTimeMs;
		oldTimeMs = newTimeMs;
		bool didPhysics = physicsEngine->simulate(deltaTimeMs);

		// If a physics simulation ran, update the camera position buffer with new location
		if (didPhysics)
		{
			cameraPosBuffer[cameraPosBufferIndex] = p1Vehicle->getPosition() + glm::vec3(p1Vehicle->getModelMatrix() * glm::vec4(0,8,-15,0));
			cameraPosBufferIndex = (cameraPosBufferIndex + 1) % CAMERA_POS_BUFFER_SIZE;
		}
		// Set camera to look at player with a positional delay
		camera.setPosition(cameraPosBuffer[cameraPosBufferIndex]);
		camera.setLookAtPosition(p1Vehicle->getPosition());
		renderingEngine->updateView(camera);

		//display
		renderingEngine->displayFunc(entities);

		//swap buffers
		SDL_GL_SwapWindow(window);
		physicsEngine->fetchSimulationResults();
	}
}

void Game::processSDLEvents()
{
	SDL_Event event;
    while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT)
		{
			gameState = GameState::EXIT;
		}
		else if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
		{
			gameState = GameState::EXIT;
		}
		else if (event.type == SDL_CONTROLLERAXISMOTION || event.type == SDL_CONTROLLERBUTTONDOWN || SDL_CONTROLLERBUTTONUP ||
			     event.type == SDL_CONTROLLERDEVICEREMOVED || event.type == SDL_CONTROLLERDEVICEADDED)
		{
			inputEngine->processControllerEvent(event);
		}
		else
		{
			// other events, do nothing yet
		}
	}
}

// TODO possible move this to a different file and make it work for different players
void Game::firePizza()
{
	DynamicEntity* pizzaBox = new DynamicEntity();
	pizzaBox->setRenderable(renderablesMap["box"]);
	glm::vec3 position = p1Vehicle->getPosition() + glm::vec3(p1Vehicle->getModelMatrix() * glm::vec4(0, 1.0, 1.0, 0));
	glm::vec3 velocity = glm::vec3(p1Vehicle->getModelMatrix() * glm::vec4(0.0, 0.0, 20.0, 0.0));
	physx::PxVec3 v = p1Vehicle->getDynamicActor()->getLinearVelocity();
	velocity = velocity + glm::vec3(v.x, v.y, v.z);
	physicsEngine->createDynamicEntity(pizzaBox, position, velocity);
	entities.push_back(pizzaBox);
}

Game::~Game(void)
{
	for (unsigned int i = 0; i < entities.size(); i++)
	{
		delete entities[i];
	}
	std::map<std::string, Renderable*>::iterator it;
	for (it = renderablesMap.begin(); it != renderablesMap.end(); ++it) {
		delete it->second;
	}
}
