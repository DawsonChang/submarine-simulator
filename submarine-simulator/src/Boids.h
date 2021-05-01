#pragma once
#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"
#include "objload.h"

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <random>

namespace Boids
{
	const int NumBoids = 50;

	struct SingleObject {
		
		glm::vec3 pos = glm::vec3(0.0, 0.0, 0.0);
		glm::vec3 speed = glm::vec3(0.0, 0.0, 0.0);
		glm::vec4 color = glm::vec4(0, 0, 0, 1);

		// size will be the scale in vertex shader
		float size = 1.0f;

		glm::vec3 axisBoid = glm::vec3(0.0, 1.0, 0.0);
		float angleBoid = 0.0f;
		glm::vec3 prePos = glm::vec3(0.0, 0.0, 0.0);

		float cameradistance = 0.0f;
		bool operator<(SingleObject& that) {
			// Sort in reverse order : far particles drawn first.
			return this->cameradistance > that.cameradistance;
		}

		SingleObject() {};

		void randomInit(glm::vec3 initPos, int _lenX, int _lenY, int _lenZ) {
			pos.x = initPos.x + rand() % _lenX;
			pos.y = initPos.y + rand() % _lenY;
			pos.z = initPos.z + rand() % _lenZ;

			speed.x = (rand() % 100) / 20;
			speed.y = (rand() % 100) / 20;
			speed.z = (rand() % 100) / 20;
		}
	};

	struct RenderContextBoids
	{
		// the boundry of the moving of the objects
		int lenX = 0;
		int lenY = 0;
		int lenZ = 0;
		
		// init pos
		int iPosX = 0;
		int iPosY = 0;
		int iPosZ = 0;

		// initialize the direction of fish (the direction of vector is from tail to head)
		glm::vec3 from_vector_boid = glm::vec3(0, 0, 1);
		

		// data in obj file
		GLuint vertexArray = 0;
		GLuint vertexBuffer = 0;
		GLuint vertexIndexBuffer = 0;
		int size = 0;
		// this variable to control how many frames to render 1 particle
		int countFrame = 0;
		// data of particle effect
		float* g_particule_position_size_data = NULL; // the data of center position and scale
		float* g_particule_rotation_data = NULL; // the data of color
		GLuint particles_position_buffer = 0;
		GLuint particles_color_buffer = 0;
		int ParticlesCount = 0;
		glm::vec3 centerOfBoids = glm::vec3(0, 0, 0);
		Boids::SingleObject* boids = new Boids::SingleObject[NumBoids];
		
		// constructor
		Boids::RenderContextBoids::RenderContextBoids(glm::vec3 initPos, int _lenX, int _lenY, int _lenZ) {
			lenX = _lenX;
			lenY = _lenY;
			lenZ = _lenZ;

			iPosX = initPos.x;
			iPosY = initPos.y;
			iPosZ = initPos.z;

			g_particule_position_size_data = new float[4 * NumBoids];
			g_particule_rotation_data = new float[4 * NumBoids];
			for (int i = 0; i < NumBoids; i++) {
				boids[i].randomInit(initPos, _lenX, _lenY, _lenZ);
			}
		}
		// load the data from obj file
		void initFromOBJ(obj::Model& model);

		// find and update variable "LastUsedParticle"
		//int FindUnusedParticle();

		void SortParticles();
		// update particles_position_buffer(g_particule_position_size_data) 
		// and particles_color_buffer(g_particule_rotation_data) to GPU
		void UpdateGPUBuffer();

		// this function will be called in renderScene() if fin == false
		void UpdateParticles(float dt, glm::vec3 cameraPos);

		// when keys are pressing, this function will be called
		// and it makes the used particles (life < 0) to initialize again
		//void ResetUsedParticle();

		// Rule 1: Boids try to fly towards the centre of mass of neighbouring boids.
		glm::vec3 rule1(int curIndex);
		// Rule 2: Boids try to keep a small distance away from other objects(including other boids).
		glm::vec3 rule2(int curIndex);
		// Rule 3: Boids try to match velocity with near boids.
		glm::vec3 rule3(int curIndex);
		// Rule 4: Bounding the position
		glm::vec3 rule4(int curIndex);
		// Rule 5: Limiting the speed
		void rule5(int curIndex);
	};
}