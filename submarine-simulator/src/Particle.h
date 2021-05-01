#pragma once
#include "glm.hpp"
#include "glew.h"
#include "objload.h"
#include "glm.hpp"
#include "ext.hpp"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

#include <random>

namespace Particle {
	const int MaxParticles = 30;

	struct SingleParticle {
		// translate randomly - make the particles more realistic
		glm::vec3 pos = glm::vec3(0.0, 0.0, 0.0) + (0.8 * glm::vec3(rand() % 10 + 1, 0, rand() % 10 + 1));
		
		// the speed also be randomly like previous one
		glm::vec3 speed = glm::vec3(0.0f, 20.0f, 0.0f) + (0.1 * glm::vec3(rand() % 20 + 1, rand() % 30 + 1, rand() % 20 + 1));
		
		// may change to different color
		//glm::vec4 color = glm::vec4(((float)rand()) / (float)RAND_MAX, ((float)rand()) / (float)RAND_MAX, ((float)rand()) / (float)RAND_MAX, 1.0);
		glm::vec4 color = glm::vec4(0, 0, 0, 1);
		// size will be the scale in vertex shader
		float size = 0.02f;

		// life = 0 means initial status (it's ready to be used)
		// life > 0 means live
		// life < 0 means dead
		float life = -1.0f;
		float cameradistance = 0.0f;
		bool operator<(SingleParticle& that) {
			// Sort in reverse order : far particles drawn first.
			return this->cameradistance > that.cameradistance;
		}
	};

	struct RenderContextParticle
	{
		// data in obj file
		GLuint vertexArray;
		GLuint vertexBuffer;
		GLuint vertexIndexBuffer;
		int size = 0;
		// this variable to control how many frames to render 1 particle
		int countFrame = 0;
		// data of particle effect
		float *g_particule_position_size_data = NULL; // the data of center position and scale
		float *g_particule_color_data = NULL; // the data of color
		GLuint particles_position_buffer = 0; 
		GLuint particles_color_buffer = 0;
		int LastUsedParticle = 0;
		int ParticlesCount = 0;
		Particle::SingleParticle ParticlesContainer[MaxParticles];
		
		// check whether it finished. 
		// the init value is true because need to wait for user press keys to activate
		bool fin = true;

		// constructor
		Particle::RenderContextParticle::RenderContextParticle() {
			vertexArray = 0;
			vertexBuffer = 0;
			vertexIndexBuffer = 0;
			particles_position_buffer = 0;
			particles_color_buffer = 0;
			g_particule_position_size_data = new float[4 * MaxParticles];
			g_particule_color_data = new float[4 * MaxParticles];
		}
		// load the data from obj file
		void initFromOBJ(obj::Model& model);

		// find and update variable "LastUsedParticle"
		int FindUnusedParticle();
		
		void SortParticles();
		// update particles_position_buffer(g_particule_position_size_data) 
		// and particles_color_buffer(g_particule_color_data) to GPU
		void UpdateGPUBuffer();
		
		// this function will be called in renderScene() if fin == false
		void UpdateParticles(float dt, glm::vec3 cameraPos);
		
		// when keys are pressing, this function will be called
		// and it makes the used particles (life < 0) to initialize again
		void ResetUsedParticle();
	};
}