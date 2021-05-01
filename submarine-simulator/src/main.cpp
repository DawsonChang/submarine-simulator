#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <random>

#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Camera.h"
#include "Texture.h"
#include "Physics.h"

#include "skybox.cpp"
#include "CatmullRom.h"
#include "Particle.h"
#include "Boids.h"

GLuint skyboxVAO;

GLuint programWithoutTex;
GLuint programSubmarine;
GLuint programSun;
GLuint programTex;
GLuint programSkybox;
GLuint programParticle;
GLuint programBump;
GLuint programBoids;

Core::Shader_Loader shaderLoader;

obj::Model shipModel;
obj::Model sphereModel;
obj::Model sharkModel;
obj::Model turtleModel;
obj::Model fishModel;
obj::Model fish2Model;

Core::RenderContext shipContext;
Core::RenderContext sphereContext;
Core::RenderContext sharkContext;
Core::RenderContext turtleContext;
Core::RenderContext fishContext;
Core::RenderContextFile skyboxContext;
Core::RenderContext fish2Context;

GLuint sunTex;
GLuint submarineTex;
GLuint submarineNormalTex;
GLuint turtleTex;
GLuint fishTex;
GLuint skyboxTex;
GLuint fish2Tex;
GLuint fish2NormalTex;



// declear the scale of skybox, so the size of skybox will be square of this number
// e.g. if the skyboxScale = 3, the coordinates of the skybox will be (9,9,9), (9,9,-9), (9,-9,9), (9,-9,-9)......
float skyboxScale = 30.0;

// camera
float cameraAngle = 0.0;
glm::vec3 cameraPos = glm::vec3(-25, 0, 0);
glm::vec3 cameraDir;

// put the sun on the top of the skybox (outside the skybox)
glm::vec3 sunPos = glm::vec3(0, skyboxScale * skyboxScale + 10.0, 0);
glm::vec3 submarinePos;
glm::mat4 cameraMatrix, perspectiveMatrix;

// because this struct is huge, use pointer and 'new' to allocate memory on heap
// included in "Particle.h"
Particle::RenderContextParticle* bubble = new Particle::RenderContextParticle;

// time variables for particle effect
float old_t = 0.0f;
float dt;
float appLoadingTime;

// moving points of fish
const int NUM_FISH_POINTS = 20;
glm::vec3 fishKeyPoints[NUM_FISH_POINTS - 2];
glm::vec3 from_vector_fish;
glm::vec3 to_vector_fish;

// moving points of turtle
const int NUM_TURTLE_POINTS = 40;
glm::vec3 turtleKeyPoints[NUM_TURTLE_POINTS];
glm::vec3 from_vector_turtle;
glm::vec3 to_vector_turtle;

// physics
Physics pxScene(9.8f /* gravity (m/s^2) */);

// fixed timestep for stable and deterministic simulation
const double physicsStepTime = 1.f / 60.f;
double physicsTimeToProcess = 0;

// physical objects
//PxRigidStatic* planeBody = nullptr;
//PxMaterial* planeMaterial = nullptr;
PxRigidDynamic* boxBody = nullptr;
PxMaterial* boxMaterial = nullptr;

PxRigidDynamic* submarineBody = nullptr;
PxMaterial* submarineMaterial = nullptr;
glm::mat4 submarineModelMatrix;

// submarine HP
int HP = 100;

// because this struct is huge, use pointer and 'new' to allocate memory on heap
// included in "Boid.h"
Boids::RenderContextBoids* fishBoids = new Boids::RenderContextBoids(glm::vec3(80, 10, 80), 50, 20, 50);
Boids::RenderContextBoids* fishBoids2 = new Boids::RenderContextBoids(glm::vec3(200, 10, -100), 50, 20, 50);
Boids::RenderContextBoids* fishBoids3 = new Boids::RenderContextBoids(glm::vec3(-150, 10, -150), 80, 20, 80);

// moving points of shark
const int NUM_SHARK_POINTS = 20;
const int NUM_SHARK_GOBACK_POINTS = 5;
const glm::vec3 from_vector_shark = glm::vec3(0, 0, 1);
// If the distance between shark and submarine lower than this number, then the shark attack
const float attackDistance = 20.0;

