#define _USE_MATH_DEFINES

#include <math.h>
#include <cstdlib>
#include <cstddef>
#include <cmath>
#include <iostream>
#include <ios>
#include <list>
#include <random>

#include <OpenSG/OSGGLUT.h>
#include <OpenSG/OSGConfig.h>
#include <OpenSG/OSGSimpleGeometry.h>
#include <OpenSG/OSGGLUTWindow.h>
#include <OpenSG/OSGMultiDisplayWindow.h>
#include <OpenSG/OSGSceneFileHandler.h>
#include <OpenSG/OSGThreadManager.h>

#include <OSGCSM/OSGCAVESceneManager.h>
#include <OSGCSM/OSGCAVEConfig.h>
#include <OSGCSM/appctrl.h>

#include <vrpn_Tracker.h>
#include <vrpn_Button.h>
#include <vrpn_Analog.h>

#include "SimpleEntity.h" 
#include "BulletEntity.h"
#include "PlayerEntity.h"
#include "ColorBlockEntity.h"

#include "PhysicCollision.h"
#include "LevelFunctions.h"
#include "Level.h"

OSG_USING_NAMESPACE

OSGCSM::CAVEConfig cfg;
OSGCSM::CAVESceneManager *mgr = nullptr;
vrpn_Tracker_Remote* tracker = nullptr;
vrpn_Button_Remote* button = nullptr;
vrpn_Analog_Remote* analog = nullptr;

int lastTime = 0;
int frameTime = 0;



void print_tracker();

// Level
Level* level;
//LevelFunctions* levelFunc = new LevelFunctions();
bool gameRunning = true;

// Geometry etc..
NodeRefPtr scene = nullptr;
NodeRecPtr root = nullptr;
NodeRecPtr notTargets = nullptr;

std::list<ColorBlockEntity *>* colorBlocks = new std::list<ColorBlockEntity *>();
std::list<SimpleEntity *>* targets = new std::list<SimpleEntity *>();
std::list<BulletEntity *> bullets = std::list<BulletEntity *>();
std::list<SimpleEntity *>* floorTiles = new std::list<SimpleEntity *>();

std::default_random_engine generator;
std::uniform_int_distribution<int> posDist(-4999, 5000);
std::uniform_int_distribution<int> speedDist(-250, 250);
auto positionRange = std::bind(posDist, generator);
auto speedRange = std::bind(speedDist, generator);

SimpleEntity* skybox;
TargetEntity* target;
PlayerEntity* player;
SimpleEntity test;

void cleanup()
{
	std::list<SimpleEntity *>::iterator i1 = targets->begin();
	while (i1 != targets->end()) {
		(*i1)->deleteEntity(root);
		targets->erase(i1++);
	}

	std::list<ColorBlockEntity *>::iterator i2 = colorBlocks->begin();
	while (i2 != colorBlocks->end())
	{
		(*i2)->deleteEntity(root);
		colorBlocks->erase(i2++);
	}

	std::list<BulletEntity *>::iterator i3 = bullets.begin();
	while (i3 != bullets.end())
	{
		(*i3)->deleteEntity(root);
		bullets.erase(i3++);
	}

	std::list<SimpleEntity *>::iterator i4 = floorTiles->begin();
	while (i4 != floorTiles->end()) {
		(*i4)->deleteEntity(notTargets);
		floorTiles->erase(i4++);
	}
	
	delete mgr;
	delete tracker;
	delete button;
	delete analog;
	delete target;
	delete player;
	delete targets;
	delete floorTiles;
	scene->clearChildren();
	root->clearChildren();
	notTargets->clearChildren();
	scene = NULL;
	root = NULL;
	notTargets = NULL;
}

// prototype functions
void shoot(Vec3f position, Vec3f direction, float speed, int damage, float traveldistance);
void bulletsTargetCollision(std::list<BulletEntity *> b, std::list<TargetEntity *>* t, int time);
void animate();
void moveBullets(int time);
void moveFloor(int time);
void display(void);
void resetGame();
void moveColorBlocks(int time);

