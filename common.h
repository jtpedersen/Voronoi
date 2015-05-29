#ifndef COMMON_H_
#define COMMON_H_


#include <iostream>
#include <fstream>
#include <vector>
#define GLM_FORCE_RADIANS
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/glm.hpp>
#include <glm/ext.hpp> // << friends

#include <random>
#include <sstream>

using namespace std;
using namespace glm;

namespace util {
    float randf();
    glm::vec3 randomVec3();

}

#endif /* !COMMON_H_ */