struct Shark {
	glm::vec3 sharkNormalKeyPoints[NUM_SHARK_POINTS];
	glm::vec3 sharkGobackKeyPoints[NUM_SHARK_GOBACK_POINTS];
	glm::vec3 to_vector_shark;
	// there are 3 states:
	// sharkMode = 0 -> normal mode, the shark swims and wait for prey
	// sharkMode = 1 -> attack mode, the shark moves toward submarine
	// sharkMode = 2 -> go back mode, the shark will go back to the initial position and swims again
	int sharkMode = 0;
	glm::vec3 sharkPos;
	glm::vec3 axisShark;
	float angleShark;
	glm::vec3 translateShark;
	float appLoadingTimeShark;
	float preAttackTime = 0;
	PxRigidDynamic* sharkBody;
	PxMaterial* sharkMaterial = pxScene.physics->createMaterial(0.9, 0.5, 0.6);
	glm::mat4 sharkactorModelMatrix;
	bool pressKey = false;
};
const int NUM_SHARK = 5;

Shark* sharkArray = new Shark[NUM_SHARK];
glm::vec3 sharkInitPosArray[NUM_SHARK] = { glm::vec3(-20, -3, -40), glm::vec3(120, -3, -50), glm::vec3(-100, -3, 40), glm::vec3(200, -3, -300), glm::vec3(-200, -3, -300) };


void initPhysicsScene()
{
	// create a submarine
	submarineBody = pxScene.physics->createRigidDynamic(PxTransform(0, 0, 0));
	submarineBody->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
	submarineBody->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	submarineMaterial = pxScene.physics->createMaterial(0.9, 0.5, 0.6);
	PxShape* submarineShape = pxScene.physics->createShape(PxSphereGeometry(4), *submarineMaterial);
	submarineBody->attachShape(*submarineShape);
	submarineShape->release();
	submarineBody->userData = &submarineModelMatrix;
	pxScene.scene->addActor(*submarineBody);

}

void createSharkActor(PxTransform initPos, Shark* shark) {
	// create a shark
	shark->sharkBody = pxScene.physics->createRigidDynamic(initPos);
	(shark->sharkBody)->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	(shark->sharkBody)->setRigidDynamicLockFlags(PxRigidDynamicLockFlag::eLOCK_ANGULAR_X | PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z);
	PxShape* sharkShape = pxScene.physics->createShape(PxSphereGeometry(5), *(shark->sharkMaterial));
	(shark->sharkBody)->attachShape(*sharkShape);
	sharkShape->release();
	(shark->sharkBody)->userData = &(shark->sharkactorModelMatrix);
	pxScene.scene->addActor(*(shark->sharkBody));
}

void updateTransforms()
{
	// Here we retrieve the current transforms of the objects from the physical simulation.
	auto actorFlags = PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC;
	PxU32 nbActors = pxScene.scene->getNbActors(actorFlags);
	if (nbActors)
	{
		std::vector<PxRigidActor*> actors(nbActors);
		pxScene.scene->getActors(actorFlags, (PxActor**)&actors[0], nbActors);
		for (auto actor : actors)
		{
			// We use the userData of the objects to set up the proper model matrices.
			if (!actor->userData) {
				continue;
			}
			glm::mat4* modelMatrix = (glm::mat4*)actor->userData;

			// get world matrix of the object (actor)
			PxMat44 transform = actor->getGlobalPose();
			auto& c0 = transform.column0;
			auto& c1 = transform.column1;
			auto& c2 = transform.column2;
			auto& c3 = transform.column3;

			// set up the model matrix used for the rendering
			*modelMatrix = glm::mat4(
				c0.x, c0.y, c0.z, c0.w,
				c1.x, c1.y, c1.z, c1.w,
				c2.x, c2.y, c2.z, c2.w,
				c3.x, c3.y, c3.z, c3.w);
		}
	}
}

void setKeyPress() {
	for (int i = 0; i < NUM_SHARK; i++) {
		sharkArray[i].pressKey = true;
	}
}

void keyboard(unsigned char key, int x, int y)
{
	float angleSpeed = 0.1f;
	float moveSpeed = 0.5f;
	switch (key)
	{
	case 'z': cameraAngle -= angleSpeed; break;
	case 'x': cameraAngle += angleSpeed; break;
	case 'w':
		cameraPos += cameraDir * moveSpeed;
		bubble->ResetUsedParticle();
		setKeyPress();
		break;
	case 's':
		cameraPos -= cameraDir * moveSpeed;
		bubble->ResetUsedParticle();
		setKeyPress();
		break;
	case 'd':
		cameraPos += glm::cross(cameraDir, glm::vec3(0, 1, 0)) * moveSpeed;
		bubble->ResetUsedParticle();
		setKeyPress();
		break;
	case 'a':
		cameraPos -= glm::cross(cameraDir, glm::vec3(0, 1, 0)) * moveSpeed;
		bubble->ResetUsedParticle();
		setKeyPress();
		break;
	case 'e': cameraPos += glm::cross(cameraDir, glm::vec3(1, 0, 0)) * moveSpeed; break;
	case 'q': cameraPos -= glm::cross(cameraDir, glm::vec3(1, 0, 0)) * moveSpeed; break;

	}
}

