#include "PhysicEntity.h"

#define TYPE_BALL = "ball"

class BulletEntity : public PhysicEntity
{

public:
	float maxTraveldistance;
	float currentTraveldistance;
	int damage;

	float green;
	float red;

	NodeRecPtr modelNode;
	DirectionalLightRecPtr light;

	void deleteEntity(NodeRecPtr parent){
		modelNode->subChild(model);
		transformNode->subChild(modelNode);
		parent->subChild(transformNode);
	}

	BulletEntity::BulletEntity(){
	}

	BulletEntity::BulletEntity(Vec3f pos, Vec3f dir, float spd, int dmg, float tdist){
		damage = dmg;
		maxTraveldistance = tdist;
		currentTraveldistance = 0.0;
		
		position = pos;
		direction = dir;
		speed = spd;
		dimension = Vec3f(20, 20, 20);
		makeDamage = true;
		gravity = true;

		green = 1;
		red = 1;

		model = SceneFileHandler::the()->read("models/ball_20.wrl");
		modelNode = Node::create();

		DirectionalLightRecPtr light = DirectionalLightRecPtr();
		light = DirectionalLight::create();
		light->setDiffuse(Color4f(red * 5, green * 5, 5, 2));
		light->setAmbient(Color4f(red * 5, green * 5, 5, 1));
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
	void BulletEntity::move(int time){
		// conversion to meter/second
		float mtime = (float)time * 0.001f;
		float mspeed = speed * 0.01f;
		
		// first take normalized direction vector and multiply with speed*time (distance)
		direction.normalize();
		float distanceMoved = mspeed * mtime;
		direction = direction * distanceMoved;

		// if gravity is on, add gravity vector to direction/speed vector
		if (gravity){
			direction += Vec3f(0, (-gravityConst * mtime * 0.01f), 0);
		}

		// move entity with this vector
		// * 100 because we calculated everything in m and coordinate system is cm
		position += (direction * 100);

		// save length of vector as new speed and normalize direction vector
		speed = (direction.length() / mtime) * 100;
		currentTraveldistance += direction.length() * 100;
		direction.normalize();

		red = 1 - currentTraveldistance / maxTraveldistance;
		
		if (currentTraveldistance >= maxTraveldistance){
			alive = false;
			return;
		}
		if (currentTraveldistance < maxTraveldistance * 0.5f){
			green = 1 - (currentTraveldistance / (maxTraveldistance * 0.5f));
			red = 0;
		}
		else {
			red = (currentTraveldistance - (maxTraveldistance * 0.5f)) / (maxTraveldistance * 0.5f);
			green = 0;
		}

		DirectionalLightRecPtr tmplight = dynamic_cast<DirectionalLight *>(modelNode->getCore());
		tmplight->setDiffuse(Color4f(red * 5, green * 5, 0, 2));
		tmplight->setAmbient(Color4f(red * 5, green * 5, 0.0, 1));

		ComponentTransformRecPtr tmp = dynamic_cast<ComponentTransform *>(transformNode->getCore());
		tmp->setTranslation(position);
	}
};