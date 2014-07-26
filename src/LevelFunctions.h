#ifndef LEVELFUNCTIONS_H
#define LEVELFUNCTIONS_H
#include <OpenSG/OSGSimpleGeometry.h>
#include <iostream>
#include <math.h>

OSG_USING_NAMESPACE

class LevelFunctions
{
public:
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
	}
};
#endif