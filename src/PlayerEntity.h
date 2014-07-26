#include "PhysicEntity.h"

class PlayerEntity : public PhysicEntity
{
private:
	std::list<SimpleEntity* >* floorTiles;

public:
	// time a player is allowed to stay in the air without falling down in seconds
	float jumpTreshold;

	void setFloor(std::list<SimpleEntity* >* list){
		this->floorTiles = list;
	}

	void deleteEntity(NodeRecPtr parent){
		transformNode->subChild(model);
		parent->subChild(transformNode);
	}

	PlayerEntity::PlayerEntity(){
	}

	PlayerEntity::PlayerEntity(const char* file){
		model = SceneFileHandler::the()->read(file);
		dimension = Vec3f(20, 180, 40);
		jumpTreshold = 0.100;

		componentTransform = ComponentTransform::create();
		componentTransform->setTranslation(position);
		componentTransform->setRotation(rotation);

		transformNode = Node::create();
		transformNode->setCore(componentTransform);
		transformNode->addChild(model);
	}

	PlayerEntity::PlayerEntity(Vec3f pos, const char* file){
		position = pos;
		dimension = Vec3f(10, 180, 10);
		jumpTreshold = 0.100;

		model = SceneFileHandler::the()->read(file);

		componentTransform = ComponentTransform::create();
		componentTransform->setTranslation(position);
		componentTransform->setRotation(rotation);

		transformNode = Node::create();
		transformNode->setCore(componentTransform);
		transformNode->addChild(model);
	}

	/*
		Put fancy movement in here
	*/
	void PlayerEntity::move(int time){
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

		if (!gravity && !isAboveEntity(floorTiles)){
			// TODO count down a specific time to avoid accidental falling
			gravity = true;
		}

		// if gravity is on, add gravity vector to direction/speed vector
		if (gravity){
			direction[1] += (-gravityConst * mtime * 0.01f);
		}

		// if player is to far down he dies!!!! GNARRR!!
		if (abs(position[0]) > boundary || abs(position[1]) > boundary || abs(position[2]) > boundary){
			alive = false;
		}

		// move entity with this vector
		// * 100 because we calculated everything in m and coordinate system is cm
		position += (direction * 100);

		// save length of vector as new speed and normalize direction vector
		speed = (direction.length() / mtime) * 100;
		direction.normalize();
		// if we have something to rotate, do it
		rotationAngle += mtime * rotationMomentum;

		ComponentTransformRecPtr tmp = dynamic_cast<ComponentTransform *>(transformNode->getCore());
		tmp->setRotation(Quaternion(Vec3f(rotation[0], 0, rotation[2]), osgDegree2Rad(0) + rotationAngle));
		tmp->setTranslation(position);
	}

	// distance between player and floor to detect falling
	bool isAboveEntity(std::list<SimpleEntity* >* list){
		bool above = false;
		float distanceTreshold = 20.0;

		std::list<SimpleEntity *>::iterator i = list->begin();
		while (i != list->end())
		{
			float pX = position[0]; float pY = position[1]; float pZ = position[2];
			float iX = (*i)->getPosition()[0]; float iY = (*i)->getPosition()[1]; float iZ = (*i)->getPosition()[2];

			if (abs(pX - iX) < ((*i)->getDimension()[0] / 2.0) + dimension[0] / 2.0
				&& abs(pZ - iZ) < ((*i)->getDimension()[2] / 2.0) + dimension[0] / 2.0
				&& abs((pY - dimension[1] / 2.0) - (iY + (*i)->getDimension()[1] / 2.0)) < distanceTreshold){
					above = true;
			}
			++i;
		}
		return above;
	}
};