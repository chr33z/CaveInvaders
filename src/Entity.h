#ifndef ENTITY_H
#define ENTITY_H

#include <OpenSG/OSGSimpleGeometry.h>
#include <OpenSG/OSGGLUTWindow.h>
#include <OpenSG/OSGMultiDisplayWindow.h>
#include <OpenSG/OSGSceneFileHandler.h>

#include <OSGCSM/OSGCAVESceneManager.h>
#include <OSGCSM/OSGCAVEConfig.h>
#include <OSGCSM/appctrl.h>

#include <iostream>

OSG_USING_NAMESPACE

class Entity
{
public:
	Vec3f position;
	Quaternion rotation;
	Vec3f dimension;
	bool receiveDamage;
	bool makeDamage;

	int health;
	bool alive;
	int timeWhenDead;
	float boundary;

	NodeRecPtr model;
	ComponentTransformRecPtr componentTransform;
	NodeRecPtr transformNode;

	~Entity(){}

	Entity(){
		position = Vec3f(0, 0, 0);
		rotation = Quaternion(Vec3f(0, 0, 0), 0);
		dimension = Vec3f(0, 0, 0);
		health = 100;
		alive = true;
		receiveDamage = false;
		makeDamage = false;
		timeWhenDead = 3;
		boundary = 5000;
	}

	NodeTransitPtr getEntity(){
		return NodeTransitPtr(transformNode);
	}

	/*
	void deleteEntity(NodeRecPtr parent){
		parent->subChild(transformNode);
	}
	*/

	void deleteEntity(NodeRecPtr parent);

	void moveAbs(Vec3f p){
		position = p;
	}

	void moveRel(Vec3f t){
		position = position + t;
	}

	void setPosition(Vec3f p){
		position = p;
	}

	Vec3f getPosition(){
		return position;
	}

	void setRotation(Quaternion r){
		this->rotation = r;
	}

	Quaternion getRotation(){
		return this->rotation;
	}

	void setMakesDamage(bool dmg){
		this->makeDamage = dmg;
	}

	bool makesDamage(){
		return this->makeDamage;
	}

	void setReceiveDamage(bool dmg){
		this->receiveDamage = dmg;
	}

	bool receivesDamage(){
		return this->receiveDamage;
	}

	void setDimension(Vec3f d){
		dimension = d;
	}

	void setTimeWhenDead(int time){
		this->timeWhenDead = time;
	}

	int getTimeWhenDead(){
		return this->timeWhenDead;
	}

	Vec3f getDimension(){
		return dimension;
	}

	void changeHealth(int h){
		health += h;
		if (health <= 0){
			//alive = false;
			health = 0;
		}
	}

	int getHealth(){
		return health;
	}

	void setAlive(bool a){
		alive = a;
	}

	bool isAlive(){
		return alive;
	}

};
#endif