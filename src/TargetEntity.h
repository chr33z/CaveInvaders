#ifndef TARGETENTITY_H
#define TARGETENTITY_H

#include "PhysicEntity.h"

class TargetEntity : public PhysicEntity
{

public:
	float xzRotation;			// rotation of object around y axis in degree
	float zElevation;			// rotation of object from xz-plane to entity
	
	float startDistance;		// distance the targets start
	float tmpDistance;			// distance while targets fly in
	float distanceToOrigin;		// final distance to origin.. who would have guessed (cm)
	
	float speedFactor;
	Vec3f lastPosition;
	
	float wobbleY;
	float wobbleFactor;
	float wobbleXZ;
	float wobbleLastTime;
	float wobbleTime;

	float maxHealth;
	float green;
	float red;

	int STATE_ATTACK;
	int STATE_FALLING;
	int STATE_INIT;

	int STATE;

	NodeRecPtr modelNode;
	DirectionalLightRecPtr light;

	void deleteEntity(NodeRecPtr parent){
		modelNode->subChild(model);
		transformNode->subChild(modelNode);
		parent->subChild(transformNode);
	}

	TargetEntity::TargetEntity(){}

	TargetEntity::TargetEntity(const char* file){
		
		model = SceneFileHandler::the()->read(file);
		xzRotation = 0;
		zElevation = 10;

		tmpDistance = 10000;
		distanceToOrigin = 500;

		speedFactor = 10;
		direction = Vec3f(0,-1,0);
		dimension = Vec3f(80, 80, 80);
		receiveDamage = true;
		this->setRotation(Quaternion(0, 1, 0, 1));
		boundary = 2000;

		green = 1;
		red = 0;

		maxHealth = health;
		
		wobbleY = 0;
		wobbleXZ = 0;
		wobbleFactor = 0;
		wobbleTime = 0;
		wobbleLastTime = 0;

		STATE_ATTACK = 1;
		STATE_FALLING = 2;
		STATE_INIT = 3;

		STATE = STATE_INIT;

		modelNode = Node::create();

		light = DirectionalLight::create();
		light->setDiffuse(Color4f(red * 5, green * 5, 0, 2));
		light->setAmbient(Color4f(red * 5, green * 5, 0.0, 1));
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
	void TargetEntity::move(int time){
		// conversion to meter/second
		float mtime = (float) time * 0.001f;
		float mspeed = speed * 0.01f;

		// while alive
		if (!gravity){
			if (getHealth() <= 0){
				gravity = true;
			}

			// color fading when hit
			if (health > maxHealth * 0.5f){
				green = 1;
				red = 1 - (maxHealth - 2 * (maxHealth - health))/maxHealth;
			}
			else {
				red = 1;
				green = ((maxHealth * 0.5f) - 2 * ((maxHealth * 0.5f) - health)) / maxHealth;
			}

			// check distance 
			if (tmpDistance - distanceToOrigin > 1){
				tmpDistance -= time;

				if (tmpDistance - distanceToOrigin > 1){
					tmpDistance -= time;
				}
				else {
					tmpDistance = distanceToOrigin;
					std::cout << "Distance reached" << std::endl;
				}
			}
			else {
				STATE = STATE_ATTACK;
			}

			// what the hell am I doing here? something with 
			// position on sphere with given angles on planes
			tmpDistance = tmpDistance + ((sin(osgDegree2Rad(wobbleTime + xzRotation))
				- sin(osgDegree2Rad(wobbleLastTime + xzRotation))) * wobbleY);

			zElevation = zElevation + ((sin(osgDegree2Rad(wobbleTime + xzRotation)) 
				- sin(osgDegree2Rad(wobbleLastTime + xzRotation))) * wobbleY);

			wobbleLastTime = wobbleTime;
			wobbleTime += mtime * wobbleFactor;
			if (wobbleTime > 3600) wobbleTime = 0;

			// no, I think this part was actually about coordinates on a sphere with given angles
			float x = tmpDistance * cos(osgDegree2Rad(zElevation)) * cos(osgDegree2Rad(xzRotation));
			float z = tmpDistance * cos(osgDegree2Rad(zElevation)) * sin(osgDegree2Rad(xzRotation));
			float y = tmpDistance * sin(osgDegree2Rad(zElevation));
			position = (Vec3f(x,y,z));

			// calculate direction
			// we already are in cm because we calculate position absolute
			direction = position - lastPosition;
			speed = direction.length() * 100;
			direction.normalize();

			lastPosition = position;

			xzRotation += mtime * speedFactor;

			// keep object facing player
		}
		else {
			STATE = STATE_FALLING;
			
			// first take normalized direction vector and multiply with speed*time (distance)
			direction.normalize();
			float distanceMoved = mspeed * mtime;
			direction *= distanceMoved;

			direction += Vec3f(0, -gravityConst * mtime * 0.01f, 0);

			if (abs(position[0]) > boundary || abs(position[1]) > boundary || abs(position[2]) > boundary){
				alive = false;
			}

			// move entity with this vector
			// * 100 because we calculated everything in m and coordinate system is cm
			position += (direction * 100);

			// save length of vector as new speed and normalize direction vector
			speed = (direction.length() / mtime) * 100;
			direction.normalize();
		}

		DirectionalLightRecPtr tmplight = dynamic_cast<DirectionalLight *>(modelNode->getCore());
		tmplight->setDiffuse(Color4f(red * 5, green * 5, 0, 2));
		tmplight->setAmbient(Color4f(red * 5, green * 5, 0.0, 1));

		ComponentTransformRecPtr tmp = dynamic_cast<ComponentTransform *>(transformNode->getCore());
		tmp->setRotation(Quaternion(Vec3f(rotation[0], rotation[1], rotation[2]), osgDegree2Rad(-xzRotation)));
		tmp->setTranslation(position);
	}

	void setDistanceToOrigin(float d){
		this->distanceToOrigin = d;
	}

	void setSpeedFactor(float d){
		this->speedFactor = d;
	}

	void setXZRotation(float d){
		this->xzRotation = d;
	}

	void setWobbleY(float w){
		this->wobbleY = w;
	}

	void setWobbleFactor(float w){
		this->wobbleFactor = w;
	}

	void setWobbleXZ(float w){
		this->wobbleXZ = w;
	}

	int getState(){
		return STATE;
	}
};
#endif