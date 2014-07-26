#include <OpenSG/OSGSimpleGeometry.h>
#include <iostream>
#include <math.h>
#include <vector>

#include "LevelFunctions.h"

#include "SimpleEntity.h"
#include "TargetEntity.h"
#include "WinLoseEntity.h"

OSG_USING_NAMESPACE

class Level
{
public:
	typedef struct level {
		int lev;
		int numberTargets;
		float speedFactor;
		float distance;
		const char* model;
		float wobbleY;
		float wobbleXZ;
		float wobbleFactor;
	};

	Node* root;
	WinLoseEntity* message;
	PlayerEntity* player;
	std::vector <level> levels;
	std::list<TargetEntity* >* targets;
	std::list<TargetEntity* >* doomedTargets;

	int currentLevel;

	// constructor
	Level(Node* root, PlayerEntity* player){
		targets = new std::list<TargetEntity* >();
		this->root = root;
		this->player = player;
		message = nullptr;
		initLevels();
		currentLevel = 0;
		initNextLevel();
	}

	void initNextLevel(){
		currentLevel++;
		if (currentLevel <= levels.size()){
			createTargets(levels.at(levels.size() - currentLevel));
		}
		else {
			std::list<TargetEntity *>::iterator i = targets->begin();
			while (i != targets->end())
			{
				(*i)->deleteEntity(root);
				++i;
			}
			createEndMessage(true);
		}
	}

	void initLevels(){
		struct level level1;
		level1.lev = 1;
		level1.numberTargets = 4;
		level1.speedFactor = 20;
		level1.distance = 400;
		level1.wobbleY = 0;
		level1.wobbleXZ = 0;
		level1.wobbleFactor = 0;
		level1.model = "models/caveInvader1.wrl";
		levels.push_back(level1);

		struct level level2;
		level2.lev = 2;
		level2.numberTargets = 6;
		level2.speedFactor = -30;
		level2.distance = 450;
		level2.wobbleY = 5;
		level2.wobbleXZ = 0;
		level2.wobbleFactor = 200;
		level2.model = "models/caveInvader2.wrl";
		levels.push_back(level2);

		struct level level3;
		level3.lev = 3;
		level3.numberTargets = 8;
		level3.speedFactor = 30;
		level3.distance = 500;
		level3.wobbleY = 10;
		level3.wobbleXZ = 300;
		level3.wobbleFactor = 200;
		level3.model = "models/caveInvader1.wrl";
		levels.push_back(level3);

		struct level level4;
		level4.lev = 4;
		level4.numberTargets = 10;
		level4.speedFactor = -20;
		level4.distance = 600;
		level4.wobbleY = 20;
		level4.wobbleXZ = 400;
		level4.wobbleFactor = 150;
		level4.model = "models/caveInvader2.wrl";
		levels.push_back(level4);

		std::reverse(levels.begin(), levels.end());
	}

	void createTargets(level l){
		//std::cout << "create targets" << std::endl;
		if (targets->size() > 0){
			std::list<TargetEntity *>::iterator i = targets->begin();
			while (i != targets->end())
			{
				(*i)->deleteEntity(root);
				++i;
			}
		}

		float degree = 0;

		for (int i = 0; i < l.numberTargets; i++){
			TargetEntity* tmp = new TargetEntity(l.model);
			tmp->setDistanceToOrigin(l.distance);
			tmp->setSpeedFactor(l.speedFactor);
			tmp->setXZRotation(degree);
			tmp->setWobbleY(l.wobbleY);
			tmp->setWobbleXZ(l.wobbleXZ);
			tmp->setWobbleFactor(l.wobbleFactor);
			degree += (360 / l.numberTargets);
			targets->push_back(tmp);

			root->addChild(tmp->getEntity());
			//std::cout << "target added to root" << std::endl;
		}
	}

	void gravityTargets(){
		getRandomEntity(targets)->setGravity(true);
	}

	std::list<TargetEntity* >* getTargets(){
		return targets;
	}

	void move(int time){
		if (currentLevel > 0 && targets->size() < 1 && currentLevel <= levels.size()){
			//std::cout << "round over! start new round" << std::endl;
			initNextLevel();
		}
		else {
			std::list<TargetEntity *>::iterator i = targets->begin();
			while (i != targets->end())
			{
				bool alive = (*i)->Entity::isAlive();
				if (!alive) {
					(*i)->deleteEntity(root);
					targets->erase(i++);
				}
				else {
					(*i)->move(time);
					++i;
				}
			}
		}

		if (message != nullptr){
			message->move(time, player->getPosition());
		}
	}