// Threads.. aahhh screw it..
// ThreadRefPtr applicationThread;
// ThreadRefPtr movementThread;
// BarrierRefPtr syncBarrier;

NodeTransitPtr buildScene()
{
	std::cout << "buildscene" << std::endl;

	notTargets = Node::create();

	DirectionalLightRecPtr blank = DirectionalLight::create();
	blank->setDiffuse(Color4f(0, 0, 0, 0));
	blank->setAmbient(Color4f(0, 0, 0, 0));
	blank->setSpecular(Color4f(0, 0, 0, 0));
	root->setCore(blank);

	// Lichtquelle
	DirectionalLightRecPtr sunLight = DirectionalLight::create();
	//color information
	sunLight->setDiffuse(Color4f(3, 3, 3, 1));
	sunLight->setAmbient(Color4f(1.0, 1.0, 1.0, 1));
	sunLight->setSpecular(Color4f(1, 1, 1, 1));

	notTargets->setCore(sunLight);
	std::cout << "light build" << std::endl;

	//set the beacon
	NodeRecPtr lightBeacon = makeSphere(2, 0);
	ComponentTransformRecPtr lightBeaconCT = ComponentTransform::create();
	lightBeaconCT->setTranslation(Vec3f(0,0,100));
	NodeRecPtr lightBeaconTrans = Node::create();
	lightBeaconTrans->setCore(lightBeaconCT);
	lightBeaconTrans->addChild(lightBeacon);
	notTargets->addChild(lightBeaconTrans);

	sunLight->setBeacon(lightBeacon);

	// create floor for player
	level->addFloor(notTargets, floorTiles);
	std::cout << "floortiles build" << std::endl;

	skybox = new SimpleEntity(Vec3f(0, 0, 0), "models/skybox.wrl");
	root->addChild(skybox->getEntity());

	player = new PlayerEntity(Vec3f(0, 95, 0), "");
	player->setFloor(floorTiles);
	notTargets->addChild(player->getEntity());
	level->setPlayer(player);

	while (colorBlocks->size() < 20){
		float x = positionRange();
		float y = positionRange();
		float z = positionRange();

		std::cout << x << " " << y << " " << z << std::endl;
		ColorBlockEntity* cBlock = new ColorBlockEntity(Vec3f(x, y, z), "models/block_200.wrl");
		cBlock->setSpeed(1000 + speedRange());
		cBlock->hue = fmodf(speedRange(), 360);
		colorBlocks->push_back(cBlock);
		root->addChild(cBlock->getEntity());
	}

	root->addChild(notTargets);

	std::cout << "hexagons build" << std::endl;

	return NodeTransitPtr(root);
}

template<typename T>
T scale_tracker2cm(const T& value)
{
	static const float scale = OSGCSM::convert_length(cfg.getUnits(), 1.f, OSGCSM::CAVEConfig::CAVEUnitCentimeters);
	return value * scale;
}

auto head_orientation = Quaternion();
auto head_position = Vec3f(0, 170, 0);	// a 1.7m Person 2m in front of the scene
//auto head_position = Vec3f(0,0,100);
void VRPN_CALLBACK callback_head_tracker(void* userData, const vrpn_TRACKERCB tracker)
{
	head_orientation = Quaternion(tracker.quat[0], tracker.quat[1], tracker.quat[2], tracker.quat[3]);
	head_position = Vec3f(scale_tracker2cm(Vec3d(tracker.pos)));
}

auto wand_orientation = Quaternion(0, 0, 0, 1);
auto wand_position = Vec3f(0,0,0);
void VRPN_CALLBACK callback_wand_tracker(void* userData, const vrpn_TRACKERCB tracker)
{
	wand_orientation = Quaternion(tracker.quat[0], tracker.quat[1], tracker.quat[2], tracker.quat[3]);
	wand_position = Vec3f(scale_tracker2cm(Vec3d(tracker.pos)));
	
}

auto analog_values = Vec3f();
void VRPN_CALLBACK callback_analog(void* userData, const vrpn_ANALOGCB analog)
{
	if (analog.num_channel >= 2)
		analog_values = Vec3f(analog.channel[0], 0, -analog.channel[1]);
}