void keyUp(unsigned char key, int x, int y) {
	if (key) {
		for (int i = 0; i < NUM_SHARK; i++) {
			sharkArray[i].pressKey = false;
		}
	}
}

glm::mat4 createCameraMatrix()
{
	// Obliczanie kierunku patrzenia kamery (w plaszczyznie x-z) przy uzyciu zmiennej cameraAngle kontrolowanej przez klawisze.
	cameraDir = glm::vec3(cosf(cameraAngle), 0.0f, sinf(cameraAngle));
	glm::vec3 up = glm::vec3(0, 1, 0);

	return Core::createViewMatrix(cameraPos, cameraDir, up);
}

void drawSkybox(GLuint curProgram, Core::RenderContextFile context, glm::mat4 modelMatrix, GLuint tex) {
	glDepthMask(GL_FALSE);
	glUseProgram(curProgram);
	glBindVertexArray(context.vertexArray);

	// to vertex shader
	glUniformMatrix4fv(glGetUniformLocation(curProgram, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(curProgram, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniform3f(glGetUniformLocation(curProgram, "submarinePos"), submarinePos.x, submarinePos.y, submarinePos.z);

	Core::SetActiveTexture(tex, "skyboxTexture", curProgram, 0);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	glUseProgram(0);
	glDepthMask(GL_TRUE);
}

void drawSubmarine(GLuint curProgram, Core::RenderContext context, glm::mat4 modelMatrix, GLuint tex, GLuint texNormal)
{
	glUseProgram(curProgram);

	// to vertex shader
	glUniform3f(glGetUniformLocation(curProgram, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform3f(glGetUniformLocation(curProgram, "lightPos"), sunPos.x, sunPos.y, sunPos.z);
	glUniform3f(glGetUniformLocation(curProgram, "submarinePos"), submarinePos.x, submarinePos.y, submarinePos.z);

	glUniformMatrix4fv(glGetUniformLocation(curProgram, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(curProgram, "transformation"), 1, GL_FALSE, (float*)&transformation);

	Core::SetActiveTexture(tex, "colorTexture", curProgram, 0);
	Core::SetActiveTexture(texNormal, "normalTexture", curProgram, 1);

	Core::DrawContext(context);
	glUseProgram(0);
}

void drawObject(GLuint curProgram, Core::RenderContext context, glm::mat4 modelMatrix, glm::vec3 color, float time)
{
	glUseProgram(curProgram);

	// to fragment shader
	glUniform3f(glGetUniformLocation(curProgram, "objectColor"), color.x, color.y, color.z);
	glUniform3f(glGetUniformLocation(curProgram, "lightPos"), sunPos.x, sunPos.y, sunPos.z);
	glUniform3f(glGetUniformLocation(curProgram, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform3f(glGetUniformLocation(curProgram, "submarinePos"), submarinePos.x, submarinePos.y, submarinePos.z);

	// to vertex shader
	glUniformMatrix4fv(glGetUniformLocation(curProgram, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(curProgram, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniform1f(glGetUniformLocation(curProgram, "time"), time);

	Core::DrawContext(context);
	glUseProgram(0);
}

void drawObjectTexture(GLuint curProgram, Core::RenderContext context, glm::mat4 modelMatrix, GLuint tex, float time, bool moving) {
	glUseProgram(curProgram);

	// to fragment shader
	glUniform3f(glGetUniformLocation(curProgram, "lightPos"), sunPos.x, sunPos.y, sunPos.z);
	glUniform3f(glGetUniformLocation(curProgram, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform3f(glGetUniformLocation(curProgram, "submarinePos"), submarinePos.x, submarinePos.y, submarinePos.z);

	// to vertex shader
	glUniformMatrix4fv(glGetUniformLocation(curProgram, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(curProgram, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniform1f(glGetUniformLocation(curProgram, "time"), time);
	glUniform1i(glGetUniformLocation(curProgram, "moving"), moving);

	Core::SetActiveTexture(tex, "colorTexture", curProgram, 0);

	Core::DrawContext(context);
	glUseProgram(0);
}

void drawObjectBump(GLuint curProgram, Core::RenderContext context, glm::mat4 modelMatrix, GLuint tex, GLuint texNormal, float time) {
	glUseProgram(curProgram);

	// to vertex shader
	glUniform3f(glGetUniformLocation(curProgram, "lightPos"), sunPos.x, sunPos.y, sunPos.z);
	glUniform3f(glGetUniformLocation(curProgram, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform3f(glGetUniformLocation(curProgram, "submarinePos"), submarinePos.x, submarinePos.y, submarinePos.z);

	glUniformMatrix4fv(glGetUniformLocation(curProgram, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(curProgram, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniform1f(glGetUniformLocation(curProgram, "time"), time);

	Core::SetActiveTexture(tex, "colorTexture", curProgram, 0);
	Core::SetActiveTexture(texNormal, "normalTexture", curProgram, 1);

	Core::DrawContext(context);
	glUseProgram(0);
}

void drawParticles(Particle::RenderContextParticle* context, GLuint curProgram, Core::RenderContext sContext, GLuint tex) {
	glUseProgram(curProgram);
	glBindVertexArray(context->vertexArray);
	glm::mat4 translateSubmarine = glm::translate(submarinePos);
	//glm::mat4 translateSmall = glm::translate(glm::vec3(0, 0, 0.1));
	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * translateSubmarine;

	glUniform3f(glGetUniformLocation(curProgram, "submarinePos"), submarinePos.x, submarinePos.y, submarinePos.z);
	glUniformMatrix4fv(glGetUniformLocation(curProgram, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glDrawElementsInstanced(GL_TRIANGLES, context->size, GL_UNSIGNED_SHORT, (void*)0, context->ParticlesCount);

	glm::mat4 modelMatrix = translateSubmarine;
	glUniformMatrix4fv(glGetUniformLocation(curProgram, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	glBindVertexArray(0);
	glUseProgram(0);
}

void drawBoids(Boids::RenderContextBoids* context, GLuint curProgram, GLuint tex) {
	glUseProgram(curProgram);
	glBindVertexArray(context->vertexArray);
	//glm::mat4 translateSubmarine = glm::translate(submarinePos);
	//glm::mat4 translateSmall = glm::translate(glm::vec3(0, 0, 0.1));


	glUniform3f(glGetUniformLocation(curProgram, "submarinePos"), submarinePos.x, submarinePos.y, submarinePos.z);
	glUniform3f(glGetUniformLocation(curProgram, "lightPos"), sunPos.x, sunPos.y, sunPos.z);
	glUniform3f(glGetUniformLocation(curProgram, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix;
	glUniformMatrix4fv(glGetUniformLocation(curProgram, "transformation"), 1, GL_FALSE, (float*)&transformation);

	Core::SetActiveTexture(tex, "colorTexture", curProgram, 0);
	glDrawElementsInstanced(GL_TRIANGLES, context->size, GL_UNSIGNED_SHORT, (void*)0, context->ParticlesCount);

	//glm::mat4 modelMatrix = translateSubmarine;
	//glUniformMatrix4fv(glGetUniformLocation(curProgram, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	glBindVertexArray(0);
	glUseProgram(0);
}

void drawString(std::string s, float x, float y) {
	//glColor3f(0.5f, 0.5f, 0.5f);
	glRasterPos2f(x, y);

	for (int i = 0; i < s.length(); i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, s.at(i));
	}
}

void renderScene()
{
	cameraMatrix = createCameraMatrix();
	perspectiveMatrix = Core::createPerspectiveMatrix(skyboxScale);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.3f, 0.3f, 1.0f);

	drawString(std::to_string(HP), -0.9, 0.9);


	float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
	dt = time - old_t;
	old_t = time;

	//skybox
	glm::mat4 skyboxModelMatrix = glm::scale(glm::vec3(skyboxScale));
	drawSkybox(programSkybox, skyboxContext, skyboxModelMatrix, skyboxTex);

	// submarine
	glm::vec3 subTransVec1 = glm::vec3(0, -200, 600);
	glm::mat4 subTrans1 = glm::translate(subTransVec1);

	glm::vec3 subScaleVec1 = glm::vec3(0.005f);
	glm::mat4 subScale1 = glm::scale(subScaleVec1);

	glm::mat4 subRotate = glm::rotate(-cameraAngle + glm::radians(90.0f), glm::vec3(0, 1, 0));

	submarinePos = cameraPos + cameraDir * 0.6f + glm::vec3(0, -0.25f, 0);

	glm::mat4 subTrans2 = glm::translate(cameraPos + cameraDir * 0.6f);

	glm::mat4 shipModelMatrix = subTrans2 * subRotate * subScale1 * subTrans1;

	submarineBody->setKinematicTarget(PxTransform(submarinePos.x, submarinePos.y - 3.0, submarinePos.z));
	drawSubmarine(programSubmarine, shipContext, shipModelMatrix, submarineTex, submarineNormalTex);


	// sun
	glm::mat4 rotateSun;
	glm::mat4 translateCenter = glm::translate(sunPos);
	rotateSun = glm::rotate(rotateSun, time / 8, glm::vec3(0, -1, 0));
	glm::mat4 sunModelMatrix = translateCenter * glm::scale(glm::vec3(2.5, 2.5, 2.5)) * rotateSun;
	drawObjectTexture(programSun, sphereContext, sunModelMatrix, sunTex, time, false);

	// shark
	for (int k = 0; k < NUM_SHARK; k++) {
		//std::cout << sharkArray[k].pressKey << std::endl;
		// normal mode
		if (sharkArray[k].sharkMode == 0) {
			std::vector<glm::vec3> outputShark = CatmullRom::objectMoving(sharkArray[k].sharkNormalKeyPoints, NUM_SHARK_POINTS, sharkArray[k].appLoadingTimeShark);
			// outputShark[0]: the position (the position)
			// outputShark[1]: the direction (the vector which shark should turn to)
			sharkArray[k].sharkPos = outputShark[0];
			sharkArray[k].to_vector_shark = glm::normalize(outputShark[1]);
			float thetaShark = glm::dot(from_vector_shark, sharkArray[k].to_vector_shark) / (glm::length(from_vector_shark) * glm::length(sharkArray[k].to_vector_shark));
			sharkArray[k].axisShark = glm::normalize(glm::cross(from_vector_shark, sharkArray[k].to_vector_shark));
			sharkArray[k].angleShark = glm::acos(thetaShark);

			// calculate the distance between submarine and shark
			float d = glm::distance(submarinePos, sharkArray[k].sharkPos);
			if (d < attackDistance) {
				std::cout << "attack" << std::endl;
				sharkArray[k].sharkMode = 1;
			}

			glm::mat4 sharkModelMatrix = glm::translate(sharkArray[k].sharkPos) * glm::rotate(sharkArray[k].angleShark, sharkArray[k].axisShark);
			drawObject(programWithoutTex, sharkContext, sharkModelMatrix, glm::vec3(1.0f, 1.0f, 1.0f), time);
		}

		// attack mode
		else if (sharkArray[k].sharkMode == 1) {
			if (!sharkArray[k].sharkBody) {
				createSharkActor(PxTransform(sharkArray[k].sharkPos.x, sharkArray[k].sharkPos.y, sharkArray[k].sharkPos.z), &sharkArray[k]);	
			}
			updateTransforms();

			if (sharkArray[k].pressKey) {
				(sharkArray[k].sharkBody)->setLinearVelocity(PxVec3(0,0,0));
			}
			
			glm::vec3 aSubmarinePos = glm::vec3(submarinePos.x, submarinePos.y - 3, submarinePos.z);
			glm::vec3 sharkPxPos = glm::vec3(sharkArray[k].sharkactorModelMatrix[3].x, aSubmarinePos.y, sharkArray[k].sharkactorModelMatrix[3].z);
			float d = glm::distance(aSubmarinePos, sharkPxPos);
			glm::vec3 movingVector = (aSubmarinePos - sharkPxPos) * 0.9;

			// if the distance between sub and shark is less then attackDistance and greater then a constant (here is 7)
			// the shark will move toward the submarine
			
			if (d < attackDistance && d > 10 && !sharkArray[k].pressKey) {
				// To adjust the position of submarine(make the shark close to submarine by y axis), we did submarinePos + (0,-3,0)
				float thetaShark = glm::dot(from_vector_shark, movingVector) / (glm::length(from_vector_shark) * glm::length(movingVector));
				sharkArray[k].axisShark = glm::normalize(glm::cross(from_vector_shark, movingVector));
				sharkArray[k].angleShark = glm::acos(thetaShark);

				(sharkArray[k].sharkBody)->setLinearVelocity(PxVec3(movingVector.x, movingVector.y, movingVector.z));
			}
			// shark will go back to the beginning point
			// use a new catmullRom to do so
			// initialize array "sharkGobackKeyPoints" below
			else if (d >= attackDistance) {
				sharkArray[k].sharkGobackKeyPoints[0] = sharkPxPos;
				sharkArray[k].sharkGobackKeyPoints[NUM_SHARK_GOBACK_POINTS - 1] = sharkArray[k].sharkNormalKeyPoints[0];
				glm::vec3 step = (sharkArray[k].sharkGobackKeyPoints[NUM_SHARK_GOBACK_POINTS - 1] - sharkArray[k].sharkGobackKeyPoints[0]) / (NUM_SHARK_GOBACK_POINTS - 1);

				for (int i = 1; i < NUM_SHARK_GOBACK_POINTS - 1; i++) {
					sharkArray[k].sharkGobackKeyPoints[i] = sharkArray[k].sharkGobackKeyPoints[i - 1] + step;
				}

				// the next frame will go to sharkMode 2
				sharkArray[k].sharkMode = 2;
				std::cout << "You escaped from shark" << std::endl;
				// set appLoadingTimeShark so the time can start from 0
				sharkArray[k].appLoadingTimeShark = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
			}
			else if (d <= 10 && !sharkArray[k].pressKey) {
				// To adjust the position of submarine(make the shark close to submarine by y axis), we did submarinePos + (0,-3,0)
				float thetaShark = glm::dot(from_vector_shark, movingVector) / (glm::length(from_vector_shark) * glm::length(movingVector));
				sharkArray[k].axisShark = glm::normalize(glm::cross(from_vector_shark, movingVector));
				sharkArray[k].angleShark = glm::acos(thetaShark);

				// attack!!
				// every half second the shark attack once
				if (sharkArray[k].preAttackTime == 0 || time - sharkArray[k].preAttackTime > 0.5) {
					HP--;
					std::cout << "Shark is attacking!" << std::endl;
					std::cout << "HP decrease 1" << std::endl;
					(sharkArray[k].sharkBody)->setLinearVelocity(PxVec3(-movingVector.x, -movingVector.y, -movingVector.z) * 5);
					sharkArray[k].preAttackTime = time;
				}
			}
			sharkArray[k].sharkactorModelMatrix[3].y = aSubmarinePos.y;
			glm::mat4 sharkModelMatrix = glm::translate(sharkPxPos) * glm::rotate(sharkArray[k].angleShark, sharkArray[k].axisShark);
			drawObject(programWithoutTex, sharkContext, sharkModelMatrix, glm::vec3(1.0f, 1.0f, 1.0f), time);
		}

		// go back mode
		else if (sharkArray[k].sharkMode == 2) {
			// set sharkBody to null
			sharkArray[k].sharkBody = nullptr;
			sharkArray[k].sharkactorModelMatrix = glm::mat4(0.0);

			std::vector<glm::vec3> outputShark = CatmullRom::objectMoving(sharkArray[k].sharkGobackKeyPoints, NUM_SHARK_GOBACK_POINTS, sharkArray[k].appLoadingTimeShark);
			sharkArray[k].sharkPos = outputShark[0];
			sharkArray[k].to_vector_shark = glm::normalize(outputShark[1]);
			float thetaShark = glm::dot(from_vector_shark, sharkArray[k].to_vector_shark) / (glm::length(from_vector_shark) * glm::length(sharkArray[k].to_vector_shark));
			sharkArray[k].axisShark = glm::normalize(glm::cross(from_vector_shark, sharkArray[k].to_vector_shark));
			sharkArray[k].angleShark = glm::acos(thetaShark);

			// if the position is very close to our end point (it's hard to check they are equal so just check they are close)
			if (glm::length(sharkArray[k].sharkPos - sharkArray[k].sharkNormalKeyPoints[0]) < 0.1) {
				// the next frame will go to sharkMode 0
				sharkArray[k].sharkMode = 0;
				// set appLoadingTimeShark so the time can start from 0
				sharkArray[k].appLoadingTimeShark = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
			}

			glm::mat4 sharkModelMatrix = glm::translate(sharkArray[k].sharkPos) * glm::rotate(sharkArray[k].angleShark, sharkArray[k].axisShark);
			drawObject(programWithoutTex, sharkContext, sharkModelMatrix, glm::vec3(1.0f, 1.0f, 1.0f), time);
		}
	}

	// turtle
	std::vector<glm::vec3> outputTurtle = CatmullRom::objectMoving(turtleKeyPoints, NUM_TURTLE_POINTS, appLoadingTime);
	to_vector_turtle = glm::normalize(outputTurtle[1]);
	float thetaTurtle = glm::dot(from_vector_turtle, to_vector_turtle) / (glm::length(from_vector_turtle) * glm::length(to_vector_turtle));
	glm::vec3 axisTurtle = glm::normalize(glm::cross(from_vector_turtle, to_vector_turtle));
	float angleTurtle = glm::acos(thetaTurtle);

	glm::mat4 turtleModelMatrix = glm::scale(glm::vec3(0.1, 0.1, 0.1)) * glm::translate(glm::vec3(100, -50, 180)) * glm::translate(outputTurtle[0]) * glm::rotate(angleTurtle, axisTurtle) * glm::rotate(glm::radians(-90.0f), glm::vec3(1, 0, 0));
	drawObjectTexture(programTex, turtleContext, turtleModelMatrix, turtleTex, time, true);

	//fish2
	std::vector<glm::vec3> outputFish = CatmullRom::objectMoving(fishKeyPoints, NUM_FISH_POINTS - 2, appLoadingTime);
	to_vector_fish = glm::normalize(outputFish[1]);
	float thetaFish = glm::dot(from_vector_fish, to_vector_fish) / (glm::length(from_vector_fish) * glm::length(to_vector_fish));
	glm::vec3 axisFish = glm::normalize(glm::cross(from_vector_fish, to_vector_fish));
	float angleFish = glm::acos(thetaFish);

	glm::mat4 fish2ModelMatrix = glm::translate(outputFish[0]) * glm::rotate(angleFish, axisFish);
	drawObjectBump(programBump, fish2Context, fish2ModelMatrix, fish2Tex, fish2NormalTex, time);

	// particles update
	if (!bubble->fin) { // if Particle is still activated
		bubble->UpdateParticles(dt, cameraPos);
		drawParticles(bubble, programParticle, sphereContext, skyboxTex);
	}

	// fish boids update
	fishBoids->UpdateParticles(dt, cameraPos);
	drawBoids(fishBoids, programBoids, fishTex);

	// fishBoids2
	fishBoids2->UpdateParticles(dt, cameraPos);
	drawBoids(fishBoids2, programBoids, fishTex);

	////fishBoids3
	fishBoids3->UpdateParticles(dt, cameraPos);
	drawBoids(fishBoids3, programBoids, fishTex);

	// Update physics
	if (dt < 1.f) {
		physicsTimeToProcess += dt;
		while (physicsTimeToProcess > 0) {
			// here we perform the physics simulation step
			pxScene.step(physicsStepTime);
			physicsTimeToProcess -= physicsStepTime;
		}
	}

	glutSwapBuffers();
}

void init()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	programWithoutTex = shaderLoader.CreateProgram("shaders/shader_without_tex.vert", "shaders/shader_without_tex.frag");
	programSubmarine = shaderLoader.CreateProgram("shaders/shader_submarine.vert", "shaders/shader_submarine.frag");
	programSun = shaderLoader.CreateProgram("shaders/shader_sun.vert", "shaders/shader_sun.frag");
	programTex = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");
	programSkybox = shaderLoader.CreateProgram("shaders/shader_skybox.vert", "shaders/shader_skybox.frag");
	programParticle = shaderLoader.CreateProgram("shaders/shader_particle.vert", "shaders/shader_particle.frag");
	programBump = shaderLoader.CreateProgram("shaders/shader_fish_bump.vert", "shaders/shader_fish_bump.frag");
	programBoids = shaderLoader.CreateProgram("shaders/shader_boids.vert", "shaders/shader_boids.frag");

	sphereModel = obj::loadModelFromFile("models/sphere.obj");
	shipModel = obj::loadModelFromFile("models/submarine.obj");
	sharkModel = obj::loadModelFromFile("models/shark.obj");
	turtleModel = obj::loadModelFromFile("models/sea_turtle.obj");
	fishModel = obj::loadModelFromFile("models/fish5.obj");
	fish2Model = obj::loadModelFromFile("models/fish2.obj");

	shipContext.initFromOBJWithBump(shipModel);
	sphereContext.initFromOBJ(sphereModel);
	sharkContext.initFromOBJ(sharkModel);
	turtleContext.initFromOBJ(turtleModel);
	fishContext.initFromOBJ(fishModel);
	fish2Context.initFromOBJWithBump(fish2Model);
	skyboxContext.initFromFile(skyboxVertices, sizeof(skyboxVertices));

	sunTex = Core::LoadTexture("textures/sun.png");
	submarineTex = Core::LoadTexture("textures/bodyColor_de_la_Superficie.png");
	submarineNormalTex = Core::LoadTexture("textures/body_normal.png");
	turtleTex = Core::LoadTexture("textures/sea_turtle.png");
	fishTex = Core::LoadTexture("textures/fish5.png");
	fish2Tex = Core::LoadTexture("textures/fish2.png");
	fish2NormalTex = Core::LoadTexture("textures/fish2_normal.png");

	std::vector<std::string> faces = {
		"textures/underwater/uw_ft.jpg",
		"textures/underwater/uw_bk.jpg",
		"textures/underwater/uw_up.jpg",
		"textures/underwater/uw_dn.jpg",
		"textures/underwater/uw_rt.jpg",
		"textures/underwater/uw_lf.jpg"
	};
	skyboxTex = Core::loadCubemap(faces);

	bubble->initFromOBJ(sphereModel);
	fishBoids->initFromOBJ(fishModel);
	fishBoids2->initFromOBJ(fishModel);
	fishBoids3->initFromOBJ(fishModel);

	// moving for shark
	static const float camRadius = 20;
	static const float camOffset = 0.6;

	for (int k = 0; k < NUM_SHARK; k++) {
		for (int i = 0; i < NUM_SHARK_POINTS; i++)
		{
			float angle = (float(i)) * (2 * glm::pi<float>() / NUM_SHARK_POINTS);
			float radius = camRadius * (0.95 + glm::linearRand(0.0f, 0.1f));
			sharkArray[k].sharkNormalKeyPoints[i] = glm::vec3(cosf(angle) + camOffset, 0.0f, sinf(angle)) * radius + sharkInitPosArray[k];
		}
	}


	// moving for fish
	fishKeyPoints[0] = glm::vec3(-10, 0, 10);
	for (int i = 1; i < NUM_FISH_POINTS / 2; i++)
	{
		fishKeyPoints[i] = glm::vec3(fishKeyPoints[i - 1].x + 5, 0, fishKeyPoints[i - 1].z);
		int symmetryIndex = (NUM_FISH_POINTS - 2) - i;
		fishKeyPoints[symmetryIndex] = fishKeyPoints[i];
	}
	// initialize the direction of fish (the direction of vector is from tail to head)
	from_vector_fish = glm::vec3(0, 0, 1);

	// moving for turtle
	static const float turtleRadius = 300;
	static const float turtleOffset = 0.5;

	for (int i = 0; i < NUM_TURTLE_POINTS; i++)
	{
		float angle = (float(i)) * (2 * glm::pi<float>() / NUM_TURTLE_POINTS);
		float radius = turtleRadius * (0.95 + glm::linearRand(0.0f, 0.1f));
		turtleKeyPoints[i] = glm::vec3(cosf(angle) + turtleOffset, 0.0f, sinf(angle)) * radius;
	}

	from_vector_turtle = glm::vec3(1, 0, 0);

	// init for physics
	initPhysicsScene();

	appLoadingTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
	// init appLoadingTimeShark
	for (int k = 0; k < NUM_SHARK; k++) {
		sharkArray[k].appLoadingTimeShark = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
	}
}

void shutdown()
{
	shaderLoader.DeleteProgram(programWithoutTex);
	shaderLoader.DeleteProgram(programSubmarine);
	shaderLoader.DeleteProgram(programSun);
	shaderLoader.DeleteProgram(programTex);
	shaderLoader.DeleteProgram(programSkybox);
	shaderLoader.DeleteProgram(programParticle);
	shaderLoader.DeleteProgram(programBump);
}

void idle()
{
	glutPostRedisplay();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(200, 200);
	glutInitWindowSize(600, 600);
	glutCreateWindow("Submarine Simulator");
	glewInit();

	init();
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyUp);
	glutDisplayFunc(renderScene);
	glutIdleFunc(idle);

	glutMainLoop();

	shutdown();

	return 0;
}