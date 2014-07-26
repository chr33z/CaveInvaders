#ifndef COLORBLOCKENTITY_H
#define COLORBLOCKENTITY_H

#define _USE_MATH_DEFINES

#include "PhysicEntity.h"

class ColorBlockEntity : public PhysicEntity
{

public:
	NodeRecPtr modelNode;
	DirectionalLightRecPtr light;
	std::default_random_engine generator;

	float red;
	float green;
	float blue;
	float hue;

	void deleteEntity(NodeRecPtr parent){
		modelNode->subChild(model);
		transformNode->subChild(modelNode);
		parent->subChild(transformNode);
	}

	ColorBlockEntity::ColorBlockEntity(){}

	ColorBlockEntity::ColorBlockEntity(Vec3f pos, const char* file){
		position = pos;
		rotationMomentum = 1;
		direction = Vec3f(0, -1, 0);
		
		red = 5;
		green = 5;
		blue = 5;

		model = SceneFileHandler::the()->read(file);

		modelNode = Node::create();

		light = DirectionalLight::create();
		light->setDiffuse(Color4f(red*5, green*5, blue*5, 2));
		light->setAmbient(Color4f(red*5, green*5, blue*5, 1));
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
	void ColorBlockEntity::move(int time){
		// conversion to meter/second
		float mtime = (float) time * 0.001f;
		float mspeed = speed * 0.01f;

		// first take normalized direction vector and multiply with speed*time (distance)
		direction.normalize();
		float distanceMoved = mspeed * mtime;
		direction *= distanceMoved;

		//direction[1] += (-gravityConst* 0.5 * mtime * 0.01f);

		hue = fmodf(hue + time*0.1f, 360);

		Vec3f color = hsv2rgb(hue / 360, 1, 0.5);
		red = color[0];
		green = color[1];
		blue = color[2];

		// move entity with this vector
		// * 100 because we calculated everything in m and coordinate system is cm
		position += (direction * 100);

		// save length of vector as new speed and normalize direction vector
		speed = (direction.length() / mtime) * 100;
		direction.normalize();

		DirectionalLightRecPtr tmplight = dynamic_cast<DirectionalLight *>(modelNode->getCore());
		tmplight->setDiffuse(Color4f(red*7, green*7, blue*7, 2));
		tmplight->setAmbient(Color4f(red*7, green*7, blue*7, 2));

		rotationAngle += mtime * rotationMomentum;
		ComponentTransformRecPtr tmp = dynamic_cast<ComponentTransform *>(transformNode->getCore());
		tmp->setRotation(Quaternion(Vec3f(rotation[0], 0, rotation[2]), osgDegree2Rad(0) + rotationAngle));
		tmp->setTranslation(position);

		if (abs(position[0]) > boundary || abs(position[1]) > boundary || abs(position[2]) > boundary){
			float x = 0;
			float z = 0;
			float y = 4999;
			
			while ((-2000 < x && x < 2000) && (-2000 < z && z < 2000)){
				x = (rand()%10000)-5000;
				z = (rand() % 10000) - 5000;
			}
			/*
			while (-2000 < z && z < 2000){
				z = (rand() % 10000) - 5000;
			}
			*/
			position = Vec3f(x, y, z);

			float randrx = (rand() % 10) - 5;
			float randrz = (rand() % 10) - 5;
			rotation = Quaternion(randrx, 0, randrz, 0);
		}
	}

	void changeColor(float red, float green, float blue){
		this->red = red;
		this->green = green;
		this->blue = blue;
	}

	// HSL to RGB color conversion taken from 
	// http://stackoverflow.com/questions/2353211/hsl-to-rgb-color-conversion
	float hue2rgb(float p, float q, float t){
		if (t < 0) t += 1;
		if (t > 1) t -= 1;
		if (t < 1 / 6) return p + (q - p) * 6 * t;
		if (t < 1 / 2) return q;
		if (t < 2 / 3) return p + (q - p) * (2 / 3 - t) * 6;
		return p;
	}

	Vec3f hslToRgb(float h, float s, float l){
		float r, g, b;

		if (s == 0){
			r = g = b = l; // achromatic
		}
		else{
			float q = l < 0.5 ? l * (1 + s) : l + s - l * s;
			float p = 2 * l - q;
			r = hue2rgb(p, q, h + 1 / 3);
			g = hue2rgb(p, q, h);
			b = hue2rgb(p, q, h - 1 / 3);
		}

		return Vec3f(r, g, b);
	}

	Vec3f hsv2rgb(float h, float s, float v) {
		h = fmodf((fmodf(h, 1) + 1), 1); // wrap hue

		int i = floor(h * 6);
		float f = h * 6 - i;
		float p = v * (1 - s);
		float q = v * (1 - s * f);
		float t = v * (1 - s * (1 - f));

		switch (i) {
		case 0: return Vec3f(v, t, p);
		case 1: return Vec3f(q, v, p);
		case 2: return Vec3f(p, v, t);
		case 3: return Vec3f(p, q, v);
		case 4: return Vec3f(t, p, v);
		case 5: return Vec3f(v, p, q);
		}
	}
};
#endif