#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <cmath>

namespace CatmullRom
{
	glm::vec3 catmullRom(glm::vec4 pointsIndex, float s, float t, glm::vec3* keyPoint);
	std::vector<glm::vec3> objectMoving(glm::vec3* keyPoint, int numPoint, float loadingTime);
}