	TargetEntity* getRandomEntity(std::list<TargetEntity* >* list){
		TargetEntity* entity = new TargetEntity();
		int ran = rand() % list->size();
		int j = 0;

		std::list<TargetEntity *>::iterator i = list->begin();
		while (i != list->end())
		{
			if (j == ran){
				entity = (*i);
				break;
			}
			++j;
			++i;
		}
		return entity;
	}

	NodeTransitPtr getSingleHexagon(std::list<SimpleEntity* >* list, Vec3f position){
		SimpleEntity* entity = new SimpleEntity(position, "models/hexagon_single.wrl");
		entity->setDimension(Vec3f(60, 5, 70));
		entity->getEntity();
		list->push_back(entity);

		return entity->getEntity();

	}

	SimpleEntity* getFarestFloorTile(std::list<SimpleEntity* >* list){
		SimpleEntity* farest = new SimpleEntity();
		float distance = 0.0;

		std::list<SimpleEntity *>::iterator i = list->begin();
		while (i != list->end())
		{
			float newDistance = sqrt(((*i)->getPosition()[0] * (*i)->getPosition()[0]) +
				((*i)->getPosition()[2] * (*i)->getPosition()[2]));

			if (newDistance >= distance) {
				farest = (*i);
				distance = newDistance;
			}
			++i;
		}
		return farest;
	}

	SimpleEntity* getRandomFloorTile(std::list<SimpleEntity* >* list){
		SimpleEntity* farest = new SimpleEntity();
		int ran = rand() % list->size();
		int j = 0;

		std::list<SimpleEntity *>::iterator i = list->begin();
		while (i != list->end())
		{
			if (j == ran){
				farest = (*i);
				break;
			}
			++j;
			++i;
		}
		return farest;
	}

	void addFloor(NodeRecPtr root, std::list<SimpleEntity* >* floor){
		root->addChild(getSingleHexagon(floor, Vec3f(-130, 0, 110)));
		root->addChild(getSingleHexagon(floor, Vec3f(0, 0, 110)));

		root->addChild(getSingleHexagon(floor, Vec3f(32.5 - 130, 0, 55)));
		root->addChild(getSingleHexagon(floor, Vec3f(32.5, 0, 55)));
		root->addChild(getSingleHexagon(floor, Vec3f(32.5 + 65, 0, 55)));
		root->addChild(getSingleHexagon(floor, Vec3f(32.5 - 65, 0, 55)));

		root->addChild(getSingleHexagon(floor, Vec3f(-65, 0, 0)));
		root->addChild(getSingleHexagon(floor, Vec3f(0, 0, 0)));
		root->addChild(getSingleHexagon(floor, Vec3f(65, 0, 0)));

		root->addChild(getSingleHexagon(floor, Vec3f(32.5 - 130, 0, -55)));
		root->addChild(getSingleHexagon(floor, Vec3f(32.5, 0, -55)));
		root->addChild(getSingleHexagon(floor, Vec3f(32.5 + 65, 0, -55)));
		root->addChild(getSingleHexagon(floor, Vec3f(32.5 - 65, 0, -55)));

		root->addChild(getSingleHexagon(floor, Vec3f(-65, 0, -110)));
		root->addChild(getSingleHexagon(floor, Vec3f(0, 0, -110)));

		std::list<SimpleEntity *>::iterator i = floor->begin();
		while (i != floor->end())
		{
			(*i)->changeColor(0,0,4);
			++i;
		}
	}

	int getState(){
		if (targets->size() > 0){
			TargetEntity* tmp = targets->back();
			return tmp->getState();
		}
		else {
			return 3;
		}
	}

	void createEndMessage(bool win){
		if (win){
			std::list<TargetEntity *>::iterator i = targets->begin();
			while (i != targets->end())
			{
				(*i)->deleteEntity(root);
				++i;
			}
			message = new WinLoseEntity("models/win.wrl", 0, 1);
			root->addChild(message->getEntity());
		}
		else {
			std::list<TargetEntity *>::iterator i = targets->begin();
			while (i != targets->end())
			{
				(*i)->deleteEntity(root);
				++i;
			}
			message = new WinLoseEntity("models/lose.wrl", 1, 0);
			root->addChild(message->getEntity());
		}
	}

	void setPlayer(PlayerEntity* player){
		this->player = player;
	}

	void reset(){
		if (message != NULL)
			message->deleteEntity(root);
		message = nullptr;
		currentLevel = 0;

		std::list<TargetEntity *>::iterator i = targets->begin();
		while (i != targets->end())
		{
			(*i)->deleteEntity(root);
			++i;
		}
		targets->clear();

		initNextLevel();
	}
};