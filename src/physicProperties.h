#ifndef PHYSICPROPERTIES_H
#define PHYSICPROPERTIES_H

#include <OpenSG/OSGSimpleGeometry.h>

OSG_USING_NAMESPACE

class PhysicProperties
{
public:
	float gravityConst;

	bool dynamic;
	bool gravity;

	bool colliding;

	float mass;
	float speed;
	Vec3f direction;
	float rotationMomentum;
	float rotationAngle;

	~PhysicProperties(){}

	PhysicProperties(){
		gravityConst = 9.81;
		dynamic = true;
		gravity = false;
		colliding = false;
		mass = 1.0;
		speed = 1.0;
		rotationAngle = 0;
		rotationMomentum = 0;
	}

	void setCollision(bool colliding){
		this->colliding = colliding;
	}

	bool isColliding(){
		return this->colliding;
	}

	void setDirection(Vec3f direction){
		this->direction = direction;
	}

	Vec3f getDirection(){
		return direction;
	}

	void setDynamic(bool dynamic){
		this->dynamic = dynamic;
	}

	void setRotationMomentum(float rot){
		rotationMomentum = rot;
	}

	float getRotationMomentum(){
		return rotationMomentum;
	}

	bool isDynamic(){
		return this->dynamic;
	}

	void setGravity(bool gravity){
		this->gravity = gravity;
	}

	bool hasGravity(){
		return this->gravity;
	}

	void setMass(float mass){
		this->mass = mass;
	}

	float getMass(){
		return this->mass;
	}

	void setSpeed(float speed){
		this->speed = speed;
	}

	float getSpeed(){
		return speed;
	}
};
#endif