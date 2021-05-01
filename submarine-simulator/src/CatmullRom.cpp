#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"
#include "CatmullRom.h"

#include <iostream>
#include <vector>
#include <string>
#include <cmath>

glm::vec3 CatmullRom::catmullRom(glm::vec4 pointsIndex, float s, float t, glm::vec3* keyPoint)
{
	// the initial pointsIndex is (-1,0,1,2)......(7,8,9,10)
	// Q = 1/2 * |t^3, t^2, t, 1| * |-1  3 -3  1| * |pi-1|
	//								| 2 -5  4 -1|   |pi  |
	//                              |-1  0  1  0|   |pi+1|
	//                              | 0  2  0  0|   |pi+2|
	// and Q has x y z three coordinates, so we have to do the calculation 3 times to get the values of x y z
	int v0 = pointsIndex.x;
	int v1 = pointsIndex.y;
	int v2 = pointsIndex.z;
	int v3 = pointsIndex.w;
	glm::vec4 t_array = glm::vec4(pow(t * s, 3.0), pow(t * s, 2.0), t * s, 1);
	// transpose of the params matrix
	glm::mat4 paras = glm::mat4(-1, 2, -1, 0,
		3, -5, 0, 2,
		-3, 4, 1, 0,
		1, -1, 0, 0);
	glm::vec4 px = glm::vec4(keyPoint[v0].x, keyPoint[v1].x, keyPoint[v2].x, keyPoint[v3].x);
	glm::vec4 py = glm::vec4(keyPoint[v0].y, keyPoint[v1].y, keyPoint[v2].y, keyPoint[v3].y);
	glm::vec4 pz = glm::vec4(keyPoint[v0].z, keyPoint[v1].z, keyPoint[v2].z, keyPoint[v3].z);

	glm::vec3 curPoint = glm::vec3(0.5 * glm::dot(t_array * paras, px), 0.5 * glm::dot(t_array * paras, py), 0.5 * glm::dot(t_array * paras, pz));

	return curPoint;
}

std::vector<glm::vec3> CatmullRom::objectMoving(glm::vec3* keyPoint, int numPoint, float loadingTime) {
	float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f - loadingTime;

	int time_i = floorf(time);

	float time_frac = time - time_i;
	// calculate the length of array cameraKeyPoints
	time_i %= numPoint;
	int v0 = time_i - 1;
	int v1 = time_i;
	int v2 = time_i + 1;
	int v3 = time_i + 2;

	if (v0 < 0) {
		v0 = numPoint - 1;
	}
	if (v2 >= numPoint) {
		v2 = v2 - numPoint;
	}
	if (v3 >= numPoint) {
		v3 = v3 - numPoint;
	}
	glm::vec4 pointsIndex = glm::vec4(v0, v1, v2, v3);
	float s = 1.0;
	glm::vec3 curPos = CatmullRom::catmullRom(pointsIndex, s, time_frac, keyPoint);
	glm::vec3 curDir = glm::normalize(CatmullRom::catmullRom(pointsIndex, s + 0.001, time_frac, keyPoint) - CatmullRom::catmullRom(pointsIndex, s - 0.001, time_frac, keyPoint));

	std::vector<glm::vec3> output;
	output.push_back(curPos);
	output.push_back(curDir);

	return output;
}