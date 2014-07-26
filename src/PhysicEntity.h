#ifndef PHYSICENTITY_H
#define PHYSICENTITY_H

#include "Entity.h"
#include "physicProperties.h"

class PhysicEntity : public Entity, public PhysicProperties
{

public:
	PhysicEntity(){}
	~PhysicEntity(){}
};
#endif