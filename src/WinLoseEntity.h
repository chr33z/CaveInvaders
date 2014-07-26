#ifndef WINLOSEENTITY_H
#define WINLOSEENTITY_H

#include "PhysicEntity.h"

class WinLoseEntity : public PhysicEntity
{

public:
	NodeRecPtr modelNode;
	DirectionalLightRecPtr light;

	float startDistance;
	float tmpDistance;
	float distanceToOrigin;		// distance to origin.. who would have guessed (cm)

	float red;
	float green;
	float blue;

	void deleteEntity(NodeRecPtr parent){
		modelNode->subChild(model);
		transformNode->subChild(modelNode);
		parent->subChild(transformNode);
	}

	WinLoseEntity::WinLoseEntity(){
	}

	WinLoseEntity::WinLoseEntity(const char* file, float red, float green){
		this->red = red;
		this->green = green;
		blue = 0;

		startDistance = 5000;
		distanceToOrigin = 400;
		tmpDistance = startDistance;

		rotation = Quaternion(Vec3f(0, 1, 0), osgDegree2Rad(90));

		position = Vec3f(0, 200, -tmpDistance);

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


	/*
		Put fancy movement in here
	*/
	void WinLoseEntity::move(int time, Vec3f playerPosition){
		
		if(tmpDistance - distanceToOrigin > 1){
			tmpDistance -= time * 2;
		}

		position = Vec3f(playerPosition[0], playerPosition[1] + 170, playerPosition[2]-tmpDistance);

		DirectionalLightRecPtr tmplight = dynamic_cast<DirectionalLight *>(modelNode->getCore());
		tmplight->setDiffuse(Color4f(red*5, green*5, blue*5, 2));
		tmplight->setAmbient(Color4f(red*5, green*5, blue*5, 1));

		ComponentTransformRecPtr tmp = dynamic_cast<ComponentTransform *>(transformNode->getCore());
		tmp->setTranslation(position);
	}

	void changeColor(float red, float green, float blue){
		this->red = red;
		this->green = green;
		this->blue = blue;
		//std::cout << red << " " << green << " " << blue << std::endl;
	}
};
#endif