bool actionState = false;
Vec3f shootDirection = Vec3f();

void VRPN_CALLBACK callback_button(void* userData, const vrpn_BUTTONCB button)
{
	if (button.button == 0 && button.state == 1){
		
		actionState = true;

		OSG::Vec3f eulerAngles;
		wand_orientation.getEulerAngleRad(eulerAngles);

		float z = 100 * cos(eulerAngles[0]) * cos(eulerAngles[1]);
		float x = 100 * cos(eulerAngles[0]) * sin(eulerAngles[1]);
		float y = 100 * sin(eulerAngles[2]);
			
		Vec3f direction = Vec3f(-x, y, -z);
	}

	if (button.button == 2 && button.state == 1){

		actionState = true;

		resetGame();
	}
}

void InitTracker(OSGCSM::CAVEConfig &cfg)
{
	try
	{
		const char* const vrpn_name = "DTrack@localhost";
		tracker = new vrpn_Tracker_Remote(vrpn_name);
		tracker->shutup = true;
		tracker->register_change_handler(NULL, callback_head_tracker, cfg.getSensorIDHead());
		tracker->register_change_handler(NULL, callback_wand_tracker, cfg.getSensorIDController());
		button = new vrpn_Button_Remote(vrpn_name);
		button->shutup = true;
		button->register_change_handler(nullptr, callback_button);
		analog = new vrpn_Analog_Remote(vrpn_name);
		analog->shutup = true;
		analog->register_change_handler(NULL, callback_analog);
	}
	catch (const std::exception& e)
	{
		std::cout << "ERROR: " << e.what() << '\n';
		return;
	}
}

void check_tracker()
{
	tracker->mainloop();
	button->mainloop();
	analog->mainloop();
}

void print_tracker()
{
	std::cout << "Head position: " << head_position << " orientation: " << head_orientation << '\n';
	std::cout << "Wand position: " << wand_position << " orientation: " << wand_orientation << '\n';
	std::cout << "Analog: " << analog_values << '\n';
}

void keyboard(unsigned char k, int x, int y)
{
	Real32 ed;
	SimpleEntity* tmp = new SimpleEntity();

	float randrx = ((float) (rand() % 10) + 1) - 5;
	float randrz = ((float) (rand() % 10) + 1) - 5;

	Vec3f pos = mgr->getTranslation() + Vec3f(0,100,0);
	
	float degree = mgr->getYRotate();
	float radians = degree * (M_PI / 180);
	Vec3f dir = Vec3f(-sin(degree), 0, -cos(degree));
	dir.normalize();

	switch (k)
	{
	case 'q':
	case 27:
		cleanup();
		exit(EXIT_SUCCESS);
		break;
	case 'z':
		ed = mgr->getEyeSeparation() * .9f;
		std::cout << "ED: " << ed << '\n';
		mgr->setEyeSeparation(ed);
		break;
	case 'x':
		ed = mgr->getEyeSeparation() * 1.1f;
		std::cout << "ED: " << ed << '\n';
		mgr->setEyeSeparation(ed);
		break;
	case 'h':
		cfg.setFollowHead(!cfg.getFollowHead());
		std::cout << "following head: " << std::boolalpha << cfg.getFollowHead() << '\n';
		break;
	case 'i':
		print_tracker();
		break;
	case 'w':
		player->moveRel(Vec3f(0, 0, -10));
		break;
	case 's':
		player->moveRel(Vec3f(0, 0, 10));
		break;
	case 'a':
		player->moveRel(Vec3f(-10, 0, 0));
		break;
	case 'd':
		player->moveRel(Vec3f(10, 0, 0));
		break;
	case 'e':
		shootDirection = Vec3f(wand_orientation[0], wand_orientation[1], wand_orientation[2]);
		shoot(pos, dir, 1500.0, 100, 2000);
		//std::cout << "Button 1 pressed" << std::endl;
		//std::cout << "Wand position " << wand_position[0] << " " << wand_position[1] << " " << wand_position[2] << std::endl;
		//std::cout << "Wand orientation" << wand_orientation << std::endl;
		break;
	case 'f':
		tmp = level->getRandomFloorTile(floorTiles);
		tmp->setRotation(Quaternion(randrx, 0, randrz, 0));
		tmp->setRotationMomentum(1);
		tmp->setGravity(true);
		break;
	case 'r':
		resetGame();
		break;
	case 'n':
		level->initNextLevel();
		break;
	case 'm':
		level->gravityTargets();
		break;
	default:
		std::cout << "Key '" << k << "' ignored\n";
	}
}

