#ifndef PHYSICCOLLISION_H
#define PHYSICCOLLISION_H

#include <OpenSG/OSGSimpleGeometry.h>
#include "PhysicEntity.h"
#include <iostream>

OSG_USING_NAMESPACE

class PhysicCollision
{
public:
	PhysicCollision(){}
	~PhysicCollision(){}

	static bool checkSimpleCollision(PhysicEntity* e1, PhysicEntity* e2){
		Vec3f e1pos = e1->getPosition();
		Vec3f e2pos = e2->getPosition();

		Vec3f e1dim = e1->getDimension();
		Vec3f e2dim = e2->getDimension();

		// somewhat more precise estimation
		if ((e1dim[0] * 0.5f + e2dim[0] * 0.5f) > abs(e1pos[0] - e2pos[0]) &&
			(e1dim[1] * 0.5f + e2dim[1] * 0.5f) > abs(e1pos[1] - e2pos[1]) &&
			(e1dim[2] * 0.5f + e2dim[2] * 0.5f) > abs(e1pos[2] - e2pos[2])){

				return true;
		}
		else return false;
	}

	static bool checkBallCollision(PhysicEntity* e1, PhysicEntity* e2){
		Vec3f e1pos = e1->getPosition();
		Vec3f e2pos = e2->getPosition();

		Vec3f e1dim = e1->getDimension();
		Vec3f e2dim = e2->getDimension();

		// radius is half the diagonal of dimension
		// calculate biggest possible sphere inside bounding box
		float e1radius = max(e1dim[0], e1dim[1], e1dim[2]) * 0.5f;
		float e2radius = max(e2dim[0], e2dim[1], e2dim[2]) * 0.5f;


		// both objects are not "colliding" at the same time
		// don't know why... 
		if ((e1->isColliding() || e2->isColliding()) &&
			(e1radius + e2radius) > (e1pos - e2pos).length() ){
			return false;
		}

		if ((e1radius + e2radius) > (e1pos - e2pos).length()){
			e1->setCollision(true);
			e2->setCollision(true);

			if (e1->receiveDamage && e2->makeDamage){
				e1->changeHealth(-25);
			}
			if (e2->receiveDamage && e1->makeDamage){
				e2->changeHealth(-25);
			}
			return true;
		}
		else {
			e1->setCollision(false);
			e2->setCollision(false);
			return false;
		}
	}

	// calculates direction and speed of colliding (sphere-shaped) objects
	// idea from: http://www.hakenberg.de/diffgeo/elastischer_stoss.htm
	static void reflectEntitys(PhysicEntity* e1, PhysicEntity* e2){
		Vec3f normale = (-1.0f) * (e2->getPosition() - e1->getPosition());
		normale.normalize();
		
		float mass_e1 = e1->getMass();
		Vec3f pre_e1_pos = e1->getPosition();
		Vec3f pre_e1_v = e1->getDirection() * e1->getSpeed();
		Vec3f post_e1_pos = Vec3f();
		Vec3f post_e1_v = Vec3f();

		float mass_e2 = e2->getMass();
		Vec3f pre_e2_pos = e2->getPosition();
		Vec3f pre_e2_v = e2->getDirection() * e2->getSpeed();
		Vec3f post_e2_pos;
		Vec3f post_e2_v;

		Vec3f factor = 2 * (((pre_e1_v * normale) - (pre_e2_v * normale)) / ((1 / mass_e1) + (1 / mass_e2) * normale * normale));
		
		pre_e1_v = pre_e1_v - (factor / mass_e1)*normale;
		pre_e2_v = pre_e2_v + (factor / mass_e2)*normale;

		e1->setSpeed(pre_e1_v.length());
		pre_e1_v.normalize();
		e1->setDirection(pre_e1_v);

		e2->setSpeed(pre_e2_v.length());
		pre_e2_v.normalize();
		e2->setDirection(pre_e2_v);

		//std::cout << "new direction e1: " << e1->getDirection()[0] << " " << e1->getDirection()[1] << " " << e1->getDirection()[2] << std::endl;
	}

private:
	// ... so i'm writing my own max() since i'm not able include the right headers 
	static float max(float a, float b, float c){
		if (a >= b){
			if (a >= c) return a;
			else return c;
		}
		else {
			if (b >= c) return b;
			else return c;
		}
	}
};
#endif