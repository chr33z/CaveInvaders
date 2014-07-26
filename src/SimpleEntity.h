#ifndef SIMPLEENTITY_H
#define SIMPLEENTITY_H

#include "PhysicEntity.h"

class SimpleEntity : public PhysicEntity
{

public:
	NodeRecPtr modelNode;
	DirectionalLightRecPtr light;

	float red;
	float green;
	float blue;

	SimpleEntity::SimpleEntity(){
	}

	SimpleEntity::SimpleEntity(const char* file){
		red = 1;
		green = 1;
		blue = 1;

		model = SceneFileHandler::the()->read(file);

		modelNode = Node::create();

		light = DirectionalLight::create();
		light->setDiffuse(Color4f(red, green, blue, 2));
		light->setAmbient(Color4f(red, green, blue, 1));
		light->setSpecular(Color4f(0, 0, 0, 1));

		//set the beacon
		NodeRecPtr lightBeacon = makeSphere(2, 0);
		ComponentTransformRecPtr lightBeaconCT = ComponentTransform::create();
		lightBeaconCT->setTranslation(Vec3f(0, 0, 100));
		NodeRecPtr lightBeaconTrans = Node::create();
		lightBeaconTrans->setCore(lightBeaconCT);
		lightBeaconTrans->addChild(lightBeacon);
		modelNode->addChild(lightBeaconTrans);

		light->setBeacon(lightBeacon);

		modelNode->setCore(light);
		modelNode->addChild(model);

		componentTransform = ComponentTransform::create();
		componentTransform->setTranslation(position);
		componentTransform->setRotation(rotation);

		transformNode = Node::create();
		transformNode->setCore(componentTransform);
		transformNode->addChild(modelNode);
	}

	SimpleEntity::SimpleEntity(Vec3f pos, const char* file){
		position = pos;

		red = 1;
		green = 1;
		blue = 1;

		model = SceneFileHandler::the()->read(file);

		modelNode = Node::create();

		light = DirectionalLight::create();
		light->setDiffuse(Color4f(red, green, blue, 2));
		light->setAmbient(Color4f(red, green, blue, 1));
		light->setSpecular(Color4f(0, 0, 0, 1));

		//set the beacon
		NodeRecPtr lightBeacon = makeSphere(2, 0);
		ComponentTransformRecPtr lightBeaconCT = ComponentTransform::create();
		lightBeaconCT->setTranslation(Vec3f(0, 0, 100));
		NodeRecPtr lightBeaconTrans = Node::create();
		lightBeaconTrans->setCore(lightBeaconCT);
		lightBeaconTrans->addChild(lightBeacon);
		modelNode->addChild(lightBeaconTrans);

		light->setBeacon(lightBeacon);

		modelNode->setCore(light);
		modelNode->addChild(model);

		componentTransform = ComponentTransform::create();
		componentTransform->setTranslation(position);
		componentTransform->setRotation(rotation);

		transformNode = Node::create();
		transformNode->setCore(componentTransform);
		transformNode->addChild(modelNode);
	}

	void deleteEntity(NodeRecPtr parent){
		modelNode->subChild(model);
		transformNode->subChild(modelNode);
		parent->subChild(transformNode);
	}

	/*
		Put fancy movement in here
	*/
	void SimpleEntity::move(int time){
		// conversion to meter/second
		float mtime = (float) time * 0.001f;
		float mspeed = speed * 0.01f;

		// first take normalized direction vector and multiply with speed*time (distance)
		direction.normalize();
		float distanceMoved = mspeed * mtime;
		direction *= distanceMoved;

		if (getHealth() == 0){
			gravity = true;
		}

		// if gravity is on, add gravity vector to direction/speed vector
		if (gravity){
			direction[1] += (-gravityConst * mtime * 0.01f);
		}

		if (abs(position[0]) > boundary || abs(position[1]) > boundary || abs(position[2]) > boundary){
			return;
		}

		// move entity with this vector
		// * 100 because we calculated everything in m and coordinate system is cm
		position += (direction * 100);

		// save length of vector as new speed and normalize direction vector
		speed = (direction.length() / mtime) * 100;
		direction.normalize();

		DirectionalLightRecPtr tmplight = dynamic_cast<DirectionalLight *>(modelNode->getCore());
		tmplight->setDiffuse(Color4f(red, green, blue, 2));
		tmplight->setAmbient(Color4f(red, green, blue, 1));

		rotationAngle += mtime * rotationMomentum;
		ComponentTransformRecPtr tmp = dynamic_cast<ComponentTransform *>(transformNode->getCore());
		//tmp->setRotation(Quaternion(Vec3f(rotation[0], 0, rotation[2]), rotation[3]));
		tmp->setRotation(Quaternion(Vec3f(rotation[0], 0, rotation[2]), osgDegree2Rad(0) + rotationAngle));
		tmp->setTranslation(position);
	}

	void changeColor(float red, float green, float blue){
		this->red = red;
		this->green = green;
		this->blue = blue;
	}
};
#endif