// varaiables to control scene with mouse
float lastMouseX = 0.f;
float lastMouseY = 0.f;

float transX = 0.f;
float transY = 0.f;
float transZ = 0.f;

float rotX = 0.f;
float rotY = 0.f;
float rotZ = 0.f;

bool buttonLeftPressed = false;
bool buttonRightPressed = false;

float areaNoMotion = 30.f;
float gentleFactor = 0.005f;
float slowRotationFactor = 0.005;

void setupGLUT(int *argc, char *argv [])
{
	glutInit(argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	glutCreateWindow("OpenSG CSMDemo with VRPN API");
	glutDisplayFunc([]()
	{
		// black navigation window
		glClear(GL_COLOR_BUFFER_BIT);
		glutSwapBuffers();
	});
	glutReshapeFunc([](int w, int h)
	{
		mgr->resize(w, h);
		glutPostRedisplay();
	});
	glutKeyboardFunc(keyboard);

	glutMouseFunc([](int buttonRight, int buttonLeft, int x, int y){

		if (buttonLeft == 0){
			buttonLeftPressed = true;
		}
		else {
			buttonLeftPressed = false;
		}

		if (buttonRight == 2){
			buttonRightPressed = true;
		}
		else {
			buttonRightPressed = false;
		}

	});

	glutMotionFunc([](int x, int y){
		float newX = x - 150.f;
		float newY = y - 150.f;

		// translation with left mouse button
		if (buttonLeftPressed && !buttonRightPressed){
			if (abs(newX) > areaNoMotion){
				if (lastMouseX - newX > 0.f)
					transX = -abs(newX / 150.f);
				else if (lastMouseX - newX < 0.f)
					transX = abs(newX / 150.f);
			}
			else {
				transX = 0.f;
			}

			if (abs(newY) > areaNoMotion){
				if (lastMouseY - newY > 0.f)
					transZ = -abs(newY / 150.f);
				else if (lastMouseY - newY < 0.f)
					transZ = abs(newY / 150.f);
			}
			else {
				transZ = 0.f;
			}

			rotX = 0.f;
			rotY = 0.f;;
			rotZ = 0.f;;

			/*lastMouseX = newX;
			lastMouseY = newY;*/
		}
		// rotation with right mouse button
		else if (buttonRightPressed){
			if (abs(newX) > areaNoMotion){
				if (lastMouseX - newX > 0.f)
					rotY = -abs(newX / 150.f);
				else if (lastMouseX - newX < 0.f)
					rotY = abs(newX / 150.f);
			}
			else {
				rotY = 0.f;
			}

			if (abs(newY) > areaNoMotion){
				if (lastMouseY - newY > 0.f)
					rotX = -abs(newY / 150.f);
				else if (lastMouseY - newY < 0.f)
					rotX = abs(newY / 150.f);
			}
			else {
				rotX = 0.f;
			}
		}
	});

	glutIdleFunc(display);
}

void display(void){
	//check if mouse is still on trackpad
	//if not, stop the motion very gently
	
	if (!buttonLeftPressed){
		if (transX > 0){
			transX -= gentleFactor;
			if (transX < 0){
				transX = 0.0f;
			}
		}
		else if (transX < 0){
			transX += gentleFactor;
		}

		if (transZ > 0){
			transZ -= gentleFactor;
			if (transZ < 0){
				transZ = 0.0f;
			}
		}
		else if (transZ < 0){
			transZ += gentleFactor;
		}
	}
	// stop rotation if right button is not pressed
	if (buttonRightPressed && !buttonLeftPressed){
		rotX = 0.f;
		rotY = 0.f;
		rotZ = 0.f;

		buttonRightPressed = false;
	}

	check_tracker();
	const auto speed = 1.f;
	mgr->setUserTransform(head_position, head_orientation);
	//mgr->setRelativeTranslation(Vec3f(transX, transY, transZ));
	mgr->addYRotate(rotY*slowRotationFactor);
	//mgr->setTranslation(head_position);

	animate();

	commitChanges();
	mgr->redraw();

	OSG::Thread::getCurrentChangeList()->clear();
}

int main(int argc, char **argv)
{
#if WIN32
	OSG::preloadSharedObject("OSGFileIO");
	OSG::preloadSharedObject("OSGImageFileIO");
#endif
	try
	{
		bool cfgIsSet = false;

		ChangeList::setReadWriteDefault();
		osgInit(argc, argv);

		// evaluate intial params
		for (int a = 1; a < argc; ++a)
		{
			if (argv[a][0] == '-')
			{
				if (strcmp(argv[a], "-f") == 0)
				{
					char* cfgFile = argv[a][2] ? &argv[a][2] : &argv[++a][0];
					if (!cfg.loadFile(cfgFile))
					{
						std::cout << "ERROR: could not load config file '" << cfgFile << "'\n";
						return EXIT_FAILURE;
					}
					cfgIsSet = true;
				}
			}
			else {
				std::cout << "Loading scene file '" << argv[a] << "'\n";
				scene = SceneFileHandler::the()->read(argv[a], NULL);
			}
		}

		// load the CAVE setup config file if it was not loaded already:
		if (!cfgIsSet)
		{
			const char* const default_config_filename = "config/mono.config";
			if (!cfg.loadFile(default_config_filename))
			{
				std::cout << "ERROR: could not load default config file '" << default_config_filename << "'\n";
				return EXIT_FAILURE;
			}
		}

		cfg.printConfig();

		// start servers for video rendering
		if (startServers(cfg) < 0)
		{
			std::cout << "ERROR: Failed to start servers\n";
			return EXIT_FAILURE;
		}

		setupGLUT(&argc, argv);
		InitTracker(cfg);

		MultiDisplayWindowRefPtr mwin = createAppWindow(cfg, cfg.getBroadcastaddress());
		std::cout << "window created" << std::endl;

		// Set random generator seed
		srand(Time(NULL));

		root = Node::create();
		level = new Level(root, player);
		if (!scene)
			scene = buildScene();
		std::cout << "scene build" << std::endl;

		mgr = new OSGCSM::CAVESceneManager(&cfg);
		mgr->setWindow(mwin);
		std::cout << "window build" << std::endl;
		mgr->setRoot(scene);
		std::cout << "rootscene build" << std::endl;
		mgr->showAll();
		std::cout << "show all build" << std::endl;
		mgr->getWindow()->init();
		std::cout << "init build" << std::endl;
		mgr->turnWandOff();
		std::cout << "wand build" << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cout << "ERROR: " << e.what() << '\n';
		return EXIT_FAILURE;
	}

	glutMainLoop();
}

void animate(){
	int elapsedTime = glutGet(GLUT_ELAPSED_TIME);
	int frameTime = elapsedTime - lastTime;
	lastTime = elapsedTime;

	// move all objects
	moveBullets(frameTime);
	moveFloor(frameTime);
	level->move(frameTime);
	moveColorBlocks(frameTime);

	// check if player has gravity (standing on a tile)
	if (!player->hasGravity()){
		Vec3f newPos = player->getPosition();
		newPos[0] = head_position[0];
		newPos[2] = head_position[2];
		player->setPosition(newPos);
	}
	else {
		mgr->setTranslation(Vec3f(0, player->getPosition()[1] - (player->getDimension()[1] / 2.0), 0));
	}

	if (!player->isAlive()){
		if (gameRunning){
			level->createEndMessage(false);
			gameRunning= false;
		}
	}

	// move other stuff
	skybox->setPosition(player->getPosition());
	player->move(frameTime);
	skybox->move(frameTime);

	// calculate collisions
	bulletsTargetCollision(bullets, level->getTargets(), frameTime);
}

void shoot(Vec3f position, Vec3f direction, float speed, int damage, float traveldistance){
	BulletEntity* bullet = new BulletEntity(position, direction, speed, damage, traveldistance);
	root->addChild(bullet->getEntity());

	bullets.push_back(bullet);
}

void bulletsTargetCollision(std::list<BulletEntity *> b, std::list<TargetEntity *>* t, int time){
	std::list<BulletEntity *>::iterator i = b.begin();
	while (i != b.end())
	{
		bool alive = (*i)->Entity::isAlive();
		if (!alive) {
			(*i)->deleteEntity(root);
			b.erase(i++);
		}
		else {
			if (t->size() > 0){
				std::list<TargetEntity *>::iterator j = t->begin();
				while (j != t->end())
				{
					if (PhysicCollision::checkBallCollision((*i), (*j))){
						//if ( !(*i)->isAlive() || !(*j)->isAlive())
						PhysicCollision::reflectEntitys((*i), (*j));
					}
					++j;
				}
				(*i)->move(time);
				++i;
			}
		}
	}
}

void moveBullets(int time){
	std::list<BulletEntity *>::iterator i = bullets.begin();
	while (i != bullets.end())
	{
		bool alive = (*i)->Entity::isAlive();
		if (!alive) {
			(*i)->deleteEntity(root);
			bullets.erase(i++);
		}
		else {
			(*i)->move(time);
			++i;
		}
	}
}

SimpleEntity* chosenFloorTile = nullptr;
float timeToNextRandomFall = 8000;
float currentTimeToNextRandomFall = 8000;
bool endMessageCalled = false;

void moveFloor(int time){
	if (chosenFloorTile == nullptr){
		chosenFloorTile = new SimpleEntity();
		if (floorTiles->size() > 0){
			chosenFloorTile = level->getRandomFloorTile(floorTiles);
			currentTimeToNextRandomFall = timeToNextRandomFall;
		}
		else {
			gameRunning = false;
		}
	}
	
	if (gameRunning){
		if (currentTimeToNextRandomFall > 0){
			if (level->getState() == 1){
				currentTimeToNextRandomFall -= time;
				float red = 5;
				float green = (currentTimeToNextRandomFall / timeToNextRandomFall) * 5;
				float blue = (currentTimeToNextRandomFall / timeToNextRandomFall) * 5;
				chosenFloorTile->changeColor(red, green, blue);
			}
		}
		else {
			chosenFloorTile->setGravity(true);
			currentTimeToNextRandomFall = timeToNextRandomFall;
			chosenFloorTile = nullptr;
		}
	}
	
	std::list<SimpleEntity *>::iterator i = floorTiles->begin();
	while (i != floorTiles->end())
	{
		bool alive = (*i)->Entity::isAlive();
		if (!alive) {
			(*i)->deleteEntity(notTargets);
			floorTiles->erase(i++);
		}
		else {
			(*i)->move(time);
			++i;
		}
	}
}

void moveColorBlocks(int time){
	std::list<ColorBlockEntity *>::iterator i = colorBlocks->begin();
	while (i != colorBlocks->end())
	{
		bool alive = (*i)->Entity::isAlive();
		if (!alive) {
			(*i)->deleteEntity(root);
			colorBlocks->erase(i++);
		}
		else {
			(*i)->move(time);
			++i;
		}
	}
}

void resetGame(){
	// clear Floor
	std::list<SimpleEntity *>::iterator i = floorTiles->begin();
	while (i != floorTiles->end())
	{
		(*i)->deleteEntity(root);
		++i;
	}
	floorTiles->clear();
	level->addFloor(notTargets, floorTiles);

	//reset Level
	level->reset();
	gameRunning = true;
	player->setGravity(false);
	player->setAlive(true);
	player->setDirection(Vec3f(0, 0, 0));
	player->setPosition(Vec3f(0, 95, 0));
	//mgr->setUserTransform(head_position, head_orientation);
	mgr->setTranslation(Vec3f(0,0,0));
}